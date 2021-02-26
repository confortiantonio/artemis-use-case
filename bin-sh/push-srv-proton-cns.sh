#!/bin/bash
export LD_LIBRARY_PATH=/sviluppo/repo-git/prometheus-cpp/_build/deploy/usr/local/lib64:/sviluppo/repo-git/qpid-proton-0.33.0/build/cpp

FILTER="bv.%.%.id"
URL="//127.0.0.1:61616"
ADDRESS="SELECTOR::QUEUE.SEL"

../push-srv-proton-cns/bin/x64/Debug/push-srv-proton-cns.out $URL  $ADDRESS "SUBJECT LIKE '$FILTER'"
