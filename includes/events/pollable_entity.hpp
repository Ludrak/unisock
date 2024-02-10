/**
 * @file pollable_entity.hpp
 * @author ROBINO Luca
 * @brief  entity that contains a handler that can be polled with events::poll
 * @version 1.0
 * @date 2024-02-07
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#pragma once

#include "events/polling_handler.hpp"

/**
 * @addindex
 */
namespace unisock {

/**
 * @addindex
 */
namespace events {

/**
 * @brief defines an entity that contains a reference to a handler that can be polled
 * 
 */
class   pollable_entity 
{
    public:
        /**
         * @brief default constructor, entity is self handeled
         * 
         */
        explicit pollable_entity()
        : handler(std::make_shared<events::handler>())//handler(), self_handled(true)
        {
        }

        /**
         * @brief handler constructor, handler is a reference to an external handler
         * 
         * @param handler   external handler to use
         */
        explicit pollable_entity(std::shared_ptr<events::handler> handler)
        : handler(handler)//, self_handled(false)
        {
        }

        /**
         * @brief copy constructor, local handler ref is deleted if it was allocated, new ref is set from **copy** handler ref
         * 
         * @param handler   external handler to use
         */
        explicit pollable_entity(const pollable_entity& copy) = default;

        /**
         * @brief destructor, destroys the handler if it was allocated in constructor
         * 
         */
        ~pollable_entity() = default;


        // pollable_entity&    operator=(const pollable_entity& other)
        // {
        //     if (self_handled)
        //         delete &handler;
        //     handler = other.handler;
        //     self_handled = false;
        //     return (*this);
        // }
    
    public:
        /**
         * @brief returns the handler of this entity
         * 
         * @return handler& 
         */
        std::shared_ptr<events::handler>    get_handler()
        {
            return handler;
        }


    protected:
        /**
         * @brief reference to the handler handeling this entity
         * 
         */
        // events::handler&    handler;


        std::shared_ptr<events::handler>    handler;
    
    private:
        /**
         * @brief true if handler was allocated in constructor
         */
        // bool                self_handled;
};


} // ******** namespace events

} // ******** namespace unisock