/**
 * @file socket.hpp
 * @author ROBINO Luca
 * @brief raw socket implemention
 * @version 0.1
 * @date 2024-02-06
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#pragma once

#include "socket/socket.hpp"
#include "socket/socket_address.hpp"

#include "events/events.hpp"
#include "events/action_hanlder.hpp"

#include <iostream>
#include <queue>
#include <fcntl.h>

/**
 * @addindex
 */
namespace unisock {

/**
 * @addindex
 */
namespace raw {

/**
 * @brief   actions to be hooked on raw::socket or childrens of raw::socket
 * 
 * @details defines structs as tags for actions of action_handler,
 *          this tags can be used on the on() and execute() members
 *          of raw::socket and its childrens to enqueue actions or
 *          execute queues of actions respectively
 * 
 * @ref raw::socket
 * @ref raw::socket_impl
 * 
 * @ref events::action_handler
 */
namespace actions
{

    /**
     * @brief   socket received bytes with recvmsg
     * 
     * @details this event will be called when raw::socket calls recvmsg(), 
     *          this should be called on action unisock::actions::READABLE available on all unisock::socket
     * 
     * @note    hook prototype: ```void  (const msghdr& message)```
     */
    struct  RECVMSG
    {
        static constexpr const char* action_name = "RECVMSG";
        static constexpr const char* callback_prototype = "void (const msghdr&)";
    };

    /**
     * @brief   socket received bytes with recvfrom
     * 
     * @details this event will be called when raw::socket calls recvfrom(), 
     *          this should be called on action unisock::actions::READABLE available on all unisock::socket
     * 
     * @note    hook prototype: ```void  (const socket_address& address, const char* message, size_t size)```
     */
    struct  RECVFROM
    {
        static constexpr const char* action_name = "RECVFROM";
        static constexpr const char* callback_prototype = "void (const socket_address&, const char*, size_t)";
    };

    /**
     * @brief   called on error
     * 
     * @details specifies function which returned the error and errno for that error
     * 
     * @note    hook prototype: ```void  (const std::string& function, int errno)```
     */
    struct  ERROR
    {
        static constexpr const char* action_name = "ERROR";
        static constexpr const char* callback_prototype = "void (const std::string&, int)";
    };
} // ******** namespace socket_actions


/**
 * @brief actions for a raw::socket
 * 
 * @tparam _ExtendedActions additionnal actions, for childrens of raw::socket actions
 */
template<typename ..._ExtendedActions>
using   socket_actions = unisock::events::actions_list<

    unisock::events::action<actions::RECVMSG, 
            std::function< void (const msghdr& message) > >,

    unisock::events::action<actions::RECVFROM, 
            std::function< void (const socket_address& address, const char *message, size_t message_len) > >,

    unisock::events::action<actions::ERROR, 
            std::function< void (const std::string& func, int error) > >,

