// #include "events/handler.hpp"
#pragma once

#ifndef _EVENTS_DEF
# include "events/events.hpp"
#endif

#include <poll.h>

UNISOCK_NAMESPACE_START

UNISOCK_EVENTS_NAMESPACE_START

UNISOCK_LIB_NAMESPACE_START

// template<>
// class handler_impl<handler_types::POLL> : public handler_impl_base
// {
//     public:
//         std::vector<_lib::socket_data<unisock::events::handler_type>> sockets;
//         std::vector<unisock::_lib::socket_wrap*>                      socket_ptrs;

//         void    add_socket(int socket, unisock::_lib::socket_wrap* socket_ptr) override;
//         void    del_socket(int socket) override;

//         void    socket_want_read(int socket, bool active = true) override;
//         void    socket_want_write(int socket, bool active = true) override;
// };

// inline void handler_impl<handler_types::POLL>::add_socket(int socket, unisock::_lib::socket_wrap* ref)
// {
//     this->sockets.push_back(_lib::make_data<handler_type>(socket));
//     this->socket_ptrs.push_back(reinterpret_cast<unisock::_lib::socket_wrap*>(ref));
// }


// inline void handler_impl<handler_types::POLL>::del_socket(int socket)
// {
//     auto it = std::find(this->sockets.begin(), this->sockets.end(), socket);
//     if (it == this->sockets.end())
//         return ;
//     this->socket_ptrs.erase(std::next(this->socket_ptrs.begin(), it - this->sockets.begin()));
//     this->sockets.erase(it);
// }


// inline void handler_impl<handler_types::POLL>::socket_want_read(int socket, bool active)
// {
//     auto it = std::find(this->sockets.begin(), this->sockets.end(), socket);
//     if (it == this->sockets.end())
//         return ;
//     if (active)
//         it->events |= POLLIN;
//     else
//         it->events &= ~POLLIN;
// }


// inline void handler_impl<handler_types::POLL>::socket_want_write(int socket, bool active)
// {
//     auto it = std::find(this->sockets.begin(), this->sockets.end(), socket);
//     if (it == this->sockets.end())
//         return ;
//     if (active)
//         it->events |= POLLOUT;
//     else
//         it->events &= ~POLLOUT;
// }


socket_data<handler_types::POLL>  make_data(int socket);
// {
//     socket_data<handler_types::POLL> data;
//     data.events = POLLIN;
//     data.revents = 0;
//     data.fd = socket;

//     return (data);
// }

template<>
void    poll_impl<handler_types::POLL>(handler& handler)
{
    (void)handler;

    int n_changes = poll(reinterpret_cast<pollfd*>(handler.sockets.data()), handler.sockets.size(), -1);
    for (auto it = handler.sockets.begin(); it != handler.sockets.end(); ++it)
    {
        auto& socket = *it;
        if (socket.revents == 0)
            continue ;
        // socket is available for reading
        if (socket.revents & POLLIN)
        {
            // client pointer will be at the same place in the socket_ptrs vector
            auto& client = handler.socket_ptrs[it - handler.sockets.begin()];
            client->get_container()->on_receive(client);
        }
        // socket is available for writing
        if (socket.revents & POLLOUT)
        {
            // client pointer will be at the same place in the socket_ptrs vector
            auto& client = handler.socket_ptrs[it - handler.sockets.begin()];
            client->get_container()->on_writeable(client);
        }

        n_changes--;
        if (n_changes == 0)
            break;
    }
}

UNISOCK_LIB_NAMESPACE_END

UNISOCK_EVENTS_NAMESPACE_END

UNISOCK_NAMESPACE_END