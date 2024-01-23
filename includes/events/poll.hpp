#include "events/events.hpp"
#include <poll.h>

UNISOCK_NAMESPACE_START

UNISOCK_EVENTS_NAMESPACE_START


template<>
class socket_data<handler_type::POLL> : public pollfd
{};


template<>
socket_data<handler_type::POLL>  make_data(int socket)
{
    socket_data<handler_type::POLL> data;
    data.events = POLLIN;
    data.revents = 0;
    data.fd = socket;
    return (data);
}


template<>
void    poll(const handler<handler_type::POLL>& handler)
{
    (void)handler;
}


UNISOCK_EVENTS_NAMESPACE_END

UNISOCK_NAMESPACE_END