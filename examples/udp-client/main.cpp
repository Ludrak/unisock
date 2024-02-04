#include <iostream>
#include "unisock.hpp"
#include "raw/common.hpp"

using namespace unisock;
using namespace udp::actions;

int main()
{
    /* basic udp send without client/server or event polling */
    inet_address server_addr = inet_address::from("127.0.0.1", 8000, AF_INET);
    size_t result = raw::send_to(server_addr, "Trying connection !");
    if (result != raw::send_result::SUCCESS)
    {
        std::cout << "Could not resolve server !" << std::endl;
        return (0);
    }

    udp::client client {};
    
    client.on<MESSAGE>([&client](const udp::socket<>& socket, const inet_address& server_address, const char* message, const size_t size){
        // send to all targetted servers
        client.send("Thanks for your message : " + std::string(message, size) + "\n");
        // send to specific socket
        (void)server_address;
        udp::send(socket, "for you ");
    });


    client.on<ERROR> ( [](const std::string& func, int error) {
        std::cout << "error: " << func << ": " << strerror(error) << std::endl;
    } );


    // sets the address of the server that will be targetted when using client.send("message")
    client.target_server("127.0.0.1", 8000);

    client.send("Hello !");

    while (true)
        events::poll(client);
    
    client.close();
}