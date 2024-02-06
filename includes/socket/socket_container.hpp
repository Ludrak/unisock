/**
 * @file socket_container.hpp
 * @author your name (you@domain.com)
 * @brief 
 * @version 1.0
 * @date 2024-02-06
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#pragma once


#include "socket/isocket_container_base.hpp"
#include "events/polling_handler.hpp"

/**
 * @addindex
 */
namespace unisock {


/** predefinition for poll
 */
template<typename ..._Data>
class socket_container;


/**
 * @addindex
 */
namespace events {

/** predefinition for socket_container
 */
template<typename ..._Data>
void                    poll(socket_container<_Data...>& container, int timeout = -1);


} // ******** namespace events



/**
 * @brief   socket container, contains sockets with some data types, and define ways of handeling socket event callbacks from events::handler
 * @details this class is a common base for every server, listener, client, etc. of the library, it provides ways to safely add and delete socket
 *          and to provide callbacks definitions (on_receive, on_writeable) for handler when events are detected.
 * 
 * @tparam _Data    data types parameter pack of sockets, these types are merged in a single type used by all sockets of this container
 */
template<typename ..._Data>
class socket_container : public isocket_container_base
{
    public:
        /**
         * @brief   types of the sockets contained in this container
         */
        using socket_type = unisock::socket<_Data...>;
        /**
         * @brief   final data types of sockets that are stored in this container
         * @details each time this class is inherited, childrens can add types to the data parameter pack,
         * these are then packed in the socket definition of this container, so from there we can retrieve
         * a final data type for the contained sockets 
         */
        using data_type = decltype(unisock::socket<_Data...>::data);

        /**
         * @brief   Construct a new socket container object
         * @details a handler class reference is required for a socket_container object, otherwise there are no way of polling events from it,
         * since no handler classes are specified in this constructor, an events::handler is allocated for this specific server instance
         * this way the events::poll call can still poll events on this container.
         */
        socket_container()
        : handler(*(new events::handler())), self_handled(true)
        {}

        /**
         * @brief Construct a new socket container object
         * @details this constructor takes in parameter a hanldler object reference, 
         *          this container will create and delete its sockets along with this handler, 
         *          so that when the handler is polled, all events are polled for sockets of this container, 
         *          this permits to have a single handler instance polling different socket 
         *          containers of different socket data types\n
         *          (this also abstracts the ways of dealing with events received so childrens which implements different protocol communications methods can still be polled together)
         * 
         * @note    the handler instance passed as reference cannot be deleted before this container, this would cause undefined behaviour
         * 
         * 
         * @param handler reference to the handler to use for this container
         */
        socket_container(events::handler& handler)
        : handler(handler), self_handled(false)
        {}

        /**
         * @brief Destroy the socket container object
         * @note  if container was created using the empty constructor, this deletes the allocated handler
         */
        virtual ~socket_container()
        {
            if (self_handled)
                delete &handler;
        }

        /**
         * @brief   finds a socket object in the list depending on socket descriptor
         * 
         * @param socket    socket file descriptor
         * @return the socket object containing the data attached to descriptor 
         */
        socket_type *find(int socket)
        {
            auto it = this->sockets.find(socket);
            if (it == this->sockets.end())
                return (nullptr);
            return (&it->second);
        }

        /* finds a socket in the list depending on unary predicate */
        /**
         * @brief   finds a socket object in the list depending on unary predicate of form bool (socket& socket)
         * 
         * @tparam _Function    unary predicate type
         * 
         * @param predicate     predicte to find the sockets
         * 
         * @return  the socket object matching the predicate requirements 
         */
        template<class _Function>
        socket_type *find(_Function predicate)
        {
            auto it = std::find_if(this->sockets.begin(), this->sockets.end(),
                        [&predicate](auto& pair){ return predicate(pair.second); });

            if (it == this->sockets.end())
                return (nullptr);
            return (&it->second);
        }

        /**
         * @brief closes all sockets properly
         * @note  must be called if overriden
         */
        virtual void    close()
        {
            while (this->sockets.size() > 0)
                this->delete_socket(this->sockets.begin()->second.get_socket());
        }
        
    protected:
        /**
         * @brief   proper way of creating a socket for this container
         * @details the socket will be created and added to the list of socket to be handeled by handler
         * 
         * @note    on most cases, when this call returns nullptr it indicates an invalid use of the socket() syscall,
         *          however in rare cases this call could return nullptr in case of a bad insertion, this however should not happen unless a socket reference was kept in the list after being closed
         * 
         * @tparam _Args    args types parameter pack of socket() call
         * 
         * @param args      arguments to forward to socket() call when creating the socket
         * 
         * @return the socket created and successfully initialized, otherwise nullptr and socket() error is available through errno
         */
        template<typename ..._Args>
        socket_type*    make_socket(_Args&&... args)
        {
            socket_type sock { this, std::forward<_Args>(args)... };
            if (sock.get_socket() == -1)
                return nullptr; // socket error
            auto insert = this->sockets.insert(std::make_pair(sock.get_socket(), sock));
            if (!insert.second)
                return nullptr; // insert error, should not happen unless socket was not previously deleted
            this->handler.add_socket(sock.get_socket(), &insert.first->second);
            return (&insert.first->second);
        }

        /**
         * @brief proper way of deleting a socket from the container
         * 
         * @param socket    socket file descriptor that references the socket to be closed
         */
        void            delete_socket(int socket)
        {
            this->handler.del_socket(socket);
            this->sockets.erase(socket);
            ::close(socket);
        }


        /**
         * @brief   sockets contained in the container
         * @details uses a std::map for indexing socket datas with file descriptors to reduce complexity of find operations on this container (only used by socket_container::find)
         */
        std::map<int, socket_type>  sockets;
        /**
         * @brief   reference to the handler that manages these sockets
         * 
         */
        events::handler&            handler;

        /**
         * @brief   defines wether the container created its handler or if it is an external reference
         * @details used to distinguish if the handler was passed as reference or allocated in the constructor of this container, the latter means that the destructor must delete the handler
         */
        bool                        self_handled;

        //TODO: this may be avoided if unisock::events::handler becomes a typedef of the implementation like raw::listener is a typedef of raw::listener_impl<..., ...>
        /**
         * @brief friend with the general specification of handler
         *
         */
        friend class unisock::events::handler;

        /**
         * @brief friend with events::poll
         * 
         */
        friend void  ::unisock::events::poll(socket_container<_Data...>&, int);
};

} // ******** namespace unisock
