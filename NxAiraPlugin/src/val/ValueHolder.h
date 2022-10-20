#pragma once

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
    void setFuture(FutureMessageType&& future, bool noLock = false) {
        if (noLock) this->future = std::make_shared<FutureMessageType>(std::move(future));
        else { auto lk = lock(); this->future = std::make_shared<FutureMessageType>(std::move(future)); }
    }
    std::shared_ptr<FutureMessageType> getFuture(bool noLock = false) {
        if (noLock) return future;
        auto lk = lock(); return future;
    }

/// subject
private:
    rxcpp::subjects::behavior<std::shared_ptr<Message>> subject;
public:
    void onNext(Message o) {
        subject.on_next(std::move(o));
    }
    rxcpp::observable<std::shared_ptr<Message>> getObservable() {
        return subject.get_observable();
    }
};

}