
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

    // /* basic server */
    // /* single server, single connection */
    // {
    //     events::handler handler {  };

    //     tcp::server<client_data> server { handler };

    //     server.listen("127.0.0.1", 9000);

    //     for (int i = 0; i < 10; ++i)
    //         events::poll(handler);

    //     server.close();
    // }


    // // /* multiple servers on one handler (with different client data types)*/
    // // /* optimized for few, but bigger servers, allows for different server types to be polled together (even with different protocol types)*/
    // {
    //     events::handler handler {};

    //     tcp::server<client_data> s1 { handler };
    //     tcp::server<> s2 { handler };

    //     s1.listen("127.0.0.1", 9000);
    //     s2.listen("127.0.0.1", 8000);

    //     events::poll(handler);

    //     s1.close();
    //     s2.close();
    // }

    // /* one servers with multiple connections on one handler (implicit handler since not specified in constructor of server) */
    // /* optimized for polling lots of connection from one server */
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


    // events::async([&](){
    // })->then([&](){
    //     events::poll(handler);
    // })->then([&]{
    //     events::poll(handler);
    // })->execute();
}