/**
 * @file server.hpp
 * @author ROBINO Luca
 * @brief implements a server with tcp::connection sockets
 * @version 1.0
 * @date 2024-02-07
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#pragma once

#include "tcp/connection.hpp"

/**
 * @addindex
 */
namespace unisock {

/**
 * @addindex
 */
namespace tcp {

/**
 * @brief   actions tags to hook tcp::server action_handler events
 * 
 * @details defines structs as tags for actions of action_handler,
 *          this tags can be used on the on() and execute() members
 *          of tcp::server
 * 
 * @ref tcp::server
 * @ref tcp::server_impl
 * 
 * @ref events::action_handler
 */
namespace server_actions
{
    /**
     * @brief   server started listening on connection
     * 
     * @details this event will be called when tcp::server::listen() is successfull, 
     * 
     * @note    hook prototype: ```void  (tcp::server::server_connection* listener)```
     */
    struct  LISTEN {};

     /**
     * @brief   server accepted a new client
     * 
     * @details this event will be called when tcp::server::accept() is successfull, 
     * 
     * @note    hook prototype: ```void  (tcp::server::client_connection* client)```
     */
    struct  ACCEPT {};

    /**
     * @brief   a client disconnected
     * 
     * @details this event will be called when connection_base::recv() is 0 on a tcp::server::client_connection, 
     * 
     * @note    hook prototype: ```void  (tcp::server::client_connection* client)```
     */
    struct  DISCONNECT {};
} // ******** namespace server_actions


/**
 * @brief  list of actions for tcp::server
 * 
 * @tparam _ServerConnection    type of the listeners sockets
 * @tparam _ClientConnection    type of the clients sockets
 * @tparam _ExtendedActions     additional actions
 * 
 * @ref unisock::events::actions_list
 */
template<typename _ServerConnection, typename _ClientConnection, typename ..._ExtendedActions>
using   server_actions_list = unisock::events::actions_list<
    events::action<common_actions::ERROR,
        std::function<void (const std::string&, int)> >,

    events::action<server_actions::LISTEN,
        std::function<void (_ServerConnection*)> >,

    events::action<common_actions::RECEIVE,
        std::function<void (_ClientConnection*, const char *, size_t)> >,

    events::action<server_actions::ACCEPT,
        std::function<void (_ClientConnection*)> >,
    
    events::action<server_actions::DISCONNECT,
        std::function<void (_ClientConnection*)> >,

    _ExtendedActions...
>;


/**
 * @brief type alias for server_impl with empty actions_list, and default entity_model
 * 
 */
using   server = server_impl<
                    unisock::events::actions_list</* no extended actions */>,
                    unisock::entity_model</* no extended data for listeners sockets */>,
                    unisock::entity_model</* no extended data for clients sockets */>
                            >;



/**
 * @brief   socket container for tcp::server listener sockets
 * 
 * @details inherits virtually socket_container
 * 
 * @tparam _ServerEntityData    data to add to servers listeners sockets 
 * 
 * @ref socket_container
 */
template<typename ..._ServerEntityData>
class server_listener_enitites_container : public virtual socket_container<tcp::connection_base<_ServerEntityData...>>
{
    public:
        /**
         * @brief constructor
         */
        server_listener_enitites_container() = default;

        /**
         * @brief constructor
         * @param handler 
         */
        server_listener_enitites_container(events::handler& handler)
        : socket_container<tcp::connection_base<_ServerEntityData...>>(handler)
        {}
};


/**
 * @brief   socket container for tcp::server accepted sockets
 * 
 * @details inherits virtually socket_container
 * 
 * @tparam _ClientEntityData    data to add to servers accepted clients sockets 
 * 
 * @ref socket_container
 */
template<typename ..._ClientEntityData>
class server_client_enitites_container : public virtual socket_container<tcp::connection_base<_ClientEntityData...>>
{
    public:
        /**
         * @brief constructor
         */
        server_client_enitites_container() = default;

