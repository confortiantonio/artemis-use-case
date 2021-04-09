#include <cstdio>
#include <iostream>
#include <vector>

#include "options.hpp"
#include <proton/connection.hpp>
#include <proton/container.hpp>
#include <proton/delivery.hpp>
#include <proton/message.hpp>
#include <proton/messaging_handler.hpp>
#include <proton/receiver_options.hpp>
#include <proton/source_options.hpp>
#include <proton/tracker.hpp>

#include <thread>
#include <chrono>

using proton::receiver_options;
using proton::source_options;







class hello_world : public proton::messaging_handler {
    std::string conn_url_;
    std::string addr_;

public:
    hello_world(const std::string& u, const std::string& a) :
        conn_url_(u), addr_(a) {}

    void on_container_start(proton::container& c) {
        c.connect(conn_url_);
    }

    void on_connection_open(proton::connection& c) {
        proton::receiver_options options;
        options.credit_window(1);
        c.open_receiver(addr_, options);
        //c.open_receiver(addr_);
    }

    void on_receiver_open(proton::receiver& rcv) override {
        std::cout << "RECEIVE: Opened receiver for source address '"
            << rcv.source().address() << "'\n";
    }

    void on_message(proton::delivery& d, proton::message& m) {
        static int count = 0;

        count++;

        std::string body = proton::get<std::string>(m.body());
        std::string messageKey = proton::get<std::string>(m.properties().get("MESSAGE_KEY"));
        long messageNumber = proton::get<long>(m.properties().get("MESSAGE_NUMBER"));

        printf("Message #%d Received: body<%s> key<%s> messageNumber<%d>\n", count, body.c_str(), messageKey.c_str(), messageNumber);
        //std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
};



int main(int argc, char** argv) {
    try {
        std::string conn_url = argc > 1 ? argv[1] : "//127.0.0.1:61616";
        std::string addr = argc > 2 ? argv[2] : "TICKERPLAN::LVQ";

        std::cout
            <<  " params:"
            << std::endl
            << " conn_url "
            << conn_url
            << std::endl
            << " addr "
            << addr
            << std::endl;

        hello_world hw(conn_url, addr);
        proton::container(hw).run();

        return 0;
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }

    return 1;
}