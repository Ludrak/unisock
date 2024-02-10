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
void    poll_impl<handler_types::POLL>(std::shared_ptr<unisock::events::handler> handler, int timeout)
{
    int n_changes = poll(reinterpret_cast<pollfd*>(handler->sockets.data()), handler->sockets.size(), timeout);
    for (auto it = handler->sockets.begin(); it != handler->sockets.end(); ++it)
    {
        auto&   socket = *it;
        ushort  handler_ref = handler->get_ref();

        if (socket.revents == 0)
            continue ;
        // socket is available for reading
        if (socket.revents & POLLIN)
        {
            // client pointer will be at the same place in the socket_ptrs vector
            unisock::socket_base* sockobj = handler->socket_ptrs[it - handler->sockets.begin()];
            sockobj->on_readable();
        }
        if (handler->ref_has_changed(handler_ref))
            break;
        // socket is available for writing
        if (socket.revents & POLLOUT)
        {
            // client pointer will be at the same place in the socket_ptrs vector
            auto* sockobj = handler->socket_ptrs[it - handler->sockets.begin()];
            sockobj->on_writeable();
        }

        n_changes--;
        if (n_changes == 0 || handler->ref_has_changed(handler_ref))
            break;
    }
}

} // ******** namespace _lib

} // ******** namespace events

} // ******** namespace unisock