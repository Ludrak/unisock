/**
 * @file action_hanlder.hpp
 * @author ROBINO Luca
 * @brief  action handler class, contains and handles actions queues
 * @version 1.0
 * @date 2024-02-04
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#pragma once

#include <functional>
#include <type_traits>

/**
 * @addindex
 */
namespace unisock {

/**
 * @addindex
 */
namespace events {

/**
 * @brief   list of unisock::actions to be handeled by action handler
 * 
 * @details defines a std::tuple containing the actions in type definition
 * 
 * @tparam _Actions     actions of the list
 * 
 * @ref events::action
 * @ref events::action_handler
 */
template<typename ..._Actions>
using actions_list = std::tuple<_Actions...>;



/**
 * @brief   bitwise flags for action of action_handler
 * @details flags to defines how the action handler should store and execute an action
 */
enum  action_flag
{
    /**
     * @brief no action flags, action executed by default
     */
    DEFAULT =       0b0000,

    /**
     * @brief skip action when flag is set
     */
    SKIP =          0b0001,

    /**
     * @brief stops the execution of the whole list after the execution of the action
     * 
     */
    STOP_AFTER =    0b0010,

    /**
     * @brief enqueues the action from end of list, if set on an action, the action must remain at the end of the list
     * 
     * @note  if previous actions where added with the QUEUE_END flag, the new action will be inserted **before** the already queued actions
     */
    QUEUE_END =     0b0100,
};


/**
 * @brief action struct
 * 
 * @tparam _ActionTag      action tag type
 * @tparam _Function    executor function prototype
 */
template<typename _ActionTag, typename _Callback>
struct action
{
    /**
     * @brief type of tag of this action
     */
    using type = _ActionTag;
    /**
     * @brief executor function type of this action
     */
    using function_prototype = _Callback;

    struct action_callback
    {
        _Callback   exec;
        ushort      flags;

        action_callback(const _Callback& func, ushort flags)
        : exec(func), flags(flags)
        {}
    };

    /**
     * @brief default constructor of action with empty function
     * 
     */   
    action() = default;

    /**
     * @brief overload operator= sets the function of the action to func
     * 
     * @param func  executor to be set for this action
     * @return this action
     */
    void    add_callback(const _Callback& func, ushort flags)//action<_ActionTag, _Callback>& operator=(const function& func)
    {
        auto it = executor_list.crbegin();
        for (; it != executor_list.crend(); ++it)
        {
            if (~it->flags & action_flag::QUEUE_END)
                break ;
        }
        
        this->executor_list.insert((it).base(), action_callback(func, flags));
    }

    template<typename ..._Args>
    void    execute(_Args&&... args)
    {
        for (action_callback& executor : executor_list)
        {
            if (executor.flags & action_flag::SKIP)
                continue;
                        
            if (executor.flags & action_flag::STOP_AFTER)
            {
                executor.exec(std::forward<_Args>(args)...);
                // if this object is destroyed by the execution, should return now
                return ;
            }
            
            executor.exec(std::forward<_Args>(args)...);
        }
    }

private:
    /**
     * @brief list of function to execute when executing
     */
    std::vector<action_callback>   executor_list;
};


/**
 * @brief   Generic definition of action_handler for every object that is not an action list
 * @details this class is declared for all action_handdler that does not specialize action_handler<actions_list<_Action...>>
 * 
 * @tparam _InvalidActionList
 */
template<typename _InvalidActionList>
class action_handler;

/**
 * @brief Represents an action handler containing a tuple of actions and defining a way to add and execute actions.
 * 
 * @tparam _Actions The types of actions.
 */
template<typename ..._Actions>
class action_handler<actions_list<_Actions...>>
{
    public:
        /**
         * @brief Construct a new action_handler object
         * 
         */
        action_handler() = default;

  
        /**
         * @brief statically get an action from the action handler, used by action_handler::execute (see specializations details for more informations)
         * 
         * @tparam _Atype tag type of action to get
         * @tparam _Aargs list of actions
         * 
         * @ref events::action
         */
        template <typename _Atype, typename ..._Aargs>
        struct get_action;

