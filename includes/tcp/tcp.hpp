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
    // server started listening
    // prototype: void  (void);
    struct LISTEN {};

    // server closed
    // prototype: void  (void);
    struct CLOSE {};

    // a client connected on some endpoint of the server
    // prototype: void  (tcp::connection<data>& client);
    struct CONNECT {};

    // a client disconnected from the server
    // prototype: void  (tcp::connection<data>& client);
    struct DISCONNECT {};

    // a client sent a tcp message to the server
    // prototype: void  (tcp::connection<data>& client, const char* message, const size_t size);
    struct MESSAGE {};

    // an error has occurred 
    // prototype: void  (int errno);
    struct ERROR {};
};

/* predefinition of tcp::server<...> for socket_data */
template<typename ..._Data>
class server;

UNISOCK_LIB_NAMESPACE_START

/* socket data for each tcp socket */
template<typename ..._Data>
class socket_data 
{
    public:
        inet_address    address;

    private:
        _lib::connection_type   type;
        std::queue<std::string> send_buffer;

        friend class server<_Data...>;
};

UNISOCK_LIB_NAMESPACE_END

/* connection type for tcp server with data */
template<typename ..._Data>
using connection = typename unisock::_lib::socket<_Data..., _lib::socket_data<_Data...>>;


template<typename ..._Data>
class server :  public unisock::events::_lib::socket_container<_Data..., _lib::socket_data<_Data...>>,
                public unisock::events::_lib::action_handler
                <
                    unisock::events::_lib::action<actions::LISTEN,  
                                                  std::function<void ()> >,

                    unisock::events::_lib::action<actions::CLOSE,  
                                                  std::function<void ()> >,

                    unisock::events::_lib::action<actions::CONNECT,
                                                  std::function<void (typename tcp::connection<_Data...>& )> >,

                    unisock::events::_lib::action<actions::DISCONNECT,
                                                  std::function<void (typename tcp::connection<_Data...>& )> >,

                    unisock::events::_lib::action<actions::MESSAGE,  
                                                  std::function<void (typename tcp::connection<_Data...>&, const char *, size_t)> >
                >
{
        using container_type = typename unisock::events::_lib::socket_container<_Data..., _lib::socket_data<_Data...>>;

    public:
        using connection = typename tcp::connection<_Data...>;

        server()
        : container_type()
        {
        }

        server(unisock::events::handler& handler)
        : container_type(handler)
        {
        }

        virtual void listen(const std::string& ip_address, const int port, const sa_family_t family = AF_INET);

        virtual void send(connection& connection, const std::string& message_str);
        virtual void send(connection& connection, const char *message, const size_t size);


    private:
        // called when some socket needs to read received data
        // (i.e. on received)
        virtual void    on_receive(unisock::_lib::socket_wrap* sptr) override;

        // called when a socket that was requesting write got writeable
        // (i.e. on queued send)
        virtual void    on_writeable(unisock::_lib::socket_wrap* sptr) override;
    
};







template<typename ..._Data>
inline void tcp::server<_Data...>::listen(const std::string& ip_address, const int port, const sa_family_t family)
{
    server::connection* sock = this->make_socket(family, SOCK_STREAM, 0);
    if (sock == nullptr)
        return ;
    sock->data.type = _lib::connection_type::SERVER;
    sock->data.address = { ip_address, port, family };
    ::bind(sock->getSocket(), sock->data.address.getAddress(), sock->data.address.getAddressSize());
    ::listen(sock->getSocket(), 10);

    this->template execute<actions::LISTEN>();
}



template<typename ..._Data>
inline void tcp::server<_Data...>::send(connection& connection, const std::string& message_str)
{
    // try single poll for writing on connection socket with timeout = 0
    bool available = events::single_poll(connection, unisock::events::_lib::WANT_WRITE, 0);
    
    // if connection has some queued data to be sent, queue message
    if (!available || !connection.data.send_buffer.empty())
    {
        // queue data to be send
        connection.data.send_buffer.push(message_str);
        this->handler.socket_want_write(connection.getSocket(), true);
        return ;
    }

    // socket is available and has an empty send_buffer, try direct send and queue message tail if any
    size_t n_bytes = ::send(connection.getSocket(), message_str.c_str(), message_str.size(), 0);
    if (n_bytes < 0)
    {
        // ::send error, queue data to be sent later
        connection.data.send_buffer.push(message_str);
        this->handler.socket_want_write(connection.getSocket(), true);
        return ;
    }
    // if all data was not sent, push back tail to send_buffer
    if (n_bytes < message_str.size())
    {
        connection.data.send_buffer.push(message_str.substr(n_bytes));
        this->handler.socket_want_write(connection.getSocket(), true);
    }
}



template<typename ..._Data>
inline void tcp::server<_Data...>::send(connection& connection, const char *message, const size_t size)
{
    this->send(connection, std::string(message, size));
}



// called when some socket needs to read received data
// (i.e. on received)
template<typename ..._Data>
inline void    tcp::server<_Data...>::on_receive(unisock::_lib::socket_wrap* sptr)
{
    auto* socket = reinterpret_cast<connection*>(sptr);
    if (socket->data.type == _lib::connection_type::SERVER)
    {
        struct sockaddr_in  s_addr {};
        socklen_t           s_len = sizeof(s_addr);
        int client = ::accept(socket->getSocket(), reinterpret_cast<sockaddr*>(&s_addr), &s_len);
        
        server::connection* client_sock = this->make_socket(client);
        if (client_sock == nullptr)
            return ;
        client_sock->data.type = _lib::connection_type::CLIENT;
        client_sock->data.address.setAddress(s_addr);

        this->template execute<actions::CONNECT>(static_cast<connection&>(*client_sock));
    }
    else /* if socket.data.type == connection_type::CLIENT */
    {
        char buffer[1024] {0};
        size_t n_bytes = ::recv(socket->getSocket(), buffer, 1024, MSG_DONTWAIT);
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
template<typename ..._Data>
inline void    tcp::server<_Data...>::on_writeable(unisock::_lib::socket_wrap* sptr)
{
    auto* socket = reinterpret_cast<connection*>(sptr);

    /* should not happen but if data is empty, clear socket write flag */
    if (socket->data.send_buffer.empty())
    {
        this->handler.socket_want_write(socket->getSocket(), false);
        return ;
    }
    
    // socket is available and has an empty send_buffer, try direct send and queue message tail if any
    size_t n_bytes = ::send(socket->getSocket(), socket->data.send_buffer.front().c_str(), socket->data.send_buffer.front().size(), 0);
    if (n_bytes < 0)
    {
        // ::send error
        return ;
    }
    // if all data was not sent, push back tail to send_buffer, keep write flag
    if (n_bytes < socket->data.send_buffer.front().size())
    {
        socket->data.send_buffer.front().substr(n_bytes);
        return ;
    }
    // data was sent entierly, pop first data in buffer and untoggle write flag
    socket->data.send_buffer.pop();
    if (socket->data.send_buffer.empty())
        this->handler.socket_want_write(socket->getSocket(), false);
}


UNISOCK_TCP_NAMESPACE_END

UNISOCK_NAMESPACE_END