#pragma once

#include "events/events_types.hpp"
#include "namespaces.hpp"
#include <poll.h>
#include <vector>


UNISOCK_NAMESPACE_START

UNISOCK_EVENTS_NAMESPACE_START

UNISOCK_LIB_NAMESPACE_START


/* handler implementation */
template<>
class handler_impl<handler_types::POLL> : public handler_impl_base
{
    public:
        virtual ~handler_impl<handler_types::POLL>() = default;

        std::vector<pollfd>                         sockets;
        std::vector<unisock::_lib::socket_wrap*>    socket_ptrs;

        void    add_socket(int socket, unisock::_lib::socket_wrap* socket_ptr) override;
        void    del_socket(int socket) override;

        void    socket_want_read(int socket, bool active = true) override;
        void    socket_want_write(int socket, bool active = true) override;
};


UNISOCK_LIB_NAMESPACE_END

UNISOCK_EVENTS_NAMESPACE_END

UNISOCK_NAMESPACE_END


/* operators for comparing pollfd with sockets */
inline bool    operator==(const struct pollfd& s_socket, int i_socket)
    { return (s_socket.fd == i_socket); }

inline bool    operator<(const struct pollfd& s_socket, int i_socket)
    { return (s_socket.fd < i_socket); }

inline bool    operator>(const struct pollfd& s_socket, int i_socket)
    { return (s_socket.fd > i_socket); }

/* operators for comparing data wraps */
inline bool    operator==(const struct pollfd& a, const struct pollfd& b)
    { return (a.fd == b.fd); }

inline bool    operator<(const struct pollfd& a, const struct pollfd& b)
    { return (a.fd < b.fd); }

inline bool    operator>(const struct pollfd& a, const struct pollfd& b)
    { return (a.fd > b.fd); }

