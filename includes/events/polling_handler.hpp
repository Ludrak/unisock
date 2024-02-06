/**
 * @file polling_handler.hpp
 * @author your name (you@domain.com)
 * @brief 
 * @version 1.0
 * @date 2024-02-06
 * 
 * @copyright Copyright (c) 2024
 * 
 */




#pragma once

#include <thread>
#include <functional>
#include <vector>
#include <map>

#include "socket/socket.hpp"
#include "events/events_types.hpp"


/* include handler implementations */
#include "events/handlers/poll/handler_impl.hpp"


/**
 * @addindex
 */
namespace unisock {

/**
 * @addindex
 */
namespace events {

/**
 * @brief   socket handler class, define a way to poll on group of sockets, \n 
 *          **socket_container** instances can be created on a single handler to be polled together
 * 
 * @details this non generic class specifies an handler implementation depending on unisock::events::handler_type (defined in events_types.hpp) \n 
 * 
 * @ref     unisock::socket_container
 */
class handler : public _lib::handler_impl<handler_type>
{
    public:
        /**
         * @brief Construct a new handler object
         */
        handler() = default;

        /**
         * @brief Destroy the handler object
         */
        virtual ~handler() = default;

        /**
         * @brief adds a socket to the handler
         * 
         * @tparam _Data data type of the socket
         * @param socket socket file descriptor
         * @param sptr   socket object to be attached to descriptor
         */
        template<typename ..._Data>
        void    add_socket(int socket, ::unisock::socket<_Data...>* sptr)
        {
            this->handler_impl::add_socket(socket, reinterpret_cast<::unisock::socket_base*>(sptr));
        }
    
    private:

        /**
         * @brief friend with the correct events::poll implementation
         * @details this is so that events::poll can access its members to route back parsed events to callbacks
         */
        friend void _lib::poll_impl<handler_type>(handler&, int);
};

} // ******** namespace events

} // ******** namespace unisock