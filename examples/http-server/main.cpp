
#include "unisock.hpp"

using namespace unisock;

class client_data
{
    public:
        int a;
        int b;
        int c;
};


int main()
{
    /* basic server */
    /* single server, single connection */
    {
        tcp::server<client_data> server { };
        
        using namespace tcp::actions;

        server.on<ERROR>(
            [](const std::string& function, int error) {
                std::cout << "error occurred: " << function << ": " << strerror(error) << std::endl;
            }
        );

        server.on<LISTEN>(
            [](tcp::connection<client_data>& connection) {
                std::cout << "listening on " << connection.data.address.getIpAddress() << ":" << connection.data.address.getPort() << std::endl;
            }
        );

        server.on<CLOSE>(
            []() {
                std::cout << "closing" << std::endl;
            }
        );

        server.on<CONNECT>(
            [](tcp::connection<client_data>& connection) {
                std::cout << "client connected from " << connection.data.address.getHostname() << " on socket " << connection.getSocket() << std::endl;
            }
        ); 

        server.on<DISCONNECT>(
            [](tcp::server<client_data>::connection& connection) {
                std::cout << "client disconnected from " << connection.data.address.getHostname() << " on socket " << connection.getSocket() << std::endl;
            }
        );

        server.on<MESSAGE>(
            [&server](auto& connection, const char* message, const size_t bytes) {
                std::cout << "received from " << connection.data.address.getHostname() << ": '" << std::string(message, bytes) << "'" <<  std::endl;
                server.send(connection, "Hey !");

                // auto* client = server.find([](auto& client) { return (client.data.address.getIpAddress() == "127.0.0.1"); });
                // if (client)
                //     server.send(*client, "you are on 127.0.0.1\n");
            }
        );

        // start listening, can throw for many reasons (bad ip, bad connection, unable to create socket, etc...)
        if (!server.listen("127.0.0.1", 9000))
            return (1);
    

        server.listen("127.0.0.1", 1000);
        server.listen("127.0.0.1", 2000);
        server.listen("127.0.0.1", 3000);
        server.listen("127.0.0.1", 4000);
        server.listen("127.0.0.1", 5000);

        // TODO: should poll until there is no socket left to poll in handler
        // while ( events::poll(server) )
        //     ;

        while ( 1 )
            events::poll(server);
        

        server.close();
    }


    /* multiple servers on one handler (with different client data types)*/
    /* optimized for few, but bigger servers, allows for different server types to be polled together (even with different protocol types)*/
    {
        events::handler handler {};

        tcp::server<client_data> s1 { handler };
        tcp::server<> s2 { handler };

        s1.listen("127.0.0.1", 9000);
        s2.listen("127.0.0.1", 8000);

        events::poll(handler);

        s1.close();
        s2.close();
    }

    /* one servers with multiple connections on one handler (implicit handler since not specified in constructor of server) */
    /* optimized for polling lots of connection from one server */
    {
        tcp::server<> server { };

        server.listen("127.0.0.1", 1000);
        server.listen("127.0.0.1", 2000);
        server.listen("127.0.0.1", 3000);
        server.listen("127.0.0.1", 4000);
        server.listen("127.0.0.1", 5000);
        
        events::poll(server);

        server.close();
    }

}