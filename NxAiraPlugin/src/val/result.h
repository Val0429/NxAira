#pragma once
#ifndef RESULT_H
#define RESULT_H

#include <iostream>

#include <nx/sdk/i_string.h>
#include <nx/sdk/helpers/error.h>

namespace val {

using namespace nx::sdk;

/** Error codes used by Plugin methods. */
enum class ErrorCode: int
{
    noError = 0,
    networkError = -22,
    unauthorized = -1,
    internalError = -1000, //< Assertion-failure-like error.
    invalidParams = -1001, //< Method arguments are invalid.
    notImplemented = -21,
    otherError = -100
};


class Error
{
public:
    Error(ErrorCode errorCode, const std::string& errorMessage):
        m_errorCode(errorCode), m_errorMessage(errorMessage)
    {}

    bool isOk() const {
        return m_errorCode == ErrorCode::noError && m_errorMessage.size() == 0;
    }

    ErrorCode errorCode() const { return m_errorCode; }
    const std::string errorMessage() const { return m_errorMessage; }

    Error(Error&&) = default;
    Error(const Error&) = default;
    Error& operator=(const Error&) = default;

private:
    ErrorCode m_errorCode;
    std::string m_errorMessage;
};

template<typename Value>
class Result {
public:
    Result(): m_error(ErrorCode::noError, "") {}

    Result(Value value): m_error(ErrorCode::noError, ""), m_value(std::move(value)) {}

    Result(Error&& error): m_error(std::move(error)) {}

    Result& operator=(Error&& error) {
        m_error = std::move(error);
        m_value = Value{};
        return *this;
    }

    Result& operator=(const Result& result) {
        m_error = Error {result.m_error.errorCode(), result.m_error.errorMessage()};
        m_value = result.m_value;
        return *this;
    }

    Result& operator=(Value value) {
        m_error = Error{ErrorCode::noError, ""};
        m_value = value;
        return *this;
    }

    bool isOk() const { return m_error.isOk(); }

    const Error& error() const { return m_error; }
    Value value() const { return m_value; }

    /// string conversion
    std::string toString() const {
        return isOk() ?
            (std::string("[OK] ") + static_cast<std::string>(value())) :
            (std::string("[FAILED] [") + std::to_string((int)error().errorCode()) + "] " + error().errorMessage())
            ;
    }
    operator std::string() { return toString(); }
    friend std::ostream& operator<<(std::ostream& os, const Result& result) {
        os << result.toString();
        return os;
    }

private:
    Error m_error;
    Value m_value{};
};

val::Error error(val::ErrorCode errorCode, std::string errorMessage);

}

#endif