/*
* export LD_LIBRARY_PATH=/usr/local/lib
*/

/* Caso d'uso relativo a un Push Server usando code temporanee

broker.xml usato in /sviluppo/amq-broker/instances/amq01/etc:
*
        <address-setting match="PUSHSRV.MULTICAST">
          <retroactive-message-count>1000000</retroactive-message-count>
          <default-last-value-key>MESSAGE_KEY</default-last-value-key>
        </address-setting>

*/


/*
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

 // START SNIPPET: demo

#include <activemq/library/ActiveMQCPP.h>
#include <decaf/lang/Thread.h>
#include <decaf/lang/Runnable.h>
#include <decaf/util/concurrent/CountDownLatch.h>
#include <decaf/lang/Integer.h>
#include <decaf/lang/Long.h>
#include <decaf/lang/System.h>
#include <activemq/core/ActiveMQConnectionFactory.h>
#include <activemq/util/Config.h>
#include <cms/Connection.h>
#include <cms/Session.h>
#include <cms/TextMessage.h>
#include <cms/BytesMessage.h>
#include <cms/MapMessage.h>
#include <cms/ExceptionListener.h>
#include <cms/MessageListener.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <memory>
#include <thread>
#include <chrono>

using namespace activemq::core;
using namespace decaf::util::concurrent;
using namespace decaf::util;
using namespace decaf::lang;
using namespace cms;
using namespace std;

class HelloWorldProducer : public Runnable {
private:

    Connection* connection;
    Session* session;
    Destination* destination;
    MessageProducer* producer;
    int numMessages;
    bool useTopic;
    bool sessionTransacted;
    std::string brokerURI;

private:

    HelloWorldProducer(const HelloWorldProducer&);
    HelloWorldProducer& operator=(const HelloWorldProducer&);

public:

    HelloWorldProducer(const std::string& brokerURI, int numMessages, bool useTopic = false, bool sessionTransacted = false) :
        connection(NULL),
        session(NULL),
        destination(NULL),
        producer(NULL),
        numMessages(numMessages),
        useTopic(useTopic),
        sessionTransacted(sessionTransacted),
        brokerURI(brokerURI) {
    }

    virtual ~HelloWorldProducer() {
        cleanup();
    }

    void close() {
        this->cleanup();
    }

    virtual void run() {

        try {

            // Create a ConnectionFactory
            auto_ptr<ConnectionFactory> connectionFactory(
                ConnectionFactory::createCMSConnectionFactory(brokerURI));

            // Create a Connection
            connection = connectionFactory->createConnection();
            connection->start();

            // Create a Session
            if (this->sessionTransacted) {
                session = connection->createSession(Session::SESSION_TRANSACTED);
            }
            else {
                session = connection->createSession(Session::AUTO_ACKNOWLEDGE);
            }

            // Create the destination (Topic or Queue)
            if (useTopic) {
                destination = session->createTopic("PUSHSRV.MULTICAST");
            }
            else {
                destination = session->createQueue("TEST.ANYCAST");
            }

            // Create a MessageProducer from the Session to the Topic or Queue
            producer = session->createProducer(destination);
            producer->setDeliveryMode(DeliveryMode::NON_PERSISTENT);

            // Create the Thread Id String
            string threadIdStr = Long::toString(Thread::currentThread()->getId());

            // Create a messages
            string text = (string)"Hello world! from thread " + threadIdStr;

            std::auto_ptr<TextMessage> message(session->createTextMessage(text));
            message->setLongProperty("MESSAGE_NUMBER", 0);
            message->setStringProperty("MESSAGE_KEY", "0");
            printf("Sent message #%d from thread %s key<%s> messageNumber<%d>\n", 1, threadIdStr.c_str(), "0", 0);
            producer->send(message.get());

            for (int ix = 0; ix < numMessages; ++ix) {
                std::auto_ptr<TextMessage> message(session->createTextMessage(text));
                message->setLongProperty("MESSAGE_NUMBER", ix);
                message->setStringProperty("MESSAGE_KEY", "1000");
                //printf("Sent message #%d from thread %s\n", ix, threadIdStr.c_str());
                printf("Sent message #%d from thread %s key<%s> messageNumber<%d>\n", ix + 1, threadIdStr.c_str(), "1000", ix);
                producer->send(message.get());
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }

        }
        catch (CMSException& e) {
            e.printStackTrace();
        }
    }

private:

    void cleanup() {

        if (connection != NULL) {
            try {
                connection->close();
            }
            catch (cms::CMSException& ex) {
                ex.printStackTrace();
            }
        }

        // Destroy resources.
        try {
            delete destination;
            destination = NULL;
            delete producer;
            producer = NULL;
            delete session;
            session = NULL;
            delete connection;
            connection = NULL;
        }
        catch (CMSException& e) {
            e.printStackTrace();
        }
    }
};


int main(int argc, char* argv) {

    activemq::library::ActiveMQCPP::initializeLibrary();
    {
        std::cout << "=====================================================\n";
        std::cout << "Starting the example:" << std::endl;
        std::cout << "-----------------------------------------------------\n";


        // Set the URI to point to the IP Address of your broker.
        // add any optional params to the url to enable things like
        // tightMarshalling or tcp logging etc.  See the CMS web site for
        // a full list of configuration options.
        //
        //  http://activemq.apache.org/cms/
        //
        // Wire Format Options:
        // =========================
        // Use either stomp or openwire, the default ports are different for each
        //
        // Examples:
        //    tcp://127.0.0.1:61616                      default to openwire
        //    tcp://127.0.0.1:61613?wireFormat=stomp     use stomp instead
        //
        // SSL:
        // =========================
        // To use SSL you need to specify the location of the trusted Root CA or the
        // certificate for the broker you want to connect to.  Using the Root CA allows
        // you to use failover with multiple servers all using certificates signed by
        // the trusted root.  If using client authentication you also need to specify
        // the location of the client Certificate.
        //
        //     System::setProperty( "decaf.net.ssl.keyStore", "<path>/client.pem" );
        //     System::setProperty( "decaf.net.ssl.keyStorePassword", "password" );
        //     System::setProperty( "decaf.net.ssl.trustStore", "<path>/rootCA.pem" );
        //
        // The you just specify the ssl transport in the URI, for example:
        //
        //     ssl://localhost:61617
        //
        std::string brokerURI =
            "failover:(tcp://localhost:61616)";

        //============================================================
        // set to true to use topics instead of queues
        // Note in the code above that this causes createTopic or
        // createQueue to be used in both consumer an producer.
        //============================================================
        bool useTopics = true;
        bool sessionTransacted = false;
        int numMessages = 20000000;

        long long startTime = System::currentTimeMillis();

        HelloWorldProducer producer(brokerURI, numMessages, useTopics);

        // Start the producer thread.
        Thread producerThread(&producer);
        producerThread.start();

        // Wait for the threads to complete.
        producerThread.join();

        long long endTime = System::currentTimeMillis();
        double totalTime = (double)(endTime - startTime) / 1000.0;

        producer.close();

        std::cout << "Time to completion = " << totalTime << " seconds." << std::endl;
        std::cout << "-----------------------------------------------------\n";
        std::cout << "Finished with the example." << std::endl;
        std::cout << "=====================================================\n";

    }
    activemq::library::ActiveMQCPP::shutdownLibrary();
}

// END SNIPPET: demo