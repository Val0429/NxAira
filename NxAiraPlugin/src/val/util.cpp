#include "util.h"

#include <algorithm>

bool toBool(std::string str) {
    std::transform(str.begin(), str.begin(), str.end(), ::tolower);
    return str == "true" || str == "1";
}
