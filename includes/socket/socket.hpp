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
#include "events/action_hanlder.hpp"
#include "events/pollable_entity.hpp"

/**
 * @addindex
 */
namespace unisock {

/**
 * @brief   list of data types to model socket type
 * 
 * @details unisock::socket takes an entity_model as template parameter to model its inner data field, \n 
 *          this is usefull for modeling data models for sockets in general, it is only used for naming purposes
 * 
 * @tparam _EntityData  data classes to add to data type of modeled socket
 */
template<typename ..._EntityData>
using entity_model = std::tuple<_EntityData...>;




/**
 * @brief   actions tags to hook unisock::socket action_handler events
 * 
 * @details defines structs as tags for actions of action_handler,
 *          this tags can be used on the on() and execute() members
 *          of unisock::socket and its childrens to enqueue actions or
 *          execute queues of actions respectively
 *          (the execute() member is a protected member of actions_handler
 *          so it is only available when inheriting unisock::socket)
 * 
 * @ref unisock::socket
 * @ref unisock::basic_actions_list
 * 
 * @ref events::action_handler
 * 
 * @addindex
 */
namespace basic_actions
{
    /**
     * @brief   socket is ready to receive bytes
     * @details this event will be called when a unisock::socket that requested reading became ready to receive bytes,
     *          the socket NEEDS to flush itself (by receiving the bytes), otherwise events::poll will poll it again next cycle
     * 
     * @note    this event will be triggered on events::poll with sockets that have their writing flag set for polling (see events::handler::socket_want_read)
     * @note    hook prototype: ```void  ()```
     * 
     * @ref     events::handler::socket_want_read
     */
    struct  READABLE
    {
        static constexpr const char* action_name = "READABLE";
        static constexpr const char* callback_prototype = "void ()";
    };

    /**
     * @brief   socket is ready to send bytes
     * @details this event will be called when a unisock::socket that requested writing became ready to send bytes,
     *
     * @note    this event will be triggered on events::poll with sockets that have their writing flag set for polling (see events::handler::socket_want_write)
     * @note    hook prototype: ```void  ()```
     * 
     * @ref     events::handler::socket_want_read
     */
    struct  WRITEABLE
    {
        static constexpr const char* action_name = "WRITEABLE";
        static constexpr const char* callback_prototype = "void ()";
    };


    /**
     * @brief   called when close() is called on a unisock::socket
     * 
     * @note    this event will be triggered either manually or when recv returned 0
     * 
     * @note    hook prototype: ```void  ()```
     */
    struct  CLOSED
    {
        static constexpr const char* action_name = "CLOSED";
        static constexpr const char* callback_prototype = "void ()";
    };
};


/**
 * @brief list of actions for unisock::socket
 * 
 * @tparam _Actions additionnal actions, for childrens of unisock::socket actions
 */
template<typename ..._Actions>
using basic_actions_list = std::tuple<
    events::action<basic_actions::READABLE, std::function< void (void) > >,
    events::action<basic_actions::WRITEABLE, std::function< void (void) > >,
    events::action<basic_actions::CLOSED, std::function< void (void) > >,
    _Actions...
>;


/**
 * @brief socket generic definition, see specialization unisock::socket< unisock::events::actions_list< _Actions... >, unisock::entity_model< _Data... > >
 * 
 * @tparam _Args any arguments
 * 
 * @ref unisock::socket< unisock::events::actions_list< _Actions... > , unisock::entity_model< _Data... > >
 */
template<typename ..._Args>
class socket;

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
template<typename ..._Actions, typename ..._Data>
class socket<
            unisock::events::actions_list   <_Actions...>,
            unisock::entity_model           <_Data...>
            >
    :   public unisock::socket_base,
        public events::action_handler<basic_actions_list<_Actions...>>,
        public events::pollable_entity
{
    public:
        
        /**
         * @brief size for the static buffer used by recv* implementations
         */
        static constexpr size_t RECV_BUFFER_SIZE = 4096;


        /**
         * @brief default constructor, allocates its own handler, and deletes it in its constructor
         * 
         * @ref events::pollable_entity
         */
        explicit socket() = default;

        /**
         * @brief construct a socket, with reference to a handler
         * 
         * @param handler   the handler that will manage this socket
         */
        explicit socket(std::shared_ptr<unisock::events::handler> handler)
        : events::pollable_entity(handler)
        {}


        /**
         * @brief construct a socket with a socket descriptor
         * 
         * @note  this constructor is usually called on accept where socket creation is not handeled by socket() call
         * 
         * @param handler   the handler that will manage this socket
         * @param socket    socket file descriptor
         */
        explicit socket(std::shared_ptr<unisock::events::handler> handler, int socket)
        : socket_base(socket), events::pollable_entity(handler)
        {}


    public:

         /**
         * @brief opens a new socket file descriptor with args for **socket()** syscall
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
        bool    open(int domain, int type, int protocol) override
        {
            if (!socket_base::open(domain, type, protocol))
                return (false);
            this->handler->add_socket(get_socket(), this);
            return (true);
        }


        /**
         * @brief closes the socket file descriptor
         */
        void    close() override
        {
            // deleting from handler
            this->handler->delete_socket(get_socket());
            // closes the socket so any operations on it on basic_actions::CLOSED will be invalid,
            // socket is set to -1
            socket_base::close();
            // calling callback closed
            this->template execute<basic_actions::CLOSED>();
        }

        /**
         * @brief   tries to bind the address of this socket
         * 
         * @return false when bind failed to bind the address
         */
        bool    bind()
        {
            if (0 > ::bind(this->get_socket(), this->address.template to<sockaddr>(), this->address.size()))
                return (false);
            return (true);
        }

        /**
         * @brief set/unset write flag for socket that will be evaluated on events::poll
         * 
         * @param want_write
         */
        void    set_want_write(bool want_write)
        {
            this->handler->socket_want_write(get_socket(), want_write);
        }

        /**
         * @brief set/unset read flag for socket that will be evaluated on events::poll
         * 
         * @param want_read
         */
        void    set_want_read(bool want_read)
        {
            this->handler->socket_want_read(get_socket(), want_read);
        }

        /**
         * @brief   called by events::poll when socket is readable
         */
        void    on_readable() override
        {
            this->template execute <basic_actions::READABLE>();
        }

        /**
         * @brief   called by events::poll when socket is writeable
         */
        void    on_writeable() override
        {
            this->template execute <basic_actions::WRITEABLE>();
        }

        // /**
        //  * @brief returns the reference to the handler handeling this socket
        //  * 
        //  * @return handler handeling this socket
        //  */
        // events::handler&    get_handler()
        // {
        //     return (this->handler); 
        // }


        /**
         * @brief data class for socket
         * @details merge all types of Data parameter pack by inheriting them publicly
         */
        class : public _Data... {}
        /**
         * @brief data of socket
         */
        data;
};

} // ******** namespace unisock
