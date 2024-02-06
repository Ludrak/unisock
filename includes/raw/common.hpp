/**
 * @file common.hpp
 * @author ROBINO Luca
 * 
 * @brief   implements listener base for handeling sockets at any level
 * 
 * @details this listener class is a base for any higher protocol listener 
 *          that can inherit this class and specify ways for creating, receiving, and sending on sockets
 * 
 * @version 1.0
 * @date 2024-02-05
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#pragma once

#include "socket/socket.hpp"
#include "socket/socket_address.hpp"

#include "events/events.hpp"
#include "events/action_hanlder.hpp"

#include <iostream>
#include <queue>
#include <fcntl.h>

/**
 * @addindex
 */
namespace unisock {


/**
 * @addindex
 */
namespace raw {

/**
 * @brief base send and recv method enum to specify raw::listener::_recv<method> and raw::listener::_send<method>
 *
 */
enum method
{
    /**
     * @brief uses **recv()** call (see [recv man page](https://man7.org/linux/man-pages/man2/recv.2.html))
     */
    recv,

    /**
     * @brief uses **recvmsg()** call (see [recvmsg man page](https://man7.org/linux/man-pages/man2/recvmsg.2.html))
     */
    recvmsg,

    /**
     * @brief uses **recvfrom()** call (see [recvfrom man page](https://man7.org/linux/man-pages/man2/recvfrom.2.html))
     */
    recvfrom,


    /**
     * @brief uses **send()** call (see [send man page](https://man7.org/linux/man-pages/man2/send.2.html))
     */
    send,

    /**
     * @brief uses **sendmsg()** call (see [sendmsg man page](https://man7.org/linux/man-pages/man2/sendmsg.2.html))
     */
    sendmsg,

    /**
     * @brief uses **sendto()** call (see [sendto man page](https://man7.org/linux/man-pages/man2/sendto.2.html))
     */
    sendto,
};


// actions to be hooked to tcp server/client with .on(lambda())
/**
 * @brief   actions to be hooked on raw::listener or childrens of raw::listener
 * 
 * @details defines structs as tags for actions of action_handler,
 *          this tags can be used on the on() and execute() members
 *          of raw::listener and its childrens to enqueue actions or
 *          execute queues of actions respectively
 *          (the execute() member is a protected member of actions_handler
 *          so it is only available when inheriting raw::listener or its childrens)
 * 
 * @ref raw::listener
 * @ref raw::_lib::listener_impl
 * 
 * @ref events::_lib::action_handler
 * @ref events::_lib::action_handler::on
 * @ref events::_lib::action_handler::execute
 */
/**
 * @addindex
 */
namespace actions
{
    /**
     * @brief   socket received bytes with recv (see details for prototype)
     * 
     * @details this event will only be called if raw::listener instance has its **recv_method** field set to method::recv
     * 
     * @ref     raw::method::recv 
     * 
     * @note    hook prototype: ```void  (socket<>* socket, const char* message, const size_t size)```
     */
    struct RECEIVED {};

    /**
     * @brief socket received message with recvmsg (see details for prototype)
     * 
     * @details this event will only be called if raw::listener instance has its **recv_method** field set to method::recvmsg
     * 
     * @ref     raw::method::recvmsg 
     * 
     * @note    hook prototype: ```void  (socket* socket, const struct msghdr& message)```
     */
    struct MESSAGE {};

    /**
     * @brief socket received packet with recvfrom (see details for prototype)
     * 
     * @details this event will only be called if raw::listener instance has its **recv_method** field set to method::recvfrom
     * 
     * @ref     raw::method::recvfrom 
     * 
     * @note    hook prototype: ```void  (socket* socket, socket_address& address, const char* message, const size_t size)```
     */
    struct PACKET {};

    /**
     * @brief called when a socket is closed (see details for prototype)
     * 
     * @note  hook prototype: ```void  (socket* socket)```
     */
    struct CLOSED {};

