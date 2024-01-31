#include "net/inet_address.hpp"

using namespace unisock;


inet_address::inet_address()
{
	memset(_address, 0, ADDRESS_SIZE);
}


inet_address::inet_address(const inet_address& other)
{
	memcpy(_address, other._address, ADDRESS_SIZE);
}


inet_address::inet_address(const char* address_data, const size_t address_size)
: inet_address()
{
	if (address_size > ADDRESS_SIZE)
		throw std::logic_error("address is too big for address buffer, maximum is " + std::to_string(ADDRESS_SIZE) + " bytes");

	memcpy(_address, address_data, address_size);
}


// Constructs a new sockaddr depending on family (i.e. for initialisation of struct sockaddr)
inet_address::inet_address(const std::string& hostname, const int port, const sa_family_t family)
: inet_address()
{
	// try to get addr from hostname
	int         err = 0;
	addrinfo    hints;
	addrinfo*   res = nullptr;
	hints.ai_family = family;
	hints.ai_socktype = 0;
	hints.ai_protocol = 0;
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
			
		else if (err != EAI_AGAIN)
			throw std::logic_error(std::string("Cannot retrieve host ip address: ") + gai_strerror(err) + ": " + std::to_string(err));
	} while (--n_retries > 0);
	if (n_retries == 0 || res == nullptr)
		throw std::logic_error(std::string("Unable to retrieve host ip address: ") + gai_strerror(err) + ": " + std::to_string(err));

	if (ADDRESS_SIZE < res->ai_addrlen)
		throw std::logic_error("address is too big for address buffer, maximum is " + std::to_string(ADDRESS_SIZE) + " bytes");

	if (res->ai_addrlen < sizeof(sockaddr_in))
		throw std::logic_error("address struct is too small to contain port, min size is sizeof(sockaddr_in) = 16bytes");

	reinterpret_cast<sockaddr_in*>(res->ai_addr)->sin_port = htons(port);

	std::memcpy(_address, res->ai_addr, res->ai_addrlen);

}


inet_address&   	inet_address::operator=(const inet_address& other)
{
	memcpy(_address, other._address, ADDRESS_SIZE);
	return (*this);
}


std::string         inet_address::hostname() const
{
	// try to find a hostname corresponding with addr struct
	char hostname_buffer[HOSTNAME_BUFFER_SIZE + 1] = {0};
	int err = 0;

	int n_retries = MAX_HOST_RESOLVE_RETRIES;
	do
	{
		err = getnameinfo(to_address<sockaddr>(), size(), hostname_buffer, HOSTNAME_BUFFER_SIZE, nullptr, 0, 0);

		if (err == 0)
			break ;
		else if (err != EAI_AGAIN)
			throw std::logic_error(std::string("Cannot retrieve host name: ") + gai_strerror(err) + ": " + std::to_string(err));

	} while (--n_retries > 0);

	if (n_retries == 0 || hostname_buffer[0] == 0)
		throw std::logic_error(std::string("Unable to retrieve host name: ") + gai_strerror(err) + ": " + std::to_string(err));

	return (std::string(hostname_buffer));
}


std::string         inet_address::ip() const
{
	char ip[IP_ADDRESS_BUFFER_SIZE] { 0 };

	if (nullptr == inet_ntop(family(), &to_address<sockaddr_in>()->sin_addr, ip, size()))
		return "";
	return std::string(ip);
}


int                 inet_address::port() const
{
	if (size() >= sizeof(sockaddr_in))
		return (ntohs(*reinterpret_cast<const int*>(_address + offsetof(sockaddr_in, sin_port))));
	return (0);
}


size_t              inet_address::size() const
{
	return (to_address<sockaddr>()->sa_len);
}


sa_family_t         inet_address::family() const
{
	return (*reinterpret_cast<const sa_family_t*>(_address + offsetof(sockaddr, sa_family)));
}
