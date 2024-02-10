#include "tcp/client.hpp"
#include "tcp/server.hpp"

using namespace unisock;
using namespace tcp::common_actions;
using namespace tcp::client_actions;

#include <chrono>

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        std::cout << "usage " << argv[0] << " <n_connections>" << std::endl;
        return 1;
    }

    std::shared_ptr<events::handler> handler = std::make_shared<events::handler>();

    const int n_clients = std::atoi(argv[1]);

    std::vector<tcp::client> clients {};

    // setting up clients
    for (int i = 0; i < n_clients; ++i)
    {
        clients.push_back(tcp::client( handler ));

        tcp::client& client = *clients.rbegin();

        client.on<CONNECT>([](tcp::client::connection* connection){
            (void)connection;
            write(1, "+", 1);
        });

        client.on<ERROR>([](const std::string& func, int err){
            std::cout << std::endl << "error: " << func << ": " << strerror(err) << std::endl;
        });
    }

    auto before_connect = std::chrono::steady_clock::now();

    auto min_time = 10000000;
    auto max_time = -1;
    auto avg_time = 0;

    int  min_idx = 0;
    int  max_idx = 0;

    int  i = 0;
    for (tcp::client& client : clients)
    {
        auto before_client = std::chrono::steady_clock::now();

        client.connect("127.0.0.1", 8000);

        auto after_client = std::chrono::steady_clock::now();

        auto time_ms = std::chrono::duration_cast<std::chrono::nanoseconds>(after_client - before_client).count();
        if (time_ms < min_time)
        {
            min_time = time_ms;
            min_idx = i;
        }
        if (time_ms > max_time)
        {
            max_time = time_ms;
            max_idx = i;
        }
        avg_time += time_ms / n_clients;
        ++i;
    }

    auto after_connect = std::chrono::steady_clock::now();

    std::cout << std::endl << std::endl;

    auto total_time = std::chrono::duration_cast<std::chrono::nanoseconds>(after_connect - before_connect).count();
    std::cout << "*************************************" << std::endl
              << "results for " << n_clients << " connections" << std::endl
              << "total connection time: " << ((float)total_time / 1000000000) << "s (" << ((float)total_time / 1000000) << "ms)" << std::endl
              << "max connection time: " << ((float)max_time / 1000000) << "ms (by client[" << max_idx << "])" << std::endl
              << "min connection time: " << ((float)min_time / 1000000) << "ms (by client[" << min_idx << "])" << std::endl
              << "avg connection time: " << ((float)avg_time / 1000000) << "ms" << std::endl;

    
    while (events::poll(handler))
        ;
    
    for (auto& client : clients)
        client.close();
}