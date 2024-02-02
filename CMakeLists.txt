cmake_minimum_required(VERSION 3.11)

project(libunisock VERSION 0.0.0 LANGUAGES CXX)


#
#	C++
#
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options(-Wall -Wextra -Wpedantic -Werror)


#
#  CMake options
#
option(build-examples	"Builds the list of examples in ./examples" OFF)
option(use-ssl			"Enables tls features, requires OpenSSL library, paths to it must be defined below in the SSL section" OFF)

#
#	SSL
#
if (use-ssl)

	set(OPENSSL_ROOT_DIR	/usr/local/opt/openssl@3)
	set(OPENSSL_INCLUDE_DIR	/usr/local/opt/openssl@3/include)
	set(OPENSSL_LIBRARIES	/usr/local/opt/openssl@3/lib)
	find_package(OpenSSL REQUIRED)

	add_compile_definitions(ENABLE_TLS)

	include_directories(${OPENSSL_INCLUDE_DIR})
	list(APPEND LIB_LIST ${OPENSSL_LIBRARIES})

endif (use-ssl)



#
#	Library target
#

set(CMAKE_SOURCE_DIR ./src)

include_directories(./includes)

add_library(cppsockets STATIC
	# library source here
	src/net/inet_address.cpp
	src/net/socket.cpp

	# socket handlers
	src/events/handlers/poll.cpp
)



#
#	Examples targets
#

if (build-examples) 

	include_directories(./examples/includes)

	# Basic tcp client 
	add_executable(tcp-server
		examples/tcp-server/main.cpp
	)
	target_link_libraries(tcp-server cppsockets)

	# Basic tcp client
	add_executable(tcp-client
		examples/tcp-client/main.cpp
	)
	target_link_libraries(tcp-client cppsockets)


	# Basic udp server
	add_executable(udp-server
		examples/udp-server/main.cpp
	)
	target_link_libraries(udp-server cppsockets)

	# Basic udp client
	add_executable(udp-client
		examples/udp-client/main.cpp
	)
	target_link_libraries(udp-client cppsockets)


	# Basic raw listener
	add_executable(raw-listener
		examples/raw-listener/main.cpp
	)
	target_link_libraries(raw-listener cppsockets)

endif(build-examples)

