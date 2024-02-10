/**
 * @file connection.hpp
 * @author ROBINO Luca
 * @brief 
 * @version 1.0
 * @date 2024-02-07
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#pragma once

#include "socket/socket.hpp"
#include "socket/socket_container.hpp"
#include "events/events.hpp"
#include <queue>

/**
 * @addindex
 */
namespace unisock {

/**
 * @addindex
 */
namespace tcp {

/**
 * @brief server definition with all args
 * 
 * @tparam _Args any argument
 */
template<typename ..._Args>
class server_impl;


template<typename ..._ExtendedActions, typename ..._ServerEntityData, typename ..._ClientEntityData>
class server_impl   <
                /* list of actions to be extended */
                unisock::events::actions_list   <_ExtendedActions...>, 
                /* list of data type to model server listeners sockets class */
                unisock::entity_model           <_ServerEntityData...>,
                /* list of data type to model server clients sockets class */
                unisock::entity_model           <_ClientEntityData...>
                    >;


/**
 * @brief client definition with all args
 * 
 * @tparam _Args any argument
 */
template<typename ..._Args>
class client_impl;


template<typename ..._ExtendedActions, typename ..._ConnectionEntityData>
class client_impl   <
                /* list of actions to be extended */
                unisock::events::actions_list   <_ExtendedActions...>, 
                /* list of data type to model connections class */
                unisock::entity_model           <_ConnectionEntityData...>
                    >;


/**
 * @brief   common actions tags for tcp::client_impl and tcp::server_impl action_handler events
 * 
 * @details defines structs as tags for actions of action_handler,
 *          this tags can be used on the on() and execute() members
 *          of tcp::client_impl and tcp::server_impl.
 * 
 * @ref tcp::client
 * @ref tcp::client_impl
 * @ref tcp::client_actions_list
 * 
 * @ref tcp::server
 * @ref tcp::server_impl
 * @ref tcp::server_actions_list
 * 
 * @ref events::action_handler
 * 
 * @addindex
 */
namespace common_actions
{
    /**
     * @brief   either a tcp::server received data from a client, or a tcp::client received data from a server
     * 
     * @details this event will be called when tcp::server clients received bytes, for tcp::client, this will be called anytime a socket receives data 
     * 
     * @note    server hook prototype: ```void  (tcp::client::connection* client, const char* message, size_t message_len)``` \n
     *          client hook prototype: ```void  (tcp::client::connection* connection, const char* message, size_t message_len)``` \n
     */
    struct  RECEIVE
    {
        static constexpr const char* action_name = "TCP::RECEIVE";
        static constexpr const char* callback_prototype = "void (connection*, const char*, size_t)";
    };

    /**
     * @brief   either a tcp::server listener connection or a tcp::client connection to a server was closed
     *
     * @note    server hook prototype: ```void  (tcp::server::server_connection* server_connection)``` \n
     *          client hook prototype: ```void  (tcp::client::connection* connection)``` \n
     */
    struct  CLOSED 
    {
        static constexpr const char* action_name = "TCP::CLOSED";
        static constexpr const char* callback_prototype = "void (connection*)";
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
        static constexpr const char* action_name = "TCP::ERROR (common actions)";
        static constexpr const char* callback_prototype = "void (const std::string&, int)";
    };
} // ******** namespace common_actions




/**
 * @brief    actions tags to hook tcp::connection_base action_handler events
 * 
 * @details defines structs as tags for actions of action_handler,
 *          this tags can be used on the on() and execute() members
 *          of tcp::connection_base
 * 
 * @ref tcp::connection_base
 * @ref tcp::connection_actions_list
 * 
 * @ref events::action_handler
 * 
 * @addindex
 */
namespace connection_actions
{
    /**
     * @brief   called when a tcp::connection_base called recv() and successfully received bytes
     *
     * 
     * @note    hook prototype: ```void  (const char* message, size_t bytes)```
     */
    struct  RECV
    {
        static constexpr const char* action_name = "TCP::RECV";
        static constexpr const char* callback_prototype = "void (const char*, size_t)";
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
        static constexpr const char* action_name = "TCP::ERROR (connection actions)";
        static constexpr const char* callback_prototype = "void (const std::string&, int)";
    };
};


/**
 * @brief list of actions for tcp::connection_base
 * 
 * @ref tcp::connection_base
 * @ref unisock::events::actions_list
 */
using connection_actions_list = unisock::events::actions_list<
    events::action<connection_actions::RECV,
        std::function<void (const char*, size_t)> >,

