#include <iostream>
#include "unisock.hpp"

using namespace unisock;
using namespace udp::actions;

int main()
{
    udp::server<> server {};

    server.on<LISTENING>( [](const udp::socket<>& socket){ 
        std::cout << "listening on " << socket.data.address.hostname() << std::endl;
    });

    server.on<CLOSED>( [](const udp::socket<>& socket){ 
        std::cout << "endpoint closed from " << socket.data.address.hostname() << std::endl;
    });


    server.on<MESSAGE>( [](const udp::socket<>& socket, const inet_address& client_address, const char* message, size_t size){ 
        std::cout << "received from " << socket.data.address.hostname() << ": [" << client_address.hostname() << "]: '" << std::string(message, size) << "'" << std::endl;
    });


    server.on<ERROR> ( [](const std::string& func, int error) {
        std::cout << "error: " << func << ": " << strerror(error) << std::endl;
    } );

    server.listen("127.0.0.1", 8000);

    while (true)
        events::poll(server);
    
    server.close();
}