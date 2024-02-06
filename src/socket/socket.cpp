#include "socket/socket.hpp"

/**
 * @addindex
 */
namespace unisock {


socket_base::socket_base(isocket_container_base* container)
: _sock(-1), container(container)
{}

socket_base::socket_base(isocket_container_base* container, int socket)
: _sock(socket), container(container)
{}

socket_base::socket_base(isocket_container_base* container, const int domain, const int type, const int protocol)
: container(container)
{
    _sock = ::socket(domain, type, protocol);
    if (_sock < 0)
        _sock = -1;
}

void    socket_base::close()
{
    if (_sock < 0)
        return;
    ::close(_sock);
    _sock = -1;
}

int     socket_base::get_socket() const
{
    return (_sock);
}



bool    socket_base::setsockopt(int level, int option_name, const void* option_value, socklen_t option_len) const
{
    return (0 == ::setsockopt(this->_sock, level, option_name, option_value, option_len));
}

bool    socket_base::getsockopt(int level, int option_name, void* option_value, socklen_t* option_len) const
{
    return (0 == ::getsockopt(this->_sock, level, option_name, option_value, option_len));
}



isocket_container_base*  socket_base::get_container() const
{
    return (this->container);
}


} // ******** namespace events