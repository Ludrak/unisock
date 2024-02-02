#pragma once

#include "net/socket.hpp"
#include "net/inet_address.hpp"

#include "events/events.hpp"
#include "events/action_hanlder.hpp"

#include <iostream>
#include <queue>
#include <fcntl.h>

UNISOCK_NAMESPACE_START

UNISOCK_RAW_NAMESPACE_START

/* send / receive methods */
enum method
{
    // receive
    recv,
    recvmsg,
    recvfrom,

    // send
    send,
    sendmsg,
    sendto,
};


// actions to be hooked to tcp server/client with .on(lambda())
namespace actions
{
    /* RECEIVE HANDLERS (wrapping events received with recv, recvmsg, recvfrom)    */
    /* Note: you can change the current way of receiving/sending                   */
    /* data with the recv_method and send_method fields of listener                */

    // socket received bytes (with recv)
    // prototype: void  (socket* socket, const char* message, const size_t size);
    struct RECEIVED {};

    // socket received message (with recvmsg)
    // prototype: void  (socket* socket, const struct msghdr& message);
    struct MESSAGE {};

    // socket received packet (with recvfrom)
    // prototype: void  (socket* socket, inet_address& address, const char* message, const size_t size);
    struct PACKET {};


    // called when a socket is closed
    // prototype: void  (socket* socket);
    struct CLOSED {};


    // an error has occurred 
    // prototype: void  (const std::string& function, int errno);
    struct ERROR {};
};


UNISOCK_LIB_NAMESPACE_START


/* predefinition of raw::_lib::socket_conainer<...> for socket_data */
template<typename ..._Data>
class listener_impl;


/* socket data for each tcp socket */
template<typename ..._Data>
class socket_data 
{
    public:
        inet_address    address;


        // TODO: find a way to make this field private
        //       + change placeholder std::string type to a lighter byte array container
        std::queue<std::string> send_buffer;
};

/* listener actions for both client and server */
template<typename _Socket, typename ..._Actions>
using listener_actions = std::tuple<
    // close handler:       void (const std::string& function, int errno)
    unisock::events::_lib::action<actions::ERROR,  
                                std::function<void (const std::string&, int)> >,

    // close handler:       void (socket* socket)
    unisock::events::_lib::action<actions::CLOSED,  
                                std::function<void (_Socket*)> >,

    // message handler:     void (socket* socket, const char* message, size_t size)
    unisock::events::_lib::action<actions::RECEIVED,  
                                std::function<void (_Socket*, const char *, size_t)> >,

    // message handler:     void (socket* socket, const inet_address& address, const char* message, size_t size)
    unisock::events::_lib::action<actions::PACKET,  
                                std::function<void (_Socket*, const inet_address&, const char*, size_t)> >,

    // message handler:     void (socket* socket, const msghdr& message, size_t size)
    unisock::events::_lib::action<actions::MESSAGE,  
                                std::function<void (_Socket*, const msghdr&)> >,

    _Actions...
>;

UNISOCK_LIB_NAMESPACE_END



/* ======================================================================== */
/* TYPES ALIASES                                                            */


/* socket type for listener */
template<typename ..._Data>
using socket = typename unisock::_lib::socket<_Data..., raw::_lib::socket_data<_Data...>>;


/* definition of standart listener with no additionnal data */
using listener = unisock::raw::_lib::listener_impl<std::tuple<>>;


/* type alias to get a listener with some additionnal data  */
template<typename ..._SocketData>
using listener_of = unisock::raw::_lib::listener_impl<std::tuple<>, _SocketData...>;

enum send_result
{
    /* message successfully sent */
    SUCCESS,

    /* sento returned an error, still available in errno */
    ERROR,

    /* socket was not available for sending */
    UNAVAILABLE,

    /* message sent by sendto is incomplete, compare this with result >= INCOMPLETE */
    /* the number of bytes sent is indicated by result - INCOMPLETE                 */
    INCOMPLETE
};


