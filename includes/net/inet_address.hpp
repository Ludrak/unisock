#pragma once

#include "namespaces.hpp"

#include <netdb.h>
#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string>

UNISOCK_NAMESPACE_START


class inet_address
{
    public:
        static constexpr size_t IP_ADDRESS_BUFFER_SIZE = 128;

        static constexpr size_t HOSTNAME_BUFFER_SIZE = 128;
        static constexpr size_t MAX_HOST_RESOLVE_RETRIES = 3;

        // Constructs an empty address (i.e. before accepting client)
        inet_address();
        inet_address(const inet_address& other);

        inet_address(const sockaddr_storage& addr);

        // Constructs a new sockaddr depending on family (i.e. for initialisation of struct sockaddr)
        inet_address(const std::string& hostname, const int port, const sa_family_t family = AF_INET);

        inet_address&   operator=(const inet_address& other);


        std::string         hostname() const;
        std::string         ip() const;

        int                 port() const;
        size_t              size() const;
        sa_family_t         family() const;

        const void*         in_addr() const
        {
            switch (to<sockaddr>()->sa_family)
            {
            case AF_INET:
                return (&to<sockaddr_in>()->sin_addr);
                break;
            
            case AF_INET6:
                return (&to<sockaddr_in6>()->sin6_addr);
                break;
            }
            return (nullptr);
        }

        template<typename T>
        const T*            to() const
        {
            return (reinterpret_cast<const T*>(&_address));
        }


        std::string         info() const
        {
            std::string info;

            switch (family())
            {
            case AF_INET:
                info = 
                    "sin_len:     " + std::to_string(size()) + "\n"
                +   "sin_family:  " + std::string("AF_INET\n")
                +   "sin_addr:    " + this->ip() + " (" + hostname() + ")\n"
                +   "sin_port:    " + std::to_string(port()) + "\n"
                +   "sin_zero:    ";
                
                for (size_t i = 0; i < sizeof(sockaddr_in::sin_zero); ++i)
                {
                    info += std::to_string(static_cast<int>((to<sockaddr_in>()->sin_zero)[i])) + " ";
                }
                info += "\n";
                return (info);

            case AF_INET6:
                return (
                    "sin6_len:    " + std::to_string(size()) + "\n"
                +   "sin6_family: " + std::string("AF_INET6") + "\n"
                +   "sin6_addr:   " + ip() + " (" + hostname() + ")\n"
                +   "sin6_port:   " + std::to_string(port()) + "\n"
                +   "sin6_flowinfo: " + std::to_string(static_cast<int>(to<sockaddr_in6>()->sin6_flowinfo)) + "\n"
                +   "sin6_scope_id: " + std::to_string(static_cast<int>(to<sockaddr_in6>()->sin6_scope_id)) + "\n"
                );

            case AF_UNIX:
                return ("");

            case AF_UNSPEC:
                return ("sa_family: AF_UNSPEC\nsa_len: " + std::to_string(size()) + "\n");

            default:
                return "unknown address family: " + std::to_string(static_cast<int>(family())) + "\n";
            }
        }

    protected:
        // TODO: later use byte_string to adjust address size with af
        struct sockaddr_storage _address;
};

UNISOCK_NAMESPACE_END