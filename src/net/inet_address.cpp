#include "net/inet_address.hpp"

using namespace unisock;

inet_address::inet_address()
{
	_ip_address = "unknown";
	_hostname = "unknown";
	_port = 0;

	// zeroing structs
	std::memset(&_address_4, 0, sizeof(_address_4));
	std::memset(&_address_6, 0, sizeof(_address_6));
}

 // constructs from IPv4 addr (i.e. on connection accepted)
inet_address::inet_address(const struct sockaddr_in& addr)
:	_address_4(addr),
	_address_family(AF_INET)
{
	_ip_address = _get_ip_of_addr4(_address_4.sin_addr);
	_hostname = _get_hostname_of_addr(reinterpret_cast<sockaddr*>(&_address_4), sizeof(addr));
	_port = ntohs(_address_4.sin_port);
	// zeroing ipv6 struct
	std::memset(&_address_6, 0, sizeof(_address_6));
}


// constructs from IPv6 addr (i.e. on connection accepted)
inet_address::inet_address(const struct sockaddr_in6& addr)
:	_address_6(addr),
	_address_family(AF_INET6)
{
	_ip_address = _get_ip_of_addr6(_address_6.sin6_addr);
	_hostname = _get_hostname_of_addr(reinterpret_cast<sockaddr*>(&_address_6), sizeof(addr));
	_port = ntohs(_address_6.sin6_port);
	// zeroing ipv4 struct
	std::memset(&_address_4, 0, sizeof(_address_4));
}


// Constructs a new sockaddr depending on family (i.e. for initialisation of struct sockaddr)
inet_address::inet_address(const std::string& hostname, const int port, const sa_family_t family)
:	_hostname(hostname),
	_port(static_cast<in_port_t>(port)),
	_address_family(family)
{
	if (_address_family == AF_INET)
	{
		_get_addr_by_hostname(reinterpret_cast<sockaddr*>(&_address_4), sizeof(_address_4), _ip_address, _hostname, AF_INET);
		_address_4.sin_port = htons(_port);
		// zeroing ipv6 struct
		std::memset(&_address_6, 0, sizeof(_address_6));
	}
	else if (_address_family == AF_INET6)
	{
		_get_addr_by_hostname(reinterpret_cast<sockaddr*>(&_address_6), sizeof(_address_6), _ip_address, _hostname, AF_INET6);
		_address_6.sin6_port = htons(_port);
		// zeroing ipv4 struct
		std::memset(&_address_4, 0, sizeof(_address_4));
	}
	else
		throw std::invalid_argument("unexpected family type, expected AF_INET or AF_INET6.");
}











 // constructs from IPv4 addr (i.e. on connection accepted)
void inet_address::setAddress(const struct sockaddr_in& addr)
{
	_address_4 = addr;
	_address_family =  AF_INET6;

	_ip_address = _get_ip_of_addr4(_address_4.sin_addr);
	_hostname = _get_hostname_of_addr(reinterpret_cast<sockaddr*>(&_address_4), sizeof(addr));
	_port = ntohs(_address_4.sin_port);
	// zeroing ipv6 struct
	std::memset(&_address_6, 0, sizeof(_address_6));
}


// constructs from IPv6 addr (i.e. on connection accepted)
void inet_address::setAddress(const struct sockaddr_in6& addr)
{
	_address_6 = addr;
	_address_family =  AF_INET6;
	_ip_address = _get_ip_of_addr6(_address_6.sin6_addr);
	_hostname = _get_hostname_of_addr(reinterpret_cast<sockaddr*>(&_address_6), sizeof(addr));
	_port = ntohs(_address_6.sin6_port);
	// zeroing ipv4 struct
	std::memset(&_address_4, 0, sizeof(_address_4));
}


// Constructs a new sockaddr depending on family (i.e. for initialisation of struct sockaddr)
void inet_address::setAddress(const std::string& hostname, const int port, const sa_family_t family)
{
	_hostname = hostname,
	_port = static_cast<in_port_t>(port),
	_address_family = family;

	if (_address_family == AF_INET)
	{
		_get_addr_by_hostname(reinterpret_cast<sockaddr*>(&_address_4), sizeof(_address_4), _ip_address, _hostname, AF_INET);
		_address_4.sin_port = htons(_port);
		// zeroing ipv6 struct
		std::memset(&_address_6, 0, sizeof(_address_6));
	}
	else if (_address_family == AF_INET6)
	{
		_get_addr_by_hostname(reinterpret_cast<sockaddr*>(&_address_6), sizeof(_address_6), _ip_address, _hostname, AF_INET6);
		_address_6.sin6_port = htons(_port);
		// zeroing ipv4 struct
		std::memset(&_address_4, 0, sizeof(_address_4));
	}
	else
		throw std::invalid_argument("unexpected family type, expected AF_INET or AF_INET6.");
}