    /**
     * @brief called on error (see details for prototype)
     * 
     * @note  hook prototype: ```void  (const std::string& function, int errno)```
     */
    struct ERROR {};
};

/**
 * @addindex
 */
namespace _lib {


/* predefinition of raw::_lib::socket_conainer<...> for socket_data */
/**
 * @brief listener impl
 * 
 * @tparam _Data 
 */
template<typename ..._Data>
class listener_impl;


/**
 * @brief raw::socket data type, base data type for unisock::raw
 * 
 * @tparam _Data 
 */
template<typename ..._Data>
class socket_data 
{
    public:
        /**
         * @brief address of the socket
         * @note  this may be moved soon directly to socket
         */
        socket_address    address;


        // TODO: find a way to make this field private
        //       + change placeholder std::string type to a lighter byte array container
        /**
         * @brief string queue containing message to send to client.
         * @note  needs to be hidden from user space
         */
        std::queue<std::string> send_buffer;
};

/* listener actions for both client and server */
/**
 * @brief   raw::listener list of actions to specify raw::listener::on and raw::listener::execute
 * @details used to define action_handler parent of raw::listener \n
 *          defines a std::tuple to be expanded (using events::_lib::expand()) into the template parameter pack of
 *          action_handler to specify its members on() and execute(), this insures that on() and execute()
 *          are always valid calls with valid arguments, otherwise a compilation error occurs.
 * 
 * @tparam _Socket 
 * @tparam _Actions 
 */
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

    // message handler:     void (socket* socket, const socket_address& address, const char* message, size_t size)
    unisock::events::_lib::action<actions::PACKET,  
                                std::function<void (_Socket*, const socket_address&, const char*, size_t)> >,

    // message handler:     void (socket* socket, const msghdr& message, size_t size)
    unisock::events::_lib::action<actions::MESSAGE,  
                                std::function<void (_Socket*, const msghdr&)> >,

    _Actions...
>;


} // ******** namespace _lib



/* ======================================================================== */
/* TYPES ALIASES                                                            */


/* socket type for listener */
/**
 * @brief   socket type of raw::listener
 * @details implements a basic socket with basic socket data (see unisock::raw::socket_data)
 * 
 * @tparam _Data additional data to add to the socket
 * 
 * @ref unisock::raw::_lib::socket_data
 */
template<typename ..._Data>
using socket = typename unisock::socket<_Data..., raw::_lib::socket_data<_Data...>>;


/**
 * @brief definition of standart raw::listener
 * 
 * @details implements a listener_impl with empty additionnal actions (empty std::tuple<>)
 * 
 * @ref unisock::raw::_lib::listener_impl
 */
using listener = unisock::raw::_lib::listener_impl<std::tuple<>>;


/* type alias to get a listener with some additionnal data  */
/**
 * @brief type alias to make a listener with some custom data infered to managed sockets
 * 
 * @tparam _SocketData  data types, sockets of this listener will publicly inherit those types.
 */
template<typename ..._SocketData>
using listener_of = unisock::raw::_lib::listener_impl<std::tuple<>, _SocketData...>;


/**
 * @brief result enum of a send operation
 *               
 */
enum send_result
{
    /**
     * @brief message successfully sent
     */
    SUCCESS,

    /**
     * @brief send/sento returned an error, still available in errno
     */
    ERROR,

    /**
     * @brief socket was not available for writing 
     */
    UNAVAILABLE,

