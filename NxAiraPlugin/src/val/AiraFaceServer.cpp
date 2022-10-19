#include "AiraFaceServer.hpp"

#include <sstream>
#include <iostream>
#include <stdexcept>

#include <nx/kit/json.h>
#include <nx/kit/debug.h>

#include "./../lib/HTTPRequest.hpp"

#define PROTOCOL "http"
#define PR_HEAD "[AiraFaceServer] "
#define LOGIN_HEAD "[LOGIN] "
#define MAINTAIN_HEAD "[MAINTAIN] "
#define LICENSE_HEAD "[LICENSE] "
#define DEBUG

namespace val {

AiraFaceServer::AiraFaceServer() :
token_maintain(&AiraFaceServer::maintain_handler, this)
{
    token_maintain.detach();
}

/* #region LOGIN */
std::shared_ptr<AiraFaceServer::FutureMessageType> AiraFaceServer::login(
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
    AiraFaceServer::FutureMessageType result = std::async(std::launch::async, [this, url]() {

        std::string jsonString, err;
        MessageType res;
        do {
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
                    res = val::error(val::ErrorCode::otherError, message);
                    break;
                }
                res = json["token"].string_value();
                break;

            } catch(const std::exception& ex) {
                res = val::error(val::ErrorCode::otherError, ex.what());
                NX_DEBUG_STREAM << PR_HEAD << LOGIN_HEAD << res NX_DEBUG_ENDL;
                break;
            }

        } while(0);
        return res;

    });
    
    shared_token = std::make_shared<AiraFaceServer::FutureMessageType>(std::move(result));

    return shared_token;
}

std::shared_ptr<AiraFaceServer::FutureMessageType> AiraFaceServer::login(bool force = false) {
    if (force) {
        return this->login(
            this->hostname, this->port,
            this->username, this->password
        );
    }
    return get_shared_token();
}

bool AiraFaceServer::getLogined() {
    auto token = get_shared_token();
    /// case 1: not initial yet
    if (token == nullptr) return false;
    /// case 2: not ready
    auto status = token->wait_for(std::chrono::milliseconds(0));
    if (status != std::future_status::ready) return false;
    /// case 3: error
    if (!token->get().isOk()) return false;
    return true;
}

std::shared_ptr<AiraFaceServer::FutureMessageType> AiraFaceServer::get_shared_token() {
    acquire_login_lock();
    return shared_token;
}

std::unique_lock<std::mutex> AiraFaceServer::acquire_login_lock() {
    static std::mutex mtx_login;
    return std::unique_lock<std::mutex>(mtx_login);
}
/* #endregion LOGIN */

