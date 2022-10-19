#include "AiraFaceServer.hpp"

#include <sstream>
#include <iostream>
#include <stdexcept>

#include <nx/kit/json.h>
#include <nx/kit/debug.h>

#include "./../lib/HTTPRequest.hpp"

#define PROTOCOL "http"
#define DEBUG

namespace val {

// AiraFaceServer::AiraFaceServer()
// {

// }

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
    NX_DEBUG_STREAM << "[AiraFaceServer] maintain token start" NX_DEBUG_ENDL;
    /// concat fullUrl
    const std::string uri = "/maintaintoken";
    std::string url = baseUrl(uri);

    /// send request
    std::future<std::string> result = std::async(std::launch::async, [this, url]() {
    #ifdef DEBUG
    NX_DEBUG_STREAM << "b11111111111111111111" NX_DEBUG_ENDL;
    #endif
        auto token_future = this->login();
        auto status = token_future->wait_for(std::chrono::milliseconds(1000));
        if (status == std::future_status::timeout) {
            NX_DEBUG_STREAM << "[AiraFaceServer] maintain token timeout, while login" NX_DEBUG_ENDL;
            return std::string();
        }
        std::string token = token_future->get();
        if (token.size() == 0) return std::string();
    #ifdef DEBUG
    NX_DEBUG_STREAM << "b22222222222222222222" NX_DEBUG_ENDL;
    #endif
        /// actual request
        std::string jsonString, err;
        try {
            http::Request request {url};
    #ifdef DEBUG
    NX_DEBUG_STREAM << "b333333333333333333" NX_DEBUG_ENDL;
    #endif
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
    #ifdef DEBUG
    NX_DEBUG_STREAM << "b4444444444444444444444" NX_DEBUG_ENDL;
    #endif
            jsonString = std::string {response.body.begin(), response.body.end()};
            nx::kit::Json json = nx::kit::Json::parse(jsonString, err);
            std::string message = json["message"].string_value();
            if (message != "ok") {
                throw message;
            }
    #ifdef DEBUG
    NX_DEBUG_STREAM << "b555555555555555555555" NX_DEBUG_ENDL;            
    #endif
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

    try {
        while (true) {
            auto lock = this->acquire_login_lock();
#ifdef DEBUG
NX_DEBUG_STREAM << "a1111111111111111111111" NX_DEBUG_ENDL;
#endif
            /// haven't login yet. wait for login
            if (this->getLogined() == false) {
                NX_DEBUG_STREAM << "[AiraFaceServer] maintain status: wait for login" NX_DEBUG_ENDL;
                lock.unlock();
                std::this_thread::sleep_for(std::chrono::milliseconds(1000));
                continue;
            }
#ifdef DEBUG
NX_DEBUG_STREAM << "a222222222222222222222222" NX_DEBUG_ENDL;
#endif

            /// haven't initial yet. rare case. wait for initial
            if (this->shared_token == nullptr) {
                NX_DEBUG_STREAM << "[AiraFaceServer] maintain status: token not initialized yet" NX_DEBUG_ENDL;
                lock.unlock();
                std::this_thread::sleep_for(std::chrono::milliseconds(1000));
                continue;
            }
#ifdef DEBUG
NX_DEBUG_STREAM << "a333333333333333333333333" NX_DEBUG_ENDL;
#endif

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
#ifdef DEBUG
NX_DEBUG_STREAM << "a44444444444444444444444444" NX_DEBUG_ENDL;
#endif

            auto res = this->maintain();
            std::string token = res.get();
            if (token.size() > 0) {
                NX_DEBUG_STREAM << "[AiraFaceServer] maintain status: success" NX_DEBUG_ENDL;
                std::this_thread::sleep_for(std::chrono::milliseconds(5000));
                continue;
            }
            NX_DEBUG_STREAM << "[AiraFaceServer] maintain status: failed. force login again." NX_DEBUG_ENDL;
            this->login(true);
            std::this_thread::sleep_for(std::chrono::milliseconds(2000));
            continue;
        }

    } catch (const std::exception& ex) {
        NX_DEBUG_STREAM << "What's the exception here?" << ex.what() NX_DEBUG_ENDL;
    }


    // try {
    //     while (true) {
    //         auto lock = this->acquire_login_lock();

    //         /// haven't login yet. wait for login
    //         if (this->getLogined() == false) {
    //             NX_DEBUG_STREAM << "[AiraFaceServer] maintain status: wait for login" NX_DEBUG_ENDL;
    //             lock.unlock();
    //             std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    //             continue;
    //         }

    //         /// haven't initial yet. rare case. wait for initial
    //         if (this->shared_token == nullptr) {
    //             NX_DEBUG_STREAM << "[AiraFaceServer] maintain status: token not initialized yet" NX_DEBUG_ENDL;
    //             lock.unlock();
    //             std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    //             continue;
    //         }

    //         /// lock release here
    //         lock.unlock();

    //         /// the login is not working 
    //         auto status = this->shared_token->wait_for(std::chrono::milliseconds(1000));
    //         if (status != std::future_status::ready) {
    //             NX_DEBUG_STREAM << "[AiraFaceServer] maintain status: token failed to fetch" NX_DEBUG_ENDL;
    //             this->login(true);
    //             std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    //             continue;
    //         }

    //         auto res = this->maintain();
    //         std::string token = res.get();
    //         if (token.size() > 0) {
    //             NX_DEBUG_STREAM << "[AiraFaceServer] maintain status: success" NX_DEBUG_ENDL;
    //             std::this_thread::sleep_for(std::chrono::milliseconds(5000));
    //             continue;
    //         }
    //         NX_DEBUG_STREAM << "[AiraFaceServer] maintain status: failed. force login again." NX_DEBUG_ENDL;
    //         this->login(true);
    //         std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    //         continue;
    //     }

    // } catch (const std::exception& ex) {
    //     NX_DEBUG_STREAM << "What's the exception here?" << ex.what() NX_DEBUG_ENDL;
    // }

}
/* #endregion MAINTAIN */

/* #region LICENSE */
AiraFaceServer::CResponseLicense AiraFaceServer::getLicenseInfo() {
    return licenseInfo;
}
std::future<AiraFaceServer::CResponseLicense> AiraFaceServer::getLicense() {
    /// concat fullUrl
    const std::string uri = "/license";
    std::string url = baseUrl(uri);

    /// send request
    std::future<AiraFaceServer::CResponseLicense> result = std::async(std::launch::async, [this, url]() {
        CResponseLicense res;

        /// messages
        const std::string msg_err_get_token = "get license, fetch token timeout";
        const std::string msg_success = "get license successfully";
        const std::string msg_failed = "get license failed";

        do {
            /// get token
            auto token_future = this->login();
            auto status = token_future->wait_for(std::chrono::milliseconds(1000));
            if (status == std::future_status::timeout) {
                res.message = msg_err_get_token;
                NX_DEBUG_STREAM << "[AiraFaceServer] " << res.message NX_DEBUG_ENDL;
                break;
            }
            std::string token = token_future->get();
            if (token.size() == 0) {
                res.message = "token timeout";
                break;
            }

            /// actual request
            std::string jsonString, err;
            try {
                const std::string finalUrl = url + "?token=" + token;
                http::Request request {finalUrl};
                const auto response = request.send("GET",
                    "",
                    {
                        {"Content-Type", "application/json"},
                        {"token", token}
                    },
                    std::chrono::milliseconds(1000));

                // const auto response = request.send("GET",
                //     std::string("?token=") + token,
                //     {
                //         {"Content-Type", "application/json"},
                //         {"token", token}
                //     },
                //     std::chrono::milliseconds(1000));

                jsonString = std::string {response.body.begin(), response.body.end()};
                nx::kit::Json json = nx::kit::Json::parse(jsonString, err);
                std::string message = json["message"].string_value();
                if (message != "ok") {
                    throw std::runtime_error(message);
                }
                NX_DEBUG_STREAM << "[AiraFaceServer] " << msg_success NX_DEBUG_ENDL;
                auto jlicense = json["license"];
                res.license = jlicense["license"].string_value();
                res.count = jlicense["fcs_amount"].int_value();
                res.success = true;
                break;

            } catch(const std::exception& ex) {
                res.message = msg_failed + ":" + ex.what();
                NX_DEBUG_STREAM << "[AiraFaceServer] " << res.message << "." << jsonString NX_DEBUG_ENDL;
                break;
            }

        } while(0);

        acquire_license_lock();
        licenseInfo = res;
        return res;
    });
    
    return result;
}
std::future<AiraFaceServer::CResponseLicense> AiraFaceServer::setLicense(const std::string license) {
    /// concat fullUrl
    const std::string uri = "/license";
    std::string url = baseUrl(uri);

    /// send request
    std::future<AiraFaceServer::CResponseLicense> result = std::async(std::launch::async, [this, url, license]() {
        CResponseLicense res;

        /// messages
        const std::string msg_err_get_token = "set license, fetch token timeout";
        const std::string msg_success = "set license successfully";
        const std::string msg_failed = "set license failed";

        do {
            /// get token
            auto token_future = this->login();
            auto status = token_future->wait_for(std::chrono::milliseconds(1000));
            if (status == std::future_status::timeout) {
                res.message = msg_err_get_token;
                NX_DEBUG_STREAM << "[AiraFaceServer] " << res.message NX_DEBUG_ENDL;
                break;
            }
            std::string token = token_future->get();
            if (token.size() == 0) {
                res.message = "token timeout";
                break;
            }

            /// actual request
            std::string jsonString, err;
            try {
                http::Request request {url};

                const auto response = request.send("POST",
R"json(
{
    "license_key": ")json" + license + R"json("
}
)json"
                    , {
                        {"Content-Type", "application/json"},
                        {"token", token}
                    },
                    std::chrono::milliseconds(1000));

                jsonString = std::string {response.body.begin(), response.body.end()};
                nx::kit::Json json = nx::kit::Json::parse(jsonString, err);
                std::string message = json["message"].string_value();
                if (message != "ok") {
                    throw message;
                }
                NX_DEBUG_STREAM << "[AiraFaceServer] " << msg_success NX_DEBUG_ENDL;
                auto jlicense = json["license"];
                res.license = jlicense["license"].string_value();
                res.count = jlicense["fcs_amount"].int_value();
                res.success = true;
                break;

            } catch(const std::exception& ex) {
                res.message = msg_failed + ":" + ex.what();
                NX_DEBUG_STREAM << "[AiraFaceServer] " << res.message << "." << jsonString NX_DEBUG_ENDL;
                break;
            }

        } while(0);

        acquire_license_lock();
        licenseInfo = res;
        return res;
    });
    
    return result;
}
std::unique_lock<std::mutex> AiraFaceServer::acquire_license_lock() {
    static std::mutex mtx_license;
    return std::unique_lock<std::mutex>(mtx_license);
}
/* #endregion LICENSE */

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