    /**
     * @brief message sent by send/sendto is incomplete for any values above INCOMPLETE
     *       (compare result >= INCOMPLETE for check), the number of bytes sent can be retrieved with result - INCOMPLETE.
     * 
     */
    INCOMPLETE
};

/**
 * @brief   sends a packet containing **message** to the specified **address**, using the socket pointed by **sock**
 * @details this call tries to send a byte string indicated by **message** to the specified **address** using **sendto()**\n
 *          before sending the message, the pointed socket is switched to non blocking mode, to be able to **sendto()** without blocking or polling single socket before, 
 *          if socket was not available for writing, send_to will return send_result::UNAVAILABLE.\n\n
 * 
 *          If the sent message was not sent entierly (**sendto()** returned a size less than message.size()), the call returns send_result::INCOMPLETE + n_bytes_sent.\n 
 *          This way, an incomplete packet may be checked by comparing `result > send_result::INCOMPLETE` \n 
 *          and the number of bytes sent retrieved in `result - send_result::INCOMPLETE`.
 *          \n\n
 *          for more informations about socket and sendto, see [socket](https://man7.org/linux/man-pages/man7/socket.7.html) man page and [sendto](https://man7.org/linux/man-pages/man2/sendto.2.html) man page;
 * @param sock      socket to use for sending
 * @param address   address to send to
 * @param message   message to send
 * 
 * @return send_result or number of bytes sent if packet is incomplete (see details)
 * 
 * @ref send_result
 */
size_t  send_to(const unisock::socket_base* sock, const socket_address& address, const std::string& message)
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


/**
 * @brief sends a packet containing **message** to the specified **address**
 * 
 * @details this call tries to send a byte string indicated by **message** to the specified **address** using **sendto()** \n
 *          this call intializes a non blocking raw socket of type SOCK_DGRAM for communication, to be able to **sendto()** without blocking or polling single socket before, 
 *          if socket was not available for writing, send_to will return send_result::UNAVAILABLE.
 *          \n\n
 *          If the sent message was not sent entierly (**sendto()** returned a size less than message.size()), the call returns send_result::INCOMPLETE + n_bytes_sent.\n 
 *          This way, an incomplete packet may be checked by comparing `result > send_result::INCOMPLETE` \n 
 *          and the number of bytes sent retrieved in `result - send_result::INCOMPLETE`.
 *          \n\n
 *          for more informations about socket and sendto, see [socket](https://man7.org/linux/man-pages/man7/socket.7.html) man page and [sendto](https://man7.org/linux/man-pages/man2/sendto.2.html) man page;
 * @param address   address to send to
 * @param message   message to send
 * 
 * @return send_result or number of bytes sent if packet is incomplete (see details)
 */
size_t  send_to(const socket_address& address, const std::string& message)
{
    auto sock = raw::socket<>(nullptr, AF_INET, SOCK_DGRAM, 0);
    // there could be an error if the newly created socket is invalid
    if (sock.get_socket() < 0)
        return (send_result::ERROR);
    
    size_t result = send_to(&sock, address, message);
    sock.close();
    return (result);
}


/**
 * @addindex
 */
namespace _lib 
{

/* ======================================================================== */
/* LISTENER IMPLEMENTATION                                                  */




/* inherited socket container which regroup listener server/client tcp operations,
   defines a listener base for server/client                                        */

/**
 * @brief base implementation for the unisock::raw listener
 * 
 * @details this defines a base for handeling groups of sockets in a socket_container
 *          along with an action_handler to retrieve callback events.
 *          It defines basic ways for reading from and sending to sockets contained in this container,
 *          And a configure_socket method to create any type of sockets. \n 
 * 
 *          This class can be inherited and specified to listen and manage groups of sockets to a higher protocol level
 *          
 * @tparam _Actions 
 * @tparam _Data 
 */
template<typename ..._Actions, typename ..._Data>
class listener_impl<std::tuple<_Actions...>, _Data...>
                       :    public unisock::socket_container<
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
    /**
     * @brief Type of the socket_container parent of that class (for constructor)
     */
    using container_type = typename unisock::socket_container<_Data..., _lib::socket_data<_Data...>>;

    public:
        /**
         * @brief The size of the buffer that the recv recvmsg and recvfrom will use
         */
        static constexpr size_t RECV_BUFFER_SIZE = 1024;

        /**
         * @brief the complete type of the sockets that are managed by this listener
         *        this specializes a generic socket with the specific data of this server 
         *        so that template arguments for data are not required with this type definition.
         */
        using socket = socket<_Data...>;

