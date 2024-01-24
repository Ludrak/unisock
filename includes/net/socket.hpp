#pragma once

#include "namespaces.hpp"
#include "events/socket_container.hpp"

#include <unistd.h>
#include <sys/socket.h>

UNISOCK_NAMESPACE_START

/* used to define null class when data is not required*/
class empty_data {};

/* Wrap of socket class */
class socket_wrap
{
    public:
        socket_wrap() = delete;

        socket_wrap(events::_lib::isocket_container* container);
        socket_wrap(events::_lib::isocket_container* container, int socket);

        bool    init(const int domain, const int type, const int protocol);

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

    public:
        class : public Data... {}   data;
};

UNISOCK_NAMESPACE_END
