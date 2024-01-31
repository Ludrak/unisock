#pragma once

#include "tcp/tcp.hpp"

UNISOCK_NAMESPACE_START

UNISOCK_TCP_NAMESPACE_START

UNISOCK_LIB_NAMESPACE_START

/* actions types for tcp::server */
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

UNISOCK_LIB_NAMESPACE_END


/* definition of server with all args */
template<typename ..._Args>
class server : public server<std::tuple<>, _Args...>
{
    public:
        server() = default;
        // handler constructor
        server(unisock::events::handler& handler)
        : server<std::tuple<>, _Args...>(handler)
        {}
};


/* standart specialization for _Actions and _Data */
template<typename ..._Actions, typename ..._Data>
class server<std::tuple<_Actions...>, _Data...>
             :  public unisock::tcp::_lib::socket_container<
                    _lib::server_actions<
                        tcp::connection<_Data...>,
                        _Actions...
                    >, 
                    _Data...
                >
{
    using container_type = unisock::tcp::_lib::socket_container<_lib::server_actions<connection<_Data...>, _Actions...>, _Data...>;

    public:
        server() = default;

        server(unisock::events::handler& handler)
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
inline bool tcp::server<std::tuple<_Actions...>, _Data...>::listen(const std::string& ip_address, const int port, const sa_family_t family)
{
    auto* sock = this->make_socket(family, SOCK_STREAM, 0);
    if (sock == nullptr)
    {
        this->template execute<actions::ERROR>("socket", errno);
        return false;
    }
    sock->data.type = _lib::connection_type::SERVER;
    sock->data.address = { ip_address, port, family };
    if (-1 == ::bind(sock->getSocket(), sock->data.address.template to_address<sockaddr>(), sock->data.address.size()))
    {
        this->template execute<actions::ERROR>("bind", errno);
        return (false);
    }
    
    if (-1 == ::listen(sock->getSocket(), 10))
    {
        this->template execute<actions::ERROR>("listen", errno);
        return (false);
    }

    this->template execute<actions::LISTENING>(*sock);

    return (true);
}



template<typename ..._Actions, typename ..._Data>
inline bool    tcp::server<std::tuple<_Actions...>, _Data...>::on_endpoint_receive(connection<_Data...>& socket)
{
    // struct sockaddr_in  s_addr {};
    // socklen_t           s_len = sizeof(s_addr);
    char        addr[inet_address::ADDRESS_SIZE] { 0 };
    socklen_t   addr_size = inet_address::ADDRESS_SIZE;
    int client = ::accept(socket.getSocket(), reinterpret_cast<sockaddr*>(addr), &addr_size);//reinterpret_cast<sockaddr*>(&s_addr), &s_len);
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
    client_sock->data.address = { addr, addr_size };

    this->template execute<actions::CONNECT>(static_cast<tcp::connection<_Data...>&>(*client_sock));
    return (false);
}



template<typename ..._Actions, typename ..._Data>
inline bool tcp::server<std::tuple<_Actions...>, _Data...>::on_client_receive(connection<_Data...>& socket)
{
    bool disconnected = container_type::on_client_receive(socket);
    if (disconnected)
    {
        this->template execute<actions::DISCONNECT>(socket);
        this->delete_socket(socket.getSocket());
    }
    return (disconnected);
}





template<typename ..._Actions, typename ..._Data>
inline bool    tcp::server<std::tuple<_Actions...>, _Data...>::on_receive(unisock::_lib::socket_wrap* sptr)
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


UNISOCK_TCP_NAMESPACE_END

UNISOCK_NAMESPACE_END