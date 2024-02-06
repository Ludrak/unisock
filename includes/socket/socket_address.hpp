/**
 * @file socket_address.hpp
 * @author ROBINO Luca
 * @brief   utils to deal with sockaddr structs, address families, and hostname resolving
 * 
 * @details defines inet_address to wrap sockaddr structs, and macros like address_type_of/address_family_of
 *          to easily like correct address structs with their families,
 *          also provides helpers like socket_addr::addrinfo, socket_addr::nameinfo for hostname resolving.
 * 
 * @version 1.0
 * @date 2024-02-04
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#pragma once

#include <netdb.h>
#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string>
#include <iostream>

/**
 * @addindex
 */
namespace unisock {

/**
 * @brief result of socket_address::addrinfo which uses getaddrinfo
 * 
 */
enum class  addrinfo_result {
    /**
     * @brief successfully retrieved address info
     */
    SUCCESS,

    /**
     * @brief error while trying to retrieve address info (error from getaddrinfo)
     */
    ERROR,

    /**
     * @brief address could not be resolved within the number of retries
     * 
     * @ref socket_address::MAX_HOST_RESOLVE_RETRIES
     */
    UNAVAILABLE,

    /**
     * @brief retrieved address is too big for sockaddr_storage 
     */
    ADDRESS_TOO_BIG
};


/**
 * @brief  result of socket_address::nameinfo which uses getnameinfo
 * 
 */
enum class  nameinfo_result {
    /**
     * @brief successfully retrieved name info
     * 
     */
    SUCCESS,

    /**
     * @brief error while trying to retrieve name info (error from getnameinfo)
     * 
     */
    ERROR,

    /**
     * @brief name could not be resolved within the number of retries
     * 
     * @ref socket_address::MAX_HOST_RESOLVE_RETRIES
     */
    UNAVAILABLE,

    /**
     * @brief name is too big for name buffer
     * 
     * @ref socket_address::IP_ADDRESS_BUFFER_SIZE 
     */
    NAME_TOO_BIG
};




/**
 * @brief   wrap of address families struct into one object, provide helpers for address structs operations
 * 
 * @details helper class to construct, store and manage sockaddr structs and all their variants, 
 *          provides helpers like socket_addr::addrinfo, socket_addr::nameinfo for hostname resolving.
 * @details 
 */
class socket_address
{
    public:
        /**
         * @brief maximum size for inet_ntop buffer for retrieving ip string
         * 
         * @ref   socket_address::get_ip
         */
        static constexpr size_t IP_ADDRESS_BUFFER_SIZE = 128;

        /**
         * @brief maximum size for getnameinfo buffer for retrieving host name
         * 
         * @ref   socket_address::addrinfo
         * @ref   socket_address::nameinfo
         */
        static constexpr size_t HOSTNAME_BUFFER_SIZE = 128;

        /**
         * @brief maximum number of retries for resolving a name or an address
         * 
         * @ref   socket_address::addrinfo
         * @ref   socket_address::nameinfo
         */
        static constexpr size_t MAX_HOST_RESOLVE_RETRIES = 3;

        /**
         * @brief default constructor of an empty address
         * 
         */
        socket_address();

        /**
         * @brief copy constructor 
         * 
         * @param other object to be copied
         */
        socket_address(const socket_address& other);

        /**
         * @brief constructor from sockaddr_storage
         * 
         * @param addr address struct to construct from
         */
        socket_address(const sockaddr_storage& addr);

        /**
         * @brief constructor from struct sockaddr
         * 
         * @param addr      address struct to construct from
         * @param addr_len  length of address struct
         */
        socket_address(const sockaddr* addr, size_t addr_len);

