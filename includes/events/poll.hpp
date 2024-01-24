// #include "events/handler.hpp"
#pragma once

#ifndef _EVENTS_DEF
# include "events/events.hpp"
#endif

#include <poll.h>

UNISOCK_NAMESPACE_START

UNISOCK_EVENTS_NAMESPACE_START

UNISOCK_LIB_NAMESPACE_START

/* data type required by poll handler */
template<>
class socket_data<handler_types::POLL> : public pollfd
{};

/* operators for comparing data wrap with sockets */
bool    operator==(const socket_data<handler_types::POLL>& s_socket, int i_socket)
    { return (s_socket.fd == i_socket); }

bool    operator<(const socket_data<handler_types::POLL>& s_socket, int i_socket)
    { return (s_socket.fd < i_socket); }

bool    operator>(const socket_data<handler_types::POLL>& s_socket, int i_socket)
    { return (s_socket.fd > i_socket); }

/* operators for comparing data wraps */
bool    operator==(const socket_data<handler_types::POLL>& a, const socket_data<handler_types::POLL>& b)
    { return (a.fd == b.fd); }

bool    operator<(const socket_data<handler_types::POLL>& a, const socket_data<handler_types::POLL>& b)
    { return (a.fd < b.fd); }

bool    operator>(const socket_data<handler_types::POLL>& a, const socket_data<handler_types::POLL>& b)
    { return (a.fd > b.fd); }




template<>
socket_data<handler_types::POLL>  make_data(int socket)
{
    socket_data<handler_types::POLL> data;
    data.events = POLLIN;
    data.revents = 0;
    data.fd = socket;

    return (data);
}

template<>
void    poll_impl<handler_types::POLL>(handler& handler)
{
    (void)handler;

    int n_changes = ::poll(reinterpret_cast<pollfd*>(handler.sockets.data()), handler.sockets.size(), -1);
    for (auto it = handler.sockets.begin(); it != handler.sockets.end(); ++it)
    {
        auto& socket = *it;
        if (socket.revents == 0)
            continue ;
        // socket is available for reading
        if (socket.revents & POLLIN)
        {
            // client pointer will be at the same place in the socket_ptrs vector
            auto& client = handler.socket_ptrs[it - handler.sockets.begin()];
            client->get_container()->on_receive(client);
        }
        // socket is available for writing
        if (socket.revents & POLLOUT)
        {
            // client pointer will be at the same place in the socket_ptrs vector
            auto& client = handler.socket_ptrs[it - handler.sockets.begin()];
            client->get_container()->on_writeable(client);
        }

        n_changes--;
        if (n_changes == 0)
            break;
    }
    // for (auto& socket : handler.sockets)
    // {
    //     if (socket.revents == 0)
    //         continue ;
    //     // socket is available for reading
    //     if (socket.revents & POLLIN)
    //         handler._receive(socket.fd);
    //     // socket is available for writing
    //     if (socket.revents & POLLOUT)
    //         handler._send(socket.fd);
        
    //     n_changes--;
    //     if (n_changes == 0)
    //         break;
    // }
}

UNISOCK_LIB_NAMESPACE_END

UNISOCK_EVENTS_NAMESPACE_END

UNISOCK_NAMESPACE_END