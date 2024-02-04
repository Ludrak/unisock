#pragma once

#include "namespaces.hpp"

#include <netdb.h>
#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string>
#include <iostream>

UNISOCK_NAMESPACE_START

/* result of inet_address::from which uses getaddrinfo */
enum class  addrinfo_result {
    // successfully retrieved address info
    SUCCESS,

    // error while trying to retrieve address info
    ERROR,

    // address could not be resolved within the number of retries
    UNAVAILABLE,

    // address is too big for sockaddr_storage 
    // (should not be a problem with byte_string in next implementation)
    ADDRESS_TOO_BIG
};


/* result of inet_address::from which uses getnameinfo */
enum class  nameinfo_result {
    // successfully retrieved name info
    SUCCESS,

    // error while trying to retrieve name info
    ERROR,

    // name could not be resolved within the number of retries
    UNAVAILABLE,

    // name is too big for static name buffer of size inet_address::IP_ADDRESS_BUFFER_SIZE 
    NAME_TOO_BIG
};




class inet_address
{
    public:
        static constexpr size_t IP_ADDRESS_BUFFER_SIZE = 128;

        static constexpr size_t HOSTNAME_BUFFER_SIZE = 128;
        static constexpr size_t MAX_HOST_RESOLVE_RETRIES = 3;

        // default constructor of an empty address
        inet_address();

        // copy constructor 
        inet_address(const inet_address& other);

        // constructor from sockaddr_storage
        inet_address(const sockaddr_storage& addr);


        inet_address&   operator=(const inet_address& other);

        // returns the size of the address
        size_t              size() const;

        // returns the familly of the address
        sa_family_t         family() const;




        /* inet_address::to<...>                                                      */
        /* use address.to<sockaddr_type> to reinterpret the inner address struct as   */
        /* the wanted address struct type. this call checks for family in sa for all  */
        /* types except sockaddr, this insures that if the pointer is not nullptr,    */
        /* the address family corresponds to the required struct type.                */
    private:
        // generic definition for each sa_family_t address families
        template<sa_family_t _AddressFamily, typename _AddressType>
        _AddressType*       _to()
        {
            // checks for family
            if (_AddressFamily != family())
                return (nullptr);
            
            return (reinterpret_cast<_AddressType*>(&_address));
        }

        // const version of _to<_AddressFamily, _AddressType>()
        template<sa_family_t _AddressFamily, typename _AddressType>
        const _AddressType* _to() const
        {
            if (_AddressFamily != family())
                return (nullptr);
            
            return (reinterpret_cast<const _AddressType*>(&_address));
        }

        // specialization for _AddressType=struct sockaddr
        // deletes the check for family, even if addr is all zeroed or incoherent,
        // getnameinfo should return an appropriate error,
        // it also applies to other callers like callers like bind, recvfrom, sendto, etc...
        template<>
        struct sockaddr*        _to<AF_UNSPEC, struct sockaddr>()
        {
            return (reinterpret_cast<struct sockaddr*>(&_address));
        }

        // const version of _to<AF_UNSPEC, struct sockaddr>
        template<>
        const struct sockaddr*  _to<AF_UNSPEC, struct sockaddr>() const
        {
            return (reinterpret_cast<const struct sockaddr*>(&_address));
        }

    public:
        /* public bindings to inet_address::_to<...> handlers           */
        /* these wraps the correct AF_ number for each sockaddr type    */
        template<typename _AddressType>
        _AddressType*       to() { return (this->_to<AF_UNSPEC, _AddressType>()); }
        template<typename _AddressType>
        const _AddressType* to() const { return (this->_to<AF_UNSPEC, _AddressType>()); }

        // reinterpret as sockaddr_in*
        template<>
        sockaddr_in*        to<sockaddr_in>() { return (this->_to<AF_INET, sockaddr_in>()); }
        template<>
        const sockaddr_in*  to<sockaddr_in>() const { return (this->_to<AF_INET, sockaddr_in>()); }

        // reinterpret as sockaddr_in6*
        template<>
        sockaddr_in6*       to<sockaddr_in6>() { return (this->_to<AF_INET6, sockaddr_in6>()); }
        template<>
        const sockaddr_in6* to<sockaddr_in6>() const { return (this->_to<AF_INET6, sockaddr_in6>()); }




        // retrieves an ip address string of the address
        // throws if address is not of inet protocol
        static std::string      get_ip(const inet_address& address);


        // retrieves the address depending on hostname and family using getaddrinfo, 
        // and puts the result address into referenced address sockaddr_storage
        // returns: see addrinfo_result enum above
        static addrinfo_result  addrinfo(struct sockaddr_storage& address, const std::string& hostname, const sa_family_t family = AF_INET);

        // overload addrinfo for inet_address
        static addrinfo_result  addrinfo(inet_address& address, const std::string& hostname, const sa_family_t family = AF_INET);


        // retrieves the hostname of addr using getnameinfo, 
        // and puts the result hostname into referenced hostname string
        // returns: see nameinfo_result enum above
        static nameinfo_result  nameinfo(std::string& hostname, const sockaddr* addr, socklen_t addr_len);

        // overload nameinfo for inet_address
        static nameinfo_result  nameinfo(std::string& hostname, const inet_address& address);





        /* creates an inet address from a hostname and family              */
        /* may throw if getaddrinfo fails to retrieve the hostname, if     */
        /* hostname is numeric ip this should not happen                   */
        static inet_address from(const std::string& hostname, const sa_family_t family = AF_INET);


        /* creates an inet address from a hostname, port, family           */
        /* may throw if getaddrinfo fails to retrieve the hostname,        */
        /* or if retrieved address is not of IPv4/IPv6 address family,     */
        /* if hostname is numeric ip this should not happen                */
        static inet_address from(const std::string& hostname, const in_port_t port, const sa_family_t family = AF_INET);
        

        /* returns a string describing the address structure */
        std::string         to_string() const;

    private:
        template<sa_family_t _AddressFamily>
        std::string     _to_string() const;

        template<>  std::string _to_string<AF_INET>() const;
        template<>  std::string _to_string<AF_INET6>() const;

    protected:
        // TODO: later use byte_string to adjust address size with af
        struct sockaddr_storage _address;
};

UNISOCK_NAMESPACE_END