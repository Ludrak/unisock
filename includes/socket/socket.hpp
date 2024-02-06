/**
 * @file socket.hpp
 * @author ROBINO Luca
 * @brief  socket objects definitions
 * @version 1.0
 * @date 2024-02-05
 * 
 * @copyright Copyright (c) 2024
 * 
 */


#pragma once

#include "socket/socket_base.hpp"

/**
 * @addindex
 */
namespace unisock {

/* Socket with data */
/**
 * @brief   socket class with data as template parameter pack
 * 
 * @details implements socket_base and adds **data** field for attaching data with socket.
 *          this means that a pointer to a socket_base will give close access to the actual data
 *          attached to this socket. thus by reinterpreting a socket_base with the correct data type,
 *          accessing the data does not need a dereference to a far pointer.
 *          (which would be needed if data were a pointer dynamically allocated, i dont know for sure if this
 *          really improves performance in any way, some testing will be needed to test that )
 * 
 * @tparam Data data types to merge in socket::data
 */
template<typename ...Data>
class socket : public socket_base
{
    public:
        
        /**
         * @brief default constructor deleted
         */
        socket() = delete;

        /**
         * @brief construct a socket contained in a socket_container
         * 
         * @param container container that will handle the socket events
         * 
         * @ref isocket_container_base
         * @ref socket_container
         */
        socket(isocket_container_base* container)
        : socket_base(container)
        {}


        /**
         * @brief construct a socket contained in a socket_container, with a socket descriptor
         * 
         * @note  this constructor is usually called on accept where socket creation is not handeled by socket() call
         * 
         * @param container container that will handle the socket events
         * @param socket    socket file descriptor
         * 
         * @ref isocket_container_base
         * @ref socket_container
         */
        socket(isocket_container_base* container, int socket)
        : socket_base(container, socket)
        {}

        /**
         * @brief   construct a socket contained in a socket_container, with args for `::socket()` call
         * 
         * @note    may return an invalid socket with a value of -1 if `::socket()` call fails, error can be retrieved if socket::get_socket() is -1 in **errno**
         * 
         * @details creates the socket file descriptor with `::socket()` with provided **domain**, **type**, and **protocol**.
         *          if `::socket()` call fails, the socket will be set to -1, this is why this constructors needs to be checked 
         *          when used, upon error, the specific error code can be retrieved in **errno**
         *          for more informations about **domain**, **type**, and **protocol**  see [socket man page](https://man7.org/linux/man-pages/man7/socket.7.html)
         * 
         * @param container container that will handle the socket events
         * @param domain    protocol family
         * @param type      type of connection
         * @param protocol  hint protocol to use for connection (usually 0 if protocol can be deduced with type and family)
         */
        socket(isocket_container_base* container, const int domain, const int type, const int protocol)
        : socket_base(container, domain, type, protocol)
        {}

    public:
        /**
         * @brief data class for socket
         * @details merge all types of Data parameter pack by inheriting them publicly
         */
        class : public Data... {}
        /**
         * @brief data of socket
         */
        data;
};

} // ******** namespace unisock
