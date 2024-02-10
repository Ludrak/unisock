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
class   socket_container : public events::pollable_entity
{
    public:
        /**
         * @brief default constructor, container is self handeled
         * 
         */
        explicit socket_container() = default;

        /**
         * @brief handler constructor, container is handeled by an external handler
         * 
         * @param handler   external handler to use
         */
        explicit socket_container(std::shared_ptr<unisock::events::handler> handler)
        : events::pollable_entity(handler)
        {
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

            _SocketType sockobj = _SocketType(this->handler, socket);
            _SocketType* sockptr = _insert_socket(sockobj);
            return (sockptr);
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
            _SocketType sockobj = _SocketType(this->handler);

            // calls socket_base::open to avoid adding sockobj pointer to the handler, socket is added to the handler below
            if (!sockobj.socket_base::open(domain, type, protocol))
                return nullptr; // socket() failed

            _SocketType* sockptr = _insert_socket(sockobj);
            return (sockptr);
        }


        /**
         * @brief closes call sockets of this container
         * 
         */
        void    close()
        {
            while (!this->sockets.empty())
            {
                // will call basic_actions::CLOSE handler that deletes the socket from the std::map
                this->sockets.begin()->second.close();
            }
        }

    protected:
        _SocketType*    _insert_socket(const _SocketType& sockobj)
        {
            auto insert = this->sockets.insert(std::make_pair(sockobj.get_socket(), sockobj));
            if (!insert.second)
                return nullptr; // insert error, should not happen unless socket was not previously deleted

            auto* sockptr { &insert.first->second };

            // adds action to delete the socket data, calls 
            int socket_key = insert.first->first;
            sockptr->template on<unisock::basic_actions::CLOSED>(
                [this, socket_key](){
                    this->sockets.erase(socket_key);
                },
                events::action_flag::QUEUE_END | events::action_flag::STOP_AFTER
            );

            this->handler->add_socket(sockptr->get_socket(), sockptr);
            return (sockptr);
        }

        /**
         * @brief map containing the socket object mapped on their file descriptor
         * 
         */
        std::map<int, _SocketType>  sockets;
};

} // ******** namespace unisock