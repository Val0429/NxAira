#include "util.h"

#include <algorithm>
#include <map>


#include "spdlog/spdlog.h"
#include "spdlog/async.h"
#include "spdlog/sinks/basic_file_sink.h"

#define DEFAULT_LOG_LEVEL spdlog::level::info

/// logger
std::shared_ptr<spdlog::logger> CreateLogger(const std::string& tag) {
    return CreateLogger(tag, "info");
}
std::shared_ptr<spdlog::logger> CreateLogger(const std::string& tag, const std::string& logLevel) {
    return CreateLogger(tag, GetLogLevel(logLevel));
}

std::shared_ptr<spdlog::logger> CreateLogger(const std::string& tag, const spdlog::level::level_enum& logLevel) {
    static std::map<std::string,
        std::shared_ptr<spdlog::sinks::basic_file_sink_mt>
        > loggerMapping;
    std::string key = "logs/val-"+tag+".log";
    auto instance = loggerMapping.find(key);
    std::shared_ptr<spdlog::sinks::basic_file_sink_mt> o;

    if (instance != loggerMapping.end()) {
        o = instance->second;
    } else {
        o = std::make_shared<spdlog::sinks::basic_file_sink_mt>(key, true);
        loggerMapping[key] = o;
    }

    std::shared_ptr<spdlog::logger> logger = std::make_shared<spdlog::logger>(tag, o);
    spdlog::flush_every(std::chrono::seconds(1));
    logger->set_level(logLevel);
    return logger;
}
// std::shared_ptr<spdlog::logger> CreateLogger(const std::string& tag, const spdlog::level::level_enum& logLevel) {
//     std::shared_ptr<spdlog::logger> logger = spdlog::basic_logger_mt(tag, "logs/val-"+tag+".log", true);
//     spdlog::flush_every(std::chrono::seconds(1));
//     logger->set_level(logLevel);
//     return logger;
// }
// std::shared_ptr<spdlog::logger> CreateLogger(const std::string& tag, const spdlog::level::level_enum& logLevel) {
//     std::shared_ptr<spdlog::logger> logger = spdlog::basic_logger_mt<spdlog::async_factory>(tag, "logs/val-"+tag+".log", true);
//     spdlog::flush_every(std::chrono::seconds(1));
//     logger->set_level(logLevel);
//     return logger;
// }

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