struct sockaddr* inet_address::getAddress()
{
	switch (_address_family)
	{
	case AF_INET:
		return (reinterpret_cast<struct sockaddr*>(&_address_4));
		break;

	case AF_INET6:
		return (reinterpret_cast<struct sockaddr*>(&_address_6));
		break;
	default:
		return (nullptr);
		break;
	}
}



size_t		inet_address::getAddressSize() const
{
	switch (_address_family)
	{
	case AF_INET:
		return (sizeof(_address_4));
		break;

	case AF_INET6:
		return (sizeof(_address_6));
		break;
	default:
		return (0);
		break;
	}
}







std::string		inet_address::getHostname() const
{
	return (_hostname + "(" + _ip_address + ")");
}

std::string		inet_address::getIpAddress() const
{
	return (_ip_address);
}

int				inet_address::getPort() const
{
	return (_port);
}

sa_family_t		inet_address::getAddressFamily() const
{
	return (_address_family);
}

sockaddr_in		inet_address::getAddress4() const
{
	return (_address_4);
}

sockaddr_in6	inet_address::getAddress6() const
{
	return (_address_6);
}


void			inet_address::_get_addr_by_hostname(sockaddr *const addr, const socklen_t addr_len, std::string& ip_address, const std::string &hostname, const sa_family_t host_family)
{
	// try to get addr from hostname
	int         err = 0;
	addrinfo    hints;
	addrinfo*   res = nullptr;
	hints.ai_family = host_family;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = 0;
	hints.ai_canonname = NULL;
	hints.ai_addr = NULL;
	hints.ai_next = NULL;

	int n_retries = MAX_HOST_RESOLVE_RETRIES;
	do
	{
		err = getaddrinfo(hostname.c_str(), NULL, &hints, &res);
		if (err == 0)
			break ;
		// else if (err == EAI_AGAIN)
		// 	std::cout << "Failed to retrieve host ip address, retrying..." << std::endl;
		else if (err != EAI_AGAIN)
			throw std::logic_error(std::string("Cannot retrieve host ip address: ") + gai_strerror(err) + ": " + std::to_string(err));
	} while (--n_retries > 0);
	if (n_retries == 0 || res == nullptr)
		throw std::logic_error(std::string("Unable to retrieve host ip address: ") + gai_strerror(err) + ": " + std::to_string(err));

	if (res->ai_family == AF_INET)
		ip_address = _get_ip_of_addr4(((sockaddr_in*)(res->ai_addr))->sin_addr);
	else if (res->ai_family == AF_INET6)
		ip_address = _get_ip_of_addr6(((sockaddr_in6*)(res->ai_addr))->sin6_addr);

	if (addr_len < res->ai_addrlen)
		throw std::logic_error("Provided sockaddr is too small for ai_addrlen (expected " + std::to_string(res->ai_addrlen) + " got " + std::to_string(addr_len) + ")");
	std::memcpy(addr, res->ai_addr, res->ai_addrlen);
}



std::string		inet_address::_get_hostname_of_addr(const struct sockaddr* addr, const socklen_t addr_len)
{
	// try to find a hostname corresponding with addr struct
	char hostname_buffer[MAX_HOSTNAME_LEN + 1] = {0};
	int err = 0;

	int n_retries = MAX_HOST_RESOLVE_RETRIES;
	do
	{
		err = getnameinfo(addr, addr_len, hostname_buffer, MAX_HOSTNAME_LEN, nullptr, 0, 0);
		if (err == 0)
			break ;
		// else if (err == EAI_AGAIN)
		// 	std::cout << "Failed to retrieve host name, retrying..." << std::endl;
		else if (err != EAI_AGAIN)
			throw std::logic_error(std::string("Cannot retrieve host name: ") + gai_strerror(err) + ": " + std::to_string(err));
	} while (--n_retries > 0);
	if (n_retries == 0 || hostname_buffer[0] == 0)
		throw std::logic_error(std::string("Unable to retrieve host name: ") + gai_strerror(err) + ": " + std::to_string(err));

	return (std::string(hostname_buffer));
}



std::string		inet_address::_get_ip_of_addr4(const struct in_addr& addr)
{
	char    ip_addr_buffer[INET_ADDRSTRLEN + 1] = {0};
	if (inet_ntop(AF_INET, &addr, ip_addr_buffer, sizeof(sockaddr_in)) == NULL)
		throw std::logic_error(std::string("Invalid address type for IPv4 sockaddr_in: cannot parse ip string: ") + std::strerror(errno));
	return (std::string(ip_addr_buffer));
}



std::string		inet_address::_get_ip_of_addr6(const struct in6_addr& addr)
{
	char    ip_addr_buffer[INET6_ADDRSTRLEN + 1] = {0};
	if (inet_ntop(AF_INET6, &addr, ip_addr_buffer, sizeof(sockaddr_in6)) == NULL)
		throw std::logic_error(std::string("Invalid address type for IPv4 sockaddr_in: cannot parse ip string: ") + std::strerror(errno));
	return (std::string(ip_addr_buffer));
}

// UNISOCK_NAMESPACE_END