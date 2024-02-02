#pragma once

#include "tcp/common.hpp"

UNISOCK_NAMESPACE_START

UNISOCK_TCP_NAMESPACE_START

UNISOCK_LIB_NAMESPACE_START

/* ======================================================================== */
/* SERVER ACTIONS                                                           */


/* actions types for tcp::server_impl */
template<typename _Connection, typename ..._Actions>
using server_actions = std::tuple<

    unisock::events::_lib::action<actions::LISTENING,
                                std::function<void (_Connection& )> >,

    // connect handler:     void (tcp::connection<...>& client)
    unisock::events::_lib::action<actions::CONNECT,
                                std::function<void (_Connection& )> >,

    // disconnect handler:  void (tcp::connection<...>& client)
    unisock::events::_lib::action<actions::DISCONNECT,
                                std::function<void (_Connection& )> >,

    _Actions...
>;



/* definition of server with all args */
template<typename ..._Args>
class server_impl;

UNISOCK_LIB_NAMESPACE_END



/* ======================================================================== */
/* TYPES ALIASES FOR CREATING SERVERS                                       */



/* definition of standart server with no additionnal data */
using server = unisock::tcp::_lib::server_impl<std::tuple<>>;




/* type alias to get a server with some additionnal data  */
template<typename ..._ConnectionData>
using server_of = unisock::tcp::_lib::server_impl<std::tuple<>, _ConnectionData...>;





/* ======================================================================== */
/* SERVER IMPLEMENTATION DEFINITION                                         */


UNISOCK_LIB_NAMESPACE_START

/* standart specialization for _Actions and _Data */
template<typename ..._Actions, typename ..._Data>
class server_impl<std::tuple<_Actions...>, _Data...>
             :  public unisock::tcp::_lib::common_impl<
                    _lib::server_actions<
                        tcp::connection<_Data...>,
                        _Actions...
                    >, 
                    _Data...
                >
{
    using container_type = unisock::tcp::_lib::common_impl<_lib::server_actions<connection<_Data...>, _Actions...>, _Data...>;

    public:
        server_impl() = default;

        server_impl(unisock::events::handler& handler)
        : container_type(handler)
        {}

        /* makes the server start listening, returns false on error */
        /* errno of the error can be retrieved in actions::ERROR    */
        virtual bool listen(const std::string& ip_address, const int port, const sa_family_t family = AF_INET);

    private:
        bool            on_endpoint_receive(connection<_Data...>& socket);
        bool            on_client_receive(connection<_Data...>& socket) override;

        // // called when some socket needs to read received data
        // // (i.e. on received)
        // // returns true if iterators of socket_container::sockets are changed
        virtual bool    on_receive(unisock::_lib::socket_wrap* sptr) override;
};





/* ======================================================================== */
/* IMPLEMENTATION                                                           */


template<typename ..._Actions, typename ..._Data>
inline bool tcp::_lib::server_impl<std::tuple<_Actions...>, _Data...>::listen(const std::string& ip_address, const int port, const sa_family_t family)
{
    auto* sock = this->make_socket(family, SOCK_STREAM, 0);
    if (sock == nullptr)
    {
        this->template execute<actions::ERROR>("socket", errno);
        return false;
    }
    sock->data.type = _lib::connection_type::SERVER;
    sock->data.address = { ip_address, port, family };
    if (-1 == ::bind(sock->get_socket(), sock->data.address.template to<sockaddr>(), sock->data.address.size()))
    {
        this->template execute<actions::ERROR>("bind", errno);
        return (false);
    }
    
    if (-1 == ::listen(sock->get_socket(), 10))
    {
        this->template execute<actions::ERROR>("listen", errno);
        return (false);
    }

    this->template execute<actions::LISTENING>(*sock);

    return (true);
}



template<typename ..._Actions, typename ..._Data>
inline bool    tcp::_lib::server_impl<std::tuple<_Actions...>, _Data...>::on_endpoint_receive(connection<_Data...>& socket)
{
    // struct sockaddr_in  s_addr {};
    // socklen_t           s_len = sizeof(s_addr);
    struct sockaddr_storage addr;
    socklen_t               addr_len = sizeof(addr);
    memset(&addr, 0, addr_len);

    int client = ::accept(socket.get_socket(), reinterpret_cast<sockaddr*>(&addr), &addr_len);//reinterpret_cast<sockaddr*>(&s_addr), &s_len);
    if (client < 0)
    {
        // accept error
        this->template execute<actions::ERROR>("accept", errno);
        return (false);
    }

    auto* client_sock = this->make_socket(client);
    if (client_sock == nullptr)
    {
        // client insertion failed
        ::close(client);
        return false;
    }
    client_sock->data.type = _lib::connection_type::CLIENT;
    client_sock->data.address = { addr };

    this->template execute<actions::CONNECT>(static_cast<tcp::connection<_Data...>&>(*client_sock));
    return (false);
}



template<typename ..._Actions, typename ..._Data>
inline bool tcp::_lib::server_impl<std::tuple<_Actions...>, _Data...>::on_client_receive(connection<_Data...>& socket)
{
    bool disconnected = container_type::on_client_receive(socket);
    if (disconnected)
    {
        this->template execute<actions::DISCONNECT>(socket);
        this->delete_socket(socket.get_socket());
    }
    return (disconnected);
}





template<typename ..._Actions, typename ..._Data>
inline bool    tcp::_lib::server_impl<std::tuple<_Actions...>, _Data...>::on_receive(unisock::_lib::socket_wrap* sptr)
{
    assert(sptr != nullptr);

    auto* socket = reinterpret_cast<connection<_Data...>*>(sptr);

    switch (socket->data.type)
    {
    case unisock::tcp::_lib::connection_type::SERVER:
        this->on_endpoint_receive(*socket);
        break;

    case unisock::tcp::_lib::connection_type::CLIENT:
        this->on_client_receive(*socket);
        break;

    default: break;
    }

    // TODO: iter may be invalid
    return (false);
}

UNISOCK_LIB_NAMESPACE_END

UNISOCK_TCP_NAMESPACE_END

UNISOCK_NAMESPACE_END