        // constructor for IPv4 / IPv6 addresses
        // ! this constructor can throw if host could not be resolved
        /**
         * @brief constructor for IPv4 / IPv6 addresses
         * 
         * @note            if a hostname is provided, this constructor can throw std::logic_error
         *                  because of getaddrinfo failing to retrieve address with provided name,
         *                  however if an ip address string is provided, addrinfo should not throw exceptions relative to hostname resolution
         * 
         * @param host      hostname to be resolved or ip address
         * @param port      port number
         * @param use_IPv6  use AF_INET6 protocol
         * 
         * @throw std::logic_error  if hostname could not be resolved (see socket_address::addrinfo return values)
         */
        socket_address(const std::string& host, in_port_t port, bool use_IPv6 = false);


        /**
         * @brief   operator= overload for socket_address, copies address of other
         * 
         * @param other object containing address to be copied
         * 
         * @return socket_address& reference to this object
         */
        socket_address&   operator=(const socket_address& other);

        /**
         * @brief returns the size of the address
         * 
         * @return size_t   value pointed by sa_len when reinterpreted as sockaddr
         */
        size_t              size() const;
 
        /**
         * @brief returns the familly of the address
         * 
         * @return sa_family_t value pointed by sa_family when reinterpreted as sockaddr
         */
        sa_family_t         family() const;



        /**
         * @brief get sockaddr struct type by address family
         * 
         * @tparam _AddressFamily address family value
         */
        template<sa_family_t _AddressFamily>
        struct address_type_of
            {   // default to sockaddr since all addresses families can be interpreted as sockaddr
                using type = sockaddr;
            };

        /**
         * @brief specialization of address_type_of for AF_INET
         */
        template<>
        struct address_type_of<AF_INET>
            { using type = sockaddr_in; };

        /**
         * @brief specialization of address_type_of for AF_INET6
         */        template<>
        struct address_type_of<AF_INET6>
            { using type = sockaddr_in6; };

        // TODO: implement more sockaddr structs

        /**
         * @brief type alias for address_type_of<_AddressFamily>::type
         * 
         * @tparam _AddressFamily address family value
         */
        template<sa_family_t _AddressFamily>
        using address_type_of_t = typename address_type_of<_AddressFamily>::type;


        /**
         * @brief address family by sockaddr struct type
         * 
         * @tparam _AddressType address struct type
         */
        template<typename _AddressType>
        struct address_family_of
            { 
                /**
                 * @brief default to AF_UNSPEC
                 */
                static constexpr sa_family_t value = AF_UNSPEC;
            };

        /**
         * @brief specialization of address_family_of for sockaddr_in
         */
        template<>
        struct address_family_of<sockaddr_in>
            {
                /**
                 * @brief value of AF_INET
                 */
                static constexpr sa_family_t value = AF_INET; 
            };

        /**
         * @brief specialization of address_family_of for sockaddr_in6
         */ 
        template<>
        struct address_family_of<sockaddr_in6>
            {
                /**
                 * @brief value of AF_INET
                 */
                static constexpr sa_family_t value = AF_INET6;
            };

        // TODO: implement more sockaddr structs

        /**
         * @brief type alias for address_family_of<_AddressType>::value
         * 
         * @tparam _AddressType address struct type
         */
        template<typename _AddressType>
        using address_family_of_v = typename address_family_of<_AddressType>::value;


        /**
         * @brief   return a reinterpreted pointer to inner address of type _AddressType
         * 
         * @details use address.to<sockaddr_type> to reinterpret the inner address struct as
         *          the wanted address struct type. this call checks for family in sa for all     
         *          types except sockaddr, this insures that if the pointer is not nullptr,
         *          the address family corresponds to the required struct type.
         * 
         * @tparam _AddressType type of address struct to reinterpret to
         * 
         * @return a reinterpreted pointer of type _AddressType or nullptr if address struct family dont match _AddressType
         */
        template<typename _AddressType>
        _AddressType*       to()
        {
            // checks for family
            if (address_family_of<_AddressType>::value != family())
                return (nullptr);
            
            return (reinterpret_cast<_AddressType*>(&_address));
        }