size_t  send_to(const unisock::_lib::socket_wrap* sock, const inet_address& address, const std::string& message)
{
    // TODO: maybe add set/get_flag in socket to wrap fcntl ?
    ::fcntl(sock->get_socket(), F_SETFL, O_NONBLOCK);

    ssize_t n_bytes = ::sendto(sock->get_socket(),
                               message.c_str(),
                               message.size(),
                               0,
                               address.template to<sockaddr>(),
                               address.size());
    if (n_bytes == EAGAIN)
        return (send_result::UNAVAILABLE);
    else if (n_bytes < 0)
        return (send_result::ERROR);
    else if (static_cast<size_t>(n_bytes) < message.size())
        // retrieve this by comparing result >= INCOMPLETE and n_bytes = result - INCOMPLETE
        return (send_result::INCOMPLETE + n_bytes);
    
    return (send_result::SUCCESS);
}


size_t  send_to(const inet_address& address, const std::string& message)
{
    auto sock = raw::socket<>(nullptr, AF_INET, SOCK_DGRAM, 0);
    // there could be an error if the newly created socket is invalid
    if (sock.get_socket() < 0)
        return (send_result::ERROR);
    
    size_t result = send_to(&sock, address, message);
    sock.close();
    return (result);
}



UNISOCK_LIB_NAMESPACE_START

/* ======================================================================== */
/* LISTENER IMPLEMENTATION                                                  */




/* inherited socket container which regroup listener server/client tcp operations,
   defines a listener base for server/client                                        */