        /**
         * @brief   enum to map which receive handler to use with this listener
         * 
         * @details this field can be changed to change the way of receiving on the server
         *          this will map the correct _recv<method> to call when on_received() is called
         *          \n
         *          these are the values that can be used for raw::listener::recv_method: \n
         *          - raw::method::recv         (calls action raw::actions::RECEIVED, default way of receiving, using only socket, and buffer to write to) \n
         *          - raw::method::recvmsg      (calls action raw::actions::MESSAGE,  uses struct msghdr when receiving, msghdr::iov[0] is passed as the message in the handler callback) \n
         *          - raw::method::recvfrom     (calls action raw::actions::PACKET,   receives packets in udp-style communications, address of the packet sender as well ass message is passed in the handler callback) \n
         * 
         * @ref     raw::method
         */
        method recv_method = method::recv;

         /**
         * @brief   enum to map which send handler to use with this listener
         * 
         * @details this field can be changed to default the way of sending on the server
         *          this will map the correct _recv<method> to call when on_received() is called
         *          \n
         *          these are the values that can be used for raw::listener::recv_method: \n
         *          - raw::method::send         (default way of sending, using only socket, and buffer to read from)
         *          - raw::method::sendmsg      (uses struct msghdr when sending, message argument is copied into msghdr::iov[0])
         *          - raw::method::sendto       (send packet in udp-style communications, sends a byte string message, requires the adddress of the receiver, and a socket to define sub protocols to deal with this packet)
         * 
         * @note    this field is used only by the raw::listener::send methods and default the way of sending leftover packets when on_writeable() is called
         * 
         * @ref     raw::method
         */
        method send_method = method::send;

    public:

        /**
         * @brief empty constructor, socket_container will be self-handeled
         * 
         * @ref   socket_container
         */
        listener_impl()
        : container_type()
        {
        }

        /**
         * @brief empty constructor, socket_container will be handeled by an external handler
         * 
         * @param handler handler to use for managing event on this listener
         * @ref   socket_container
         */
        listener_impl(unisock::events::handler& handler)
        : container_type(handler)
        {
        }

        /**
         * @brief configure a socket 
         * 
         * @details use this call to add a custom socket to the listener. 
         *          the socket will be created with the first 3 **domain**, **type** and **protocol** field,
         *          then if a valid success is returned, the **configure** handler is called
         *          for further configuration of the socket (socket_address, setsockopt, fcntl, ioctl, etc.)
         *          \n\n
         *          for more informations on **domain**, **type** and **protocol** see [socket man page](https://man7.org/linux/man-pages/man7/socket.7.html)
         *
         * @param domain        first argument of socket() call
         * @param type          second argument of socket() call
         * @param protocol      third argument of socket() call
         * @param configure     function called just after socket creation if socket call using **domain**, **type** and **protocol** did not fail
         * 
         * @return true if socket() call retured a valid socket and that the configure handler passed returned true.
         */
        virtual bool    configure_socket(int domain, int type, int protocol, const std::function<bool (socket&)>& configure);


        /**
         * @brief sends a message using the send method defined by raw::listener::send_method
         * 
         * @param socket        the socket to send to
         * @param buffer        buffer containing the message to send
         * @param size          length of the message in bytes
         */
        virtual void    send(socket* socket, const void *buffer, const size_t size);

        /**
         * @brief sends a message using the a struct msghdr to store the message, use method::sendmsg
         * 
         * @param socket        socket to send to
         * @param header        msghdr struct to send
         */
        virtual void    send(socket* socket, const struct msghdr& header);

        /**
         * @brief sends a message using the send method defined by raw::listener::send_method
         * 
         * @param socket        socket to send to
         * @param message_str   string containing the message to send
         */
        virtual void    send(socket* socket, const std::string& message_str);

        
        /**
         * @brief   sends a message using the sendto call (for udp-style communications)
         * 
         * @details sends a message to the specified address using the specified socket
         * 
         * @note    this call should be used instead of the other raw::listener::send methods for udp style communication
         * 
         * @param sock          socket to send to
         * @param address       address of the receiver 
         * @param message_str   message to send
         * @return send_result 
         */
        virtual send_result send_to(socket* sock, const socket_address& address, const std::string& message_str);


