#include "tcp/tcp.hpp"
#include "events/events.hpp"
#include "events/poll.hpp"

#include <tuple>

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
    tcp::server<client_data> s1 { "127.0.0.1", 9000 };
    tcp::server<> s2 { "127.0.0.1", 8000 };
    s1.listen();
    s2.listen();

    events::handler<> h {};
    h.subscribe(s1);
    h.subscribe(s2);

    events::poll(h);

    s1.close();
    s2.close();

    tcp::server<client_data> server { "127.0.0.1", 9000 };
    server.listen();

    events::handler<> handler {};

    events::async([&](){
        events::poll(handler);
    })->then([&](){
        events::poll(handler);
    })->then([&]{
        events::poll(handler);
    })->execute();
}