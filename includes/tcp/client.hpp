#pragma once

#include "tcp/tcp.hpp"

UNISOCK_NAMESPACE_START

UNISOCK_TCP_NAMESPACE_START


UNISOCK_LIB_NAMESPACE_START

/* actions types for tcp::client */
template<typename _Connection, typename ..._Actions>
using client_actions = std::tuple<

    unisock::events::_lib::action<actions::CONNECTED,
                                std::function<void (_Connection& )> >,

    _Actions...
>;

UNISOCK_LIB_NAMESPACE_END


/* definition of client with all args */
template<typename ..._Args>
class client : public client<std::tuple<>, _Args...>
{
    public:
        client() = default;
        // handler constructor
        client(unisock::events::handler& handler)
        : client<std::tuple<>, _Args...>(handler)
        {}
};


/* standart specialization for _Actions and _Data */
template<typename ..._Actions, typename ..._Data>
class client<std::tuple<_Actions...>, _Data...> :  
                public unisock::tcp::_lib::socket_container<
                    tcp::_lib::client_actions<
                        tcp::connection<_Data...>,
                        _Actions...
                    >, 
                    _Data...
                >
{
        using container_type = unisock::tcp::_lib::socket_container<_lib::client_actions<connection<_Data...>, _Actions...>, _Data...>;

        static constexpr size_t RECV_BLOCK_SIZE = 1024;

    public:
        client() = default;

        client(unisock::events::handler& handler)
        : container_type(handler)
        {
        }

        /* makes the client try connecting to the provided address, returns false on error */
        /* errno of the error can be retrieved in actions::ERROR    */
        virtual bool    connect(const std::string& ip_address, const int port, const sa_family_t family = AF_INET);

    private:
        bool            on_client_receive(connection<_Data...>& socket) override;

        // called when some socket needs to read received data
        // (i.e. on received)
        // returns true if iterators of socket_container::sockets are changed
        virtual bool    on_receive(unisock::_lib::socket_wrap* sptr) override;
};






/* ======================================================================== */
/* IMPLEMENTATION                                                           */



template<typename ..._Actions, typename ..._Data>
inline bool tcp::client<std::tuple<_Actions...>, _Data...>::connect(const std::string& ip_address, const int port, const sa_family_t family)
{
    auto* sock = this->make_socket(family, SOCK_STREAM, 0);
    if (sock == nullptr)
    {
        this->template execute<actions::ERROR>("socket", errno);
        return false;
    }
    sock->data.type = _lib::connection_type::CLIENT;
    sock->data.address = { ip_address, port, family };

    if (-1 == ::connect(sock->getSocket(), sock->data.address.template to_address<sockaddr>(), sock->data.address.size()))
    {
        this->template execute<actions::ERROR>("connect", errno);
        return (false);
    }

    this->template execute<actions::CONNECTED>(*sock);

    return (true);
}




template<typename ..._Actions, typename ..._Data>
inline bool tcp::client<std::tuple<_Actions...>, _Data...>::on_client_receive(connection<_Data...>& socket)
{
    bool disconnected = container_type::on_client_receive(socket);
    if (disconnected)
    {
        this->template execute<actions::CLOSED>(socket);
        this->delete_socket(socket.getSocket());
    }
    return (disconnected);
}





// called when some socket needs to read received data
// (i.e. on received)
template<typename ..._Actions, typename ..._Data>
inline bool    tcp::client<std::tuple<_Actions...>, _Data...>::on_receive(unisock::_lib::socket_wrap* sptr)
{
    assert(sptr != nullptr);

    auto* socket = reinterpret_cast<connection<_Data...>*>(sptr);
    this->on_client_receive(*socket);

    // TODO: CHECK ITER INVALIDATION
    return (false);
}



UNISOCK_TCP_NAMESPACE_END

UNISOCK_NAMESPACE_END