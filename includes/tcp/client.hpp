/**
 * @file client.hpp
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2024-02-07
 * 
 * @copyright Copyright (c) 2024
 * 
 */

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
 * @brief   actions tags to hook tcp::client action_handler events
 * 
 * @details defines structs as tags for actions of action_handler,
 *          this tags can be used on the on() and execute() members
 *          of tcp::client and its childrens to enqueue actions or
 *          execute queues of actions respectively
 * 
 * @ref tcp::client
 * @ref tcp::client_impl
 * @ref tcp::client_actions_list
 * 
 * @ref events::action_handler
 * 
 * @addindex 
 */
namespace client_actions
{
    /**
     * @brief   successfully connected on server
     * 
     * @details this event will be called when tcp::client::connect() is successfull, 
     * 
     * @note    hook prototype: ```void  (tcp::client::connection*)```
     */
    struct  CONNECT
    {
        static constexpr const char* action_name = "TCP::CONNECT";
        static constexpr const char* callback_prototype = "void (connection*)";
    };
} // ******** namespace client_actions



/**
 * @brief list of actions for tcp::client
 * 
 * @tparam _Connection      type of connection (depends on entity_model specified for tcp client)
 * @tparam _ExtendedActions additionnal actions to be added
 * 
 * @ref unisock:events::actions_list
 */
template<typename _Connection, typename ..._ExtendedActions>
using   client_actions_list = unisock::events::actions_list<

    events::action<common_actions::ERROR,
        std::function<void (const std::string&, int)> >,

    events::action<client_actions::CONNECT,
        std::function<void (_Connection*)> >,

    events::action<common_actions::CLOSED,
        std::function<void (_Connection*)> >,

    events::action<common_actions::RECEIVE,
        std::function<void (_Connection*, const char *, size_t)> >,

    
    _ExtendedActions...
>;


/**
 * @brief type alias for client_impl with empty actions_list, and default entity_model
 * 
 * @ref client_impl
 * @ref unisock::events::actions_list
 * @ref unisock::entity_model
 */
using   client = client_impl<
                            unisock::events::actions_list</* no extended actions */>,
                            unisock::entity_model</* no extended data for connected sockets */>
                            >;

/**
 * @brief type alias for client_impl to set client data, with empty actions_list
 * 
 * @ref client_impl
 * @ref unisock::events::actions_list
 * @ref unisock::entity_model
 */
template<typename ..._ConnectionEntityData>
using   client_of = client_impl <
                                unisock::events::actions_list</* no extended actions */>,
                                unisock::entity_model<_ConnectionEntityData...>
                                >;


/**
 * @brief   implements a tcp client
 * 
 * @tparam _ExtendedActions         additional actions to extend client implementation
 * @tparam _ConnectionEntityData    additional data to add to connection objects
 */
template<typename ..._ExtendedActions, typename ..._ConnectionEntityData>
class client_impl   <
                /* list of actions to be extended */
                unisock::events::actions_list   <_ExtendedActions...>, 
                /* list of data type to model connections class */
                unisock::entity_model           <_ConnectionEntityData...>
                    >
    :   public events::action_handler<
            client_actions_list<
                /* uses public type tcp::connection here,
                   so that hooks socket types are reinterpreted with correct public member accesses */
                tcp::connection<_ConnectionEntityData...>,
                _ExtendedActions...
            >
        >,
        public events::pollable_entity
{
    protected:
        /**
         * @brief   typedef protected connection type here
         * @details this type defines all its member in public to be accessed by tcp::server and tcp::client,
         *          this type is then reinterpreted to a public tcp::connection type which hides non wanted members
         */
        using connection_type = tcp::connection_base<_ConnectionEntityData...>;


        /**
         * @brief the type of the socket_container parent that holds the sockets
         */
        using container_type = socket_container<tcp::connection_base<_ConnectionEntityData...>>;

    public:
        /**
         * @brief   typedef public connection type here
         * @details this type inherits privately the definition of tcp::connection_base, and show only wanted member in public to be accessed by from user space,
         *          this type is reinterpreted from public tcp::connection_base type which contains all member definitions
         */
        using connection = tcp::connection<_ConnectionEntityData...>;

        /**
         * @brief Construct a new client_impl object, container must be created with reference to handler created by pollable_entity
         */
        client_impl() 
        : events::pollable_entity(handler), container(get_handler())
        {
        }

        /**
         * @brief Construct a new client impl_object handeled by an external handler
         * 
         * @param handler   the handler that will handle this client
         */
        client_impl(std::shared_ptr<unisock::events::handler> handler)
        : events::pollable_entity(handler), container(get_handler())
        {
        }

        /**
         * @brief move protected inherited member of socket_container to public
         * 
         * @ref socket_container<_SocketType>::close
         */
        // using container_type::close;
        void    close()
        {
            container.close();
        }


        /**
         * @brief tries to connect to server specified by **hostname** and **port**, uses IPv6 if **use_IPv6** is enabled
         * 
         * @details
         * 
         * @param hostname  hostname to connect to
         * @param port      port to connect to
         * @param use_IPv6  use IPv6
         * 
         * @return true if connection succeeded, false otherwise, error can be retrieved in errno and in tcp::common_actions::ERROR hook of tcp::client 
         */
        bool    connect(const std::string& hostname, ushort port, bool use_IPv6 = false)
        {
            connection_type* conn = this->container.make_socket(use_IPv6 ? AF_INET6 : AF_INET, SOCK_STREAM, 0);
            if (conn == nullptr)
            {
                this->template execute<common_actions::ERROR>("socket", errno);
                return false;
            }

            conn->template on<unisock::basic_actions::CLOSED>(
                [this, conn]()
                {
                    this->template execute<common_actions::CLOSED>(reinterpret_cast<connection*>(conn));
                }
            );

            if (addrinfo_result::SUCCESS != socket_address::addrinfo(conn->address, hostname, use_IPv6 ? AF_INET6 : AF_INET))
            {
                this->template execute<common_actions::ERROR>("getaddrinfo", errno);
                conn->close();
                // this->delete_socket(conn->get_socket());
                return false;
            }
            if (!use_IPv6)
                conn->address.template to<sockaddr_in>()->sin_port = htons(port);
            else
                conn->address.template to<sockaddr_in6>()->sin6_port = htons(port);

            if (!conn->connect())
            {
                conn->close();
                // this->delete_socket(conn->get_socket());
                this->template execute<common_actions::ERROR>("listen", errno);
                return false;
            }

            // receive events with this action handler 
            conn->template on<unisock::basic_actions::READABLE>(
                [conn]()
                {
                    conn->recv();
                }
            );

            conn->template on<tcp::connection_actions::RECV>(
                [this, conn](const char* message, size_t message_len)
                {
                    this->template execute<common_actions::RECEIVE>(reinterpret_cast<connection*>(conn), message, message_len);
                }
            );

            this->template execute<client_actions::CONNECT>(reinterpret_cast<connection*>(conn));
            return (true);
        }


        /**
         * @brief   send a message to all connections of this client
         * 
         * @param message       message to send
         * @param message_len   message size
         * 
         */
        void    send(const char* message, size_t message_len)
        {
            // TODO: error check on global send
            for (connection_type* connection : this->sockets)
            {
                connection->send(message, message_len);
            }
        }


    protected:
        container_type  container;
};


} // ******** namespace tcp

} // ******** namespace unisock
