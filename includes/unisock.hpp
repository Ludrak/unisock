#pragma once

#include "namespaces.hpp"

UNISOCK_NAMESPACE_START

enum  handler_type
{
    POLL,
    EPOLL,
    KQUEUE,
    SELECT
};


static constexpr handler_type DEFAULT_SOCKET_HANDLER = handler_type::POLL;

UNISOCK_NAMESPACE_END

