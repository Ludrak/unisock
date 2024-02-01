#pragma once

#include "net/socket.hpp"
#include "net/inet_address.hpp"

#include "events/events.hpp"
#include "events/action_hanlder.hpp"

#include <iostream>
#include <queue>

UNISOCK_NAMESPACE_START

UNISOCK_TCP_NAMESPACE_START


UNISOCK_LIB_NAMESPACE_START

enum  connection_type
{
    // listener connection
    SERVER,

    // connection accepted by listener
    CLIENT
};

UNISOCK_LIB_NAMESPACE_END


// actions to be hooked to tcp server/client with .on(lambda())
namespace actions
{
    //  server: N/A
    //          
    //  client: 
    //      client connected to an endpoint
    //      prototype: void  (tcp::connection<data>& server_connection);
    struct CONNECTED {};


    //  server: 
    //      server started listening
    //      prototype: void  (tcp::connection<data>& endpoint_connection);
    //
    //  client: N/A
    struct LISTENING {};


    //  server: 
    //      server endpoint closed
    //      prototype: void  (tcp::connection<data>& endpoint_connection);
    //
    //  client:
    //      client endpoint connection closed
    //      prototype: void  (tcp::connection<data>& server_connection);
    struct CLOSED {};

    //  server: 
    //      a client connected on an endpoint of the server
    //      prototype: void  (tcp::connection<data>& endpoint, tcp::connection<data>& client);
    //
    //  client: N/A
    struct CONNECT {};

    //  server: 
    //      a client disconnected from the server
    //      prototype: void  (tcp::connection<data>& client);
    //
    //  client: N/A
    struct DISCONNECT {};

    //  server: 
    //      server received data from a client
    //      prototype: void  (tcp::connection<data>& client, const char* message, const size_t size);
    //
    //  client:
    //      client received data from one of its connections
    //      prototype: void  (tcp::connection<data>& connection, const char* message, const size_t size);
    struct MESSAGE {};

    // an error has occurred 
    // prototype: void  (const std::string& function, int errno);
    struct ERROR {};
};

/* predefinition of tcp::server<...> for socket_data */
template<typename ..._Args>
class server_impl;

/* predefinition of tcp::client<...> for socket_data */
template<typename ..._Args>
class client_impl;


UNISOCK_LIB_NAMESPACE_START

/* predefinition of tcp::_lib::socket_conainer<...> for socket_data */
template<typename ..._Data>
class common_impl;

/* socket data for each tcp socket */
template<typename ..._Data>
class socket_data 
{
    public:
        inet_address    address;

    // private:
        _lib::connection_type   type;
        std::queue<std::string> send_buffer;

        // friend class socket_container<_Data...>;
        // friend class server_impl<_Data...>;
        // friend class client_impl<_Data...>;
};

UNISOCK_LIB_NAMESPACE_END


/* connection type for tcp server/client with data */
template<typename ..._Data>
using connection = typename unisock::_lib::socket<_Data..., _lib::socket_data<_Data...>>;


UNISOCK_LIB_NAMESPACE_START

/* common actions for both client and server */
template<typename _Connection, typename ..._Actions>
using common_actions = std::tuple<
    // close handler:       void ()
    unisock::events::_lib::action<actions::ERROR,  
                                std::function<void (const std::string&, int)> >,

    // close handler:       void (tcp::connection<...>& connection)
    unisock::events::_lib::action<actions::CLOSED,  
                                std::function<void (_Connection&)> >,

    // message handler:     void (tcp::connection<...>& client, const char* message, size_t size)
    unisock::events::_lib::action<actions::MESSAGE,  
                                std::function<void (_Connection&, const char *, size_t)> >,

    _Actions...
>;


/* inherited socket container which regroup common server/client tcp operations,
   defines a common base for server/client                                        */
template<typename ..._Actions, typename ..._Data>
class common_impl<std::tuple<_Actions...>, _Data...>
                       :    public unisock::events::_lib::socket_container<
                                _Data...,
                                _lib::socket_data<_Data...>
                            >,
                            public unisock::events::_lib::action_handler<
                                common_actions<
                                    tcp::connection<_Data...>,
                                    _Actions...
                                >
                            >
{
    static constexpr size_t RECV_BLOCK_SIZE = 1024;

    using container_type = typename unisock::events::_lib::socket_container<_Data..., _lib::socket_data<_Data...>>;

    public:
        using connection = typename tcp::connection<_Data...>;

    public:
        common_impl()
        : container_type()
        {
        }

        common_impl(unisock::events::handler& handler)
        : container_type(handler)
        {
        }

        virtual void    send(connection& connection, const std::string& message_str);
        virtual void    send(connection& connection, const char *message, const size_t size);


    protected:
        // received on an accepted or connected socket
        // MUST be inherited in child class and call delete_socket on the socket if it returns true
        // this is because disconnection is handeled by different actions on server and client (i.e. actions::DISCONNECT, actions::CLOSED respectively)
        virtual bool    on_client_receive(connection& socket);


        virtual bool    on_writeable(unisock::_lib::socket_wrap* sptr) override;
};




