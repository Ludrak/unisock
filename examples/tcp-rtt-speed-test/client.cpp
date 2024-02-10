#include "tcp/client.hpp"
#include "tcp/server.hpp"

using namespace unisock;
using namespace tcp::common_actions;
using namespace tcp::client_actions;

#include <chrono>

#include "message.h"

struct client_entity
{
    size_t total_recv_bytes = 0;
    std::chrono::steady_clock::time_point send_time;
};

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        std::cout << "usage " << argv[0] << " <n_connections>" << std::endl;
        return 1;
    }

    std::shared_ptr<events::handler> handler = std::make_shared<events::handler>();

    const int n_clients = std::atoi(argv[1]);


    using test_client = tcp::client_of<client_entity>;

    std::vector<test_client> clients {};



    int  min_send_time = 100000;
    int  max_send_time = -1;
    int  avg_send_time = 0;

    int  max_send_idx = 0;
    int  min_send_idx = 0;



    int  min_rtt = 100000;
    int  max_rtt = -1;
    int  avg_rtt = 0;

    int  max_rtt_idx = 0;
    int  min_rtt_idx = 0;


    for (int i = 0; i < n_clients; ++i)
    {
        clients.push_back(handler);

        test_client& client = *clients.rbegin();

        client.on<CONNECT>([n_clients, i, &avg_send_time, &min_send_idx, &max_send_idx, &min_send_time, &max_send_time](test_client::connection* connection){
            (void)connection;
            write(1, "+", 1);

            auto before_send = std::chrono::steady_clock::now();
            connection->data.send_time = before_send;

            connection->send(MESSAGE_STR, sizeof(MESSAGE_STR));

            auto after_send = std::chrono::steady_clock::now();

            auto send_time_ms = std::chrono::duration_cast<std::chrono::nanoseconds>(after_send - before_send).count();

            if (send_time_ms < min_send_time)
            {
                min_send_time = send_time_ms;
                min_send_idx = i;
            }
            if (send_time_ms > max_send_time)
            {
                max_send_time = send_time_ms;
                max_send_idx = i;
            }
            avg_send_time += send_time_ms / n_clients;
        });

        client.on<RECEIVE>([n_clients, i, &avg_rtt, &min_rtt_idx, &max_rtt_idx, &min_rtt, &max_rtt](test_client::connection* conn, const char* message, size_t bytes){
            (void)message;
            auto after_recv = std::chrono::steady_clock::now();

            write(1, "r", 1);
            conn->data.total_recv_bytes += bytes;
            // received too many bytes
            if (conn->data.total_recv_bytes > sizeof(MESSAGE_STR))
            {
                std::cout << std::endl << "error: connection received more bytes: total_recv_bytes: " << conn->data.total_recv_bytes << "" << std::endl;
            }
            // message is complete
            else if (conn->data.total_recv_bytes == sizeof(MESSAGE_STR))
            {
                write (1, "*", 1);
                // send back message str
                auto rtt_ms = std::chrono::duration_cast<std::chrono::nanoseconds>(after_recv - conn->data.send_time).count();

                if (rtt_ms < min_rtt)
                {
                    min_rtt = rtt_ms;
                    min_rtt_idx = i;
                }
                if (rtt_ms > max_rtt)
                {
                    max_rtt = rtt_ms;
                    max_rtt_idx = i;
                }
                avg_rtt += rtt_ms / n_clients;
                conn->close();
            }
        });


        client.on<tcp::common_actions::CLOSED>([](const test_client::connection* conn){
            std::cout << "closed" << std::endl;
            if (conn->data.total_recv_bytes != sizeof(MESSAGE_STR))
                std::cout << std::endl << "error: disconnected connection did not recv all bytes: total_recv_bytes: " << conn->data.total_recv_bytes << "" << std::endl;
            else
                write(1, "-", 1);
        });

        client.on<ERROR>([](const std::string& func, int err){
            std::cout << std::endl << "error: " << func << ": " << strerror(err) << std::endl;
        });
    }

    auto before_connect = std::chrono::steady_clock::now();

    for (test_client& client : clients)
        client.connect("127.0.0.1", 8000);

    auto after_connect = std::chrono::steady_clock::now();
    
    std::cout << "handler count " << handler->count() << std::endl;
    while (events::poll(handler))
        ;    
    
    // for (auto& client : clients)
    //     client.close();
    

    std::cout << std::endl << std::endl;

    auto total_time = std::chrono::duration_cast<std::chrono::nanoseconds>(after_connect - before_connect).count();
    std::cout << "*************************************" << std::endl
              << "results for " << n_clients << " connections" << std::endl << std::endl
              << "total connection time: " << ((float)total_time / 1000) << "us (" << ((float)total_time / 1000000) << "ms)" << std::endl << std::endl
              << "send time (time to send first bytes of message, message may have been queued and flushed after)" << std::endl
              << "  max send time: " << ((float)max_send_time / 1000000) << "ms (by client[" << max_send_idx << "])" << std::endl
              << "  min send time: " << ((float)min_send_time / 1000000) << "ms (by client[" << min_send_idx << "])" << std::endl
              << "  avg send time: " << ((float)avg_send_time / 1000000) << "ms" << std::endl << std::endl
              << "rount trip time, time from send to recv response" << std::endl
              << "  max rtt: " << ((float)max_rtt / 1000000) << "ms (by client[" << max_rtt_idx << "])" << std::endl
              << "  min rtt: " << ((float)min_rtt / 1000000) << "ms (by client[" << min_rtt_idx << "])" << std::endl
              << "  avg rtt: " << ((float)avg_rtt / 1000000) << "ms" << std::endl;

}