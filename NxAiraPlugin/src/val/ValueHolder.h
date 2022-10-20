#pragma once

#include <future>
#include <memory>
#include <mutex>

#include "./../lib/rxcpp/rx.hpp"

#include "./../val/result.h"

template<typename ValueType>
class ValueHolder {
private:
    typedef ValueType
            Message;
    typedef val::Result<Message>
            MessageType;
    typedef std::shared_future<MessageType>
            FutureMessageType;

/// future
private:
    std::shared_ptr<FutureMessageType> future;
public:
    std::unique_lock<std::mutex> acquire_future_lock() {
        static std::mutex mtx;
        return std::unique_lock<std::mutex>(mtx);
    }
private:
    void setFuture(std::shared_ptr<FutureMessageType> future) {
        acquire_future_lock();
        this->future = future;
    }
public:
    std::shared_ptr<FutureMessageType> getFuture() {
        acquire_future_lock();
        return future;
    }

/// subject
private:
    rxcpp::subjects::behavior<std::shared_ptr<Message>> subject;
    void onNext(Message o);
public:
    rxcpp::subjects::obserable<std::shared_ptr<Message>> getObservable();
};