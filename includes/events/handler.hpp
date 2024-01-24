#pragma once

#ifndef _EVENTS_DEF
# include "events/events.hpp"
#endif

UNISOCK_NAMESPACE_START

UNISOCK_EVENTS_NAMESPACE_START


template<typename ..._Data>
inline handler::handler(_lib::socket_container<_Data...>& container)
{
    this->subscribe(container);
}


template<typename ..._Data>
inline void    handler::subscribe(_lib::socket_container<_Data...>& container)
{
    container.handler = this;
}


inline void    handler::_receive(int socket)
{
    (void)socket;
}

inline void    handler::_send(int socket)
{
    (void)socket;
}

template<typename ..._Data>
inline void    handler::_add_socket(int socket, unisock::_lib::socket<_Data...>* ref)
{
    this->sockets.push_back(_lib::make_data<handler_type>(socket));
    this->socket_ptrs.push_back(reinterpret_cast<unisock::_lib::socket_wrap*>(ref));
}

inline void    handler::_del_socket(int socket)
{
    auto it = std::find(this->sockets.begin(), this->sockets.end(), socket);
    if (it == this->sockets.end())
        return ;
    this->socket_ptrs.erase(std::next(this->socket_ptrs.begin(), it - this->sockets.begin()));
    this->sockets.erase(it);
}




UNISOCK_EVENTS_NAMESPACE_END

UNISOCK_NAMESPACE_END