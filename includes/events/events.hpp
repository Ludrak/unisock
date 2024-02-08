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

#define _EVENTS_DEF

/* include correct poll() implementations here */
#include "events/handlers/poll/poll_impl.hpp"
#include "pollable_entity.hpp"


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
bool                    poll(handler& handler, int timeout = -1)
{
    if (handler.empty())
        return (false);
    unisock::events::_lib::poll_impl<unisock::events::handler_type>(handler, timeout);
    return (true);
}


// predefinition for poll(entity, timeout)
// class   pollable_entity;

/**
 * @brief   poll events on entity
 * 
 * @tparam _PollableEntity type of the entity to poll on
 * 
 * @param entity    the entity to poll on 
 * @param timeout   timeout in milliseconds for poll, -1 waits indefinitely, 0 dont wait
 */
template<typename _PollableEntity>
typename std::enable_if<std::is_base_of<unisock::events::pollable_entity, _PollableEntity>::value, bool>::type
poll(_PollableEntity& entity, int timeout = -1)
{
    return unisock::events::poll(entity.get_handler(), timeout);
}

// /**
//  * @brief poll on single socket without handeling callback events
//  * 
//  * @param socket    the socket to poll on
//  * @param events    the events to wait for on this socket (see WANT_READ, WANT_WRITE bitwise values)
//  * @param timeout   timeout in milliseconds for poll, -1 waits indefinitely, 0 dont wait
//  * 
//  * @return true if all events specified where polled, false on poll error or events not fullfilled in specified timeout 
//  */
// bool                    single_poll(const unisock::socket_base& socket, int events, int timeout = -1)
// {
//     return unisock::events::_lib::single_poll_impl<unisock::events::handler_type>(socket, events, timeout);
// }

} // ******** namespace events

} // ******** namespace unisock