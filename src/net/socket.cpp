#include "net/socket.hpp"

UNISOCK_NAMESPACE_START

UNISOCK_LIB_NAMESPACE_START

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
    if (_sock < 0)
        return;
    ::close(_sock);
    _sock = -1;
}

int     socket_wrap::get_socket() const
{
    return (_sock);
}



bool    socket_wrap::setsockopt(int level, int option_name, const void* option_value, socklen_t option_len) const
{
    return (0 == ::setsockopt(this->_sock, level, option_name, option_value, option_len));
}

bool    socket_wrap::getsockopt(int level, int option_name, void* option_value, socklen_t* option_len) const
{
    return (0 == ::getsockopt(this->_sock, level, option_name, option_value, option_len));
}



events::_lib::isocket_container*  socket_wrap::get_container() const
{
    return (this->container);
}

UNISOCK_LIB_NAMESPACE_END

UNISOCK_EVENTS_NAMESPACE_END