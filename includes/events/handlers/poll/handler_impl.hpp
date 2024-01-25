#pragma once

#include "events/events_types.hpp"
#include "namespaces.hpp"
#include <poll.h>
#include <vector>


UNISOCK_NAMESPACE_START

UNISOCK_EVENTS_NAMESPACE_START

UNISOCK_LIB_NAMESPACE_START

/* data types required by poll handler */

/* data type per socket */
template<>
class socket_data<handler_types::POLL> : public pollfd
{};


/* operators for comparing data wrap with sockets */
inline bool    operator==(const socket_data<handler_types::POLL>& s_socket, int i_socket)
    { return (s_socket.fd == i_socket); }

inline bool    operator<(const socket_data<handler_types::POLL>& s_socket, int i_socket)
    { return (s_socket.fd < i_socket); }

inline bool    operator>(const socket_data<handler_types::POLL>& s_socket, int i_socket)
    { return (s_socket.fd > i_socket); }

/* operators for comparing data wraps */
inline bool    operator==(const socket_data<handler_types::POLL>& a, const socket_data<handler_types::POLL>& b)
    { return (a.fd == b.fd); }

inline bool    operator<(const socket_data<handler_types::POLL>& a, const socket_data<handler_types::POLL>& b)
    { return (a.fd < b.fd); }

inline bool    operator>(const socket_data<handler_types::POLL>& a, const socket_data<handler_types::POLL>& b)
    { return (a.fd > b.fd); }



/* handler implementation */
template<>
class handler_impl<handler_types::POLL> : public handler_impl_base
{
    public:
        std::vector<_lib::socket_data<unisock::events::handler_type>> sockets;
        std::vector<unisock::_lib::socket_wrap*>                      socket_ptrs;

        void    add_socket(int socket, unisock::_lib::socket_wrap* socket_ptr) override;
        void    del_socket(int socket) override;

        void    socket_want_read(int socket, bool active = true) override;
        void    socket_want_write(int socket, bool active = true) override;
};

UNISOCK_LIB_NAMESPACE_END

UNISOCK_EVENTS_NAMESPACE_END

UNISOCK_NAMESPACE_END