template<typename ..._Actions, typename ..._Data>
class listener_impl<std::tuple<_Actions...>, _Data...>
                       :    public unisock::events::_lib::socket_container<
                                _Data...,
                                _lib::socket_data<_Data...>
                            >,
                            public unisock::events::_lib::action_handler<
                                listener_actions<
                                    raw::socket<_Data...>,
                                    _Actions...
                                >
                            >
{
    static constexpr size_t RECV_BLOCK_SIZE = 1024;

    using container_type = typename unisock::events::_lib::socket_container<_Data..., _lib::socket_data<_Data...>>;

    public:
        static constexpr size_t RECV_BUFFER_SIZE = 1024;

        using socket = socket<_Data...>;

        method recv_method = method::recv;
        method send_method = method::send;

    public:
        listener_impl()
        : container_type()
        {
        }

        listener_impl(unisock::events::handler& handler)
        : container_type(handler)
        {
        }

        virtual bool    configure_socket(int domain, int type, int protocol, const std::function<bool (socket&)>& configure);

        virtual void    send(socket* socket, const void *buffer, const size_t size);
        virtual void    send(socket* socket, const struct msghdr& header);
        virtual void    send(socket* socket, const std::string& message_str);

        // should be used instead of the ones above for udp style communication
        virtual send_result send_to(socket* sock, const inet_address& address, const std::string& message_str);


    protected:

        /* general definition for method */
        template<method _Method>
        bool            _recv(socket* sock) { (void)sock; }

        /* definition for recv */
        template<>
        bool            _recv<method::recv>(socket* sock)
        {
            assert(sock != nullptr);
            assert(sock->get_socket() > 0);

            char buffer[RECV_BUFFER_SIZE] { 0 };

            int n_bytes = ::recv(sock->get_socket(), buffer, RECV_BUFFER_SIZE, MSG_DONTWAIT);
            if (n_bytes < 0)
            {
                this->template execute<actions::ERROR>("recv", errno);
                return (false);
            }

            this->template execute<actions::RECEIVED>(sock, buffer, n_bytes);
            return (true);
        }

        /* definition for recvmsg */
        template<>
        bool            _recv<method::recvmsg>(socket* sock)
        {
            assert(sock != nullptr);
            assert(sock->get_socket() > 0);

            struct msghdr   header;
            struct iovec    iov[1];
            char            buffer[RECV_BUFFER_SIZE] { 0 };

            std::memset(&header, 0, sizeof(header));
            std::memset(iov, 0, sizeof(iov));

            iov[0].iov_base = buffer;
            iov[0].iov_len  = sizeof(buffer);
            header.msg_iov     = iov;
            header.msg_iovlen  = 1;

            int n_bytes = ::recvmsg(sock->get_socket(), &header, MSG_DONTWAIT);
            if (n_bytes < 0)
            {
                this->template execute<actions::ERROR>("recvmsg", errno);
                return (false);
            }

            this->template execute<actions::MESSAGE>(sock, header);
            return (true);
        }

        template<>
        bool            _recv<method::recvfrom>(socket* sock)
        {
            assert(sock != nullptr);
            assert(sock->get_socket() > 0);

            char        buffer[RECV_BUFFER_SIZE] { 0 };

            // TODO: change this when refractoring inet_address
            struct sockaddr_storage addr;
            socklen_t               addr_len = sizeof(addr);
            memset(&addr, 0, addr_len);

            int n_bytes = ::recvfrom(sock->get_socket(),
                                     buffer,
                                     RECV_BUFFER_SIZE, 
                                     MSG_DONTWAIT,
                                     reinterpret_cast<sockaddr*>(&addr),
                                     &addr_len);
            if (n_bytes < 0)
            {
                this->template execute<actions::ERROR>("recv", errno);
                return (false);
            }

            inet_address address { addr };
            this->template execute<actions::PACKET>(sock, address, buffer, n_bytes);
            return (true);
        }



        /* general definition for method */
        template<method _Method>
        bool            _send(socket* sock) { (void)sock; }

        /* definition for send */
        template<>
        bool            _send<method::send>(socket* sock)
        {
            assert(sock != nullptr);
            assert(sock->get_socket() > 0);

            ssize_t n_bytes = ::send(sock->get_socket(), sock->data.send_buffer.front().c_str(), sock->data.send_buffer.front().size(), MSG_DONTWAIT);
            if (n_bytes < 0)
            {
                this->template execute<actions::ERROR>("send", errno);
                return (false);
            }
            if (static_cast<size_t>(n_bytes) < sock->data.send_buffer.front().size())
            {
                sock->data.send_buffer.front().substr(n_bytes);
                return (true);
            }
            sock->data.send_buffer.pop();
            this->handler.socket_want_write(sock->get_socket(), false);
            return (true);
        }

        /* definition for sendmsg */
        template<>
        bool            _send<method::sendmsg>(socket* sock)
        {
            assert(sock != nullptr);
            assert(sock->get_socket() > 0);

            ssize_t n_bytes = ::sendmsg(sock->get_socket(),
                            reinterpret_cast<const struct msghdr*>(sock->data.send_buffer.front().c_str()), MSG_DONTWAIT);
            if (n_bytes < 0)
            {
                this->template execute<actions::ERROR>("sendmsg", errno);
                return (false);
            }
            if (static_cast<size_t>(n_bytes) < sock->data.send_buffer.front().size())
            {
                sock->data.send_buffer.front().substr(n_bytes);
                return (true);
            }
            sock->data.send_buffer.pop();
            this->handler.socket_want_write(sock->get_socket(), false);
            return (true);
        }

        /* definition for sendto */
        template<>
        bool            _send<method::sendto>(socket* sock)
        {
            assert(sock != nullptr);
            assert(sock->get_socket() > 0);

            ssize_t n_bytes = ::sendto(sock->get_socket(),
                                       sock->data.send_buffer.front().c_str(),
                                       sock->data.send_buffer.front().size(),
                                       0,
                                       sock->data.address.template to<sockaddr>(),
                                       sock->data.address.size());
            if (n_bytes < 0)
            {
                this->template execute<actions::ERROR>("sendmsg", errno);
                return (false);
            }
            if (static_cast<size_t>(n_bytes) < sock->data.send_buffer.front().size())
            {
                sock->data.send_buffer.front().substr(n_bytes);
                return (true);
            }
            return (true);
        }

        virtual bool    on_receive(unisock::_lib::socket_wrap* sptr) override;
        virtual bool    on_writeable(unisock::_lib::socket_wrap* sptr) override;
};




/* ======================================================================== */
/* IMPLEMENTATION                                                           */




template<typename ..._Actions, typename ..._Data>
inline bool raw::_lib::listener_impl<std::tuple<_Actions...>, _Data...>::
            configure_socket(int domain, int type, int protocol, const std::function<bool (socket&)>& configure)
{
    auto* sock = this->make_socket(domain, type, protocol);
    if (sock == nullptr)
    {
        this->template execute<actions::ERROR>("socket", errno);
        return (false);
    }

    return (configure(*sock));
}






