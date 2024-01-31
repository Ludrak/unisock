#include <iostream>
#include "unisock.hpp"

using namespace unisock;
using namespace tcp::actions;

int main()
{
    /* creating tcp client */
    tcp::client<>   client { };

    /* setting up hooks for events on that client */
    client.on<CONNECTED> ( [&client](auto& connection) {
        std::cout << "connected on " << connection.data.address.hostname() << std::endl;
        client.send(connection, "Hello server !");
    } );

    client.on<CLOSED> ( [](auto& connection) {
        std::cout << "connection closed from " << connection.data.address.hostname() << std::endl;
    } );


    client.on<MESSAGE> ( [](auto& connection, const char* message, size_t size) {
        std::cout << "received from connection " << connection.data.address.hostname() << ": '" << std::string(message, size) << "'" << std::endl;
    } );


    client.on<ERROR> ( [](const std::string& func, int error) {
        std::cout << "error: " << func << ": " << strerror(error) << std::endl;
    } );


    /* connect the client */
    client.connect("127.0.0.1", 8000);

    /* poll events */
    while (true)
        events::poll(client);

    /* close client */
    client.close();
}