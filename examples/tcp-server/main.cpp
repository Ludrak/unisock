#include "tcp/server.hpp"

using namespace unisock;
using namespace unisock::tcp::common_actions;
using namespace unisock::tcp::server_actions;

int main()
{
    tcp::server server {};

    server.on<LISTEN>([](tcp::server::server_connection* conn){
        std::cout << "server listening on " << socket_address::get_ip(conn->address) << " on socket " << conn->get_socket() << std::endl;
    });

    // server.on<CLOSE>([](){
    //     std::cout << "server closed" << std::endl;
    // });

    server.on<RECEIVE>([&server](tcp::server::client_connection* conn, const char* message, size_t bytes){
        std::cout << "received from " << socket_address::get_ip(conn->address) << ": " << std::string(message, bytes) << std::endl;

        if (std::string(message, bytes) == "close\n")
        {
            std::cout << "closing" << std::endl;
            server.close();
        }
    });

    server.on<ACCEPT>([](tcp::server::client_connection* conn){
        std::cout << "client connected from " << socket_address::get_ip(conn->address) << std::endl;
    });

    server.on<DISCONNECT>([](tcp::server::client_connection* conn){
        std::cout << "client disconnected from " << socket_address::get_ip(conn->address) << std::endl;
    });


    server.on<ERROR>([](const std::string& func, int err){
        std::cout << "error: " << func << ": " << strerror(err) << std::endl;
    });


    server.listen("127.0.0.1", 8000);
    server.listen("::1", 8000, true);

    while (events::poll(server))
        ;
    
    server.close();
}