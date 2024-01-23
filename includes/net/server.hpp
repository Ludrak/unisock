#pragma once

#include "events/events.hpp"
#include "namespaces.hpp"

UNISOCK_NAMESPACE_START

template<typename ..._Data>
class server :  public unisock::events::socket_container<_Data...>
{
public:
    typedef unisock::events::socket_container<_Data...> container_type;
    typedef typename container_type::socket_type        socket_type;
    
    server()
    : unisock::events::socket_container<_Data...>()
    {}

    virtual ~server()
    {}

    void    listen()
    {
        if (this->sockets.size() > 0)
            ::listen(this->sockets.begin()->second.getSocket(), 10 /* max pending connections */);
    }

    void    close()
    {
        std::for_each(this->sockets.begin(), this->sockets.end(), [](auto& pair) { pair.second.close(); });
    };
};

UNISOCK_NAMESPACE_END
