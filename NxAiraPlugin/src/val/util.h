#pragma once
#ifndef UTIL_H
#define UTIL_H

#include <iostream>
#include <memory>

#include "spdlog/fwd.h"
/// also need this for enum
#include "spdlog/common.h"

/// logger
std::shared_ptr<spdlog::logger> CreateLogger(const std::string& tag);
std::shared_ptr<spdlog::logger> CreateLogger(const std::string& tag, const std::string& logLevel);
std::shared_ptr<spdlog::logger> CreateLogger(const std::string& tag, const spdlog::level::level_enum& logLevel);
spdlog::level::level_enum GetLogLevel(const std::string& level);

/// string
std::string ToLower(const std::string& input);

#endif