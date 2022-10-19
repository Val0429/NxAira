#pragma once

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
    Error(ErrorCode errorCode, const IString* errorMessage):
        m_errorCode(errorCode), m_errorMessage(errorMessage)
    {
    }

    bool isOk() const
    {
        return m_errorCode == ErrorCode::noError && m_errorMessage == nullptr;
    }

    ErrorCode errorCode() const { return m_errorCode; }
    const IString* errorMessage() const { return m_errorMessage; }

    Error(Error&&) = default;
    Error(const Error&) = default;
    Error& operator=(const Error&) = default;

private:
    ErrorCode m_errorCode;
    const IString* m_errorMessage;
};

template<typename Value>
class Result {
public:
    Result(): m_error(ErrorCode::noError, nullptr) {}

    Result(Value value): m_error(ErrorCode::noError, nullptr), m_value(std::move(value)) {}

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
        m_error = Error{ErrorCode::noError, nullptr};
        m_value = value;
        return *this;
    }

    bool isOk() const { return m_error.isOk(); }

    const Error& error() const { return m_error; }
    Value value() const { return m_value; }

    friend std::ostream& operator<<(std::ostream& os, const Result& result) {
        result.isOk() ?
            (os << "[ok] " << result.value())
            :
            (os << "[failed] [" << ((int)result.error().errorCode()) << "] " << result.error().errorMessage())
            ;
        return os;
    }

private:
    Error m_error;
    Value m_value{};
};

val::Error error(val::ErrorCode errorCode, std::string errorMessage);

}
