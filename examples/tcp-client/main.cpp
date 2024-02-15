#include "tcp/client.hpp"
#include "tcp/server.hpp"

using namespace unisock;
using namespace tcp::common_actions;
using namespace tcp::client_actions;

int main()
{
    std::shared_ptr<events::handler> handler = std::make_shared<events::handler>();
    std::cout << "handler:     " << handler.get() << std::endl;

    tcp::client client = tcp::client(handler);

    client.on<CONNECT>([](tcp::client::connection* connection){
        std::cout << "connected to " << socket_address::get_ip(connection->address) << std::endl;
        connection->send("hello world", 11);
    });

    client.on<CLOSED>([](tcp::client::connection* connection){
        std::cout << "closed connection to " << socket_address::get_ip(connection->address) << std::endl;
    });

    client.on<RECEIVE>([](tcp::client::connection* connection, const char* message, size_t bytes){
        std::cout << "received from connection " << socket_address::get_ip(connection->address) << ": " << std::string(message, bytes) << std::endl;
    });

    client.on<basic_actions::ERROR>([](const std::string& func, int err){
        std::cout << "error: " << func << ": " << strerror(err) << std::endl;
    });

    client.connect("127.0.0.1", 8000);


    std::cout << "handlers:    " << client.get_handler().get() << std::endl ;
            //    <<"             " << client2.get_handler().get() << std::endl;

    while (events::poll( handler ))
        ;
    
    client.close();
}