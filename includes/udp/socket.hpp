#pragma once

#include "socket/socket.hpp"
#include "socket/socket_container.hpp"
#include "events/events.hpp"
#include "raw/socket.hpp"


/**
 * @addindex
 */
namespace unisock {


/**
 * @addindex
 */
namespace udp {



// /**
//  * @brief server definition with all args
//  * 
//  * @tparam _Args any argument
//  */
// template<typename ..._Args>
// class server_impl;


// template<typename ..._ExtendedActions, typename ..._SocketEntityData>
// class server_impl   <
//                 /* list of actions to be extended */
//                 unisock::events::actions_list   <_ExtendedActions...>, 
//                 /* list of data type to model server listeners sockets class */
//                 unisock::entity_model           <_SocketEntityData...>
//                     >;


// /**
//  * @brief client definition with all args
//  * 
//  * @tparam _Args any argument
//  */
// template<typename ..._Args>
// class client_impl;


// template<typename ..._ExtendedActions, typename ..._SocketEntityData>
// class client_impl   <
//                 /* list of actions to be extended */
//                 unisock::events::actions_list   <_ExtendedActions...>, 
//                 /* list of data type to model sockets class */
//                 unisock::entity_model           <_SocketEntityData...>
//                     >;
template<typename ..._Args>
class socket_impl;

template<typename ..._ExtendedActions, typename ..._EntityData>
class socket_impl<
                    /* list of actions to be extended */
                    unisock::events::actions_list   <_ExtendedActions...>,
                    /* list of data type to model sockets class */
                    unisock::entity_model           <_EntityData...>
                 >;

/**
 * @brief   common actions tags for udp::client_impl and udp::server_impl action_handler events
 * 
 * @details defines structs as tags for actions of action_handler,
 *          this tags can be used on the on() and execute() members
 *          of udp::client_impl and udp::server_impl.
 * 
 * @ref udp::client
 * @ref udp::client_impl
 * @ref udp::client_actions_list
 * 
 * @ref udp::server
 * @ref udp::server_impl
 * @ref udp::server_actions_list
 * 
 * @ref events::action_handler
 * 
 * @addindex
 */
namespace actions
{
    /**
     * @brief   either a udp::server received data from a client, or a udp::client received data from a server
     * 
     * @details this event will be called when udp::server clients or udp::client received bytes using recvfrom 
     * 
     * @note    server hook prototype: ```void (const socket_address& address, const char* message, size_t message_len)``` \n
     */
    struct  RECEIVE
    {
        static constexpr const char* action_name = "udp::RECEIVE";
        static constexpr const char* callback_prototype = "void (const socket_address& address, const char* message, size_t message_len)";
    };

    /**
     * @brief   udp::socket successfully bound to address
     * 
     * @details this event will be called when udp::server clients or udp::client received bytes using recvfrom 
     * 
     * @note    hook prototype: ```void (const socket_address& address)``` \n
     */
    struct  BIND
    {
        static constexpr const char* action_name = "udp::BIND";
        static constexpr const char* callback_prototype = "void (const socket_address& address)";
    };

    /**
     * @brief   either a udp::server listener socket or a udp::client socket to a server was closed
     *
     * @note    hook prototype: ```void (const socket_address& address)``` \n
     */
    struct  CLOSED 
    {
        static constexpr const char* action_name = "udp::CLOSED";
        static constexpr const char* callback_prototype = "void (const socket_address& address)";
    };
} // ******** namespace common_actions



template<typename ..._ExtendeedActions>
using actions_list = std::tuple <
    unisock::events::action<actions::RECEIVE,
        std::function< void (const socket_address&, const char*, size_t)> >,

    unisock::events::action<actions::BIND,
        std::function< void (const socket_address&)> >,

    unisock::events::action<actions::CLOSED,
        std::function< void (const socket_address&)> >,
    
