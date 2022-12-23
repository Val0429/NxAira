#pragma once
#ifndef VALUE_HOLDER_H
#define VALUE_HOLDER_H

#include <future>
#include <memory>
#include <mutex>

#include "./../lib/rxcpp/rx.hpp"

#include "./../val/result.h"

namespace val {

template<typename ValueType>
class ValueHolder {
public:
    ValueHolder<ValueType>() : subject(nullptr) {}

public:
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
    std::unique_lock<std::mutex> lock() {
        static std::mutex mtx;
        return std::unique_lock<std::mutex>(mtx);
    }
public:
    std::shared_ptr<FutureMessageType> setFuture(FutureMessageType&& future, bool noLock = false) {
        if (noLock) this->future = std::make_shared<FutureMessageType>(std::move(future));
        else { auto lk = lock(); this->future = std::make_shared<FutureMessageType>(std::move(future)); }
        return this->future;
    }
    std::shared_ptr<FutureMessageType> getFuture(bool noLock = false) {
        if (noLock) return future;
        auto lk = lock(); return future;
    }

/// subject
private:
    rxcpp::subjects::behavior<std::shared_ptr<MessageType>> subject;
public:
    void onNext(MessageType o) {
        subject.get_subscriber().on_next(
            std::make_shared<MessageType>(std::move(o))
            );
    }
    rxcpp::observable<std::shared_ptr<MessageType>> getObservable() {
        return subject.get_observable();
    }
    /// Val: thread-safe?
    std::shared_ptr<MessageType> getValue() const {
        return subject.get_value();
    }
};

}

#endif