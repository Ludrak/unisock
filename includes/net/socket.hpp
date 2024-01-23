#pragma once

#include "namespaces.hpp"

#include <unistd.h>
#include <sys/socket.h>

UNISOCK_NAMESPACE_START

/* used to define null class when data is not required*/
class empty_data {};

/* Wrap of socket class */
class socket_wrap
{
    public:
        socket_wrap();
        socket_wrap(int socket);

        bool    init(const int domain, const int type, const int protocol);

        int     getSocket() const;

        void    close();

    private:
        int _sock;
};

/* Socket with data */
template<typename ...Data>
class socket : public socket_wrap
{
    public:
        socket()
        : socket_wrap()
        {}

        socket(int socket)
        : socket_wrap(socket)
        {}

    public:
        class : public Data... {}   data;
};

UNISOCK_NAMESPACE_END
