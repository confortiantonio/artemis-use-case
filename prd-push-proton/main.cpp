//export LD_LIBRARY_PATH=/sviluppo/repo-git/prometheus-cpp/_build/deploy/usr/local/lib64:/sviluppo/repo-git/qpid-proton-0.33.0/build/cpp
#include "options.hpp"
#include "Util.h"

#include <proton/container.hpp>
#include <proton/connection.hpp>
#include <proton/message.hpp>
#include <proton/messaging_handler.hpp>
#include <proton/sender.hpp>
#include <proton/tracker.hpp>
#include <proton/work_queue.hpp>

#include <iostream>


static const std::string BODY = "300111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111";
//static const std::string BODY = "900111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111300111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111300111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111";
// Send messages at a constant rate one per interval. cancel after a timeout.
class scheduled_sender : public proton::messaging_handler {
private:
    std::string url;
    proton::sender sender;
    proton::duration interval, timeout;
    proton::work_queue* work_queue;
    bool ready, canceled;

public:

    scheduled_sender(const std::string& s, double d, double t) :
        url(s),
        interval(int(d /* * proton::duration::SECOND.milliseconds()*/)), // Send interval.
        timeout(int(t* proton::duration::SECOND.milliseconds())), // Cancel after timeout.
        work_queue(0),
        ready(true),            // Ready to send.
        canceled(false)         // Canceled.
    {}

    // The awkward looking double lambda is necessary because the scheduled lambdas run in the container context
    // and must arrange lambdas for send and close to happen in the connection context.
    void on_container_start(proton::container& c) {
        c.open_sender(url);
    }

    void on_sender_open(proton::sender& s) {
        sender = s;
        work_queue = &s.work_queue();
        // Call this->cancel after timeout.
        s.container().schedule(timeout, [this]() { this->work_queue->add([this]() { this->cancel(); }); });
        // Start regular ticks every interval.
        s.container().schedule(interval, [this]() { this->work_queue->add([this]() { this->tick(); }); });
    }

    void cancel() {
        canceled = true;
        sender.connection().close();
    }

    void tick() {
        // Schedule the next tick unless we have been cancelled.
        if (!canceled)
            sender.container().schedule(interval, [this]() { this->work_queue->add([this]() { this->tick(); }); });
        if (sender.credit() > 0) // Only send if we have credit
            send();
        else
            ready = true;  // Set the ready flag, send as soon as we get credit.
    }

    void on_sendable(proton::sender&) {
        if (ready)              // We have been ticked since the last send.
            send();
    }

    void send() {
        //std::cout << "send" << std::endl;
        proton::message message = proton::message(BODY);
        proton::scalar writeTime = proton::scalar(epochInMicro());
        message.properties().put("HEADER_WriteTimeMicro", writeTime);
        sender.send(message);
        ready = false;
    }
};


int main(int argc, char** argv) {
    std::string address("127.0.0.1:61616/ap.inv.quote");
    double interval = 1.0;
    double timeout = 5.0;

    example::options opts(argc, argv);

    opts.add_value(address, 'a', "address", "connect and send to URL", "URL");
    opts.add_value(interval, 'i', "interval", "send a message every INTERVAL seconds", "INTERVAL");
    opts.add_value(timeout, 't', "timeout", "stop after T seconds", "T");

    try {
        opts.parse();
        scheduled_sender h(address, interval, timeout);
        proton::container(h).run();
        return 0;
    }
    catch (const example::bad_option& e) {
        std::cout << opts << std::endl << e.what() << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }

    return 1;
}