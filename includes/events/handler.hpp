#pragma once

// #include "handler_types.hpp"

// #include <vector>
// #include "net/socket.hpp"
// #include "event.hpp"

#ifndef _EVENTS_DEF
# include "events/events.hpp"
#endif

UNISOCK_NAMESPACE_START

UNISOCK_EVENTS_NAMESPACE_START


template<typename ..._Data>
inline handler::handler(_lib::socket_container<_Data...>& container)
{
    this->subscribe(container);
}

// template<typename _InputIterator>
// inline handler::handler(_InputIterator begin, _InputIterator end,
// typename std::enable_if<
//             std::is_base_of<
//                 handler,
//                 typename _InputIterator::value_type>::value,
//             int
//         >::type = 0)
// {
//     std::for_each(begin, end, [&](auto& container) { subscribe(container); });
// }

template<typename ..._Data>
inline void    handler::subscribe(_lib::socket_container<_Data...>& container)
{
    // for (const auto& s : container.sockets)
    // {
    //     sockets.push_back( make_data<unisock::handler_type>(s.second.getSocket()) );
    // }
    container.handler = this;
}


inline void    handler::_receive(int socket)
{
    (void)socket;
}

inline void    handler::_send(int socket)
{
    (void)socket;
}

template<typename ..._Data>
inline void    handler::_add_socket(int socket, unisock::socket<_Data...>* ref)
{
    this->sockets.push_back(_lib::make_data<handler_type>(socket));
    this->socket_ptrs.push_back(reinterpret_cast<socket_wrap*>(ref));
}






// /* predefinition of socket_container */
// template<typename ..._Data>
// class socket_container;


// class handler
// {
//     public:
//         handler() = default;
//         ~handler()
//         { 
//         }

//         template<typename ..._Data>
//         handler(const socket_container<_Data...>& container)
//         {
//             this->subscribe(container);
//         }

//         template<typename _InputIterator>
//         handler(_InputIterator begin, _InputIterator end,
//         typename std::enable_if<
//             std::is_base_of<
//                 handler,
//                 typename _InputIterator::value_type>::value
//         >::type = 0)
//         {
//             std::for_each(begin, end, [&](auto& container) { subscribe(container); });
//         }

//         template<typename ..._Data>
//         void    subscribe(const socket_container<_Data...>& container)
//         {
//             // for (const auto& s : container.sockets)
//             // {
//             //     sockets.push_back( make_data<unisock::handler_type>(s.second.getSocket()) );
//             // }
//             container.handler = this;
//         }
    
//     private:

//         void    _receive(int socket)
//         {
//             (void)socket;
//         }

//         void    _send(int socket)
//         {
//             (void)socket;
//         }

//         template<typename ..._Data>
//         void    _add_socket(int socket, const unisock::socket<_Data...>& ref)
//         {
//             this->sockets.push_back(_make_data<unisock::events::handler_type>(socket));
//             this->socket_ptrs.push_back(&ref);
//         }


//         std::vector<socket_data<unisock::events::handler_type>> sockets;
//         std::vector<std::shared_ptr<unisock::socket_wrap>>      socket_ptrs;

//         friend void unisock::events::_poll_impl<unisock::events::handler_type>(handler&);
// };







UNISOCK_EVENTS_NAMESPACE_END

UNISOCK_NAMESPACE_END