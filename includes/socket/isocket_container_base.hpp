/**
 * @file socket_container.hpp
 * @author ROBINO Luca
 * @brief  base interface class for socket container
 * @version 1.0
 * @date 2024-02-04
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#pragma once

/**
 * @addindex
 */
namespace unisock {


/* predefinition of socket_base */
class socket_base;

/**
 * @brief   base class of socket_container
 * @details defines callbacks for events
 */
class isocket_container_base
{
    public:
        virtual ~isocket_container_base() = default;

        /**
         * @brief called when a socket that requested reading is available
         * @note  implementation should return true if iterators of the sockets map are invalidated
         * 
         * @param socket_ptr    pointer to the socket that requested reading
         * @return true         the internal socket container changed (i.e. most cases a socket was deleted)
         * 
         * @return false        nothing changed on internal container
         */
        virtual bool    on_receive(unisock::socket_base* socket_ptr) = 0;
        
        /**
         * @brief called when a socket that requested writing is available
         * @note  implementation should return true if iterators of the sockets map are invalidated
         * 
         * @param socket_ptr    pointer to the socket that requested writing
         * @return true         the internal socket container changed (i.e. most cases a socket was deleted)
         * 
         * @return false        nothing changed on internal container
         */
        virtual bool    on_writeable(unisock::socket_base* socket_ptr) = 0;    
};

} // ******** namespace unisock