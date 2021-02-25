//export LD_LIBRARY_PATH=/sviluppo/repo-git/prometheus-cpp/_build/deploy/usr/local/lib64:/sviluppo/repo-git/qpid-proton-0.33.0/build/cpp

/*
* Caso d'uso: ricezione con filtri.
* 
*/


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

namespace {

    // Example custom function to configure an AMQP filter,
    // specifically an APACHE.ORG:SELECTOR
    // (http://www.amqp.org/specification/1.0/filters)

    void set_filter(proton::source_options& opts, const std::string& selector_str) {
        proton::source::filter_map map;
        proton::symbol filter_key("selector");
        proton::value filter_value;
        // The value is a specific AMQP "described type": binary string with symbolic descriptor
        proton::codec::encoder enc(filter_value);
        enc << proton::codec::start::described()
            << proton::symbol("apache.org:selector-filter:string")
            << selector_str
            << proton::codec::finish();
        // In our case, the map has this one element
        map.put(filter_key, filter_value);
        opts.filters(map);
    }
}


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
    std::string filter_;
    Statistic stat_;

public:
    hello_world(const std::string& u, const std::string& a, const std::string& f) :
        conn_url_(u), addr_(a), filter_(f) {}

    void on_container_start(proton::container& c) {
        c.connect(conn_url_);
    }

    void on_connection_open(proton::connection& c) {
        /*IMPOSTAZIONE FILTRO */
       proton::source_options opts;
        set_filter(opts, filter_); // example filter: "SUBJECT LIKE 'bv.quote.%.id'"
        c.open_receiver(addr_, proton::receiver_options().source(opts));
        
        /*NO FILTRO */
        //c.open_receiver(addr_);
    }

    void on_sendable(proton::sender& s) {
        proton::message m("Hello World!");
        s.send(m);
        s.close();
    }

    void on_message(proton::delivery& d, proton::message& m) {
        static int count = 0;

        count++;

        std::string body = proton::get<std::string>(m.body());
        std::string messageKey = proton::get<std::string>(m.properties().get("MESSAGE_KEY"));
        long messageNumber = proton::get<long>(m.properties().get("MESSAGE_NUMBER"));

        printf("Message #%d Received: body<%s> key<%s> messageNumber<%d>\n", count, body.c_str(), messageKey.c_str(), messageNumber);
        //statToFile(m);  
    }

    void statToFile(proton::message& m) {
        long nowInMicro = epochInMicro();
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
        std::string addr = argc > 2 ? argv[2] : "SELECTOR::QUEUE.SEL";
        std::string filter = argc > 3 ? argv[3] : "SUBJECT LIKE 'bv.quote.%.id'";

        std::cout
            << printNow() << " params:"
            << std::endl
            << " conn_url "
            << conn_url
            << std::endl
            << " addr "
            << addr
            << std::endl
            << " filter "
            << filter            
            << std::endl;

        hello_world hw(conn_url, addr, filter);
        proton::container(hw).run();

        return 0;
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }

    return 1;
}