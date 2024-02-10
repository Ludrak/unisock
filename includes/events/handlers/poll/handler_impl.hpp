/**
 * @file handler_impl.hpp
 * @author ROBINO Luca
 * @brief events handler implementation for poll
 * @version 1.0
 * @date 2024-02-04
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#pragma once

#include <poll.h>
#include <vector>

#include "events/events_types.hpp"


/**
 * @addindex
 */
namespace unisock {

/**
 * @addindex
 */
namespace events {

/**
 * @addindex
 */
namespace _lib {

/**
 * @brief handler implementation for poll
 * 
 * @tparam  
 */
template<>
class handler_impl<handler_types::POLL> : public handler_impl_base
{
    public:

        explicit handler_impl() = default;

        /**
         * @brief Destroy the handler impl<handler types::POLL> object
         * 
         */
        virtual ~handler_impl<handler_types::POLL>() = default;

        /**
         * @brief vector of struct pollfd, stores informations about events to poll on sockets
         */
        std::vector<pollfd>                         sockets;
        /**
         * @brief vector of pointer to sockets in socket container 
         * @note  the socket and socket_ptrs vectors are always the same size, this way pointers to socket objects are retrieved directly by index
         */
        std::vector<unisock::socket_base*>          socket_ptrs;

        /**
         * @brief adds a socket to the handler
         * 
         * @param socket        socket file descriptor
         * @param socket_ptr    socket object to be attached to descriptor
         */
        void    add_socket(int socket, unisock::socket_base* socket_ptr) override;

        /**
         * @brief deletes a socket from the handler
         * @note  this will also delete the socket object attached to this descriptor
         * @param socket        socket file descriptor to be deleted
         */
        void    del_socket(int socket) override;

        /**
         * @brief returns true if handler handles no socket
         */
        bool    empty() const override;

        /**
         * @brief returns the number of sockets handeled by this handler
         */
        size_t  count() const override;

        /**
         * @brief set/unset read flag on socket for next poll on handler
         * 
         * @param socket        socket descriptor in handler
         * @param active        state to set to read event flag for socket
         */
        void    socket_want_read(int socket, bool active = true) override;

        /**
         * @brief set/unset write flag on socket for next poll on handler
         * 
         * @param socket        socket descriptor in handler
         * @param active        state to set to write event flag for socket 
         */
        void    socket_want_write(int socket, bool active = true) override;
};


} // ******** namespace _lib

} // ******** namespace events

} // ******** namespace unisock


/**
 * @brief operator== for comparing pollfd with sockets
 * 
 * @param s_socket pollfd struct
 * @param i_socket socket file descriptor
 *
 * @return compare inner fd of pollfd struct with filedesc
 */
inline bool    operator==(const struct pollfd& s_socket, int i_socket)
    { return (s_socket.fd == i_socket); }

/**
 * @brief operator< for comparing pollfd with sockets
 * 
 * @param s_socket pollfd struct
 * @param i_socket socket file descriptor
 *
 * @return compare inner fd of pollfd struct with filedesc
 */
inline bool    operator<(const struct pollfd& s_socket, int i_socket)
    { return (s_socket.fd < i_socket); }

/**
 * @brief operator> for comparing pollfd with sockets
 * 
 * @param s_socket pollfd struct
 * @param i_socket socket file descriptor
 *
 * @return compare inner fd of pollfd struct with filedesc
 */
inline bool    operator>(const struct pollfd& s_socket, int i_socket)
    { return (s_socket.fd > i_socket); }



/**
 * @brief operator== for comparing pollfd between eachother
 * 
 * @param a pollfd struct
 * @param b pollfd struct
 *
 * @return compare inner fd of pollfd structs
 */
inline bool    operator==(const struct pollfd& a, const struct pollfd& b)
    { return (a.fd == b.fd); }

/**
 * @brief operator== for comparing pollfd between eachother
 * 
 * @param a pollfd struct
 * @param b pollfd struct
 *
 * @return compare inner fd of pollfd structs
 */
inline bool    operator<(const struct pollfd& a, const struct pollfd& b)
    { return (a.fd < b.fd); }

/**
 * @brief operator== for comparing pollfd between eachother
 * 
 * @param a pollfd struct
 * @param b pollfd struct
 *
 * @return compare inner fd of pollfd structs
 */
inline bool    operator>(const struct pollfd& a, const struct pollfd& b)
    { return (a.fd > b.fd); }

