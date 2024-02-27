#include "common.h"

std::string trimWhitespace(std::string inputStr) {
    auto end = inputStr.find_last_not_of(' ');
    return inputStr.substr(0, end);
}
