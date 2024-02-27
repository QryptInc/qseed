#include "common.h"

#include <chrono>
#include <iomanip>
#include <sstream>

std::string trimWhitespace(std::string inputStr) {
    auto end = inputStr.find_last_not_of(' ');
    return inputStr.substr(0, end + 1);
}

std::string getTimestamp() {

    std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
    std::time_t time_t_now = std::chrono::system_clock::to_time_t(now);
    auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch() % std::chrono::seconds(1)).count();

    std::stringstream ss;
    ss << std::put_time(gmtime(&time_t_now), "%FT%T") << '.' << std::setfill('0') << std::setw(3) << millis << 'Z';
    return ss.str();

}