/* ======================================================================== */
/* IMPLEMENTATION                                                           */


template<typename ..._Actions, typename ..._Data>
inline void tcp::_lib::common_impl<std::tuple<_Actions...>, _Data...>::send(connection& connection, const std::string& message_str)
{
    // try single poll for writing on connection socket with timeout = 0
    bool available = events::single_poll(connection, unisock::events::_lib::WANT_WRITE, 0);
    
    // if connection has some queued data to be sent, queue message
    if (!available || !connection.data.send_buffer.empty())
    {
        // queue data to be send
        connection.data.send_buffer.push(message_str);
        this->handler.socket_want_write(connection.get_socket(), true);
        return ;
    }

    // socket is available and has an empty send_buffer, try direct send and queue message tail if any
    size_t n_bytes = ::send(connection.get_socket(), message_str.c_str(), message_str.size(), 0);
    if (n_bytes < 0)
    {
        // ::send error, queue data to be sent later
        this->template execute<actions::ERROR>("send", errno);
        connection.data.send_buffer.push(message_str);
        this->handler.socket_want_write(connection.get_socket(), true);
        return ;
    }
    // if all data was not sent, push back tail to send_buffer
    if (n_bytes < message_str.size())
    {
        connection.data.send_buffer.push(message_str.substr(n_bytes));
        this->handler.socket_want_write(connection.get_socket(), true);
    }
}




template<typename ..._Actions, typename ..._Data>
inline void tcp::_lib::common_impl<std::tuple<_Actions...>, _Data...>::send(connection& connection, const char *message, const size_t size)
{
    this->send(connection, std::string(message, size));
}




template<typename ..._Actions, typename ..._Data>
inline bool tcp::_lib::common_impl<std::tuple<_Actions...>, _Data...>::on_client_receive(connection& socket)
{
    char buffer[RECV_BLOCK_SIZE] {0};
    ssize_t n_bytes = ::recv(socket.get_socket(), buffer, RECV_BLOCK_SIZE, MSG_DONTWAIT);
    if (n_bytes < 0)
    {
        // recv error
        this->template execute<actions::ERROR>("recv", errno);
        return false; 
    }
    else if (n_bytes == 0)
    {
        // client sent 0 (disconnected)
        // socket MUST be deleted right after by children
        return true;
    }
    // received n_bytes in buffer
    this->template execute<actions::MESSAGE>(socket, buffer, n_bytes);
    return (false);
}




template<typename ..._Actions, typename ..._Data>
inline bool tcp::_lib::common_impl<std::tuple<_Actions...>, _Data...>::on_writeable(unisock::_lib::socket_wrap* sptr)
{
    auto* socket = reinterpret_cast<connection*>(sptr);

    /* should not happen but if data is empty, clear socket write flag */
    if (socket->data.send_buffer.empty())
    {
        this->handler.socket_want_write(socket->get_socket(), false);
        return false;
    }

    // socket is available and has an empty send_buffer, try direct send and queue message tail if any
    size_t n_bytes = ::send(socket->get_socket(), socket->data.send_buffer.front().c_str(), socket->data.send_buffer.front().size(), 0);
    if (n_bytes < 0)
    {
        // ::send error
        this->template execute<actions::ERROR>("send", errno);
        return false;
    }
    // if all data was not sent, push back tail to send_buffer, keep write flag
    if (n_bytes < socket->data.send_buffer.front().size())
    {
        socket->data.send_buffer.front().substr(n_bytes);
        return false;
    }
    // data was sent entierly, pop first data in buffer and untoggle write flag
    socket->data.send_buffer.pop();
    if (socket->data.send_buffer.empty())
        this->handler.socket_want_write(socket->get_socket(), false);

    return (false);
}



UNISOCK_LIB_NAMESPACE_END

UNISOCK_TCP_NAMESPACE_END

UNISOCK_NAMESPACE_END