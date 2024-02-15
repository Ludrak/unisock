#include "tcp/server.hpp"

using namespace unisock;
using namespace unisock::tcp::common_actions;
using namespace unisock::tcp::server_actions;


int main()
{
    tcp::server server { };

    server.on<LISTEN>([](tcp::server::server_connection* conn){
        std::cout << "server listening on " << socket_address::get_ip(conn->address) << " on socket " << conn->get_socket() << std::endl
                  << "server will send + or - when connections are accepted/disconnected" << std::endl;
    });

    server.on<ACCEPT>([](tcp::server::client_connection* conn){
        (void)conn;
        write(1, "+", 1);
    });

    server.on<DISCONNECT>([](tcp::server::client_connection* conn){
        (void)conn;
        write(1, "-", 1);
    });


    server.on<basic_actions::ERROR>([](const std::string& func, int err){
        std::cout << std::endl << "error: " << func << ": " << strerror(err) << std::endl;
    });


    server.listen("127.0.0.1", 8000);

    while (events::poll(server))
        ;
  
    server.close();
}