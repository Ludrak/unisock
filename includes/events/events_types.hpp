/**
 * @file events_types.hpp
 * @author ROBINO Luca
 * @brief  defines generic type definitions for poll event handlers
 * @version 1.0
 * @date 2024-02-04
 * 
 * @copyright Copyright (c) 2024
 * 
 */


#pragma once


#include "socket/socket_base.hpp"

/**
 * @addindex
 */
namespace unisock {


/**
 * @addindex
 */
namespace events {

/**
 */
class handler;

/**
 * @brief handler types enum
 * 
 */
enum  handler_types
{
    POLL,
    EPOLL,
    KQUEUE,
    SELECT
};

/**
 * @brief default poll handler depending on OS if USE_POLL_HANDLER is not defined
 */
#define _POLL_HANDLER


// skip _POLL_HANDLER macro definition in doxygen
#ifndef DOXYGEN_SHOULD_SKIP_THIS

/* select poll handler */
#ifndef USE_POLL_HANDLER

// redefining _POLL_HANDLER
# undef _POLL_HANDLER

# if     defined(__LINUX__)

#  define _POLL_HANDLER handler_types::EPOLL

# elif   defined(__APPLE__) || defined(__NetBSD__) || defined(__FreeBSD__)

#  define _POLL_HANDLER handler_types::POLL
//#  include "events/poll.hpp"

// # elif   defined(__WIN32__) || defined(__WIN64__)
// #  define _POLL_HANDLER handler_types::POLL

# else
#  warning "could not retrieve system os for selecting poll handler (see "__FILENAME__":"__LINE__"), setting default to select() (select.h)"


# define _POLL_HANDLER handler_types::SELECT

# endif

#else


# define _POLL_HANDLER USE_POLL_HANDLER

#endif /* USE_POLL_HANDLER */

#endif /* DOXYGEN_SHOULD_SKIP_THIS */

/**
 * @brief handler type that will be used when compiling, to switch this handler type, define USE_POLL_HANDLER macro along with _POLL_HANDLER macro set to the desired value in handler_types
 * 
 */
static constexpr handler_types handler_type = _POLL_HANDLER;


/**
 * @addindex
 */
namespace _lib {

/**
 * @brief base class of handler implementation
 * 
 */
class handler_impl_base
{
    public:
        /**
         * @brief Destroy the handler impl base object
         * 
         */
        virtual ~handler_impl_base() = default;

        /**
         * @brief adds a socket to the handler
         * 
         * @param socket        socket file descriptor
         * @param socket_ptr    socket object to be attached to descriptor
         */
        virtual void    add_socket(int socket, unisock::socket_base* socket_ptr) = 0;

        /**
         * @brief deletes a socket from the handler
         * @note  this will also delete the socket object attached to this descriptor
         * @param socket        socket file descriptor to be deleted
         */
        virtual void    del_socket(int socket) = 0;

        /**
         * @brief returns true if handler handles no socket
         */
        virtual bool    empty() const = 0;

        /**
         * @brief returns the number of sockets handeled by this handler
         */
        virtual size_t  count() const = 0;

        /**
         * @brief set/unset read flag on socket for next poll on handler
         * 
         * @param socket        socket descriptor in handler
         * @param active        state to set to read event flag for socket
         */
        virtual void    socket_want_read(int socket, bool active) = 0;

        /**
         * @brief set/unset write flag on socket for next poll on handler
         * 
         * @param socket        socket descriptor in handler
         * @param active        state to set to write event flag for socket
         */
        virtual void    socket_want_write(int socket, bool active) = 0;
};


/* templates to be specialized for each SockHandler type*/


/**
 * @brief generic definition of handler_impl, each specialization implements differents bases of nsock handlers
 * 
 * @tparam S 
 */
template<handler_types S>
class handler_impl : public handler_impl_base {};


/**
 * @brief generic definition of implementation of poll, needs to be defined for each handler
 * 
 * @tparam _Handler 
 * @param handler   the handler containing the sockets to poll
 * @param timeout   timeout in milliseconds for poll, -1 waits indefinitely, 0 dont wait
 */
template<handler_types _Handler>
void                    poll_impl(std::shared_ptr<unisock::events::handler> handler, int timeout);

// /**
//  * @brief poll on single sockets without handeling callback events
//  * 
//  * @param socket    the socket to be polled
//  * @param events    events required when polling (see WANT_READ, WANT_WRITE)
//  * @param timeout   timeout in milliseconds for poll, -1 waits indefinitely, 0 dont wait
//  * @return true if all events specified where polled, otherwise or on poll error, returns false
//  */
// template<handler_types _Handler>
// bool                    single_poll_impl(const unisock::socket_base& socket, int events, int timeout);


} // ******** namespace _lib

} // ******** namespace events

} // ******** namespace unisock
