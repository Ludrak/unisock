#pragma once

#include <thread>
#include <functional>
#include <vector>
#include <map>

#include "namespaces.hpp"
#include "net/socket.hpp"
//#include <iostream>

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

/* templates to be specialized for each SockHandler type*/
template<handler_types S>
class socket_data;

/* data creation, needs to be defined for each handler */
template<handler_types _Handler>
socket_data<_Handler>   make_data(int socket);

/* implementation of poll, needs to be defined for each handler */
template<handler_types _Handler>
void                    poll_impl(handler& handler);

UNISOCK_LIB_NAMESPACE_END



/* poll on selected handler type */
void                    poll(handler& handler);




/* handles a group of socket for one or multiple socket_container */
/* the container needs to subscribe itself to the handler */
class handler
{
    public:
        handler() = default;
        ~handler() = default;

        template<typename ..._Data>
        handler(_lib::socket_container<_Data...>& container);

        template<typename _InputIterator>
        handler(_InputIterator begin, _InputIterator end,
        typename std::enable_if<
                    std::is_base_of<
                        handler,
                        typename _InputIterator::value_type>::value,
                    int
                >::type = 0)
        {
            std::for_each(begin, end, [&](auto& container) { subscribe(container); });
        }

        template<typename ..._Data>
        void    subscribe(_lib::socket_container<_Data...>& container);

        template<typename ..._Data>
        void    _add_socket(int socket, unisock::socket<_Data...>* ref);
    
    private:

        void    _receive(int socket);
        void    _send(int socket);

        std::vector<_lib::socket_data<unisock::events::handler_type>> sockets;
        std::vector<unisock::socket_wrap*>                            socket_ptrs;

        friend void unisock::events::_lib::poll_impl<unisock::events::handler_type>(handler&);
};



UNISOCK_LIB_NAMESPACE_START



#include "socket_container.hpp"

/* contains sockets, can be attached to a handler to poll on those socket 
   a handler needs to be attached to create a socket                      */
template<typename ..._Data>
class socket_container : public isocket_container
{
    public:
        using socket_type = unisock::socket<_Data...>;
        using poll_data = unisock::events::_lib::socket_data<unisock::events::handler_type>;

        socket_container() = default;
        virtual ~socket_container() = default;
        
    protected:
        template<typename ..._Args>
        socket_type*  make_socket(_Args&&... args)
        {
            assert(this->handler != nullptr);

            socket_type sock { this };
            sock.init(std::forward<_Args>(args)...);
            auto insert = this->sockets.insert(std::make_pair(sock.getSocket(), sock));
            if (!insert.second)
                return nullptr; // insert error
            this->handler->_add_socket(sock.getSocket(), &insert.first->second);
            return (&insert.first->second);
        }


        std::map<int, socket_type>  sockets;
        handler*                    handler;

        friend class unisock::events::handler;
};


UNISOCK_LIB_NAMESPACE_END


UNISOCK_EVENTS_NAMESPACE_END

UNISOCK_NAMESPACE_END

#define _EVENTS_DEF

/* include correct poll handler here */
#include "events/poll.hpp"

#include "events/handler.hpp"
#include "events/socket_container.hpp"


UNISOCK_NAMESPACE_START

UNISOCK_EVENTS_NAMESPACE_START

/* poll on selected handler type */
void                    poll(handler& handler)
{
    unisock::events::_lib::poll_impl<unisock::events::handler_type>(handler);
}

UNISOCK_EVENTS_NAMESPACE_END

UNISOCK_NAMESPACE_END