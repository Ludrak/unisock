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
        static constexpr size_t ADDRESS_SIZE = 32;

        static constexpr size_t IP_ADDRESS_BUFFER_SIZE = 255;

        static constexpr size_t HOSTNAME_BUFFER_SIZE = 255;
        static constexpr size_t MAX_HOST_RESOLVE_RETRIES = 3;

        // Constructs an empty address (i.e. before accepting client)
        inet_address();
        inet_address(const inet_address& other);

        inet_address(const char* address_data, const size_t address_size);

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
            switch (to_address<sockaddr>()->sa_family)
            {
            case AF_INET:
                return (&to_address<sockaddr_in>()->sin_addr);
                break;
            
            case AF_INET6:
                return (&to_address<sockaddr_in6>()->sin6_addr);
                break;
            }
            return (nullptr);
        }

        template<typename T>
        const T*            to_address() const
        {
            return (reinterpret_cast<const T*>(this->_address));
        }

    protected:
        char                _address[ADDRESS_SIZE];
};

UNISOCK_NAMESPACE_END