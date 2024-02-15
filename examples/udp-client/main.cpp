#include "udp/socket.hpp"

using namespace unisock;
using namespace unisock::udp::actions;

int main()
{
    udp::socket client {};
    
    
    socket_address serv_address = socket_address::from("127.0.0.1", 8000, AF_INET);


    client.on<RECEIVE>([](const socket_address& address, const char* message,  size_t message_len) {
        std::cout << "received from " << socket_address::get_ip(address) << ":" << std::endl;
        std::cout << std::string(message, message_len) << std::endl;
        
    });

    client.on<CLOSED>([](const socket_address& addr){
        std::cout << "client endpoint closed: " << socket_address::get_ip(addr) << std::endl;
    });

    client.on<basic_actions::ERROR>([](const std::string& func, int err){
        std::cout << "error: " << func << ": " << strerror(err) << std::endl;
    });

    client.open(AF_INET);

    int on = 1;
    fcntl(client.get_socket(), F_SETFL, O_NONBLOCK, &on, sizeof(on));

    client.send_to(serv_address, "BEGIN", 6);

    while (events::poll(client))
        client.send_to(serv_address, "hello world", 12);

    client.close();
}