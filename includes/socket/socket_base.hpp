/**
 * @file socket_base.hpp
 * @author your name (you@domain.com)
 * @brief  socket base class definition
 * @version 1.0
 * @date 2024-02-06
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#pragma once


#include "socket/socket_address.hpp"

#include <unistd.h>
#include <sys/socket.h>

/**
 * @addindex
 */
namespace unisock {


/* Wrap of socket class */
/**
 * @brief   base of socket<...>
 * 
 * @details implements a base for socket that wraps general utils for managing socket file descriptors
 * 
 * @ref socket
 */
class socket_base
{
    public:
        /**
         * @brief default constructor default
         */
        explicit socket_base();


        /**
         * @brief construct a socket with a socket file descriptor
         * 
         * @note  this constructor is usually called on accept where socket creation is not handeled by socket() call
         * 
         */
        socket_base(int socket);


        /**
         * @brief opens a new socket file descriptor with args for **socket()** syscall
         * 
         * @note    this call is inherited in unisock::socket<_Data> where socket is added to handler for poll events
         * 
         * @details creates the socket file descriptor with **socket()** with provided **domain**, **type**, and **protocol**.
         *          if **socket()** call fails, the socket will be set to -1, this is why this constructors needs to be checked 
         *          when used, upon error, the specific error code can be retrieved in **errno**
         *          for more informations about **domain**, **type**, and **protocol** see [socket man page](https://man7.org/linux/man-pages/man7/socket.7.html)
         * 
         * @param domain    protocol family
         * @param type      type of connection
         * @param protocol  hint protocol to use for connection (usually 0 if protocol can be deduced with type and family)
         * 
         * @return false if **socket()** call fails, error can be retrieved in **errno**
         */
        virtual bool    open(int domain, int type, int protocol);

        /**
         * @brief closes the socket file descriptor
         * @note    this call is inherited in unisock::socket<_Data> where socket is added to handler for poll events
         */
        virtual void    close();
        
        /**
         * @brief returns the socket file descriptor
         * 
         * @return socket file descriptor
         */
        int     get_socket() const;

        /**
         * @brief set option on socket, wraps `::setsockopt` see [socket](https://man7.org/linux/man-pages/man7/socket.7.html) manpage and [setsockopt](https://man7.org/linux/man-pages/man2/setsockopt.2.html) manpage
         * 
         * @details sets the option at level **level** with name **option_name** to option value specified by **option_value** and **option_len** 
         * 
         * @param level         protocol level
         * @param option_name   name of the option
         * @param option_value  pointer to option value buffer
         * @param option_len    option value buffer size
         * 
         * @return true if setsockopt returned 0 otherwise false, error can be retrieved in **errno**
         */
        bool    setsockopt(int level, int option_name, const void* option_value, socklen_t option_len) const;

        /**
         * @brief get option on socket, wraps **getsockopt()** see [socket](https://man7.org/linux/man-pages/man7/socket.7.html) manpage and [getsockopt](https://man7.org/linux/man-pages/man2/getsockopt.2.html) manpage
         * 
         * @details gets the option at level **level** with name **option_name** and write it to buffer specified by **option_value** and **option_len** 
         * 
         * @param level         protocol level
         * @param option_name   name of the option
         * @param option_value  pointer to option value buffer
         * @param option_len    option value buffer size
         * 
         * @return true if getsockopt returned 0 otherwise false, error can be retrieved in **errno**
         */
        bool    getsockopt(int level, int option_name, void* option_value, socklen_t* option_len) const;


        /**
         * @brief   virtual definition for events::poll to call
         */
        virtual void    on_readable() = 0;

        /**
         * @brief   virtual definition for events::poll to call
         */
        virtual void    on_writeable() = 0;


    private:
        /**
         * @brief socket file descriptor
         */
        int             _sock;

    public:
        /**
         * @brief address of this socket
         * 
         */
        socket_address  address;
};

} // ******** namespace unisock
