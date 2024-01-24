#include "net/socket.hpp"

UNISOCK_NAMESPACE_START

socket_wrap::socket_wrap(events::_lib::isocket_container* container)
: _sock(-1), container(container)
{}

socket_wrap::socket_wrap(events::_lib::isocket_container* container, int socket)
: _sock(socket), container(container)
{}

socket_wrap::socket_wrap(events::_lib::isocket_container* container, const int domain, const int type, const int protocol)
: container(container)
{
    _sock = ::socket(domain, type, protocol);
    if (_sock < 0)
        _sock = -1;
}

void    socket_wrap::close()
{
    if (_sock > 0)
    {
        ::close(_sock);
        _sock = -1;
    }
}

int     socket_wrap::getSocket() const
{
    return (_sock);
}


events::_lib::isocket_container*  socket_wrap::get_container() const
{
    return (this->container);
}

UNISOCK_EVENTS_NAMESPACE_END