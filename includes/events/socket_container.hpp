
#pragma once

#include "namespaces.hpp"

UNISOCK_NAMESPACE_START


UNISOCK_LIB_NAMESPACE_START
/* predefinition of socket_wrap */
class socket_wrap;

UNISOCK_LIB_NAMESPACE_END


UNISOCK_EVENTS_NAMESPACE_START

UNISOCK_LIB_NAMESPACE_START

// class handler;

class isocket_container
{
    public:
        virtual ~isocket_container() = default;

        // called when a socket contains some data to be read
        virtual void    on_receive(unisock::_lib::socket_wrap* socket_ptr) = 0;
        
        // called when a socket is writeable
        virtual void    on_writeable(unisock::_lib::socket_wrap* socket_ptr) = 0;    
};

UNISOCK_LIB_NAMESPACE_END

UNISOCK_EVENTS_NAMESPACE_END

UNISOCK_NAMESPACE_END