#pragma once

#include <thread>
#include <functional>
#include <vector>
#include <map>

#include "namespaces.hpp"
#include "net/socket.hpp"

#include "events/events_types.hpp"
//#include <iostream>


/* include handler implementations */
#include "events/handlers/poll/handler_impl.hpp"


UNISOCK_NAMESPACE_START

UNISOCK_EVENTS_NAMESPACE_START


/* poll on selected handler type */
void                    poll(handler& handler);



/* handles a group of socket for one or multiple socket_container */
/* the container needs to subscribe itself to the handler */
class handler : public _lib::handler_impl<unisock::events::handler_type>
{
    public:
        handler() = default;
        ~handler() = default;

        template<typename ..._Data>
        void    add_socket(int socket, unisock::_lib::socket<_Data...>* ref)
        {
            this->handler_impl::add_socket(socket, reinterpret_cast<unisock::_lib::socket_wrap*>(ref));
        }
    
    private:

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
        using socket_type = unisock::_lib::socket<_Data...>;
        using poll_data = unisock::events::_lib::socket_data<unisock::events::handler_type>;

        socket_container() = delete;
        socket_container(handler& handler)
        : handler(handler)
        {}

        virtual ~socket_container() = default;
        
    protected:
        template<typename ..._Args>
        socket_type*    make_socket(_Args&&... args)
        {
            socket_type sock { this, std::forward<_Args>(args)... };
            auto insert = this->sockets.insert(std::make_pair(sock.getSocket(), sock));
            if (!insert.second)
                return nullptr; // insert error
            this->handler.add_socket(sock.getSocket(), &insert.first->second);
            return (&insert.first->second);
        }

        void            delete_socket(int socket)
        {
            this->handler.del_socket(socket);
            this->sockets.erase(socket);
            ::close(socket);
        }


        std::map<int, socket_type>  sockets;
        handler&                    handler;

        friend class unisock::events::handler;
};


UNISOCK_LIB_NAMESPACE_END


UNISOCK_EVENTS_NAMESPACE_END

UNISOCK_NAMESPACE_END

#define _EVENTS_DEF

/* include correct poll() implementations here */
#include "events/handlers/poll/poll_impl.hpp"

// #include "events/handler.hpp"
// #include "events/socket_container.hpp"


UNISOCK_NAMESPACE_START

UNISOCK_EVENTS_NAMESPACE_START

/* poll on selected handler type */
void                    poll(handler& handler)
{
    unisock::events::_lib::poll_impl<unisock::events::handler_type>(handler);
}

UNISOCK_EVENTS_NAMESPACE_END

UNISOCK_NAMESPACE_END