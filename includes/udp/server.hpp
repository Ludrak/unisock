#pragma once

#include "udp/udp.hpp"

UNISOCK_NAMESPACE_START

UNISOCK_UDP_NAMESPACE_START

UNISOCK_LIB_NAMESPACE_START

template<typename _Socket, typename ..._Actions>
using server_actions = std::tuple<

    unisock::events::_lib::action<actions::LISTENING,
                                std::function<void (_Socket&)> >,
    
    _Actions...
>;

UNISOCK_LIB_NAMESPACE_END




template<typename ..._Data>
class server : public udp::server<std::tuple<>, _Data...>
{
    public:
        server() = default;

        server(const unisock::events::handler& handler)
        : udp::server<std::tuple<>, _Data...>(handler)
        {}
};


template<typename ..._Actions, typename ..._Data>
class server<std::tuple<_Actions...>, _Data...>
            :   public unisock::udp::_lib::socket_container<
                        _lib::server_actions<
                            udp::socket<_Data...>,
                            _Actions...
                        >,
                        _Data...
                    >
{
    using container_type = unisock::udp::_lib::socket_container<_lib::server_actions<socket<_Data...>, _Actions...>, _Data...>;

    public:
        server() = default;

        server(const unisock::events::handler& handler)
        : container_type(handler)
        {}

        /* makes the server start listening, returns false on error */
        /* errno of the error can be retrieved in actions::ERROR    */
        virtual bool listen(const std::string& ip_address, const int port, const sa_family_t family = AF_INET);


        /* makes the server start listening on multicast,              */
        /* on specified address group and port, returns false on error */
        /* errno of the error can be retrieved in actions::ERROR       */
        virtual bool listen_multicast(const std::string& multiaddr, const std::string& interface, const int port, const sa_family_t interface_family);


        /* makes the server start listening on multicast on specified port, returns false on error */
        /* errno of the error can be retrieved in actions::ERROR                                   */
        virtual bool listen_broadcast(const std::string& interface, const int port, const sa_family_t interface_family);
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
    if (-1 == ::bind(sock->get_socket(), sock->data.address.template to_address<sockaddr>(), sock->data.address.size()))
    {
        this->template execute<actions::ERROR>("bind", errno);
        return (false);
    }

    this->template execute<actions::LISTENING>(*sock);
    return (true);
}




template<typename ..._Actions, typename ..._Data>
inline bool udp::server<std::tuple<_Actions...>, _Data...>::listen_multicast(const std::string& multiaddr, const std::string& interface, const int port, const sa_family_t interface_family)
{
    auto* sock = this->make_socket(interface_family, SOCK_DGRAM, 0);
    if (sock == nullptr)
    {
        this->template execute<actions::ERROR>("socket", errno);
        return false;
    }
    sock->data.address = { interface, port, interface_family };
    if (-1 == ::bind(sock->get_socket(), sock->data.address.template to_address<sockaddr>(), sock->data.address.size()))
    {
        this->template execute<actions::ERROR>("bind", errno);
        return (false);
    }

    struct ip_mreq mreq;
    mreq.imr_multiaddr.s_addr = inet_addr(multiaddr.c_str());
    mreq.imr_interface.s_addr = *reinterpret_cast<const in_addr_t*>(sock->data.address.in_addr());
    if (!sock->setsockopt(IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*) &mreq, sizeof(mreq)))
    {
        this->template execute<actions::ERROR>("setsockopt", errno);
        return (false);
    }

    this->template execute<actions::LISTENING>(*sock);
    return (true);
}




template<typename ..._Actions, typename ..._Data>
inline bool udp::server<std::tuple<_Actions...>, _Data...>::listen_broadcast(const std::string& interface, const int port, const sa_family_t interface_family)
{
    auto* sock = this->make_socket(interface_family, SOCK_DGRAM, 0);
    if (sock == nullptr)
    {
        this->template execute<actions::ERROR>("socket", errno);
        return false;
    }
    sock->data.address = { interface, port, interface_family };
    if (-1 == ::bind(sock->get_socket(), sock->data.address.template to_address<sockaddr>(), sock->data.address.size()))
    {
        this->template execute<actions::ERROR>("bind", errno);
        return (false);
    }

    int broadcast = 1;
    if (!sock->setsockopt(SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast)))
    {
        this->template execute<actions::ERROR>("setsockopt", errno);
        return (false);
    }

    this->template execute<actions::LISTENING>(*sock);
    return (true);
}

UNISOCK_UDP_NAMESPACE_END

UNISOCK_NAMESPACE_END