        /**
         * @brief   return a reinterpreted const pointer to innet address of type _AddressType
         * 
         * @details use address.to<sockaddr_type> to reinterpret the inner address struct as
         *          the wanted address struct type. this call checks for family in sa for all     
         *          types except sockaddr, this insures that if the pointer is not nullptr,
         *          the address family corresponds to the required struct type.
         * 
         * @tparam _AddressType type of address struct to reinterpret to
         * 
         * @return a reinterpreted pointer of type _AddressType or nullptr if address struct family dont match _AddressType
         */        template<typename _AddressType>
        const _AddressType* to() const
        {
            if (address_family_of<_AddressType>::value != family())
                return (nullptr);
            
            return (reinterpret_cast<const _AddressType*>(&_address));
        }

        /**
         * @brief   specialization for _AddressType=struct sockaddr
         * 
         * @details deletes the check for family, even if addr is all zeroed or incoherent,
         *          getnameinfo should return an appropriate error,
         *          it also applies to other callers like callers like bind, recvfrom, sendto, etc...
         * 
         * @tparam  _AddressType type of address struct to reinterpret to
         * 
         * @note    since all addresses families can be represented as sockaddr, this call never returns nullptr
         * 
         * @return  a reinterpreted pointer of type struct sockaddr
         */
        template<>
        struct sockaddr*        to<struct sockaddr>()
        {
            return (reinterpret_cast<struct sockaddr*>(&_address));
        }

        /**
         * @brief   specialization for _AddressType=struct sockaddr
         * 
         * @details deletes the check for family, even if addr is all zeroed or incoherent,
         *          getnameinfo should return an appropriate error,
         *          it also applies to other callers like callers like bind, recvfrom, sendto, etc...
         * 
         * @tparam  _AddressType type of address struct to reinterpret to
         * 
         * @note    since all addresses families can be represented as sockaddr, this call never returns nullptr
         * 
         * @return  a reinterpreted pointer of type struct sockaddr
         */        template<>
        const struct sockaddr*  to<struct sockaddr>() const
        {
            return (reinterpret_cast<const struct sockaddr*>(&_address));
        }




        /**
         * @brief       retrieves an ip address string of the address
         * 
         * @note        this call will throw is not used on IPv4/IPv6 addresses
         * 
         * @details     uses inet_ntop to parse the ip string from the address struct, may fail if buffer size is too small
         * 
         * @param address   socket_address to retrieve ip from
         * 
         * @return std::string  ip string 
         * 
         * @ref IP_ADDRESS_BUFFER_SIZE
         * 
         * @throws std::logic_error if address is not of inet protocol
         */
        static std::string      get_ip(const socket_address& address);


     
        /**
         * @brief       retrieves the address depending on **hostname** and **family**
         * 
         * @details     uses getaddrinfo to retrieve address informations of **hostname** with **family**,
         *              on success, the retrieved address structure is written to the storage referenced by **address** \n 
         *              this call might fail for many reasons that are described in addrinfo_result.
         * 
         * @param address   sockaddr_storage reference to write retrieved address to
         * @param hostname  hostname to be resolved
         * @param family    expected return address family (from getaddrinfo)
         * 
         * @return see results in addrinfo_result
         * 
         * @ref addrinfo_result
         * @ref MAX_HOST_RESOLVE_RETRIES
         */
        static addrinfo_result  addrinfo(struct sockaddr_storage& address, const std::string& hostname, const sa_family_t family = AF_INET);

        /**
         * @brief       overload of addrinfo to take in socket_address instead of sockaddr_storage
         * 
         * @param address   address reference to write retrieved address to
         * @param hostname  hostname to be resolved
         * @param family    expected return address family (from getaddrinfo)
         * 
         * @return see results in addrinfo_result
         * @ref MAX_HOST_RESOLVE_RETRIES
         */
        static addrinfo_result  addrinfo(socket_address& address, const std::string& hostname, const sa_family_t family = AF_INET);


