#pragma once

#include "udp/common.hpp"

UNISOCK_NAMESPACE_START

UNISOCK_UDP_NAMESPACE_START

UNISOCK_LIB_NAMESPACE_START

template<typename _Socket, typename ..._Actions>
using client_actions = std::tuple<

    unisock::events::_lib::action<actions::LISTENING,
                                std::function<void (_Socket& )> >,
    
    _Actions...
>;


template<typename ..._Data>
class client_impl;

UNISOCK_LIB_NAMESPACE_END




/* ======================================================================== */
/* TYPES ALIASES FOR CREATING CLIENTS                                       */


/* definition of standart client with no additionnal data */
using client = unisock::udp::_lib::client_impl<std::tuple<>>;


/* type alias to get a client with some additionnal data  */
template<typename ..._ConnectionData>
using client_of = unisock::udp::_lib::client_impl<std::tuple<>, _ConnectionData...>;



/* ======================================================================== */
/* CLIENT IMPLEMENTATION DEFINITION                                         */

UNISOCK_LIB_NAMESPACE_START


template<typename ..._Actions, typename ..._Data>
class client_impl<std::tuple<_Actions...>, _Data...>
            :   public unisock::udp::_lib::common_impl<
                        _lib::client_actions<
                            udp::socket<_Data...>,
                            _Actions...
                        >,
                        _Data...
                    >
{
    using container_type = unisock::udp::_lib::common_impl<_lib::client_actions<socket<_Data...>, _Actions...>, _Data...>;

    public:
        client_impl() = default;

        client_impl(unisock::events::handler& handler)
        : container_type(handler)
        {}

        /* adds a target server for send calls,                                 */
        /* All client::send calls will send to this address,                    */
        /* To send to specific address, use global udp::send(address, message)  */
        /* errno of the error can be retrieved in actions::ERROR                */
        virtual bool target_server(const std::string& address, const int port, const sa_family_t family = AF_INET);
        virtual bool target_server(const inet_address& address);

        /* sends a message to all targetted addresses             */
        /* errno of the error can be retrieved in actions::ERROR  */
        virtual void send(const std::string& message);
        virtual void send(const char* message, size_t bytes);
};





/* ======================================================================== */
/* IMPLEMENTATION                                                           */


template<typename ..._Actions, typename ..._Data>
inline bool udp::_lib::client_impl<std::tuple<_Actions...>, _Data...>::target_server(const std::string& address, const int port, const sa_family_t family)
{
    return (target_server(inet_address(address, port, family)));
}

template<typename ..._Actions, typename ..._Data>
inline bool udp::_lib::client_impl<std::tuple<_Actions...>, _Data...>::target_server(const inet_address& address)
{
    auto* sock = this->make_socket(address.family(), SOCK_DGRAM, 0);
    if (sock == nullptr)
    {
        this->template execute<actions::ERROR>("socket", errno);
        return false;
    }

    sock->data.address = address;
    return (true);
}


template<typename ..._Actions, typename ..._Data>
inline void udp::_lib::client_impl<std::tuple<_Actions...>, _Data...>::send(const char* message, size_t bytes)
{
    const std::string message_str { message, bytes };
    this->send(message_str);
}

template<typename ..._Actions, typename ..._Data>
inline void udp::_lib::client_impl<std::tuple<_Actions...>, _Data...>::send(const std::string& message)
{
    for (auto& pair : this->sockets)
    {
        (void)pair;
        int result = udp::send(pair.second, message);
        if (result == send_result::SUCCESS)
            return;

        else if (result == send_result::ERROR)
        {
            this->template execute<actions::ERROR>("sendto", errno);
            return ;
        }

        // sent incomplete data or socket was not available, queue message to send later
        size_t bytes_sent = (result > send_result::INCOMPLETE) ? (result - send_result::INCOMPLETE) : 0 /* send_result::UNAVAILABLE */;
        if (bytes_sent <= message.size())
            return;
        pair.second.data.send_buffer.push(message.substr(bytes_sent));
        this->handler.socket_want_write(pair.second.get_socket(), true);
    }
}

UNISOCK_LIB_NAMESPACE_END

UNISOCK_UDP_NAMESPACE_END

UNISOCK_NAMESPACE_END