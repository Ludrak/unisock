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
x */
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
        void    add_socket(int socket, unisock::socket_base* sptr)
        {
            this->handler_impl::add_socket(socket, sptr);
            ++invalid; // changes 
        }

        /**
         * @brief deletes a socket from the handler
         * 
         * @param socket socket file descriptor
         */
        void    delete_socket(int socket)
        {
            this->handler_impl::del_socket(socket);
            ++invalid; // changes 
        }


        /**
         * @brief Gets a reference to the invalid field
         * 
         * @details the invalid field is changed every time a socket is added or deleted from the container, 
         *          this way by comparing the returned value later with ref_has_changed we can know if iterators
         *          on this handler were invalidated.
         * 
         * 
         * @return ushort 
         */
        ushort  get_ref()
        {
            // returns the address of data, if vector changes, so as the data() address
            return (this->invalid);
        }

        /**
         * @brief compares reference passed in arguments with the actual reference value
         * 
         * @param old_ref   the old reference returned by get_ref()
         * @return true if references are same
         */
        bool    ref_has_changed(ushort old_ref)
        {
            return (old_ref != get_ref());
        }
    
    private:
        /**
         * @brief short to keep track of inner containers iterators validity,
         * @details each time add_socket or delete_socket is called. 
         *          the value can overflow as long as the overflowed value is not same as the old value
         * 
         */
        ushort  invalid;

        /**
         * @brief friend with the correct events::poll implementation
         * @details this is so that events::poll can access its members to route back parsed events to callbacks
         */
        friend void _lib::poll_impl<handler_type>(handler&, int);
};


} // ******** namespace events

} // ******** namespace unisock