#include "events/events_types.hpp"
#include "events/handlers/poll/handler_impl.hpp"

UNISOCK_NAMESPACE_START

UNISOCK_EVENTS_NAMESPACE_START

UNISOCK_LIB_NAMESPACE_START

template<>
socket_data<handler_types::POLL>  make_data(int socket)
{
    socket_data<handler_types::POLL> data;
    data.events = POLLIN;
    data.revents = 0;
    data.fd = socket;

    return (data);
}


inline void handler_impl<handler_types::POLL>::add_socket(int socket, unisock::_lib::socket_wrap* ref)
{
    this->sockets.push_back(_lib::make_data<handler_type>(socket));
    this->socket_ptrs.push_back(reinterpret_cast<unisock::_lib::socket_wrap*>(ref));
}


inline void handler_impl<handler_types::POLL>::del_socket(int socket)
{
    auto it = std::find(this->sockets.begin(), this->sockets.end(), socket);
    if (it == this->sockets.end())
        return ;
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