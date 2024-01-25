// #include "events/handler.hpp"
#pragma once

#ifndef _EVENTS_DEF
# include "events/events.hpp"
#endif

#include <poll.h>

UNISOCK_NAMESPACE_START

UNISOCK_EVENTS_NAMESPACE_START

UNISOCK_LIB_NAMESPACE_START

/* */
socket_data<handler_types::POLL>  make_data(int socket);


/* poll implementation for poll.h */
template<>
void    poll_impl<handler_types::POLL>(handler& handler)
{
    int n_changes = poll(reinterpret_cast<pollfd*>(handler.sockets.data()), handler.sockets.size(), -1);
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
}

UNISOCK_LIB_NAMESPACE_END

UNISOCK_EVENTS_NAMESPACE_END

UNISOCK_NAMESPACE_END