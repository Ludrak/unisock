/**
 * @file poll_impl.hpp
 * @author ROBINO Luca
 * @brief  events polling implementation for poll
 * @version 0.1
 * @date 2024-02-04
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#pragma once

#ifndef _EVENTS_DEF
# include "events/events.hpp"
#endif

#include <poll.h>

/**
 * @addindex
 */
namespace unisock {

/**
 * @addindex
 */
namespace events {

/**
 * @addindex
 */
namespace _lib {


/**
 * @brief   events::poll implementation for poll.h, polls on all sockets stored in handler
 * @details this call will for every socket readable or writeable, respectively call the on_readable, and on_writeable members of their container, which indicates to the container to handle the appropriate event.
 * 
 * @tparam  
 * @param handler the handler to poll on
 * @param timeout timeout in milliseconds for poll, -1 waits indefinitely, 0 dont wait
 */
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

/**
 * @brief bitwise field for events::single_poll to select read events to poll on
 */
static constexpr int    WANT_READ = POLLIN;
/**
 * @brief bitwise field for events::single_poll to select write events to poll on
 */
static constexpr int    WANT_WRITE = POLLOUT;

/**
 * @brief  events::poll implementation for poll, polls on a single socket once, returns true if poll set all events set in events bitwise selector
 * 
 * @param socket    the socket to be polled
 * @param events    events required when polling (see WANT_READ, WANT_WRITE)
 * @param timeout   timeout in milliseconds for poll, -1 waits indefinitely, 0 dont wait
 * 
 * 
 * @return true if events specified in events are available, otherwise or on poll error, returns false
 */
template<>
bool    single_poll_impl<handler_types::POLL>(const unisock::socket_base& socket, int events, int timeout)
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

} // ******** namespace _lib

} // ******** namespace events

} // ******** namespace unisock