        /**
         * @brief       retrieves the host name depending on **addr** and **family**
         * 
         * @details     uses getnameinfo to retrieve name information of **addr** with **family**,
         *              on success, the retrieved name string is written to the string referenced by **hostname** \n 
         *              this call might fail for many reasons that are described in nameinfo_result.
         * 
         * @param hostname      string reference to write retrieved name to
         * @param addr          address struct to resolve the name from
         * @param addr_len      length of address struct
         * 
         * @return see results in nameinfo_result
         * 
         * @ref nameinfo_result
         * @ref HOSTNAME_BUFFER_SIZE
         * @ref MAX_HOST_RESOLVE_RETRIES
         */
        static nameinfo_result  nameinfo(std::string& hostname, const sockaddr* addr, socklen_t addr_len);

        /**
         * @brief       overload of addrinfo to take in socket_address instead of address struct pointer and address length
         * 
         * @param hostname      string reference to write retrieved name to
         * @param address       address to resolve the name from
         * 
         * @return see results in nameinfo_result
         * 
         * @ref nameinfo_result
         * @ref HOSTNAME_BUFFER_SIZE
         * @ref MAX_HOST_RESOLVE_RETRIES
         */
        static nameinfo_result  nameinfo(std::string& hostname, const socket_address& address);





        /**
         * @brief   creates an socket_address from a hostname and family
         * @details uses socket_address::addrinfo to retrieve hostname with family, and creates an inet address from it,
         *          since addrinfo can fail this call can also fail for the reasons listed in addrinfo_result
         * 
         * @param hostname  the host name to resolve or ip address to create the address from
         * @param family    family of the address
         * 
         * @return  an inet address created from the resolved hostname and family
         *
         * @throws std::logic_error if socket_address::addrinfo failed to retrieve the hostname, if hostname is numeric ip this should not happen
         * 
         * @ref addrinfo_result
         */
        static socket_address from(const std::string& hostname, const sa_family_t family = AF_INET);


        /**
         * @brief   creates an socket_address from a hostname, family, and port
         * @details uses socket_address::addrinfo to retrieve hostname with family, and creates an inet address from it,
         *          since addrinfo can fail this call can also fail for the reasons listed in addrinfo_result
         * 
         * @param hostname  the host name to resolve or ip address to create the address from
         * @param port      the port to set the resolved on
         * @param family    family of the address
         * 
         * @return an inet address created from the resolved hostname, port and family
         *
         * @throws std::logic_error if socket_address::addrinfo failed to retrieve the hostname, or if retrieved address is not of IPv4/IPv6 address family, if hostname is numeric ip this should not happen
         * 
         * @ref addrinfo_result
         */
        static socket_address from(const std::string& hostname, const in_port_t port, const sa_family_t family = AF_INET);
        

        /**
         * @brief returns a string describing the address structure
         * 
         * @return std::string string describing the inner address structure
         */
        std::string         to_string() const;

    private:
        /**
         * @brief to_string helpers generic definition for each address family
         * 
         * @tparam _AddressFamily address family value
         * 
         * @return std::string string describing the inner address structure as value
         */
        template<sa_family_t _AddressFamily>
        std::string     _to_string() const;

        /**
         * @brief   specialization of _to_string for AF_INET
         * @tparam  AF_INET
         * @return  std::string string describing the inner address structure as sockaddr_in
         */
        template<>  std::string _to_string<AF_INET>() const;
        /**
         * @brief   specialization of _to_string for AF_INET6
         * @tparam  AF_INET6
         * @return  std::string string describing the inner address structure as sockaddr_in6
         */
        template<>  std::string _to_string<AF_INET6>() const;

    protected:
        /**
         * @brief   storage for address
         * @note    this type will later be replaced by byte_string to adjust address size depending on af
         */
        struct sockaddr_storage _address;
};

} // ******** namespace unisock