    protected:

        /**
         * @brief generic definition of _recv for any method
         * 
         * @tparam _Method  receive method (see raw::method)
         * @param sock      socket to receive from
         * 
         * @return true if socket received successfully, false on error and errno indicates the error
         */
        template<method _Method>
        bool            _recv(socket* sock) { (void)sock; }

        /**
         * @brief specialization for method::recv
         * 
         * @param sock      socket to receive from
         * 
         * @return true if socket received successfully, false on error and errno indicates the error     
         */
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

        /**
         * @brief specialization for method::recvmsg
         * 
         * @param sock      socket to receive from
         * 
         * @return true if socket received successfully, false on error and errno indicates the error     
         */
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

        /**
         * @brief specialization for method::recvfrom
         * 
         * @param sock      socket to receive from
         * 
         * @return true if socket received successfully, false on error and errno indicates the error     
         */
        template<>
        bool            _recv<method::recvfrom>(socket* sock)
        {
            assert(sock != nullptr);
            assert(sock->get_socket() > 0);

            char        buffer[RECV_BUFFER_SIZE] { 0 };

            // TODO: change this when refractoring socket_address
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

            socket_address address { addr };
            this->template execute<actions::PACKET>(sock, address, buffer, n_bytes);
            return (true);
        }



        /**
         * @brief generic definition of _send for any method
         * 
         * @tparam _Method  send method (see raw::method)
         * @param sock      socket to send to (containing the data to send enqueued into its send_buffer)
         * 
         * @return true if socket sent successfully, false on error or unavailable for writing and errno indicates the error
         */
        template<method _Method>
        bool            _send(socket* sock) { (void)sock; }


        /**
         * @brief specialization for method::send
         * 
         * @param sock      socket to send to (containing the data to send enqueued into its send_buffer)
         * 
         * @return true if socket sent successfully, false on error or unavailable for writing and errno indicates the error
         */
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

        /**
         * @brief specialization for method::sendmsg
         * 
         * @param sock      socket to send to (containing the data to send enqueued into its send_buffer)
         * 
         * @return true if socket sent successfully, false on error or unavailable for writing and errno indicates the error
         */
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

        /**
         * @brief specialization for method::sendto
         * 
         * @param sock      socket to send to (containing the data to send enqueued into its send_buffer)
         * 
         * @return true if socket sent successfully, false on error or unavailable for writing and errno indicates the error
         */
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

        /**
         * @brief   receive callback called by events handler when a socket has received some data
         * 
         * @details defines the way of receiving on a specific socket, 
         *          sptr can be reinterpreted safely as raw::listener::socket to retrieve the full socket object description
         * 
         * @param sptr      the socket that needs to receive
         * @return          inner socket list (socket_container::sockets) has changed iterators (i.e. most cases socket disconnected)
         */
        virtual bool    on_receive(unisock::socket_base* sptr) override;

        /**
         * @brief   writeable callback called by events handler when a socket that requested writing got writeable
         * 
         * @details defines the way of writing on a specific socket, 
         *          sptr can be reinterpreted safely as raw::listener::socket to retrieve the full socket object description
         * 
         * @param sptr      the socket that needs to send
         * @return          inner socket list (socket_container::sockets) has changed iterators (i.e. most cases socket disconnected)
         */
        virtual bool    on_writeable(unisock::socket_base* sptr) override;
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

    // pass socket non blocking 
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
            send_to(socket* sock, const socket_address& address, const std::string& message_str)
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
            on_receive(unisock::socket_base* sptr)
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
            on_writeable(unisock::socket_base* sptr)
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


} // ******** namespace _lib


} // ******** namespace raw


} // ******** namespace unisock
