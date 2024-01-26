#pragma once


#include "namespaces.hpp"
#include "net/socket.hpp"


UNISOCK_NAMESPACE_START

UNISOCK_EVENTS_NAMESPACE_START

UNISOCK_LIB_NAMESPACE_START

/* predefinition of socket_container */
template<typename ..._Data>
class socket_container;

UNISOCK_LIB_NAMESPACE_END


/* predefinition of handler */
class handler;

/* handler types enum */
enum  handler_types
{
    POLL,
    EPOLL,
    KQUEUE,
    SELECT
};


/* select poll handler */
#ifndef USE_POLL_HANDLER

# if     defined(__LINUX__)
#  define _POLL_HANDLER handler_types::EPOLL

# elif   defined(__APPLE__) || defined(__NetBSD__) || defined(__FreeBSD__)
#  define _POLL_HANDLER handler_types::POLL
//#  include "events/poll.hpp"

// # elif   defined(__WIN32__) || defined(__WIN64__)
// #  define _POLL_HANDLER handler_types::POLL

# else
#  warning "could not retrieve system os for selecting poll handler (see "__FILENAME__":"__LINE__"), setting default to select() (select.h)"
#  define _POLL_HANDLER handler_types::SELECT

# endif

#else
# define _POLL_HANDLER USE_POLL_HANDLER
#endif

/* selected poll handler */
static constexpr handler_types handler_type = _POLL_HANDLER;


UNISOCK_LIB_NAMESPACE_START

/* base of handler implementation */
class handler_impl_base
{
    public:
        virtual ~handler_impl_base() = default;

        virtual void    add_socket(int socket, unisock::_lib::socket_wrap* socket_ptr) = 0;
        virtual void    del_socket(int socket) = 0;

        virtual void    socket_want_read(int socket, bool active) = 0;
        virtual void    socket_want_write(int socket, bool active) = 0;
};


/* templates to be specialized for each SockHandler type*/

/* data for each socket in the handler*/
template<handler_types S>
class socket_data;

/* defines how socket_data are stored and managed */
template<handler_types S>
class handler_impl : public handler_impl_base {};


/* implementation of poll, needs to be defined for each handler */
template<handler_types _Handler>
void                    poll_impl(handler& handler, int timeout);

/* poll on single sockets without handeling callback events, 
   returns true if all events specified where polled        */
template<handler_types _Handler>
bool                    single_poll_impl(const unisock::_lib::socket_wrap& socket, int events, int timeout);


UNISOCK_LIB_NAMESPACE_END

UNISOCK_EVENTS_NAMESPACE_END

UNISOCK_NAMESPACE_END