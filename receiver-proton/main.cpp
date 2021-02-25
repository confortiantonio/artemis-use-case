//export LD_LIBRARY_PATH=/sviluppo/repo-git/prometheus-cpp/_build/deploy/usr/local/lib64:/sviluppo/repo-git/qpid-proton-0.33.0/build/cpp
#include "Util.h"

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

using proton::receiver_options;
using proton::source_options;

long COUNTER_OVER_TEN_MILLI = 0;
long COUNTER_SUB_MILLI = 0;
long COUNTER_TEN_MILLI_SUB_MILLI = 0;
long INTERVAL = 10 * 1000 * 1000;


class Statistic {
private:
    long min = 999999;
    long max;
    long summary;
    long count = 0;
    long lastTime = 0;
    long subMilli = 0;

public:
    void recordEvent(long latency) {
        count++;
        summary += latency;
        if (latency < min)
            min = latency;
        if (latency > max)
            max = latency;
        if (latency < 1000)
            subMilli++;
    }
    void reset() {
        min = 999999;
        max = 0;
        summary = 0;
        count = 0;
        subMilli = 0;
    }

    bool check(long nowInMicro) {
        bool intervalTerminated = (nowInMicro - lastTime) > INTERVAL;
        if (intervalTerminated) {
            lastTime = nowInMicro;
        }
        return intervalTerminated;
    }

    void dump() {
        if (count != 0) {
            double percSubMilli = (double)subMilli / (double)count * 100;
            double avgLatency = (double)summary / (double)count;            
            std::cout 
                << printNow()
                << " stat total count " 
                << count
                << " summary "
                << summary
                << " subMilli "
                << subMilli
                << " percSubMilli "
                << percSubMilli
                << "% min "
                << min
                << " average "
                << avgLatency
                << " max "
                << max
                << std::endl;
        }
        else
            std::cout << "count = 0 no statistic available to dump" << std::endl;
    }

};




class hello_world : public proton::messaging_handler {
    std::string conn_url_;
    std::string addr_;
    Statistic stat_;

public:
    hello_world(const std::string& u, const std::string& a) :
        conn_url_(u), addr_(a) {}

    void on_container_start(proton::container& c)  {
        c.connect(conn_url_);
    }

    void on_connection_open(proton::connection& c)  {
        c.open_receiver(addr_);
        //c.open_sender(addr_);
    }

    void on_sendable(proton::sender& s)  {
        proton::message m("Hello World!");
        s.send(m);
        s.close();
    }

    void on_message(proton::delivery& d, proton::message& m)  {
        long now = epochInMicro();
        statToFile(m, now);
        //std::cout << "on_message body: " << m.body() << std::endl;
        //d.connection().close();
    }

    void statToFile(proton::message& m, long nowInMicro) {
        proton::scalar writeTime = m.properties().get("HEADER_WriteTimeMicro");
        if (writeTime.type() != proton::type_id::NULL_TYPE)
        {
            long writeTimeMicro = proton::get<long>(m.properties().get("HEADER_WriteTimeMicro"));


            stat_.recordEvent(nowInMicro - writeTimeMicro);
            //std::cout << "nowInMicro: " << nowInMicro << " writeTimeMicro: " << writeTimeMicro << std::endl;
            if (stat_.check(nowInMicro))
            {
                stat_.dump();
                stat_.reset();
            }
        }
    }
};



int main(int argc, char** argv) {
    try {
        std::string conn_url = argc > 1 ? argv[1] : "//127.0.0.1:61616";
        std::string addr = argc > 2 ? argv[2] : "ap.inv.quote";

        hello_world hw(conn_url, addr);
        proton::container(hw).run();

        return 0;
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }

    return 1;
}