// #include "unisock.hpp"
#include "raw/socket.hpp"

using namespace unisock;
using namespace unisock::raw::actions;

int main()
{
    /* basic implementation of udp server using raw interface */
    raw::socket socket { };


    socket.on<unisock::basic_actions::READABLE>([&socket](){
        socket.recvfrom();
    });

    socket.on<RECVFROM>([](const socket_address& address, const char* message, size_t message_len){
        std::cout << "received message from " << address.to_string() << ": " << std::string(message, message_len) << std::endl;
    });

    socket.on<ERROR>([](const std::string& func, int message){
        std::cout << "error: " << func << ": " << message << std::endl;
    });

    socket.open(AF_INET, SOCK_DGRAM, 0);

    socket.address = socket_address::from("127.0.0.1", 8000, AF_INET);
    socket.bind();

    while (events::poll(socket))
        ;
    
    socket.close();


    // socket.recv_method = raw::method::recvfrom;
    // socket.send_method = raw::method::sendto;

    // socket.on<PACKET>(
    //     [&listener](raw::socket* socket, const socket_address& address, const char* message, size_t size) {
    //         (void)socket;
    //         std::cout << "received: '" << std::string(message, size) << std::endl << "'" << std::endl;
    //         std::cout << "from address:" << std::endl << address.to_string();
    //         std::cout << "sending: " << listener.send_to(socket, address, "Hey !") << std::endl;
            
    //     }
    // );

    // socket.on<ERROR> ( [](const std::string& func, int error) {
    //     std::cout << "error: " << func << ": " << strerror(error) << std::endl;
    // } );

    // // Adds a socket to the listener, which can be polled later
    // socket.init(AF_INET, SOCK_DGRAM, 0);

    // try {

    //     socket.address = socket_address::from("127.0.0.1", 8000, AF_INET);
    //     socket.bind();

    // } catch (std::logic_error& err)
    // {
    //     std::cout << "error while creating address: " << err.what() << std::endl;
    //     return (1);
    // }


    // while (true)
    // {
    //     std::cout << "poll" << std::endl;
    //     events::poll(socket);
    // }

    // socket.close();
}




// int main()
// {
//     /* basic implementation of udp server using raw interface */
//     raw::listener listener { };

//     listener.recv_method = raw::method::recvfrom;
//     listener.send_method = raw::method::sendto;

//     listener.on<PACKET>(
//         [&listener](raw::listener::socket* socket, const socket_address& address, const char* message, size_t size) {
//             (void)socket;
//             std::cout << "received: '" << std::string(message, size) << std::endl << "'" << std::endl;
//             std::cout << "from address:" << std::endl << address.to_string();
//             std::cout << "sending: " << listener.send_to(socket, address, "Hey !") << std::endl;
            
//         }
//     );

//     listener.on<ERROR> ( [](const std::string& func, int error) {
//         std::cout << "error: " << func << ": " << strerror(error) << std::endl;
//     } );

//     // Adds a socket to the listener, which can be polled later
//     listener.configure_socket( AF_INET, SOCK_DGRAM, 0, 
//         [](raw::listener::socket& socket) -> bool
//         {
//             try {
//                 std::cout << "before addr" << std::endl;
//                 socket.data.address = socket_address::from("127.0.0.1", 8000, AF_INET);
//                 std::cout << "after addr" << std::endl;
//             } catch (std::logic_error& err)
//             {
//                 std::cout << "error while creating address: " << err.what() << std::endl;
//                 return (true);
//             }
//             ::bind(socket.get_socket(), socket.data.address.template to<sockaddr>(), socket.data.address.size());
//             return (false);
//         }
//     );

//     while (true)
//     {
//         std::cout << "poll" << std::endl;
//         events::poll(listener);
//     }

//     listener.close();
// }