#pragma once

#include "namespaces.hpp"
#include "events/socket_container.hpp"

#include <unistd.h>
#include <sys/socket.h>

UNISOCK_NAMESPACE_START

UNISOCK_LIB_NAMESPACE_START


/* Wrap of socket class */
class socket_wrap
{
    public:
        socket_wrap() = delete;

        socket_wrap(events::_lib::isocket_container* container);
        socket_wrap(events::_lib::isocket_container* container, int socket);
        socket_wrap(events::_lib::isocket_container* container, const int domain, const int type, const int protocol);

        int     getSocket() const;

        void    close();

        events::_lib::isocket_container*  get_container() const;

    private:
        int                               _sock;
        events::_lib::isocket_container*  container;
};

/* Socket with data */
template<typename ...Data>
class socket : public socket_wrap
{
    public:
        socket() = delete;

        socket(events::_lib::isocket_container* container)
        : socket_wrap(container)
        {}

        socket(events::_lib::isocket_container* container, int socket)
        : socket_wrap(container, socket)
        {}

        socket(events::_lib::isocket_container* container, const int domain, const int type, const int protocol)
        : socket_wrap(container, domain, type, protocol)
        {}

    public:
        class : public Data... {}   data;
};

UNISOCK_LIB_NAMESPACE_END

UNISOCK_NAMESPACE_END
