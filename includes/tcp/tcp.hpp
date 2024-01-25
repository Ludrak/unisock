#pragma once

#include "net/socket.hpp"
#include "net/inet_address.hpp"
#include "net/server.hpp"

#include "events/events.hpp"

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

class socket_data 
{
    public:
        connection_type type;
        inet_address    address;
};

template<typename ..._Data>
class server : public unisock::_lib::server<_Data..., socket_data> //, public unisock::inet_address
{
    public:
        using server_type = typename unisock::_lib::server<_Data..., socket_data>;
        using socket_type = typename server_type::socket_type;

        server() = delete;
        server(events::handler& handler)
        : unisock::_lib::server<_Data..., socket_data>(handler)
        {
        }

    private:
        /* putting inherited listen() in private to replace it with new version */
        void listen() override {}

    public:
        virtual void listen(const std::string& ip_address, const int port, const sa_family_t family = AF_INET)
        {
            server::socket_type* sock = this->make_socket(family, SOCK_STREAM, 0);
            if (sock == nullptr)
                return ;
            sock->data.type = connection_type::SERVER;
            sock->data.address = { ip_address, port, family };
            ::bind(sock->getSocket(), sock->data.address.getAddress(), sock->data.address.getAddressSize());
            ::listen(sock->getSocket(), 10);
        }

    private:
        // called when some socket needs to read received data
        // (i.e. on received)
        virtual void    on_receive(unisock::_lib::socket_wrap* s) override
        {
            auto* socket = reinterpret_cast<socket_type*>(s);
            if (socket->data.type == connection_type::SERVER)
            {
                struct sockaddr_in  s_addr {};
                socklen_t           s_len = sizeof(s_addr);
                int client = ::accept(socket->getSocket(), reinterpret_cast<sockaddr*>(&s_addr), &s_len);
                
                server::socket_type* client_sock = this->make_socket(client);
                if (client_sock == nullptr)
                    return ;
                client_sock->data.type = connection_type::CLIENT;
                client_sock->data.address.setAddress(s_addr);
                std::cout << "client connected from " << client_sock->data.address.getHostname() << " on socket " << client_sock->getSocket() << std::endl;
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
                    std::cout << "client disconnected from " << socket->data.address.getHostname() << " on socket " << socket->getSocket() << std::endl;
                    // close client
                    this->delete_socket(socket->getSocket());
                    return ;
                }
                // received n_bytes in buffer
                std::cout << "received from " << socket->data.address.getHostname() << " on socket " << socket->getSocket() << ": '" << std::string(buffer, n_bytes) << "'" << std::endl;
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