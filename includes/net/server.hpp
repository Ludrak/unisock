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

    virtual void    close()
    {
        std::for_each(this->sockets.begin(), this->sockets.end(), [](auto& pair) { pair.second.close(); });
    };
};

UNISOCK_NAMESPACE_END