/* #region MAINTAIN */
AiraFaceServer::FutureMessageType AiraFaceServer::maintain() {
    NX_DEBUG_STREAM << PR_HEAD << "maintain token start" NX_DEBUG_ENDL;
    /// concat fullUrl
    const std::string uri = "/maintaintoken";
    std::string url = baseUrl(uri);

    /// send request
    FutureMessageType result = std::async(std::launch::async, [this, url]() {

#ifdef DEBUG
NX_DEBUG_STREAM << "b11111111111111111111" NX_DEBUG_ENDL;
#endif

        std::string jsonString, err;
        MessageType res;
        do {
            auto token_future = this->login();
            auto status = token_future->wait_for(std::chrono::milliseconds(1000));
            if (status == std::future_status::timeout) {
                res = val::error(val::ErrorCode::networkError, "maintain token timeout@login");
                break;
            }
            MessageType token = token_future->get();
            if (!token.isOk()) {
                res = token;
                break;
            }

#ifdef DEBUG
NX_DEBUG_STREAM << "b22222222222222222222" NX_DEBUG_ENDL;
#endif

            /// actual request
            try {
                http::Request request {url};

#ifdef DEBUG
NX_DEBUG_STREAM << "b333333333333333333" NX_DEBUG_ENDL;
#endif

                const auto response = request.send("POST",
    R"json(
    {
        "token": ")json" + token.value() + R"json("
    }
    )json"
                , {
                    {"Content-Type", "application/json"},
                    {"token", token.value()}
                }, std::chrono::milliseconds(1000));

#ifdef DEBUG
NX_DEBUG_STREAM << "b4444444444444444444444" NX_DEBUG_ENDL;
#endif

                jsonString = std::string {response.body.begin(), response.body.end()};
                nx::kit::Json json = nx::kit::Json::parse(jsonString, err);
                std::string message = json["message"].string_value();
                if (message != "ok") {
                    res = val::error(val::ErrorCode::otherError, message);
                    break;
                }

#ifdef DEBUG
NX_DEBUG_STREAM << "b555555555555555555555" NX_DEBUG_ENDL;            
#endif

                NX_DEBUG_STREAM << "[AiraFaceServer] maintain function successfully" NX_DEBUG_ENDL;
                res = json["token"].string_value();
                break;

            } catch(const std::exception& ex) {
                res = val::error(val::ErrorCode::otherError, ex.what());
                NX_DEBUG_STREAM << PR_HEAD << LOGIN_HEAD << res NX_DEBUG_ENDL;
                break;
            }
        } while(0);
        return res;

    });
    
    return result;
}

