#include "socket/socket_address.hpp"

using namespace unisock;


socket_address::socket_address()
{
	memset(&_address, 0, sizeof(_address));
}


socket_address::socket_address(const socket_address& other)
{
	memcpy(&_address, &other._address, sizeof(_address));
}


socket_address::socket_address(const sockaddr_storage& addr)
{
	memcpy(&_address, &addr, sizeof(_address));
}


// constructor from struct sockaddr
socket_address::socket_address(const sockaddr* addr, size_t addr_len)
: socket_address()
{
	memcpy(&_address, addr, addr_len);
}

// constructor for IPv4 / IPv6 addresses
socket_address::socket_address(const std::string& host, in_port_t port, bool use_IPv6)
: socket_address()
{
	addrinfo_result result = socket_address::addrinfo(_address, host, use_IPv6 ? AF_INET6 : AF_INET);
	if (result != addrinfo_result::SUCCESS)
		throw std::logic_error("unable to retrieve host address");
	this->to<sockaddr_in>()->sin_port = htons(port);
}



socket_address&   	socket_address::operator=(const socket_address& other)
{
	memcpy(&_address, &other._address, sizeof(_address));
	return (*this);
}



std::string 		socket_address::get_ip(const socket_address& address)
{
	char ip[IP_ADDRESS_BUFFER_SIZE] { 0 };
	// if (address.family() != AF_INET && address.family() != AF_INET6)
	//     throw (std::logic_error(std::string("getting ip address of invalid address family: ") + std::to_string(address.family())));
	switch (address.family())
	{
	case AF_INET:
		if (nullptr == inet_ntop(AF_INET, &address.to<sockaddr_in>()->sin_addr, ip, address.size()))
			return "[inet_ntop error: " + std::string(strerror(errno)) + "]";
		return std::string(ip);
	
	case AF_INET6:
		if (nullptr == inet_ntop(AF_INET6, &address.to<sockaddr_in6>()->sin6_addr, ip, address.size()))
			return "[inet_ntop error: " + std::string(strerror(errno)) + "]";
		return std::string(ip);
	}
	return ("[invalid address family]");            
}



size_t              socket_address::size() const
{
	return (to<sockaddr>()->sa_len);
}


sa_family_t         socket_address::family() const
{
	return (to<sockaddr>()->sa_family);
}




// retrieves the address depending on hostname and family using getaddrinfo, 
// and puts the result address into referenced address sockaddr_storage
// returns: see addrinfo_result enum above
addrinfo_result  socket_address::addrinfo(struct sockaddr_storage& address, const std::string& hostname, const sa_family_t family)
{
	// try to get addr from hostname
	int         err = 0;
	struct addrinfo    hints;
	struct addrinfo*   res = nullptr;
	std::memset(&hints, 0, sizeof(hints));
	hints.ai_family = family;

	int n_retries = MAX_HOST_RESOLVE_RETRIES;
	do
	{
		err = getaddrinfo(hostname.c_str(), NULL, &hints, &res);
		if (err == 0)
			break ;
			
		else if (err != EAI_AGAIN)
			return (addrinfo_result::ERROR);
	} while (--n_retries > 0);
	if (n_retries == 0 || res == nullptr)
		return (addrinfo_result::UNAVAILABLE);

	if (res->ai_addrlen > sizeof(address))
		return (addrinfo_result::ADDRESS_TOO_BIG);

	std::memcpy(&address, res->ai_addr, res->ai_addrlen);
	return (addrinfo_result::SUCCESS);
}

// overload addrinfo for socket_address
addrinfo_result  socket_address::addrinfo(socket_address& address, const std::string& hostname, const sa_family_t family)
{
	return (socket_address::addrinfo(address._address, hostname, family));
}



// retrieves the hostname of addr using getnameinfo, 
// and puts the result hostname into referenced hostname string
// returns: see nameinfo_result enum above
nameinfo_result  socket_address::nameinfo(std::string& hostname, const sockaddr* addr, socklen_t addr_len)
{
	// try to find a hostname corresponding with addr struct
	char hostname_buffer[NI_MAXHOST + 1] = {0};
	int err = 0;

	int n_retries = MAX_HOST_RESOLVE_RETRIES;
	do
	{
		// TODO: check NAMEREQD like indicated in man getnameinfo to differentiate between FQDN and numeric strings
		err = getnameinfo(addr, addr_len, hostname_buffer, HOSTNAME_BUFFER_SIZE, nullptr, 0, 0);

		if (err == 0)
			break ;
		else if (err != EAI_AGAIN)
			return (nameinfo_result::ERROR);
	} while (--n_retries > 0);

	if (n_retries == 0 || hostname_buffer[0] == 0)
		return (nameinfo_result::UNAVAILABLE);

	hostname = hostname_buffer;
	return (nameinfo_result::SUCCESS);
}

