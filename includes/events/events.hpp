/**
 * @file events.hpp
 * @author ROBINO Luca
 * @brief  defines generic implementation of poll event handlers and socket containers
 * @version 1.0
 * @date 2024-02-04
 * 
 * @copyright Copyright (c) 2024
 * 
 */


#pragma once

#include "events/polling_handler.hpp"
#include "socket/socket_container.hpp"

#define _EVENTS_DEF

/* include correct poll() implementations here */
#include "events/handlers/poll/poll_impl.hpp"


/**
 * @addindex
 */
namespace unisock {


/**
 * @addindex
 */
namespace events {


/**
 * @brief       poll events on events::handler
 * 
 * @param handler   the handler to poll on
 * @param timeout   timeout in milliseconds for poll, -1 waits indefinitely, 0 dont wait
 */
void                    poll(handler& handler, int timeout = -1)
{
    unisock::events::_lib::poll_impl<unisock::events::handler_type>(handler, timeout);
}

/**
 * @brief   poll events on single socket_container (server, client, listener, etc.) 
 * 
 * @tparam _Data        data type of sockets of this container
 * 
 * @param container the container to poll on 
 * @param timeout   timeout in milliseconds for poll, -1 waits indefinitely, 0 dont wait
 */
template<typename ..._Data>
void                    poll(socket_container<_Data...>& container, int timeout)
{
    unisock::events::_lib::poll_impl<unisock::events::handler_type>(container.handler, timeout);
}

/**
 * @brief poll on single socket without handeling callback events
 * 
 * @param socket    the socket to poll on
 * @param events    the events to wait for on this socket (see WANT_READ, WANT_WRITE bitwise values)
 * @param timeout   timeout in milliseconds for poll, -1 waits indefinitely, 0 dont wait
 * 
 * @return true if all events specified where polled, false on poll error or events not fullfilled in specified timeout 
 */
bool                    single_poll(const unisock::socket_base& socket, int events, int timeout = -1)
{
    return unisock::events::_lib::single_poll_impl<unisock::events::handler_type>(socket, events, timeout);
}

} // ******** namespace events

} // ******** namespace unisock