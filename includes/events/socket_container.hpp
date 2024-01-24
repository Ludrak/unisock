
#pragma once

#include "namespaces.hpp"

UNISOCK_NAMESPACE_START


/* predefinition of socket_wrap */
class socket_wrap;


UNISOCK_EVENTS_NAMESPACE_START

UNISOCK_LIB_NAMESPACE_START

class isocket_container
{
    public:
        isocket_container() = default;

        // called when a socket contains some data to be read
        virtual void    on_receive(socket_wrap* socket_ptr) = 0;
        
        // called when a socket is writeable
        virtual void    on_writeable(socket_wrap* socket_ptr) = 0;    
};

UNISOCK_LIB_NAMESPACE_END

UNISOCK_EVENTS_NAMESPACE_END

UNISOCK_NAMESPACE_END