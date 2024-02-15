#include "udp/socket.hpp"

using namespace unisock;
using namespace unisock::udp::actions;

int main()
{
    udp::socket server {};

    server.on<RECEIVE>([&server](const socket_address& address, const char* message, size_t message_len) {
        std::cout << "received from " << socket_address::get_ip(address) << " on endpoint listening on " << socket_address::get_ip(server.address) << ":" << std::endl;
        std::cout << std::string(message, message_len) << std::endl;
        server.send_to(address, "received msg", 13);
    });

    server.on<BIND>([](const socket_address& address){
        std::cout << "server bound to " << socket_address::get_ip(address) << std::endl;
    });

    server.on<CLOSED>([](const socket_address& address){
        std::cout << "server endpoint closed: " << socket_address::get_ip(address) << std::endl;
    });

    server.on<basic_actions::ERROR>([](const std::string& func, int err){
        std::cout << "error: " << func << ": " << strerror(err) << std::endl;
    });

    server.bind("127.0.0.1", 8000);

    while (events::poll(server))
        ;

    server.close();
}