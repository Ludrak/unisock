#include "tcp/client.hpp"

using namespace unisock;
using namespace tcp::common_actions;
using namespace tcp::client_actions;

int main()
{
    tcp::client client {};

    client.on<CONNECT>([](tcp::client::connection* connection){
        std::cout << "connected to " << socket_address::get_ip(connection->address) << std::endl;


        connection->send("hello world", 11);
    });

    client.on<RECEIVE>([](tcp::client::connection* connection, const char* message, size_t bytes){
        std::cout << "received from connection " << socket_address::get_ip(connection->address) << ": " << std::string(message, bytes) << std::endl;
    });

    client.on<ERROR>([](const std::string& func, int err){
        std::cout << "error: " << func << ": " << strerror(err) << std::endl;
    });


    client.connect("127.0.0.1", 8000);
    
    while (events::poll(client))
        ;
    
    client.close();
}