    _ExtendeedActions...
>;


/**
 * @brief   type alias for udp socket implementation (see udp::socket_impl<std::tuple<_Actions...>, _Data...>)
 * 
 * @ref udp::socket_impl<std::tuple<_Actions...>, _Data...>
 */
using socket = udp::socket_impl  <
                            unisock::events::actions_list</* no extended actions*/>,
                            unisock::entity_model</* no extended socket data*/>
                            >;



/**
 * @brief   type alias for udp socket implementation with custom data (see udp::socket_impl<std::tuple<_Actions...>, _Data...>)
 * 
 * @ref udp::socket_impl<std::tuple<_Actions...>, _Data...>
 * 
 * @tparam  _Data custom data to append to socket data
 */
template<typename ..._SocketModelData>
using socket_of = udp::socket_impl   <
                                unisock::events::actions_list</* no extended actions*/>,
                                unisock::entity_model<_SocketModelData...>
                                >;

/**
 * @brief socket type for a udp socket, inherits from complete udp::socket_type and hides non wanted members
 * 
 * @tparam _EntityData 
 */
template<typename ..._ExtendedActions, typename ..._EntityData>
class socket_impl<
                    /* list of actions to be extended */
                    unisock::events::actions_list   <_ExtendedActions...>,
                    /* list of data type to model sockets class */
                    unisock::entity_model           <_EntityData...>
                 >
 : private ::unisock::raw::socket_impl<
                                        udp::actions_list<_ExtendedActions...>,
                                        unisock::entity_model<_EntityData...>
                                      >
{
    /**
     * @brief no empty constructor, this type is only available as pointer of reinterpreted udp::socket of same _EntityData.
     * @details this type is usefull for hiding features of udp::server and udp::client that are not wanted in user space.\n 
     *          it inherits privately the definition for the server and client implementations and move only wanted members to public
     */
    public:
        /**
         * @brief base type of udp::socket
         */
        using base_type = ::unisock::raw::socket_impl<
                                                        udp::actions_list<_ExtendedActions...>,
                                                        unisock::entity_model<_EntityData...>
                                                     >;
        
        socket_impl()
        : base_type()
        {
            this->init_socket();
        }

        socket_impl(std::shared_ptr<events::handler> handler)
        : base_type(handler)
        {
            this->init_socket();
        }
    

    protected:
        void    init_socket()
        {
            this->template on<basic_actions::READABLE>([this](){ this->recvfrom(); });

            // remap raw::actions::RECVFROM to udp::common_action::receive
            this->template on<raw::actions::RECVFROM>(
                [this](const socket_address& addr, const char* message, size_t size){ 
                    this->template execute<udp::actions::RECEIVE>(addr, message, size);
                }
            );

            this->template on<basic_actions::CLOSED>(
                [this](){ this->template execute<udp::actions::CLOSED>(this->address); }
            );
        }

    public:
        bool    open(sa_family_t af = AF_INET)
        {
            if (get_socket() < 0)
            {
                if (!this->base_type::open(af, SOCK_DGRAM, 0))
                {
                    return (false);
                }
            }
            
            return (true);
        }

        bool    bind(const std::string hostname, int port, bool use_IPv6 = false)
        {
            // socket should not be bound more that once,, thus should be uninitialized
            assert(get_socket() == -1);

            this->open(use_IPv6 ? AF_INET6 : AF_INET);

            addrinfo_result result = socket_address::addrinfo(this->address, hostname, use_IPv6 ? AF_INET6 : AF_INET);
            if (result != addrinfo_result::SUCCESS)
            {
                this->template execute<basic_actions::ERROR>("addrinfo", errno);
                return false;
            }

            if (use_IPv6)
                this->address.template to<sockaddr_in6>()->sin6_port = htons(port);
            else
                this->address.template to<sockaddr_in>()->sin_port = htons(port);

            if (!this->base_type::bind())
            {
                return (false);
            }

            this->template execute<udp::actions::BIND>(this->address);
            return (true);
        };


        using base_type::on;
        using base_type::get_handler;

        using base_type::get_socket;
        using base_type::close;
        using base_type::address;

        using base_type::send_to;

        using base_type::data;        
};





/**
 * @brief 
 * 
 * @details uses sendto to send an udp packet using the specified socket
 * 
 * @return send_result 
 */
bool    send_to(const socket_address& address, const char* message, size_t message_len, int flags = 0)
{
    raw::socket sock {};
    if (!sock.open(address.family(), SOCK_DGRAM, 0))
        return (false);
    bool result = sock.send_to(address, message, message_len, flags);
    sock.close();
    return (result);
}



} // ******** namespace udp

} // ******** namespace unisock


