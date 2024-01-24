#pragma once

#include "namespaces.hpp"

UNISOCK_NAMESPACE_START

// enum  handler_types
// {
//     POLL,
//     EPOLL,
//     KQUEUE,
//     SELECT
// };

// #ifndef USE_POLL_HANDLER

// # if     defined(__LINUX__)
// #  define _POLL_HANDLER handler_types::EPOLL

// # elif   defined(__APPLE__) || defined(__NetBSD__) || defined(__FreeBSD__)
// #  define _POLL_HANDLER handler_types::POLL
// #  include "events/poll.hpp"

// // # elif   defined(__WIN32__) || defined(__WIN64__)
// // #  define _POLL_HANDLER handler_types::POLL

// # else
// #  warning "could not retrieve system os for selecting poll handler (see "__FILENAME__":"__LINE__"), setting default to select() (select.h)"
// #  define _POLL_HANDLER handler_types::SELECT

// # endif

// #else
// # define _POLL_HANDLER USE_POLL_HANDLER
// #endif

// static constexpr handler_types handler_type = _POLL_HANDLER;

UNISOCK_NAMESPACE_END

