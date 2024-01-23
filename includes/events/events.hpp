#pragma once

#include <thread>
#include <functional>
#include <vector>
#include <map>

#include "unisock.hpp"
#include "net/socket.hpp"

#include <iostream>

UNISOCK_NAMESPACE_START

UNISOCK_EVENTS_NAMESPACE_START


/* predefinition of handler */
template<handler_type _Handler>
class handler;




/* templates to be specialized for each SockHandler type*/

template<handler_type S>
class socket_data
{
};

template<handler_type _Handler>
socket_data<_Handler>   make_data(int socket);


template<handler_type _Handler>
void                    poll(const handler<_Handler>& handler);

/*******************************************************/


/* Generic types for different types of client data / sock handler*/

template<typename ..._Data>
class socket_container
{
    public:
        typedef unisock::socket<_Data...> socket_type;

        socket_container() {}
        virtual ~socket_container() {}
        
    protected:
        // void    attach_handler(const handler<unisock::DEFAULT_SOCKET_HANDLER>* handler)
        // {
        //     this->handler = handler;
        // }

        // void    detach_handler()
        // {
        //     this->handler = nullptr;
        // }

        std::map<int, socket_type>                  sockets;

        friend class handler<unisock::DEFAULT_SOCKET_HANDLER>;
};


template<handler_type _Handler = unisock::DEFAULT_SOCKET_HANDLER>
class handler
{
    public:
        handler() = default;
        ~handler()
        { 
        }

        template<typename ..._Data>
        handler(const socket_container<_Data...>& container)
        {
            this->subscribe(container);
        }

        template<typename _InputIterator>
        handler(_InputIterator begin, _InputIterator end,
        typename std::enable_if<std::is_base_of<handler, typename _InputIterator::value_type>::value>::type = 0)
        {
            std::for_each(begin, end, this->subscribe);
        }

        template<typename ..._Data>
        void    subscribe(const socket_container<_Data...>& container)
        {
            for (const auto& s : container.sockets)
            {
                sockets.push_back( make_data<_Handler>(s.second.getSocket()) );
            }
            // container.attach_handler(this);
        }
    
    private:
        std::vector<socket_data<_Handler>>  sockets;

        friend void unisock::events::poll(const handler<_Handler>&);
};





/* poll (poll.h) */
// template<class _InputIterator, class ..._Data>
// typename ::std::enable_if_t<unisock::DEFAULT_SOCKET_HANDLER == handler_type::POLL && std::is_base_of<isocket_container, typename _InputIterator::value_type>::value, void>
// poll(_InputIterator begin, _InputIterator end)
// {
//     // connect lists accordingly, poll, then connect back to original 
// };



// template<typename _SockHandler>
// void    poll(const typename std::vector<_SockHandler>::iterator& begin, const typename std::vector<_SockHandler>::iterator& end)
// {

// };


struct AsyncOperation
{
    std::vector<std::function<void(void)>> operations;

    AsyncOperation *then(std::function<void(void)> next_operation)
    {
        this->operations.push_back(next_operation);
        return (this);
    }

    void execute()
    {
        std::thread t([&]() {
            std::for_each(this->operations.begin(), this->operations.end(),
                [](const std::function<void(void)>& op) {
                    op();
            });
        });
    }

    AsyncOperation(std::function<void(void)> operation)
        : operations(1, operation)
    {}
};

AsyncOperation *async(std::function<void(void)> operation)
{
    return (new AsyncOperation(operation));
}


UNISOCK_EVENTS_NAMESPACE_END

UNISOCK_NAMESPACE_END
