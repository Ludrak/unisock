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
 * @addindex
 */
namespace _lib {


/**
 * @brief action struct
 * 
 * @tparam _Action      action tag type
 * @tparam _Function    executor function prototype
 */
template<typename _Action, typename _Function>
struct action
{
    /**
     * @brief type of tag of this action
     */
    using type = _Action;
    /**
     * @brief executor function type of this action
     */
    using function = _Function;

    /**
     * @brief empty function definition for empty constructor of action
     * 
     */
    struct _empty
    {
        /** operator() overload to pass struct as empty function */
        operator _Function()
        { return [](auto&&...){}; }
    } /** @brief empty struct */ empty_function;

    /**
     * @brief default constructor of action with empty function
     * 
     */   
    action()
    : execute(empty_function)
    {}

    /**
     * @brief overload operator= sets the function of the action to func
     * 
     * @param func  executor to be set for this action
     * @return this action
     */
    action<_Action, _Function>& operator=(const function& func)
    {
        this->execute = func;
        return (*this);
    }

    /**
     * @brief function to execute when executing
     * @note  this will be changed to std::queue<function> to queue multiple tasks per actions
     */
    function    execute;
};


/**
 * @brief Represents an action handler containing a tuple of actions and defining a way to add and execute actions.
 * 
 * @tparam _Actions The types of actions.
 */
template<typename ..._Actions>
class action_handler
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
         * @ref events::_lib::action
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
         */
        template<typename _ActionType, typename _Function>
        constexpr void    on(_Function function)
        {
            using action_type = typename get_action<_ActionType, _Actions...>::type;

            static_assert(!std::is_same<
                            action_type, 
                            void
                        >::value, "action type not found in actions list");

            static_assert(!std::is_same<
                            typename action_type::function,
                            _Function
                        >::value, "invalid function handler for action");

            std::get<action_type>(actions) = static_cast<typename action_type::function>(function);
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
         * @brief actions of the action_handler
         * 
         */
        std::tuple<_Actions...> actions;
};


/**
 * @brief expand tuple to parameter pack util
 * 
 * @tparam _Tuple   the tuple to expand into the class parameter pack
 * @tparam T        class template of some parameter pack 
 */
template<typename _Tuple, template<typename...> class T>
struct expand;

/**
 * @brief   specialization of expand where _Tuple is an std::tuple of _Args types
 *
 * @details this configuration allow to define a **type** of the type of the template class defined by T
 *          with infered template parameter pack from the first std::tuple argument deduced with _Args
 *  
 * @tparam T 
 * @tparam _Args 
 */
template<template<typename...> class T, typename... _Args>
struct expand<std::tuple<_Args...>, T>
{
    /**
     * @brief definition of type, where T is the specified template class and where _Args are deduced from the first std::tuple parameter pack in specialization
     */
    using type = T<_Args...>;
};


/**
 * @brief Represents an action handler class specialization with a tuple as the actions parameter pack.
 * 
 * @tparam _Args Type of the parameter pack in the tuple.
 */
template<typename ..._Args>
class action_handler<std::tuple<_Args...>> : public _lib::expand<std::tuple<_Args...>, action_handler>::type
{
    public:
        action_handler() = default;
};


} // ******** namespace _lib


} // ******** namespace events


} // ******** namespace unisock
