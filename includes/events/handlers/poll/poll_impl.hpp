// #include "events/handler.hpp"
#pragma once

#ifndef _EVENTS_DEF
# include "events/events.hpp"
#endif

#include <poll.h>

UNISOCK_NAMESPACE_START

UNISOCK_EVENTS_NAMESPACE_START

UNISOCK_LIB_NAMESPACE_START


/* poll implementation for poll.h */
template<>
void    poll_impl<handler_types::POLL>(handler& handler, int timeout)
{
    int n_changes = poll(reinterpret_cast<pollfd*>(handler.sockets.data()), handler.sockets.size(), timeout);
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

static constexpr int    WANT_READ = POLLIN;
static constexpr int    WANT_WRITE = POLLOUT;

/* single poll, poll on a single socket, return true if poll set all events set in events bitwise selector */
template<>
bool    single_poll_impl<handler_types::POLL>(const unisock::_lib::socket_wrap& socket, int events, int timeout)
{
    struct pollfd poll_data;
    poll_data.events = events;
    poll_data.revents = 0;
    poll_data.fd = socket.get_socket();

    int n_changes = ::poll(&poll_data, 1, timeout);

    if (n_changes == 0)
        return (false);

    if (poll_data.revents & events)
        return (true);
    
    return (false);
}

UNISOCK_LIB_NAMESPACE_END

UNISOCK_EVENTS_NAMESPACE_END

UNISOCK_NAMESPACE_END