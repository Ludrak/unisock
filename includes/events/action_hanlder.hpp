#pragma once

#include "namespaces.hpp"
#include <functional>
#include <type_traits>

UNISOCK_NAMESPACE_START

UNISOCK_EVENTS_NAMESPACE_START

UNISOCK_LIB_NAMESPACE_START



template<typename _Action, typename _Function>
struct action
{
    using type = _Action;
    using function = _Function;

    /* empty function for default constructor */
    struct _empty
    {
        operator _Function()
        { return [](auto&&...){}; }
    } empty_function;

    /* default constructor with empty function */
    action()
    : execute(empty_function)
    {}

    /* operator= to set function */
    action<_Action, _Function>& operator=(const function& func)
    {
        this->execute = func;
        return (*this);
    }

    function    execute;
};




/* action handler with parameter pack template */
template<typename ..._Actions>
class action_handler
{
    public:
        action_handler() = default;
  
        template <typename _Atype, typename ..._Aargs>
        struct get_action;

        // no more arguments in _Aargs, action was not found
        template <typename _Atype>
        struct get_action<_Atype>
        {
            using type = void;
        };

        // head type of actions is same as _Atype, define type of action in ::type
        template <typename _Atype, typename _Afunc, typename... _Aargs>
        struct get_action<_Atype, action<_Atype, _Afunc>, _Aargs...>
        {
            using type = action<_Atype, _Afunc>;
        };

        // head type of actions is different than _Atype (_Aunknown), strip head of actions
        template <typename _Atype, typename _Aunkown, typename... _Aargs>
        struct get_action<_Atype, _Aunkown, _Aargs...> : get_action<_Atype, _Aargs...>
        {};


        /* ovveride the last function defintion for _ActionType in actions */
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

        std::tuple<_Actions...> actions;
};


UNISOCK_LIB_NAMESPACE_START

/* expand tuple to parameter pack util */
template<typename _Tuple, template<typename...> class T>
struct expand;

template<template<typename...> class T, typename... _Args>
struct expand<std::tuple<_Args...>, T>
{
    using type = T<_Args...>;
};

UNISOCK_LIB_NAMESPACE_END


/* action handler with tuple template */
template<typename ..._Args>
class action_handler<std::tuple<_Args...>> : public _lib::expand<std::tuple<_Args...>, action_handler>::type
{
    public:
        action_handler() = default;
};




UNISOCK_LIB_NAMESPACE_END

UNISOCK_EVENTS_NAMESPACE_END

UNISOCK_NAMESPACE_END