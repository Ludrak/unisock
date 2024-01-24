#pragma once

#include "net/socket.hpp"
#include "net/inet_address.hpp"
#include "net/server.hpp"

#include "events/events.hpp"
#include "unisock.hpp"

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
class server : public unisock::server<_Data..., socket_data> //, public unisock::inet_address
{
    public:
        using server_type = typename unisock::server<_Data..., socket_data>;
        using socket_type = typename server_type::socket_type;

       // server() = delete;
        server(/* const std::string& ip_address, const int port, const sa_family_t family = AF_INET */)
        :   unisock::server<_Data..., socket_data>()
        //    unisock::inet_address(ip_address, port, family)
        {
            // server::socket_type sock = server::socket_type();
            // sock.data.type = connection_type::SERVER;
            // sock.init(family, SOCK_STREAM, 0);
            // this->sockets.insert(std::make_pair(unisock::events::make_data<unisock::handler_type>(sock.getSocket()), sock));
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
            // this->server_type::listen();
        }

        // called when some socket needs to read received data
        // (i.e. on received)
        virtual void    on_receive(socket_wrap* socket) override
        {
            auto* client = reinterpret_cast<socket_type*>(socket);
            
            std::cout << "received on " << client->getSocket() << std::endl;

            // 1. if socket is a listener 
            // -> accept
            // -> create client
            // -> add socket to handler

            // 2. if socket is client (that has already been accepted)
            // -> recv
            // -> call ::on handler
        }

        // called when a socket that was requesting write got writeable
        // (i.e. on queued send)
        virtual void    on_writeable(socket_wrap* socket) override
        {
            (void)socket;
            // send queued data if any
        }
    
    private:
};


UNISOCK_TCP_NAMESPACE_END

UNISOCK_NAMESPACE_END