        /**
         * @brief constructor
         * @param handler
         */
        server_client_enitites_container(events::handler& handler)
        : socket_container<tcp::connection_base<_ClientEntityData...>>(handler)
        {}
};


/**
 * @brief implementation of tcp::server
 * 
 * @tparam _ExtendedActions     additional actions
 * @tparam _ServerEntityData    data types to add to listeners sockets
 * @tparam _ClientEntityData    data types to add to accepted clients sockets
 */
template<typename ..._ExtendedActions, typename ..._ServerEntityData, typename ..._ClientEntityData>
class server_impl   <
                /* list of actions to be extended */
                unisock::events::actions_list   <_ExtendedActions...>, 
                /* list of data type to model server listeners sockets class */
                unisock::entity_model           <_ServerEntityData...>,
                /* list of data type to model server clients sockets class */
                unisock::entity_model           <_ClientEntityData...>
                    >
    :   public events::action_handler<
            server_actions_list <
                /* uses public type tcp::connection here,
                so that hooks socket types are reinterpreted with correct public member accesses */
                tcp::connection<_ServerEntityData...>,
                tcp::connection<_ClientEntityData...>,
                _ExtendedActions...
            >
        >,
        protected server_listener_enitites_container<_ServerEntityData...>,
        protected server_client_enitites_container<_ClientEntityData...>
{
    protected:
        /**
         * @brief   typedef protected client_connection type here
         * @details tcp::connection_base defines all its member in public to be accessed by tcp::server and tcp::client,
         *          it is then reinterpreted to a more private tcp::server::client_connection type which hides non wanted members
         */
        using client_connection_type = tcp::connection_base<_ClientEntityData...>;
        /**
         * @brief   typedef protected server_connection type here
         * @details tcp::connection_base defines all its member in public to be accessed by tcp::server and tcp::client,
         *          it is then reinterpreted to a more private tcp::server::server_connection type which hides non wanted members
         */
        using server_connection_type = tcp::connection_base<_ServerEntityData...>;


        /**
         * @brief the type of the socket_container parent that holds the accepted sockets
         */
        using client_container_type = server_client_enitites_container<_ClientEntityData...>;
        /**
         * @brief the type of the socket_container parent that holds the listeners sockets
         */
        using server_container_type = server_listener_enitites_container<_ServerEntityData...>;

    public:
        /**
         * @brief   typedef public client_connection type here
         * @details tcp::connection inherits privately the definition of tcp::connection_base, and show only wanted member in public to be accessed by from user space,
         *          this type is reinterpreted from public tcp::server::client_connection_type type which contains all member definitions
         */
        using client_connection = tcp::connection<_ClientEntityData...>;
                /**
         * @brief   typedef public client_connection type here
         * @details tcp::connection inherits privately the definition of tcp::connection_base, and show only wanted member in public to be accessed by from user space,
         *          this type is reinterpreted from public tcp::server::server_connection_type type which contains all member definitions
         */
        using server_connection = tcp::connection<_ServerEntityData...>;


        /**
         * @brief   Construct a new server impl object
         * @details server_container_type will allocate a handler and client_container_type will be handeled on that handler
         */
        server_impl()
        : server_container_type(), client_container_type(this->server_container_type::get_handler())
        {}

        /**
         * @brief Construct a new server impl object handeled by an external handler
         * 
         * @param handler   handler that will handle this tcp::server
         */
        server_impl(events::handler& handler)
        : server_container_type(handler), client_container_type(handler)
        {}


        /**
         * @brief move protected inherited member of socket_container to public
         * 
         * @ref socket_container<_SocketType>::get_handler
         */
        using server_container_type::get_handler;
        
        /**
         * @brief calls both listener and accepted clients containers to close()
         * 
         * @ref socket_container<_SocketType>::close
         */
        void    close()
        {
            client_container_type::close();
            server_container_type::close();
        }


        /**
         * @brief makes the server start to listen on hostname and port, using IPv6 is use_IPv6 is specified
         * 
         * @param hostname  the host to listen on
         * @param port      the port to listen on
         * @param use_IPv6  use IPv6
         * 
         * @return false if server was not able to listen on address (error is called in tcp::common_actions::ERROR hook of tcp::server)
         */
        bool    listen(const std::string& hostname, ushort port, bool use_IPv6 = false)
        {
            server_connection_type* socket = this->server_container_type::make_socket(use_IPv6 ? AF_INET6 : AF_INET, SOCK_STREAM, 0);
            if (socket == nullptr)
            {
                this->template execute<common_actions::ERROR>("socket", errno);
                return false;
            }

            if (addrinfo_result::SUCCESS != socket_address::addrinfo(socket->address, hostname, use_IPv6 ? AF_INET6 : AF_INET))
            {
                this->template execute<common_actions::ERROR>("getaddrinfo", errno);
                this->server_container_type::delete_socket(socket->get_socket());
                return false;
            }
            if (!use_IPv6)
                socket->address.template to<sockaddr_in>()->sin_port = htons(port);
            else
                socket->address.template to<sockaddr_in6>()->sin6_port = htons(port);

            if (!socket->bind())
            {
                this->template execute<common_actions::ERROR>("bind", errno);
                this->server_container_type::delete_socket(socket->get_socket());
                return false;
            }

            if (!socket->listen())
            {
                this->template execute<common_actions::ERROR>("listen", errno);
                this->server_container_type::delete_socket(socket->get_socket());
                return false;
            }

            // receive events with accept 
            socket->template on<unisock::basic_actions::READABLE>([this, socket](){ this->accept(socket); });

            // execute handler on listen
            this->template execute<server_actions::LISTEN>(reinterpret_cast<server_connection*>(socket));

            return (true);
        }


        /**
         * @brief called when a listener socket got readable, tries to accept client
         * 
         * @param connection the server listener to accept from
         */
        void    accept(server_connection_type* connection)
        {
            assert(connection != nullptr);
            assert(connection->get_socket() >= 0);

            socket_address  address;
            // address size is only needded for accept here since real address size will also be written to address._address.sa_len
            socklen_t       address_size = socket_address::ADDRESS_STORAGE_SIZE;
            int socket = ::accept(connection->get_socket(), address.to<sockaddr>(), &address_size);
            if (socket < 0)
            {
                this->template execute<common_actions::ERROR>("accept", errno);
                return ;
            }

            client_connection_type* client = this->client_container_type::make_socket(socket);
            if (client == nullptr)
            {
                // insert could have failed and returned a nullptr however this should not happen
                this->template execute<common_actions::ERROR>("insert", 0);
                return ;
            }
            client->address = address;
            client->template on<unisock::basic_actions::READABLE>(
                [this, client]() {
                    if (client->recv() == 0)
                    {
                        this->template execute<server_actions::DISCONNECT>(reinterpret_cast<client_connection*>(client));
                        this->client_container_type::delete_socket(client->get_socket());
                    }
                }
            );
            
            client->template on<unisock::basic_actions::WRITEABLE>(
                [client]() {
                    client->send_flush();
                }
            );

            client->template on<connection_actions::RECV>(
                [this, client](const char* message, size_t bytes) {
                    this->template execute<common_actions::RECEIVE>(reinterpret_cast<client_connection*>(client), message, bytes);
                }
            );

            this->template execute<server_actions::ACCEPT>(reinterpret_cast<client_connection*>(client));
        }

};



} // ******** namespace tcp

} // ******** namespace unisock
