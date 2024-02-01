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


/* handles a group of socket for one or multiple socket_container */
/* the container needs to subscribe itself to the handler */
class handler : public _lib::handler_impl<unisock::events::handler_type>
{
    public:
        handler() = default;
        ~handler() = default;

        template<typename ..._Data>
        void    add_socket(int socket, unisock::_lib::socket<_Data...>* sptr)
        {
            this->handler_impl::add_socket(socket, reinterpret_cast<unisock::_lib::socket_wrap*>(sptr));
        }
    
    private:

        friend void unisock::events::_lib::poll_impl<unisock::events::handler_type>(handler&, int);
};



UNISOCK_LIB_NAMESPACE_START



#include "socket_container.hpp"

/* predefinition for poll(socket_container<...>&)*/
template<typename ..._Data>
class socket_container;


UNISOCK_LIB_NAMESPACE_END

/* predefinition for socket_container */
template<typename ..._Data>
void                    poll(_lib::socket_container<_Data...>& container, int timeout = -1);

UNISOCK_LIB_NAMESPACE_START

/* contains sockets, can be attached to a handler to poll on those socket 
   a handler needs to be attached to create a socket                      */
template<typename ..._Data>
class socket_container : public isocket_container
{
    public:
        using socket_type = unisock::_lib::socket<_Data...>;
        using data_type = decltype(unisock::_lib::socket<_Data...>::data);
        using poll_data = unisock::events::_lib::socket_data<unisock::events::handler_type>;

        /* implicitly creating an handler for this server, */
        /* the handler, in this case must be on the heap   */
        socket_container()
        : handler(*(new events::handler())), self_handled(true)
        {}

        socket_container(handler& handler)
        : handler(handler), self_handled(false)
        {}

        /* destroying the eventual implicitly allocated handler */
        virtual ~socket_container()
        {
            if (self_handled)
                delete &handler;
        }

        /* finds a socket in the list depending on socket*/
        socket_type *find(int socket)
        {
            auto it = this->sockets.find(socket);
            if (it == this->sockets.end())
                return (nullptr);
            return (&it->second);
        }

        /* finds a socket in the list depending on unary predicate */
        template<class _Function>
        socket_type *find(_Function predicate)
        {
            auto it = std::find_if(this->sockets.begin(), this->sockets.end(),
                        [&predicate](auto& pair){ return predicate(pair.second); });

            if (it == this->sockets.end())
                return (nullptr);
            return (&it->second);
        }

        /* closes all sockets properly, MUST be called if inherited ! */
        virtual void    close()
        {
            while (this->sockets.size() > 0)
                this->delete_socket(this->sockets.begin()->second.get_socket());
        }
        
    protected:
        template<typename ..._Args>
        socket_type*    make_socket(_Args&&... args)
        {
            socket_type sock { this, std::forward<_Args>(args)... };
            auto insert = this->sockets.insert(std::make_pair(sock.get_socket(), sock));
            if (!insert.second)
                return nullptr; // insert error
            this->handler.add_socket(sock.get_socket(), &insert.first->second);
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
        bool                        self_handled;

        friend class unisock::events::handler;
        friend void  unisock::events::poll(socket_container<_Data...>&, int);
};


UNISOCK_LIB_NAMESPACE_END


UNISOCK_EVENTS_NAMESPACE_END

UNISOCK_NAMESPACE_END

#define _EVENTS_DEF

/* include correct poll() implementations here */
#include "events/handlers/poll/poll_impl.hpp"


UNISOCK_NAMESPACE_START

UNISOCK_EVENTS_NAMESPACE_START

/* poll events on selected handler type */
void                    poll(handler& handler, int timeout = -1)
{
    unisock::events::_lib::poll_impl<unisock::events::handler_type>(handler, timeout);
}

/* poll events on single socket_container (server/client) */
template<typename ..._Data>
void                    poll(_lib::socket_container<_Data...>& container, int timeout)
{
    unisock::events::_lib::poll_impl<unisock::events::handler_type>(container.handler, timeout);
}

// /* poll on single sockets without handeling callback events, 
//    returns true if all events specified where polled        */
bool                    single_poll(const unisock::_lib::socket_wrap& socket, int events, int timeout = -1)
{
    return unisock::events::_lib::single_poll_impl<unisock::events::handler_type>(socket, events, timeout);
}

UNISOCK_EVENTS_NAMESPACE_END

UNISOCK_NAMESPACE_END