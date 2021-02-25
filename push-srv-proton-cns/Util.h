#pragma once
#include <chrono>
#include <sys/time.h>
#include <time.h>
#include <string>
#include <iomanip>

long epochInMicro() {
    std::chrono::microseconds micro = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch());
    return micro.count();
}

std::string printNow() {
    auto timenow = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::string timeStamp = ctime(&timenow);
    timeStamp.pop_back();
    return timeStamp;
}
