#include "tcp/server.hpp"

#include "message.h"

using namespace unisock;
using namespace unisock::tcp::common_actions;
using namespace unisock::tcp::server_actions;

struct client_entity {
    size_t total_recv_bytes = 0;
};

int main()
{
    using test_server = tcp::server_impl<
                            unisock::events::actions_list<>,
                            unisock::entity_model<>,
                            unisock::entity_model<client_entity>
                                        >;

    test_server server { };

    server.on<LISTEN>([](test_server::server_connection* conn){
        std::cout << "server listening on " << socket_address::get_ip(conn->address) << " on socket " << conn->get_socket() << std::endl
                  << "server will send + or - when connections are accepted/disconnected" << std::endl 
                  << "server will send r when an accepted connection received data" << std::endl;
    });

    server.on<RECEIVE>([](test_server::client_connection* conn, const char* message, size_t bytes){
        (void)message;

        write(1, "r", 1);
        conn->data.total_recv_bytes += bytes;
        // received too many bytes
        if (conn->data.total_recv_bytes > sizeof(MESSAGE_STR))
        {
            std::cout << std::endl << "error: connection received more bytes: total_recv_bytes: " << conn->data.total_recv_bytes << "" << std::endl;
        }
        // message is complete
        else if (conn->data.total_recv_bytes == sizeof(MESSAGE_STR))
        {
            write (1, "*", 1);
            // send back message str
            conn->send(MESSAGE_STR, sizeof(MESSAGE_STR));
        }
    });

    server.on<ACCEPT>([](test_server::client_connection* conn){
        (void)conn;
        write(1, "+", 1);
    });

    server.on<DISCONNECT>([](test_server::client_connection* conn){
        if (conn->data.total_recv_bytes != sizeof(MESSAGE_STR))
            std::cout << std::endl << "error: disconnected connection did not recv all bytes: total_recv_bytes: " << conn->data.total_recv_bytes << "" << std::endl;
        else
            write(1, "-", 1);
    });


    server.on<basic_actions::ERROR>([](const std::string& func, int err){
        std::cout << std::endl << "error: " << func << ": " << strerror(err) << std::endl;
    });


    server.listen("127.0.0.1", 8000);

    while (events::poll(server))
        ;
  
    server.close();
}