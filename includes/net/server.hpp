#pragma once

#include "events/events.hpp"
#include "namespaces.hpp"

UNISOCK_NAMESPACE_START

UNISOCK_LIB_NAMESPACE_START

template<typename ..._Data>
class server :  public unisock::events::_lib::socket_container<_Data...>
{
public:
    using container_type = unisock::events::_lib::socket_container<_Data...>;
    using socket_type = typename container_type::socket_type;
    
    server() = delete;

    server(events::handler& handler)
    : unisock::events::_lib::socket_container<_Data...>(handler)
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
        while (this->sockets.size() > 0)
            this->delete_socket(this->sockets.begin()->second.getSocket());
    };
};

UNISOCK_LIB_NAMESPACE_END

UNISOCK_NAMESPACE_END
