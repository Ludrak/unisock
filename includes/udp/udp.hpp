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
        
        std::queue<std::string> send_buffer;
};

UNISOCK_LIB_NAMESPACE_END


/* udp socket type defintion */
template<typename ..._Data>
using socket = unisock::_lib::socket<_Data..., _lib::socket_data>;


/* actions for udp server */
template<typename _Socket, typename ..._Actions>
using common_actions = std::tuple<
    unisock::events::_lib::action<actions::CLOSED,
                        std::function<void (const _Socket&)> >,

    unisock::events::_lib::action<actions::MESSAGE,
                        std::function<void (const _Socket&, const inet_address&, const char*, size_t)> >,

    unisock::events::_lib::action<actions::ERROR,
                        std::function<void (const std::string&, int)> >,

    _Actions...
>;


enum send_result
{
    /* message successfully sent */
    SUCCESS,

    /* sento thrown an error, still available in errno */
    ERROR,

    /* socket was not available for sending */
    UNAVAILABLE,

    /* message sent by sendto is incomplete, compare this with result >= INCOMPLETE */
    /* the number of bytes sent is indicated by result - INCOMPLETE                 */
    INCOMPLETE,
};
/* send definition for udp, it is not mandatory to have an udp client to send udp packets.
   tries to send the message imediately on this address, if socket is not available for
   writing or that packet was not sent entierly, this function DOES NOT queue the end of the byte
   stream to be sent later, for this must secure behaviour, use udp::server / udp::client and poll
   their events with events::poll()
   
   returns a send_result (see enum send_result above) */
template<typename ..._Data>
int send(const udp::socket<_Data...>& socket, const std::string& message)
{
    bool available = events::single_poll(socket, events::_lib::WANT_WRITE, 0);
    if (!available)
        return (send_result::UNAVAILABLE);
    
    ssize_t n_bytes = sendto(socket.get_socket(), message.c_str(), message.size(), 0, socket.data.address.template to_address<sockaddr>(), socket.data.address.size());
    if (n_bytes < 0)
        return (send_result::ERROR);
    else if (n_bytes < static_cast<ssize_t>(message.size()))
        // retrieve this by comparing result >= INCOMPLETE and n_bytes = result - INCOMPLETE
        return (send_result::INCOMPLETE + n_bytes);
    
    return (send_result::SUCCESS);
}

/* send version on inet_address, creates an unhandled socket with null handler */
int send(const inet_address& address, const std::string& message)
{
    auto socket = udp::socket<>(nullptr, address.family(), SOCK_DGRAM, 0);
    socket.data.address = address;
    
    return (udp::send(socket, message));
}


UNISOCK_LIB_NAMESPACE_START

/* common server/client definition with all args */
template<typename ..._Args>
class socket_container;

/* socket_container definition with _Action and _Data */
template<typename ..._Actions, typename ..._Data>
class socket_container<std::tuple<_Actions...>, _Data...>
            : public unisock::events::_lib::socket_container<_Data..., _lib::socket_data>,
              public unisock::events::_lib::action_handler<common_actions<udp::socket<_Data...>, _Actions...>>
{
    static constexpr size_t RECV_BLOCK_SIZE = 1024;

    using container_type = unisock::events::_lib::socket_container<_Data..., _lib::socket_data>;

    public:
        socket_container() = default;

        socket_container(const unisock::events::handler& handler)
        : container_type(handler)
        {}

    private:
        virtual bool    on_receive(unisock::_lib::socket_wrap* sptr) override;
        virtual bool    on_writeable(unisock::_lib::socket_wrap* sptr) override;
};


template<typename ..._Actions, typename ..._Data>
inline bool    udp::_lib::socket_container<std::tuple<_Actions...>, _Data...>::on_receive(unisock::_lib::socket_wrap* sptr)
{
    auto* socket = reinterpret_cast<udp::socket<_Data...>*>(sptr);
    char  buffer[RECV_BLOCK_SIZE];

    // TODO: change this when refractoring inet_address
    char        addr[inet_address::ADDRESS_SIZE];
    socklen_t   addr_len = inet_address::ADDRESS_SIZE;

    size_t n_bytes = ::recvfrom(socket->get_socket(), buffer, RECV_BLOCK_SIZE, 0, reinterpret_cast<sockaddr*>(&addr), &addr_len);
    if (n_bytes < 0)
    {
        this->template execute<actions::ERROR>("recvfrom", errno);
        return (false);
    }

    inet_address client_address { addr, addr_len };
    this->template execute<actions::MESSAGE>(*socket, client_address, buffer, n_bytes);
    return (false);
}



template<typename ..._Actions, typename ..._Data>
inline bool    udp::_lib::socket_container<std::tuple<_Actions...>, _Data...>::on_writeable(unisock::_lib::socket_wrap* sptr) 
{
    auto* socket = reinterpret_cast<udp::socket<_Data...>*>(sptr);

    if (socket->data.send_buffer.empty())
        return (false);
    
    ssize_t n_bytes = ::sendto(socket->get_socket(),
                         socket->data.send_buffer.front().c_str(),
                         socket->data.send_buffer.size(),
                         0,
                         socket->data.address.template to_address<sockaddr>(),
                         socket->data.address.size());

    // error occurred on sendto
    if (n_bytes < 0)
    {
        this->template execute<actions::ERROR>("sendto", errno);
        return (false);
    }
    // message sent successfully
    if (static_cast<size_t>(n_bytes) == socket->data.send_buffer.size())
    {
        socket->data.send_buffer.pop();
        if (socket->data.send_buffer.empty())
            this->handler.socket_want_write(socket->get_socket(), false);
        return (false);
    }
    // message sent incomplete, sub sent bytes from send_buffer
    socket->data.send_buffer.front().substr(n_bytes);
    return (false);
};


UNISOCK_LIB_NAMESPACE_END

UNISOCK_UDP_NAMESPACE_END

UNISOCK_NAMESPACE_END