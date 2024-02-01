#include "events/events_types.hpp"
#include "events/handlers/poll/handler_impl.hpp"

UNISOCK_NAMESPACE_START

UNISOCK_EVENTS_NAMESPACE_START

UNISOCK_LIB_NAMESPACE_START


inline void handler_impl<handler_types::POLL>::add_socket(int socket, unisock::_lib::socket_wrap* ref)
{
    struct pollfd data;
    data.events = POLLIN;
    data.revents = 0;
    data.fd = socket;
    this->sockets.push_back(data);
    this->socket_ptrs.push_back(reinterpret_cast<unisock::_lib::socket_wrap*>(ref));
}


inline void handler_impl<handler_types::POLL>::del_socket(int socket)
{
    auto it = std::find(this->sockets.begin(), this->sockets.end(), socket);
    if (it == this->sockets.end())
        return ;
    // erase from socket_ptrs the element at same position as it from sockets
    // since both vectors are always the same size and conserve respectively the order of contained sockets
    this->socket_ptrs.erase(std::next(this->socket_ptrs.begin(), it - this->sockets.begin()));
    this->sockets.erase(it);
}


inline void handler_impl<handler_types::POLL>::socket_want_read(int socket, bool active)
{
    auto it = std::find(this->sockets.begin(), this->sockets.end(), socket);
    if (it == this->sockets.end())
        return ;
    if (active)
        it->events |= POLLIN;
    else
        it->events &= ~POLLIN;
}


inline void handler_impl<handler_types::POLL>::socket_want_write(int socket, bool active)
{
    auto it = std::find(this->sockets.begin(), this->sockets.end(), socket);
    if (it == this->sockets.end())
        return ;
    if (active)
        it->events |= POLLOUT;
    else
        it->events &= ~POLLOUT;
}


UNISOCK_LIB_NAMESPACE_END

UNISOCK_EVENTS_NAMESPACE_END

UNISOCK_NAMESPACE_END