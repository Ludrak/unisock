#pragma once

#include "events/events.hpp"
#include "namespaces.hpp"

UNISOCK_NAMESPACE_START

template<typename ..._Data>
class server :  public unisock::events::_lib::socket_container<_Data...>
{
public:
    using container_type = unisock::events::_lib::socket_container<_Data...>;
    using socket_type = typename container_type::socket_type;
    
    server()
    : unisock::events::_lib::socket_container<_Data...>()
    {}

    virtual ~server()
    {}

    virtual void    listen()
    {
        if (this->sockets.size() > 0)
            ::listen(this->sockets.begin()->second.getSocket(), 10 /* max pending connections */);
    }

    void    close()
    {
        std::for_each(this->sockets.begin(), this->sockets.end(), [](auto& pair) { pair.second.close(); });
    };

    // // called when some socket needs to read received data
    // // (i.e. on received)
    // virtual void    on_receive(int socket) = 0;

    // // called when a socket that was requesting write got writeable
    // // (i.e. on queued send)
    // virtual void    on_writeable(int socket) = 0;
};

UNISOCK_NAMESPACE_END
