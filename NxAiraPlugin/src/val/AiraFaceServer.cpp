#include "AiraFaceServer.hpp"

#include <sstream>
#include <iostream>

#include <nx/kit/json.h>
#include <nx/kit/debug.h>

#include "./../lib/HTTPRequest.hpp"

#define PROTOCOL "http"

namespace val {

AiraFaceServer::AiraFaceServer() :
token_maintain(&AiraFaceServer::maintain_handler, this)
{
    token_maintain.detach();
}

/* #region LOGIN */
std::shared_ptr<std::shared_future<std::string>> AiraFaceServer::login(
        std::string hostname, std::string port,
        std::string username, std::string password
) {
    auto lk = acquire_login_lock();

    this->hostname = hostname;
    this->port = port;
    this->username = username;
    this->password = password;

    /// concat fullUrl
    const std::string uri = "/generatetoken";
    std::string url = baseUrl(uri);

    /// send request
    std::shared_future<std::string> result = std::async(std::launch::async, [this, url]() {

        std::string jsonString, err;
        try {
            http::Request request {url};

            const auto response = request.send("POST",
R"json(
{
    "username": ")json" + this->username + R"json(",
    "password": ")json" + this->password + R"json("
}
)json"
            , {
                {"Content-Type", "application/json"}
            }, std::chrono::milliseconds(1000));

            jsonString = std::string {response.body.begin(), response.body.end()};
            nx::kit::Json json = nx::kit::Json::parse(jsonString, err);
            std::string message = json["message"].string_value();
            if (message != "ok") {
                throw message;
            }
            this->logined = true;
            return json["token"].string_value();

        } catch(const std::exception& ex) {
            NX_DEBUG_STREAM << "[AiraFaceServer] failed to login, ex:" << ex.what() << ", err:" << err << ", json:" << jsonString NX_DEBUG_ENDL;
            return std::string();
        }

    });
    
    shared_token = std::make_shared<std::shared_future<std::string>>(std::move(result));

    return shared_token;
}

std::shared_ptr<std::shared_future<std::string>> AiraFaceServer::login(bool force = false) {
    if (force) {
        return this->login(
            this->hostname, this->port,
            this->username, this->password
        );
    }
    auto lk = acquire_login_lock();
    return shared_token;
}

bool AiraFaceServer::getLogined() {
    return logined;
}

std::unique_lock<std::mutex> AiraFaceServer::acquire_login_lock() {
    static std::mutex mtx_login;
    return std::unique_lock<std::mutex>(mtx_login);
}
/* #endregion LOGIN */

/* #region MAINTAIN */
std::future<std::string> AiraFaceServer::maintain() {
    /// concat fullUrl
    const std::string uri = "/maintaintoken";
    std::string url = baseUrl(uri);

    /// send request
    std::future<std::string> result = std::async(std::launch::async, [this, url]() {
        auto token_future = this->login();
        auto status = token_future->wait_for(std::chrono::milliseconds(1000));
        if (status == std::future_status::timeout) {
            NX_DEBUG_STREAM << "[AiraFaceServer] maintain token timeout, while login" NX_DEBUG_ENDL;
            return std::string();
        }
        std::string token = token_future->get();
        if (token.size() == 0) return std::string();

        /// actual request
        std::string jsonString, err;
        try {
            http::Request request {url};

            const auto response = request.send("POST",
R"json(
{
    "token": ")json" + token + R"json("
}
)json"
            , {
                {"Content-Type", "application/json"},
                {"token", token}
            }, std::chrono::milliseconds(1000));

            jsonString = std::string {response.body.begin(), response.body.end()};
            nx::kit::Json json = nx::kit::Json::parse(jsonString, err);
            std::string message = json["message"].string_value();
            if (message != "ok") {
                throw message;
            }
            NX_DEBUG_STREAM << "[AiraFaceServer] maintain function successfully" NX_DEBUG_ENDL;
            return json["token"].string_value();

        } catch(const std::exception& ex) {
            NX_DEBUG_STREAM << "[AiraFaceServer] failed to login, ex:" << ex.what() << ", err:" << err << ", json:" << jsonString NX_DEBUG_ENDL;
            return std::string();
        }

    });
    
    return result;
}

void AiraFaceServer::maintain_handler() {
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    while (true) {
        auto lock = this->acquire_login_lock();

        /// haven't login yet. wait for login
        if (this->getLogined() == false) {
            NX_DEBUG_STREAM << "[AiraFaceServer] maintain status: wait for login" NX_DEBUG_ENDL;
            lock.unlock();
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            continue;
        }

        /// haven't initial yet. rare case. wait for initial
        if (this->shared_token == nullptr) {
            NX_DEBUG_STREAM << "[AiraFaceServer] maintain status: token not initialized yet" NX_DEBUG_ENDL;
            lock.unlock();
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            continue;
        }

        /// lock release here
        lock.unlock();

        /// the login is not working 
        auto status = this->shared_token->wait_for(std::chrono::milliseconds(1000));
        if (status != std::future_status::ready) {
            NX_DEBUG_STREAM << "[AiraFaceServer] maintain status: token failed to fetch" NX_DEBUG_ENDL;
            this->login(true);
            std::this_thread::sleep_for(std::chrono::milliseconds(2000));
            continue;
        }

        auto res = this->maintain();
        std::string token = res.get();
        if (token.size() > 0) {
            NX_DEBUG_STREAM << "[AiraFaceServer] maintain status: success" NX_DEBUG_ENDL;
            std::this_thread::sleep_for(std::chrono::milliseconds(60000));
            continue;
        }
        NX_DEBUG_STREAM << "[AiraFaceServer] maintain status: failed. force login again." NX_DEBUG_ENDL;
        this->login(true);
        std::this_thread::sleep_for(std::chrono::milliseconds(2000));
        continue;
    }
}
/* #endregion MAINTAIN */

std::string AiraFaceServer::baseUrl(std::string uri) {
    std::ostringstream strm;
    strm << PROTOCOL << "://";
    strm << this->hostname << ":" << this->port;
    strm << "/airafacelite" << uri;
    return strm.str();
}

}


/// HTTPRequest:
/// https://github.com/elnormous/HTTPRequest
/// JSON11:
/// https://github.com/dropbox/json11/blob/master/json11.hpp
