#include "net/socket.hpp"

namespace unisock
{

socket_wrap::socket_wrap()
: _sock(-1)
{}

socket_wrap::socket_wrap(int socket)
: _sock(socket)
{}

bool    socket_wrap::init(const int domain, const int type, const int protocol)
{
    this->_sock = ::socket(domain, type, protocol);
    if (this->_sock < 0)
    {
        this->_sock = -1;
        return (false);
    }
    return (true);
}

void    socket_wrap::close()
{
    if (this->_sock > 0)
    {
        ::close(this->_sock);
        this->_sock = -1;
    }
}

int     socket_wrap::getSocket() const
{
    return (this->_sock);
}

} // namespace unisock
