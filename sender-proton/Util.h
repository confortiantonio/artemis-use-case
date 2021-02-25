#pragma once
#include <chrono>
#include <sys/time.h>
#include <time.h>
long epochInMicro() {
    std::chrono::microseconds micro = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch());
    return micro.count();
}