void AiraFaceServer::maintain_handler() {
#ifdef DEBUG
NX_DEBUG_STREAM << "a000000000000000000000" NX_DEBUG_ENDL;
#endif
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    try {
        while (true) {
            auto token = get_shared_token();
#ifdef DEBUG
NX_DEBUG_STREAM << "a1111111111111111111111" NX_DEBUG_ENDL;
#endif
            /// login failed because of network error
            if (token != nullptr) {
                auto status = token->wait_for(std::chrono::milliseconds(0));
                if (status == std::future_status::ready && !token->get().isOk()) {
                    auto token_result = token->get();
                    if (token_result.error().errorCode() == val::ErrorCode::networkError) {
                        NX_DEBUG_STREAM << PR_HEAD << "network error. retry again..." NX_DEBUG_ENDL;
                        this->login(true);
                        std::this_thread::sleep_for(std::chrono::milliseconds(2000));
                        continue;
                    }
                    /// 2nd case, wait for login
                    NX_DEBUG_STREAM << "[AiraFaceServer] maintain status: wait for login" NX_DEBUG_ENDL;
                    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
                    continue;
                }
            }
#ifdef DEBUG
NX_DEBUG_STREAM << "a222222222222222222222222" NX_DEBUG_ENDL;
#endif

            auto res = this->maintain();
            MessageType mtoken = res.get();
            if (mtoken.isOk()) {
                NX_DEBUG_STREAM << PR_HEAD << MAINTAIN_HEAD << "status: success" NX_DEBUG_ENDL;
                std::this_thread::sleep_for(std::chrono::milliseconds(5000));
                continue;
            }
            NX_DEBUG_STREAM << PR_HEAD << MAINTAIN_HEAD << "status: failed. force login again." NX_DEBUG_ENDL;
            this->login(true);
            std::this_thread::sleep_for(std::chrono::milliseconds(2000));
            continue;
        }

    } catch (const std::exception& ex) {
        NX_DEBUG_STREAM << PR_HEAD << MAINTAIN_HEAD << "What's the exception here?" << ex.what() NX_DEBUG_ENDL;
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
AiraFaceServer::LicenseMessageType AiraFaceServer::getLicenseInfo() {
    return licenseInfo;
}
AiraFaceServer::FutureLicenseMessageType AiraFaceServer::getLicense() {
    /// concat fullUrl
    const std::string uri = "/license";
    std::string url = baseUrl(uri);

    /// send request
    FutureLicenseMessageType result = std::async(std::launch::async, [this, url]() {
        std::string jsonString, err;
        LicenseMessageType res;

        /// messages
        const std::string msg_err_get_token = "fetch token timeout@getlicense";
        const std::string msg_success = "success@getlicense";
        const std::string msg_failed = "failed@getlicense";

        do {
            /// get token
            auto token_future = this->login();
            auto status = token_future->wait_for(std::chrono::milliseconds(1000));
            if (status == std::future_status::timeout) {
                res = val::error(val::ErrorCode::networkError, msg_err_get_token);
                NX_DEBUG_STREAM << PR_HEAD << LICENSE_HEAD << res NX_DEBUG_ENDL;
                break;
            }
            MessageType token = token_future->get();
            if (!token.isOk()) {
                res = val::error(val::ErrorCode::networkError, msg_err_get_token + "[2]");
                break;
            }

            /// actual request
            try {
                const std::string finalUrl = url + "?token=" + token.value();
                http::Request request {finalUrl};
                const auto response = request.send("GET",
                    "",
                    {
                        {"Content-Type", "application/json"},
                        {"token", token.value()}
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
                    res = val::error(val::ErrorCode::otherError, message);
                    break;
                }
                NX_DEBUG_STREAM << PR_HEAD << LICENSE_HEAD << msg_success NX_DEBUG_ENDL;
                CLicenseInfo info;
                auto jlicense = json["license"];
                info.license = jlicense["license"].string_value();
                info.count = jlicense["fcs_amount"].int_value();
                res = std::move(info);
                break;

            } catch(const std::exception& ex) {
                res = val::error(val::ErrorCode::otherError, ex.what());
                NX_DEBUG_STREAM << PR_HEAD << LICENSE_HEAD << res NX_DEBUG_ENDL;
                break;
            }

        } while(0);

        acquire_license_lock();
        licenseInfo = res;
        return res;
    });
    
    return result;
}
AiraFaceServer::FutureLicenseMessageType AiraFaceServer::setLicense(const std::string license) {
    /// concat fullUrl
    const std::string uri = "/license";
    std::string url = baseUrl(uri);

    /// send request
    FutureLicenseMessageType result = std::async(std::launch::async, [this, url, license]() {
        std::string jsonString, err;
        LicenseMessageType res;

        /// messages
        const std::string msg_err_get_token = "fetch token timeout@setlicense";
        const std::string msg_success = "success@setlicense";
        const std::string msg_failed = "failed@setlicense";

        do {
            /// get token
            auto token_future = this->login();
            auto status = token_future->wait_for(std::chrono::milliseconds(1000));
            if (status == std::future_status::timeout) {
                res = val::error(val::ErrorCode::networkError, msg_err_get_token);
                NX_DEBUG_STREAM << PR_HEAD << LICENSE_HEAD << res NX_DEBUG_ENDL;
                break;
            }
            MessageType token = token_future->get();
            if (!token.isOk()) {
                res = val::error(val::ErrorCode::networkError, msg_err_get_token + "[2]");
                break;
            }

            /// actual request
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
                        {"token", token.value()}
                    },
                    std::chrono::milliseconds(1000));

                jsonString = std::string {response.body.begin(), response.body.end()};
                nx::kit::Json json = nx::kit::Json::parse(jsonString, err);
                std::string message = json["message"].string_value();
                if (message != "ok") {
                    res = val::error(val::ErrorCode::otherError, message);
                    break;
                }
                NX_DEBUG_STREAM << PR_HEAD << LICENSE_HEAD << msg_success NX_DEBUG_ENDL;
                CLicenseInfo info;
                auto jlicense = json["license"];
                info.license = jlicense["license"].string_value();
                info.count = jlicense["fcs_amount"].int_value();
                res = std::move(info);
                break;

            } catch(const std::exception& ex) {
                res = val::error(val::ErrorCode::otherError, ex.what());
                NX_DEBUG_STREAM << PR_HEAD << LICENSE_HEAD << res NX_DEBUG_ENDL;
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