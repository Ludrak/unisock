#pragma once

#include "net/socket.hpp"
#include "net/inet_address.hpp"
#include "net/server.hpp"

#include "events/events.hpp"
#include "unisock.hpp"

UNISOCK_NAMESPACE_START

UNISOCK_TCP_NAMESPACE_START



class socket_data 
{
    public:
        int data_to_send;
};

template<typename ..._Data>
class server : public unisock::server<_Data..., socket_data>, public unisock::inet_address
{
    public:
        typedef typename unisock::server<_Data..., socket_data>::socket_type socket_type;

        server() = delete;
        server(const std::string& ip_address, const int port, const sa_family_t family = AF_INET)
        :   unisock::server<_Data..., socket_data>(),
            unisock::inet_address(ip_address, port, family)
        {
            server::socket_type sock = server::socket_type();
            sock.init(family, SOCK_STREAM, 0);
            this->sockets.insert(std::make_pair(sock.getSocket(), sock));
        }

        void send()
        {
            socket_type client;
            client.data_to_send = 2;
        }
};


UNISOCK_TCP_NAMESPACE_END

UNISOCK_NAMESPACE_END