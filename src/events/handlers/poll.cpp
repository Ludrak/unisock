/**
 * @file poll.cpp
 * @author ROBINO Luca
 * @brief events handler implementation for poll
 * @version 1.0
 * @date 2024-02-04
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include "events/events_types.hpp"
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
 * @addindex
 */
namespace _lib {



void handler_impl<handler_types::POLL>::add_socket(int socket, unisock::socket_base* ref)
{
    struct pollfd data;
    data.events = POLLIN;
    data.revents = 0;
    data.fd = socket;
    this->sockets.push_back(data);
    this->socket_ptrs.push_back(ref);
}



void handler_impl<handler_types::POLL>::del_socket(int socket)
{
    auto it = std::find(this->sockets.begin(), this->sockets.end(), socket);
    if (it == this->sockets.end())
        return ;
    // erase from socket_ptrs the element at same position as it from sockets
    // since both vectors are always the same size and conserve respectively the order of contained sockets
    this->socket_ptrs.erase(std::next(this->socket_ptrs.begin(), it - this->sockets.begin()));
    this->sockets.erase(it);
}



bool handler_impl<handler_types::POLL>::empty() const
{
    return (this->sockets.empty());
}


size_t handler_impl<handler_types::POLL>::count() const
{
    return (this->sockets.size());
}



void handler_impl<handler_types::POLL>::socket_want_read(int socket, bool active)
{
    auto it = std::find(this->sockets.begin(), this->sockets.end(), socket);
    if (it == this->sockets.end())
        return ;
    if (active)
        it->events |= POLLIN;
    else
        it->events &= ~POLLIN;
}



void handler_impl<handler_types::POLL>::socket_want_write(int socket, bool active)
{
    auto it = std::find(this->sockets.begin(), this->sockets.end(), socket);
    if (it == this->sockets.end())
        return ;
    if (active)
        it->events |= POLLOUT;
    else
        it->events &= ~POLLOUT;
}


} // ******** namespace _lib

} // ******** namespace events

} // ******** namespace unisock