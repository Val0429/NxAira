#include "AiraFaceServer.hpp"

#include <sstream>
#include <iostream>

#include <nx/kit/json.h>
#include <nx/kit/debug.h>

#include "./../lib/HTTPRequest.hpp"

#define PROTOCOL "http"

namespace val {

/* #region LOGIN */
std::shared_ptr<std::shared_future<std::string>> AiraFaceServer::login(
        std::string hostname, std::string port,
        std::string username, std::string password
) {
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
            return json["token"].string_value();

        } catch(const std::exception& ex) {
            NX_DEBUG_STREAM << "[AiraFaceServer] failed to login, ex:" << ex.what() << ", err:" << err << ", json:" << jsonString NX_DEBUG_ENDL;
            return std::string();
        }

    });
    
    shared_token = std::make_shared<std::shared_future<std::string>>(std::move(result));

    return shared_token;
}

std::shared_ptr<std::shared_future<std::string>> AiraFaceServer::login() {
    return shared_token;
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
            return json["token"].string_value();

        } catch(const std::exception& ex) {
            NX_DEBUG_STREAM << "[AiraFaceServer] failed to login, ex:" << ex.what() << ", err:" << err << ", json:" << jsonString NX_DEBUG_ENDL;
            return std::string();
        }

    });
    
    return result;
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