    events::action<connection_actions::ERROR,
        std::function<void (const std::string&, int)> >
>;


/**
 * @brief represents a tcp connection, all members are publicly accessible for tcp::server and tcp::client, see tcp::connection
 * 
 * @tparam _EntityData    data to append to socket data
 */
template<typename ..._EntityData>
class connection_base
    :   public unisock::socket<
                                connection_actions_list,
                                unisock::entity_model<_EntityData...>
                              >
{
    public:
        /**
         * @brief type of the base socket
         */
        using base_type = unisock::socket<
                                            connection_actions_list,
                                            unisock::entity_model<_EntityData...>
                                         >;

        /**
         * @brief no empty constructor, tcp connections are always handeled by a tcp::server or tcp::client
         */
        connection_base() = delete;


        /**
         * @brief handler constructor, connection will be handeled by a tcp::server
         * 
         * @param handler handler to use for managing event on this socket
         */
        connection_base(std::shared_ptr<unisock::events::handler> handler)
        : base_type(handler)
        {}

        /**
         * @brief accept constructor, connection is created from socket file descriptor, connection will be handeled by a tcp::server
         * 
         * @param handler handler to use for managing event on this socket
         * @param socket  socket file descriptor
         */
        connection_base(std::shared_ptr<unisock::events::handler> handler, int socket)
        : base_type(handler, socket)
        {}


        /**
         * @brief   send a message using this connection socket
         * 
         * @details uses send to send **message** of size **message_len**. 
         *          if send did not send all the bytes, the leftover message
         *          is pushed into the send_buffer of this connection and
         *          it asks its referenced handler for writing.
         *          next poll, if socket is writeable, the send_buffer will be
         *          flushed by send_flush until it becames empty.
         * 
         * @param message       message to send
         * @param message_len   size of the message to send
         * 
         * @return false on send error, otherwise true (returns also true when packet was not sent entierly, anyways leftover packet is set to be send later by send_flush)
         * 
         * @ref tcp::connection_base::send_flush
         */
        bool    send(const char* message, size_t message_len)
        {
            int n_bytes = ::send(this->get_socket(), message, message_len, 0);
            if (n_bytes < 0)
            {
                this->template execute<connection_actions::ERROR>("send", errno);
                return false;
            }
            if (static_cast<size_t>(n_bytes) < message_len)
            {
                send_buffer.push(std::string(message + n_bytes, message_len - n_bytes));
                this->handler->socket_want_write(this->get_socket(), true);
                return (true);
            }
            return (true);
        }
    
        /**
         * @brief  tries to listen using current socket
         * 
         * @return false on listen error
         */
        bool    listen()
        {
            if (0 > ::listen(this->get_socket(), 10 /* TODO: max pending connections */))
                return (false);
            return (true);
        }


        /**
         * @brief   tries to connect using current socket and address in that socket
         * 
         * @note    socket address must be pre configured with a valid IPv4 or IPv6 address before calling connect()
         * 
         * @return false on connect error
         */
        bool    connect()
        {
            if (0 > ::connect(this->get_socket(), this->address.template to<sockaddr>(), this->address.size()))
                return (false);
            return (true);
        }


        /**
         * @brief   tries to recv on current socket
         * 
         * @return the result of recv: 
         *         >0 : number of bytes received
         *          0 : disconnected
         *         <0 : recv error
         */
        ssize_t recv()
        {
            char    buffer[base_type::RECV_BUFFER_SIZE] { 0 };
            int n_bytes = ::recv(this->get_socket(), buffer, base_type::RECV_BUFFER_SIZE, 0);
            if (n_bytes < 0)
            {
                this->template execute<connection_actions::ERROR>("recv", errno);
                return n_bytes;
            }
            if (n_bytes == 0)
            {
                this->close();//template execute<basic_actions::CLOSED>();
                return n_bytes;
            }
            this->template execute<connection_actions::RECV>(buffer, n_bytes);
            return n_bytes;
        }

        /**
         * @brief sends contents stored to the send buffer, usually called when send failed to send the whole message
         * 
         * 
         * @ref tcp::connection_base<_EntityData>::send
         */
        void    send_flush()
        {
            if (send_buffer.empty())
                return ;
            
            int n_bytes = ::send(this->get_socket(), send_buffer.front().c_str(), send_buffer.front().size(), 0);
            if (n_bytes < 0)
            {
                this->template execute<connection_actions::ERROR>("send", errno);
                return ;
            }
            if (static_cast<size_t>(n_bytes) < send_buffer.front().size())
            {
                send_buffer.front().substr(n_bytes);
                return ;
            }
            send_buffer.pop();
            this->handler->socket_want_write(this->get_socket(), false);
        }
        
    private:

        /**
         * @brief send buffer, gets filled when send was not able to send the whole message once
         * 
         */
        std::queue<std::string> send_buffer;
};


/**
 * @brief socket type for a tcp connection, inherits from complete tcp::connection_type and hides non wanted members
 * 
 * @tparam _EntityData 
 */
template<typename ..._EntityData>
class connection : private tcp::connection_base<_EntityData...>
{
    /**
     * @brief no empty constructor, this type is only available as pointer of reinterpreted tcp::connection of same _EntityData.
     * @details this type is usefull for hiding features of tcp::server and tcp::client that are not wanted in user space.\n 
     *          it inherits privately the definition for the server and client implementations and move only wanted members to public
     */
    connection() = delete;

    public:
        /**
         * @brief base type of tcp::connection
         */
        using base_type = tcp::connection_base<_EntityData...>;

        /**
         * @brief move of on() member to public
         */
        using base_type::on;

        /**
         * @brief move of get_socket() member to public
         */
        using base_type::get_socket;

        /**
         * @brief move of get_socket() member to public
         */
        using base_type::close;

        /**
         * @brief move of address field to public
         */
        using base_type::address;

        /**
         * @brief move of address field to public
         */
        using base_type::send;

        /**
         * @brief move of data field to public
         */
        using base_type::data;        
};



} // ******** namespace tcp

} // ******** namespace unisock
