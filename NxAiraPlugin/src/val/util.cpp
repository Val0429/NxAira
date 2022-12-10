#include "util.h"

#include <algorithm>

#define DEFAULT_LOG_LEVEL spdlog::level::info

bool toBool(std::string str) {
    std::transform(str.begin(), str.begin(), str.end(), ::tolower);
    return str == "true" || str == "1";
}

/// logger
std::shared_ptr<spdlog::logger> CreateLogger(const std::string& tag, const std::string& logLevel) {
    return CreateLogger(tag, GetLogLevel(logLevel));
}
std::shared_ptr<spdlog::logger> CreateLogger(const std::string& tag, const spdlog::level::level_enum& logLevel) {
    std::shared_ptr<spdlog::logger> logger = spdlog::stdout_color_mt(tag);
    logger->set_level(logLevel);
    return logger;
}

spdlog::level::level_enum GetLogLevel(const std::string& level) {
    auto llevel = ToLower(level);
    if (llevel == "trace") return spdlog::level::trace;
    else if (llevel == "debug") return spdlog::level::debug;
    else if (llevel == "info") return spdlog::level::info;
    else if (llevel == "warn") return spdlog::level::warn;
    else if (llevel == "err") return spdlog::level::err;
    else if (llevel == "critical") return spdlog::level::critical;
    else if (llevel == "off") return spdlog::level::off;
    /// default
    else return DEFAULT_LOG_LEVEL;
}

/// string
std::string ToLower(const std::string& input) {
    std::string rtn;
    rtn.resize(input.length());
    std::transform(input.begin(), input.end(), rtn.begin(), [](unsigned char c) { return std::tolower(c); });
    return rtn;
}