// overload nameinfo for socket_address
nameinfo_result  socket_address::nameinfo(std::string& hostname, const socket_address& address)
{
	return (socket_address::nameinfo(hostname, address.to<sockaddr>(), address.size()));
}






/* creates an inet address from a hostname and family              */
/* may throw if getaddrinfo fails to retrieve the hostname, if     */
/* hostname is numeric ip this should not happen                   */
socket_address socket_address::from(const std::string& hostname, const sa_family_t family)
{
	socket_address address {};
	addrinfo_result result = socket_address::addrinfo(address, hostname, family);
	if (result != addrinfo_result::SUCCESS)
		throw std::logic_error(std::string("error when trying to get address info: ") + strerror(errno));
	return (address);
}


/* creates an inet address from a hostname, port, family           */
/* may throw if getaddrinfo fails to retrieve the hostname,        */
/* or if retrieved address is not of IPv4/IPv6 address family,     */
/* if hostname is numeric ip this should not happen                */
socket_address socket_address::from(const std::string& hostname, const in_port_t port, const sa_family_t family)
{
	socket_address address {};
	addrinfo_result result = socket_address::addrinfo(address, hostname, family);
	if (result != addrinfo_result::SUCCESS)
		throw std::logic_error(std::string("error when trying to get address info: ") + strerror(errno));
	
	if (address.family() != AF_INET && address.family() != AF_INET6)
		throw std::logic_error("invalid inet address from constructor: required port but address is not of type AF_INET or AF_INET6");
	
	std::cout << "set port " <<std::endl;
	address.to<sockaddr_in>()->sin_port = htons(port);
	return (address);
}



/* returns a string describing the address structure */
std::string         socket_address::to_string() const
{
	std::string info;

	switch (family())
	{
	case AF_INET:
		return (this->_to_string<AF_INET>());

	case AF_INET6:
		return (this->_to_string<AF_INET6>());

	case AF_UNIX:
		return (this->_to_string<AF_UNIX>());

	case AF_UNSPEC:
		return ("sa_family: AF_UNSPEC\nsa_len: " + std::to_string(size()) + "\n");

	default:
		return "unknown address family: " + std::to_string(static_cast<int>(family())) + "\n";
	}
}


template<sa_family_t _AddressFamily>
std::string     socket_address::_to_string() const
{
	return ("unkown address family: " + std::to_string(family()));
}

template<>
std::string     socket_address::_to_string<AF_INET>() const
{
	std::string host;
	nameinfo_result result = socket_address::nameinfo(host, *this);
	if (result != nameinfo_result::SUCCESS)
	{
		host = "could not retrieve host: ";
		host += strerror(errno);
	}

	std::string info = 
		"sin_len:     " + std::to_string(size()) + "\n"
	+   "sin_family:  " + std::string("AF_INET\n")
	+   "sin_addr:    " + socket_address::get_ip(*this) + " (" + host + ")\n"
	+   "sin_port:    " + std::to_string(to<sockaddr_in>()->sin_port) + "\n"
	+   "sin_zero:    ";
	
	for (size_t i = 0; i < sizeof(sockaddr_in::sin_zero); ++i)
	{
		info += std::to_string(static_cast<int>((to<sockaddr_in>()->sin_zero)[i])) + " ";
	}
	info += "\n";
	return (info);
}

template<>
std::string     socket_address::_to_string<AF_INET6>() const
{
	std::string host;
	nameinfo_result result = socket_address::nameinfo(host, *this);
	if (result != nameinfo_result::SUCCESS)
	{
		host = "could not retrieve host: ";
		host += strerror(errno);
	}
	
	return (
		"sin6_len:    " + std::to_string(size()) + "\n"
	+   "sin6_family: " + std::string("AF_INET6") + "\n"
	+   "sin6_addr:   " + socket_address::get_ip(*this) + " (" + host + ")\n"
	+   "sin6_port:   " + std::to_string(to<sockaddr_in>()->sin_port) + "\n"
	+   "sin6_flowinfo: " + std::to_string(static_cast<int>(to<sockaddr_in6>()->sin6_flowinfo)) + "\n"
	+   "sin6_scope_id: " + std::to_string(static_cast<int>(to<sockaddr_in6>()->sin6_scope_id)) + "\n"
	);
}