#pragma once

#include "namespaces.hpp"

#include <netdb.h>
#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string>

UNISOCK_NAMESPACE_START

#define MAX_HOST_RESOLVE_RETRIES 3
#define MAX_HOSTNAME_LEN         255

class inet_address
{
    public:
        inet_address();

        // constructs from IPv4 addr (i.e. on connection accepted)
        inet_address(const struct sockaddr_in& addr);

        // constructs from IPv6 addr (i.e. on connection accepted)
        inet_address(const struct sockaddr_in6& addr);

        // Constructs a new sockaddr depending on family (i.e. for initialisation of struct sockaddr)
        inet_address(const std::string& hostname, const int port, const sa_family_t family = AF_INET);


        // constructs from IPv4 addr (i.e. on connection accepted)
        void setAddress(const struct sockaddr_in& addr);

        // constructs from IPv6 addr (i.e. on connection accepted)
        void setAddress(const struct sockaddr_in6& addr);

        // Constructs a new sockaddr depending on family (i.e. for initialisation of struct sockaddr)
        void setAddress(const std::string& hostname, const int port, const sa_family_t family = AF_INET);


        std::string         getHostname() const;

        std::string         getIpAddress() const;
        int                 getPort() const;

        sa_family_t         getAddressFamily() const;

        sockaddr_in         getAddress4() const;
        sockaddr_in6        getAddress6() const;

        struct sockaddr*    getAddress();
        size_t              getAddressSize() const;
    

    private:
        void        _get_addr_by_hostname(sockaddr *const addr, const socklen_t addr_len, std::string& ip_address, const std::string &hostname, const sa_family_t host_family);
        std::string _get_hostname_of_addr(const struct sockaddr* addr, const socklen_t addr_len);
        std::string _get_ip_of_addr4(const struct in_addr& addr);
        std::string _get_ip_of_addr6(const struct in6_addr& addr);

    protected:
        // hostname, ip, and port, from sockaddr structs
        std::string _hostname;
        std::string _ip_address;
        in_port_t   _port;

        // actual addr structs, _address_family indicates the current one
        struct sockaddr_in  _address_4;
        struct sockaddr_in6 _address_6;
        sa_family_t         _address_family;
};

UNISOCK_NAMESPACE_END