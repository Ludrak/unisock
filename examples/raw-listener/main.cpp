#include "unisock.hpp"
#include "raw/common.hpp"

using namespace unisock;
using namespace unisock::raw::actions;

int main()
{
    /* basic implementation of udp server using raw interface */
    raw::listener listener { };

    listener.recv_method = raw::method::recvfrom;
    listener.send_method = raw::method::sendto;

    listener.on<PACKET>(
        [&listener](raw::listener::socket* socket, const inet_address& address, const char* message, size_t size) {
            (void)socket;
            std::cout << "received: '" << std::string(message, size) << std::endl << "'" << std::endl;
            std::cout << "from address:" << std::endl << address.to_string();
            std::cout << "sending: " << listener.send_to(socket, address, "Hey !") << std::endl;
            
        }
    );

    listener.on<ERROR> ( [](const std::string& func, int error) {
        std::cout << "error: " << func << ": " << strerror(error) << std::endl;
    } );

    // Adds a socket to the listener, which can be polled later
    listener.configure_socket( AF_INET, SOCK_DGRAM, 0, 
        [](raw::listener::socket& socket) -> bool
        {
            try {
                std::cout << "before addr" << std::endl;
                socket.data.address = inet_address::from("127.0.0.1", 8000, AF_INET);
                std::cout << "after addr" << std::endl;
            } catch (std::logic_error& err)
            {
                std::cout << "error while creating address: " << err.what() << std::endl;
                return (true);
            }
            ::bind(socket.get_socket(), socket.data.address.template to<sockaddr>(), socket.data.address.size());
            return (false);
        }
    );

    while (true)
    {
        std::cout << "poll" << std::endl;
        events::poll(listener);
    }

    listener.close();
}