#pragma once

#include <iostream>
#include <memory>

#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"

bool toBool(std::string str);

/// logger
std::shared_ptr<spdlog::logger> CreateLogger(const std::string& tag, const std::string& logLevel);
std::shared_ptr<spdlog::logger> CreateLogger(const std::string& tag, const spdlog::level::level_enum& logLevel);
spdlog::level::level_enum GetLogLevel(const std::string& level);

/// string
std::string ToLower(const std::string& input);