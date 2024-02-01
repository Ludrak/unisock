#include <iostream>
#include "unisock.hpp"

using namespace unisock;
using namespace tcp::actions;

struct a{int a;};

int main()
{
    /* creating tcp server */
    tcp::server server = tcp::server();

    /* setting up hooks for events on that server */
    server.on<LISTENING> ( [](auto& connection) {
        std::cout << "listening on " << connection.data.address.hostname() << " ip " << connection.data.address.ip() << " " << connection.data.address.port() << std::endl;
    } );

    server.on<CLOSED> ( [](auto& connection) {
        std::cout << "endpoint closed from " << connection.data.address.hostname() << std::endl;
    } );


    server.on<CONNECT> ( [](auto& connection) {
        std::cout << "[+] client connected (" << connection.data.address.hostname() << ")" << std::endl;
    } );

    server.on<DISCONNECT> ( [](auto& connection) {
        std::cout << "[-] client disconnected (" << connection.data.address.hostname() << ")" << std::endl;
    } );


    server.on<MESSAGE> ( [](auto& client, const char* message, size_t size) {
        std::cout << "<" << client.data.address.hostname() << ">: '" << std::string(message, size) << "'" << std::endl;
    } );


    server.on<ERROR> ( [](const std::string& func, int error) {
        std::cout << "error: " << func << ": " << strerror(error) << std::endl;
    } );


    /* connect the server */
    server.listen("127.0.0.1", 8000);

    /* poll events */
    while (true)
        events::poll(server);

    /* close server */
    server.close();
}