template<typename ..._Actions, typename ..._Data>
inline void raw::_lib::listener_impl<std::tuple<_Actions...>, _Data...>::
            send(socket* sock, const std::string& message_str)
{
    assert(sock != nullptr);
    assert(sock->get_socket() > 0);

    // queue data to be send
    sock->data.send_buffer.push(message_str);

    // // try single poll for writing on sock socket with timeout = 0
    // bool available = events::single_poll(sock, unisock::events::_lib::WANT_WRITE, 0);
    
    // // if socket is not available, set write flag on socket
    // if (!available)
    // {
    //     this->handler.socket_want_write(sock->get_socket(), true);
    //     return ;
    // }
    int flags = ::fcntl(sock->get_socket(), F_GETFL, 0);
    ::fcntl(sock->get_socket(), F_SETFL, flags | O_NONBLOCK);

    // try send message
    int send_result;
    switch (send_method)
    {
    case method::send:
        send_result = this->_send<method::send>(sock);
        break;
    
    case method::sendmsg:
        send_result = this->_send<method::sendmsg>(sock);
        break;
    
    default:
        send_result = send_result::ERROR;
    }
    if (send_result > send_result::INCOMPLETE || send_result == send_result::UNAVAILABLE)    
        // set socket flag for writing message end next poll
        this->handler.socket_want_write(sock->get_socket(), true);
}



template<typename ..._Actions, typename ..._Data>
inline void raw::_lib::listener_impl<std::tuple<_Actions...>, _Data...>::
            send(socket* sock, const void *message, const size_t size)
{
    this->send(sock, std::string(reinterpret_cast<const char*>(message), size));
}


template<typename ..._Actions, typename ..._Data>
inline void raw::_lib::listener_impl<std::tuple<_Actions...>, _Data...>::
            send(socket* sock, const struct msghdr& header)
{
    this->send(sock, std::string(reinterpret_cast<const char*>(&header), sizeof(header)));
}



template<typename ..._Actions, typename ..._Data>
inline send_result raw::_lib::listener_impl<std::tuple<_Actions...>, _Data...>::
            send_to(socket* sock, const inet_address& address, const std::string& message_str)
{
    int send_result = raw::send_to(sock, address, message_str);
    
    if (send_result == send_result::ERROR)
    {
        this->template execute<actions::ERROR>("send_to", errno);
        return send_result::ERROR;
    }
    if (send_result > send_result::INCOMPLETE || send_result == send_result::UNAVAILABLE)  
    {  
        // push message and set socket flag for writing message end next poll
        if (send_result == send_result::UNAVAILABLE)
            sock->data.send_buffer.push(message_str);
        else
            sock->data.send_buffer.push(message_str.substr(send_result - send_result::INCOMPLETE));
        this->handler.socket_want_write(sock->get_socket(), true);

        // keep socket alive to be polled, will be deleted when message is sent
        return (send_result::SUCCESS);
    }
    return (send_result::SUCCESS);
}









template<typename ..._Actions, typename ..._Data>
inline bool raw::_lib::listener_impl<std::tuple<_Actions...>, _Data...>::
            on_receive(unisock::_lib::socket_wrap* sptr)
{
    auto* sock = reinterpret_cast<socket*>(sptr);

    switch (recv_method)
    {
    case method::recv:
        return this->_recv<method::recv>(sock);
    
    case method::recvmsg:
        return this->_recv<method::recvmsg>(sock);
    
    case method::recvfrom:
        return this->_recv<method::recvfrom>(sock);
    
    default:
        return false;
    }
}

template<typename ..._Actions, typename ..._Data>
inline bool raw::_lib::listener_impl<std::tuple<_Actions...>, _Data...>::
            on_writeable(unisock::_lib::socket_wrap* sptr)
{
    auto* sock = reinterpret_cast<socket*>(sptr);

    switch (send_method)
    {
    case method::send:
        return this->_send<method::send>(sock);
    
    case method::sendmsg:
        return this->_send<method::sendmsg>(sock);

    case method::sendto:
        return this->_send<method::sendto>(sock);
    
    default:
        return false;
    }
}


UNISOCK_LIB_NAMESPACE_END

UNISOCK_RAW_NAMESPACE_END

UNISOCK_NAMESPACE_END