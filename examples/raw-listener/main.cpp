// #include "unisock.hpp"
#include "raw/socket.hpp"

using namespace unisock;
using namespace unisock::raw::actions;

int main()
{
    /* basic implementation of udp server using raw interface */
    raw::socket socket { };


    socket.on<basic_actions::READABLE>([&socket](){
        socket.recvfrom();
    });

    socket.on<RECVFROM>([](const socket_address& address, const char* message, size_t message_len){
        std::cout << "received message from " << address.to_string() << ": " << std::string(message, message_len) << std::endl;
    });

    socket.on<basic_actions::ERROR>([](const std::string& func, int message){
        std::cout << "error: " << func << ": " << message << std::endl;
    });

    socket.open(AF_INET, SOCK_DGRAM, 0);

    socket.address = socket_address::from("127.0.0.1", 8000, AF_INET);
    if (!socket.bind())
        return (1);

    while (events::poll(socket))
        ;
    
    socket.close();
}
