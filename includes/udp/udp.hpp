#pragma once

#include "namespaces.hpp"
#include "net/inet_address.hpp"
#include "events/events.hpp"
#include "events/action_hanlder.hpp"
#include "sys/socket.h"

UNISOCK_NAMESPACE_START

UNISOCK_UDP_NAMESPACE_START


// actions to be hooked to udp server with .on(lambda())
namespace actions
{
    //  server started listening
    //  prototype: void  (const inet_addr& address);
    struct LISTENING {};


    //  server endpoint closed
    //  prototype: void  (inet_address& endpoint);
    struct CLOSED {};

 
    //  server received data from an address
    //  prototype: void  (inet_address& endpoint, inet_address& );
    struct MESSAGE {};

    // an error has occurred 
    // prototype: void  (const std::string& function, int errno);
    struct ERROR {};
};

UNISOCK_LIB_NAMESPACE_START

class socket_data
{
    public:
        inet_address    address;
};

UNISOCK_LIB_NAMESPACE_END


/* udp socket type defintion */
template<typename ..._Data>
using socket = unisock::_lib::socket<_Data..., _lib::socket_data>;


/* actions for udp server */
template<typename _Socket, typename ..._Actions>
using server_actions = std::tuple<
    unisock::events::_lib::action<actions::LISTENING,
                        std::function<void (const _Socket&)> >,

    unisock::events::_lib::action<actions::CLOSED,
                        std::function<void (const _Socket&)> >,

    unisock::events::_lib::action<actions::MESSAGE,
                        std::function<void (const _Socket&, const inet_address&, const char*, size_t)> >,

    unisock::events::_lib::action<actions::ERROR,
                        std::function<void (const std::string&, int)> >,

    _Actions...
>;


/* server definition with all args */
template<typename ..._Args>
class server : public server<std::tuple<>, _Args...>
{
    public:
        server() = default;

        server(const unisock::events::handler& handler)
        : server<std::tuple<>, _Args...>(handler)
        {}
};

/* server definition with _Action and _Data */
template<typename ..._Actions, typename ..._Data>
class server<std::tuple<_Actions...>, _Data...>
            : public unisock::events::_lib::socket_container<_Data..., _lib::socket_data>,
              public unisock::events::_lib::action_handler<server_actions<udp::socket<_Data...>, _Actions...>>
{
    static constexpr size_t RECV_BLOCK_SIZE = 1024;

    using container_type = unisock::events::_lib::socket_container<_Data..., _lib::socket_data>;

    public:
        server() = default;

        server(const unisock::events::handler& handler)
        : container_type(handler)
        {}

        /* makes the server start listening, returns false on error */
        /* errno of the error can be retrieved in actions::ERROR    */
        virtual bool listen(const std::string& ip_address, const int port, const sa_family_t family = AF_INET);


    private:
        virtual bool    on_receive(unisock::_lib::socket_wrap* sptr) override;
        virtual bool    on_writeable(unisock::_lib::socket_wrap* sptr) override { (void)sptr; return (false); };
};






template<typename ..._Actions, typename ..._Data>
inline bool udp::server<std::tuple<_Actions...>, _Data...>::listen(const std::string& ip_address, const int port, const sa_family_t family)
{
    auto* sock = this->make_socket(family, SOCK_DGRAM, 0);
    if (sock == nullptr)
    {
        this->template execute<actions::ERROR>("socket", errno);
        return false;
    }
    sock->data.address = { ip_address, port, family };
    if (-1 == ::bind(sock->getSocket(), sock->data.address.template to_address<sockaddr>(), sock->data.address.size()))
    {
        this->template execute<actions::ERROR>("bind", errno);
        return (false);
    }

    this->template execute<actions::LISTENING>(*sock);

    return (true);
}




template<typename ..._Actions, typename ..._Data>
inline bool    udp::server<std::tuple<_Actions...>, _Data...>::on_receive(unisock::_lib::socket_wrap* sptr)
{
    auto* socket = reinterpret_cast<udp::socket<_Data...>*>(sptr);
    char  buffer[RECV_BLOCK_SIZE];

    // TODO: change this when refractoring inet_address
    char        addr[inet_address::ADDRESS_SIZE];
    socklen_t   addr_len = inet_address::ADDRESS_SIZE;

    size_t n_bytes = ::recvfrom(socket->getSocket(), buffer, RECV_BLOCK_SIZE, 0, reinterpret_cast<sockaddr*>(&addr), &addr_len);
    if (n_bytes < 0)
    {
        this->template execute<actions::ERROR>("recvfrom", errno);
        return (false);
    }

    inet_address client_address { addr, addr_len };
    this->template execute<actions::MESSAGE>(*socket, client_address, buffer, n_bytes);
    return (false);
}


UNISOCK_UDP_NAMESPACE_END

UNISOCK_NAMESPACE_END