/**
 * @file socket_container.hpp
 * @author ROBINO Luca
 * @brief   class to contain an manage groups of sockets
 * @version 1.0
 * @date 2024-02-07
 * 
 * @copyright Copyright (c) 2024
 * 
 */


#pragma once

#include "events/pollable_entity.hpp"

/**
 * @addindex
 */
namespace unisock {

/**
 * @brief defines an entity that can be
 * 
 */
template<typename _SocketType>
class   socket_container : public virtual events::pollable_entity
{
    public:
        /**
         * @brief default constructor, container is self handeled
         * 
         */
        socket_container()
        : events::pollable_entity()
        {}

        /**
         * @brief handler constructor, container is handeled by an external handler
         * 
         * @param handler   external handler to use
         */
        socket_container(events::handler& handler)
        : events::pollable_entity(handler)
        {}

        /**
         * @brief destructor, closes the container
         * 
         */
        ~socket_container()
        {
            close();
        }

    public:
        /**
         * @brief creates properly a new socket from an existing file descriptor, returned socket is handeled on handler
         * 
         * @param socket    socket file descriptor
         * 
         * @return a pointer to the created socket 
         * 
         * @ref unisock::socket
         */
        _SocketType*    make_socket(int socket)
        {
            assert(socket > 0);

            _SocketType sockobj { this->handler, socket };
            auto insert = this->sockets.insert(std::make_pair(sockobj.get_socket(), sockobj));
            if (!insert.second)
                return nullptr; // insert error, should not happen unless socket was not previously deleted
            this->handler.add_socket(sockobj.get_socket(), &insert.first->second);
            return (&insert.first->second);
        }

        /**
         * @brief creates properly a new socket, returned socket is handeled on handler
         * 
         * @param domain    protocol family
         * @param type      type of connection
         * @param protocol  hint protocol to use for connection (usually 0 if protocol can be deduced with type and family)
         * 
         * @return a pointer to the created socket 
         * 
         * @ref unisock::socket
         */
        _SocketType*    make_socket(int domain, int type, int protocol)
        {
            _SocketType sockobj { this->handler };

            if (!sockobj.open(domain, type, protocol))
                return nullptr; // socket() failed

            auto insert = this->sockets.insert(std::make_pair(sockobj.get_socket(), sockobj));
            if (!insert.second)
                return nullptr; // insert error, should not happen unless socket was not previously deleted
            
            return (&insert.first->second);
        }

        /**
         * @brief deletes a socket from the container
         * 
         * @param socket socket file descriptor of the socket object to delete
         */
        void            delete_socket(int socket)
        {
            // TODO: maybe call close handler here
            this->handler.delete_socket(socket);
            this->sockets.erase(socket);
            ::close(socket);
        }


        /**
         * @brief closes call sockets of this container
         * 
         */
        void    close()
        {
            while (!this->sockets.empty())
            {
                this->handler.delete_socket(this->sockets.begin()->first);
                this->sockets.begin()->second.close();
                this->sockets.erase(this->sockets.begin());
            }
        }

    protected:
        /**
         * @brief map containing the socket object mapped on their file descriptor
         * 
         */
        std::map<int, _SocketType>  sockets;
};

} // ******** namespace unisock