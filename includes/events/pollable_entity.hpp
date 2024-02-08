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
        pollable_entity()
        : handler(*(new events::handler())), self_handled(true)
        {}

        /**
         * @brief handler constructor, handler is a reference to an external handler
         * 
         * @param handler   external handler to use
         */
        pollable_entity(events::handler& handler)
        : handler(handler), self_handled(false)
        {}

        /**
         * @brief destructor, destroys the handler if it was allocated in constructor
         * 
         */
        ~pollable_entity()
        {
            if (self_handled)
                delete &handler;
        }
    
    public:
        /**
         * @brief returns the handler of this entity
         * 
         * @return handler& 
         */
        handler&  get_handler()
        {
            return (this->handler);
        }


    protected:
        /**
         * @brief reference to the handler handeling this entity
         * 
         */
        events::handler&    handler;
    
    private:
        /**
         * @brief true if handler was allocated in constructor
         */
        bool                self_handled;
};


} // ******** namespace events

} // ******** namespace unisock