    _ExtendedActions...
>;

/**
 * @brief   socket_impl generic definition, see specialization socket_impl<std::tuple<_Actions...>, _Data...>
 * 
 * @tparam _Args any arguments
 */
template<typename ..._Args>
class socket_impl;


/**
 * @brief   type alias for raw socket implementation (see raw::socket_impl<std::tuple<_Actions...>, _Data...>)
 * 
 * @ref raw::socket_impl<std::tuple<_Actions...>, _Data...>
 */
using socket = socket_impl  <
                            unisock::events::actions_list</* no extended actions*/>,
                            unisock::entity_model</* no extended socket data*/>
                            >;



/**
 * @brief   type alias for raw socket implementation with custom data (see raw::socket_impl<std::tuple<_Actions...>, _Data...>)
 * 
 * @ref raw::socket_impl<std::tuple<_Actions...>, _Data...>
 * 
 * @tparam  _Data custom data to append to socket data
 */
template<typename ..._SocketModelData>
using socket_of = socket_impl   <
                                unisock::events::actions_list</* no extended actions*/>,
                                unisock::entity_model<_SocketModelData...>
                                >;



/**
 * @brief   socket_impl definition, defines raw::socket
 * 
 * @details defines a raw socket implementation,
 *          action_handler can be extended by actions specified in **_ExtendedActions**,
 *          socket data can be extended using **_SocketModelData**
 * 
 * @tparam _ExtendedActions            actions to add to action_handler
 * @tparam _SocketModelData    data to add to socket
 * 
 * @ref     raw::socket
 */
template<typename ..._ExtendedActions, typename ..._SocketModelData>
class socket_impl<
                    unisock::events::actions_list<_ExtendedActions...>,
                    unisock::entity_model<_SocketModelData...>
                 >
    :   public unisock::socket<
                                socket_actions<_ExtendedActions...>,
                                unisock::entity_model<_SocketModelData...>
                              >
{
    public:
        /**
         * @brief type of the base socket
         */
        using base_type = unisock::socket<socket_actions<_ExtendedActions...>, unisock::entity_model<_SocketModelData...>>;

        /**
         * @brief empty constructor, socket will be self-handeled
         */
        socket_impl() = default;

        /**
         * @brief empty constructor, socket will be handeled by an external handler
         * 
         * @param handler handler to use for managing event on this socket
         */
        socket_impl(std::shared_ptr<unisock::events::handler> handler)
        : base_type(handler)
        {}

        /**
         * @brief   sends a the message referenced by **message** with specified **flags**
         * @details see [man recvmsg](https://man7.org/linux/man-pages/man2/recvmsg.2.html) for more informations about struct msghdr\n
         *          see [man sendmsg](https://man7.org/linux/man-pages/man2/sendmsg.2.html) for more informations about sendmsg
         * @param   message struct msghdr containing a message to send in iovec
         * @param   flags   flags for sendmsg, 0 by default
         * 
         * @return true if message was sent, false on error, ERROR hook is called with errno of error
         */
        bool    sendmsg(const msghdr& message, int flags = 0)
        {
            assert(this->get_socket() > 0);

            ssize_t n_bytes = ::sendmsg(this->get_socket(), &message, flags);
            if (n_bytes < 0)
            {
                this->template execute<actions::ERROR>("sendmsg", errno);
                return (false);
            }
            return (true);
        }

        /**
         * @brief   sends a the message referenced by **message** with specified **size** and **flags** to **address**
         * @details see [man sendto](https://man7.org/linux/man-pages/man2/sendto.2.html) for more informations about sendto
         * 
         * @param   address the address to send the message to
         * @param   message message to send as char buffer
         * @param   size    size of the **message** buffer
         * @param   flags   flags for sendto, 0 by default
         * 
         * @return true if message was sent, false on error, ERROR hook is called with errno of error
         */
        bool    sendto(const socket_address& address, const char* message, size_t size, int flags = 0)
        {
            assert(this->get_socket() > 0);

            ssize_t n_bytes = ::sendto(this->get_socket(),
                                        message,
                                        size,
                                        flags,
                                        address.template to<sockaddr>(),
                                        address.size());
            if (n_bytes < 0)
            {
                this->template execute<actions::ERROR>("sendto", errno);
                return (false);
            }
            return (true);
        }


        /**
         * @brief   receives data to be read on this socket, calls back RECVMSG handler with received bytes
         * @details see [man recvmsg](https://man7.org/linux/man-pages/man2/recvmsg.2.html) for more informations about recvmsg
         * 
         * @return true if bytes were received, false on error 
         */
        bool    recvmsg()
        {
            assert(this->get_socket() > 0);

            struct msghdr   header;
            struct iovec    iov[1];
            char            buffer[base_type::RECV_BUFFER_SIZE] { 0 };

            std::memset(&header, 0, sizeof(header));
            std::memset(iov, 0, sizeof(iov));

            iov[0].iov_base = buffer;
            iov[0].iov_len  = sizeof(buffer);
            header.msg_iov     = iov;
            header.msg_iovlen  = 1;

            int n_bytes = ::recvmsg(this->get_socket(), &header, MSG_DONTWAIT);
            if (n_bytes < 0)
            {
                this->template execute<actions::ERROR>("recvmsg", errno);
                return (false);
            }

            this->template execute<actions::RECVMSG>(header);
            return (true);
        }



        /**
         * @brief   receives data to be read on this socket, calls back RECVFROM handler with received bytes
         * @details see [man recvfrom](https://man7.org/linux/man-pages/man2/recvfrom.2.html) for more informations about recvfrom
         * 
         * @return true if bytes were received, false on error 
         */
        bool    recvfrom()
        {
            assert(this->get_socket() > 0);

            char        buffer[base_type::RECV_BUFFER_SIZE] { 0 };

            // TODO: change this when refractoring socket_address
            struct sockaddr_storage addr;
            socklen_t               addr_len = sizeof(addr);
            memset(&addr, 0, addr_len);

            int n_bytes = ::recvfrom(this->get_socket(),
                                        buffer,
                                        base_type::RECV_BUFFER_SIZE, 
                                        MSG_DONTWAIT,
                                        reinterpret_cast<sockaddr*>(&addr),
                                        &addr_len);
            if (n_bytes < 0)
            {
                this->template execute<actions::ERROR>("recv", errno);
                return (false);
            }

            socket_address address { addr };
            this->template execute<actions::RECVFROM>(address, buffer, n_bytes);
            return (true);
        }
};



} // ******** namespace raw

} // ******** namespace unisock
