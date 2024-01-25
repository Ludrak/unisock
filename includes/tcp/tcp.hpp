#pragma once

#include "net/socket.hpp"
#include "net/inet_address.hpp"

#include "events/events.hpp"
#include "events/action_hanlder.hpp"

#include <iostream>

UNISOCK_NAMESPACE_START

UNISOCK_TCP_NAMESPACE_START

enum  connection_type
{
    // listener connection
    SERVER,

    // connection accepted by listener
    CLIENT
};

struct actions
{
    struct LISTEN {};
    struct CLOSE {};

    struct CONNECT {};
    struct DISCONNECT {};

    struct MESSAGE {};

    struct ERROR {};
};


class socket_data 
{
    public:
        connection_type type;
        inet_address    address;
};


template<typename ..._Data>
struct connection
{
    using type = typename unisock::events::_lib::socket_container<_Data..., socket_data>::socket_type;
};



template<typename ..._Data>
class server :  public unisock::events::_lib::socket_container<_Data..., socket_data>,
                public unisock::events::_lib::action_handler
                <
                    unisock::events::_lib::action<actions::LISTEN,  
                                                  std::function<void ()> >,

                    unisock::events::_lib::action<actions::CLOSE,  
                                                  std::function<void ()> >,

                    unisock::events::_lib::action<actions::CONNECT,
                                                  std::function<void (typename tcp::connection<_Data...>::type& )> >,

                    unisock::events::_lib::action<actions::DISCONNECT,
                                                  std::function<void (typename tcp::connection<_Data...>::type& )> >,

                    unisock::events::_lib::action<actions::MESSAGE,  
                                                  std::function<void (typename tcp::connection<_Data...>::type&, const char *, size_t)> >
                >
{
        using container_type = typename unisock::events::_lib::socket_container<_Data..., socket_data>;

    public:
        using connection = typename tcp::connection<_Data...>::type;

        server()
        : container_type()
        {
        }

        server(unisock::events::handler& handler)
        : container_type(handler)
        {
        }

    public:
        virtual void listen(const std::string& ip_address, const int port, const sa_family_t family = AF_INET)
        {
            server::connection* sock = this->make_socket(family, SOCK_STREAM, 0);
            if (sock == nullptr)
                return ;
            sock->data.type = connection_type::SERVER;
            sock->data.address = { ip_address, port, family };
            ::bind(sock->getSocket(), sock->data.address.getAddress(), sock->data.address.getAddressSize());
            ::listen(sock->getSocket(), 10);

            this->template execute<actions::LISTEN>();
        }

    private:
        // called when some socket needs to read received data
        // (i.e. on received)
        virtual void    on_receive(unisock::_lib::socket_wrap* s) override
        {
            auto* socket = reinterpret_cast<connection*>(s);
            if (socket->data.type == connection_type::SERVER)
            {
                struct sockaddr_in  s_addr {};
                socklen_t           s_len = sizeof(s_addr);
                int client = ::accept(socket->getSocket(), reinterpret_cast<sockaddr*>(&s_addr), &s_len);
                
                server::connection* client_sock = this->make_socket(client);
                if (client_sock == nullptr)
                    return ;
                client_sock->data.type = connection_type::CLIENT;
                client_sock->data.address.setAddress(s_addr);

                this->template execute<actions::CONNECT>(static_cast<connection&>(*client_sock));
            }
            else /* if socket.data.type == connection_type::CLIENT */
            {
                char buffer[1024] {0};
                int n_bytes = ::recv(socket->getSocket(), buffer, 1024, MSG_DONTWAIT);
                if (n_bytes < 0)
                {
                    return ; // recv error
                }
                else if (n_bytes == 0)
                {
                    this->template execute<actions::DISCONNECT>(static_cast<connection&>(*socket));
                    // close client
                    this->delete_socket(socket->getSocket());
                    return ;
                }
                // received n_bytes in buffer
                this->template execute<actions::MESSAGE>(static_cast<connection&>(*socket), buffer, n_bytes);
            }
        }

        // called when a socket that was requesting write got writeable
        // (i.e. on queued send)
        virtual void    on_writeable(unisock::_lib::socket_wrap* socket) override
        {
            (void)socket;
            // send queued data if any
        }
    
    private:
};


UNISOCK_TCP_NAMESPACE_END

UNISOCK_NAMESPACE_END