        /**
         * @brief specialization where _Aargs dont exist (i.e. is empty()), defines void as type, means that no actions were found for _Atype in _Aargs
         * 
         * @tparam _Atype tag type of action to get
         */
        template <typename _Atype>
        struct get_action<_Atype>
        {
            /**
             * @brief void type, action not found in list
             */
            using type = void;
        };

        /**
         * @brief   specialization where first argument in template argument pack matches with _Atype
         * 
         * @details this specialization occurs when first argument in template argument pack
         *          is an action with the same type as type, 
         *          it defines type as action of type _Atype and deduced executor type _Afunc 
         *          since this is the only action that could match is the action that we looked for.
         * 
         * @tparam _Atype tag type of action to get
         * @tparam _Afunc function exector type of action
         * @tparam _Aargs end list of actions (may be empty)
         */
        template <typename _Atype, typename _Afunc, typename... _Aargs>
        struct get_action<_Atype, action<_Atype, _Afunc>, _Aargs...>
        {
            /**
             * @brief type of action with specified _Atype and deduced function executor _Afunc
             */
            using type = action<_Atype, _Afunc>;
        };

        /**
         * @brief   specialization where first argument in template argument pack dont match with _Atype
         * @details this specialization occurs when first argument in template argument pack is not an action matching _Atype (_Aunknown may be anything),
         *          this means that there are other arguments left in the parameter pack to be processed, 
         *          to do that we can inherit get_action with _Atype and the leftover _Aargs thus throwing off _Aunkown,
         *          if was _Aargs is empty, the get_action<_Atype> specialization is used and type is defined as void,
         *          otherwise, either this class ends up being specified again until _Aargs becames empty,
         *          or it specializes get_action<_Atype, action<_Atype, _Afunc>, _Aargs> which means that the first action type matches _Atype.
         * 
         * @tparam _Atype       tag type of action to get
         * @tparam _Aunknown    any type, but not an action of type _Atype
         * @tparam _Aargs       leftover args in list, may be empty
         */
        template <typename _Atype, typename _Aunkown, typename... _Aargs>
        struct get_action<_Atype, _Aunkown, _Aargs...> : get_action<_Atype, _Aargs...>
        {};


        /**
         * @brief adds a task to do when an action of type _ActionType is executed
         *
         * @tparam _ActionType Type of the action (must be contained in action handler).
         * @tparam _Function   Task to perform when the action is executed.
         * 
         * @param function     The function to be executed.
         * @param flags        flags of the action (see action_flag)
         *
         * @ref action_flag
         */
        template<typename _ActionType, typename _Function>
        constexpr void    on(_Function function, ushort flags = action_flag::DEFAULT)
        {
            using action_type = typename get_action<_ActionType, _Actions...>::type;

            static_assert(!std::is_same<
                            action_type, 
                            void
                        >::value, "action type not found in actions list");

            static_assert(!std::is_same<
                            typename action_type::function_prototype,
                            _Function
                        >::value, "invalid function handler for action");

            std::get<action_type>(actions).add_callback(
                static_cast<typename action_type::function_prototype>(function), flags
            );
        }

    protected:
        /**
         * @brief executes all tasks of the action of type _ActionType
         * 
         * @tparam _ActionType type of the action (must be contained in action handler)
         * @tparam _Args       type of args forwarded to the function executor
         * @param args         args to forward to function executor
         */
        template<typename _ActionType, typename ..._Args>
        constexpr void    execute(_Args&&... args)
        {
            using action_type = typename get_action<_ActionType, _Actions...>::type;

            static_assert(!std::is_same<
                            action_type, 
                            void
                        >::value, "action type not found in actions list");

            std::get<action_type>(actions).execute(std::forward<_Args>(args)...);
        }

        /**
         * @brief   actions of the action_handler
         * 
         */
        actions_list<_Actions...> actions;
};


} // ******** namespace events


} // ******** namespace unisock
