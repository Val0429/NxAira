#include "AiraFaceServer.hpp"

#include <sstream>
#include <iostream>
#include <stdexcept>

#include <nx/kit/json.h>
#include <nx/kit/debug.h>

#include <ixwebsocket/IXUserAgent.h>

#include "./../nx/vms_server_plugins/analytics/sample/engine.h"

#include "./../lib/HTTPRequest.hpp"

#include "util.h"
#include "spdlog/spdlog.h"

#define PROTOCOL "http"
#define PR_HEAD "[AiraServer] "
#define LOGIN_HEAD "[LOGIN] "
#define MAINTAIN_HEAD "[MAINTAIN] "
#define LICENSE_HEAD "[LICENSE] "
#define DETECT_HEAD "[DETECT] "
#define WS_HEAD "[WEBSOCKET]"
#define DEBUG

namespace val {

const std::string TIMEOUT = std::string("Request timed out");

AiraFaceServer::AiraFaceServer(nx::vms_server_plugins::analytics::aira::Engine& engine) :
engine(engine),
th_maintainhandle(&AiraFaceServer::maintain_handler, this),
logger(CreateLogger("AiraServer"))
{
    th_maintainhandle.detach();
}

/* #region LOGIN */
decltype(AiraFaceServer::tokenHolder.getFuture()) AiraFaceServer::login(
        std::string hostname, std::string port,
        std::string username, std::string password
) {
    auto lk = tokenHolder.lock();

    this->hostname = hostname;
    this->port = port;
    this->username = username;
    this->password = password;

    /// concat fullUrl
    const std::string uri = "/generatetoken";
    std::string url = baseUrl(uri);

    /// send request
    decltype(tokenHolder)::FutureMessageType result = std::async(std::launch::async, [this, url]() {

        std::string jsonString, err;
        decltype(tokenHolder)::MessageType res;
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
                    {"Content-Type", "application/json"},
                    /// Val: todo remove
                    {"token", "83522758"}
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
                res = val::error(ex.what() == TIMEOUT ? val::ErrorCode::networkError : val::ErrorCode::otherError, ex.what());
                NX_DEBUG_STREAM << PR_HEAD << LOGIN_HEAD << res NX_DEBUG_ENDL;
                break;
            }

        } while(0);
        pushEvent(res.isOk() ? EventCode::LoginSuccess : EventCode::LoginFailed, res);
        tokenHolder.onNext(res);
        return res;

    });

    return tokenHolder.setFuture(std::move(result), true);
}

decltype(AiraFaceServer::tokenHolder.getFuture()) AiraFaceServer::login(bool force = false) {
    if (force) {
        return this->login(
            this->hostname, this->port,
            this->username, this->password
        );
    }
    return tokenHolder.getFuture();
}

bool AiraFaceServer::getLogined() {
    auto token = tokenHolder.getFuture();
    /// case 1: not initial yet
    if (token == nullptr) return false;
    /// case 2: not ready
    auto status = token->wait_for(std::chrono::milliseconds(0));
    if (status != std::future_status::ready) return false;
    /// case 3: error
    if (!token->get().isOk()) return false;
    return true;
}
/* #endregion LOGIN */

/* #region MAINTAIN */
decltype(AiraFaceServer::tokenHolder.getFuture()) AiraFaceServer::maintain() {
    NX_DEBUG_STREAM << PR_HEAD << "maintain token start" NX_DEBUG_ENDL;
    /// concat fullUrl
    const std::string uri = "/maintaintoken";
    std::string url = baseUrl(uri);

    /// send request
    decltype(tokenHolder)::FutureMessageType result = std::async(std::launch::async, [this, url]() {

// #ifdef DEBUG
// NX_DEBUG_STREAM << "b11111111111111111111" NX_DEBUG_ENDL;
// #endif

        std::string jsonString, err;
        decltype(tokenHolder)::MessageType res;
        do {
            auto token_future = this->login();
            auto status = token_future->wait_for(std::chrono::milliseconds(1000));
            if (status == std::future_status::timeout) {
                res = val::error(val::ErrorCode::networkError, "maintain token timeout@login");
                break;
            }
            auto token = token_future->get();
            if (!token.isOk()) {
                res = token;
                break;
            }

// #ifdef DEBUG
// NX_DEBUG_STREAM << "b22222222222222222222" NX_DEBUG_ENDL;
// #endif

            /// actual request
            try {
                http::Request request {url};

// #ifdef DEBUG
// NX_DEBUG_STREAM << "b333333333333333333" NX_DEBUG_ENDL;
// #endif

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

// #ifdef DEBUG
// NX_DEBUG_STREAM << "b4444444444444444444444" NX_DEBUG_ENDL;
// #endif

                jsonString = std::string {response.body.begin(), response.body.end()};
                nx::kit::Json json = nx::kit::Json::parse(jsonString, err);
                std::string message = json["message"].string_value();
                if (message != "ok") {
                    res = val::error(val::ErrorCode::otherError, message);
                    break;
                }

// #ifdef DEBUG
// NX_DEBUG_STREAM << "b555555555555555555555" NX_DEBUG_ENDL;            
// #endif

                NX_DEBUG_STREAM << PR_HEAD << "maintain function successfully" NX_DEBUG_ENDL;
                res = json["token"].string_value();
                break;

            } catch(const std::exception& ex) {
                res = val::error(ex.what() == TIMEOUT ? val::ErrorCode::networkError : val::ErrorCode::otherError, ex.what());
                NX_DEBUG_STREAM << PR_HEAD << MAINTAIN_HEAD << res NX_DEBUG_ENDL;
                break;
            }
        } while(0);
        pushEvent(res.isOk() ? EventCode::MaintainSuccess : EventCode::MaintainFailed, res);
        return res;
    });
    
    return std::make_shared<decltype(tokenHolder)::FutureMessageType>(std::move(result));
}
/* #endregion MAINTAIN */

/* #region WS_HANDLER */
void AiraFaceServer::InitWebSocket() {
    const std::string uri = "/metadata";
    std::string url = baseUrl(uri);
    webSocket.setUrl(url);
    /// reconnect
    webSocket.setMinWaitBetweenReconnectionRetries(1000);
    webSocket.enableAutomaticReconnection();

    /// connect websocket
    logger->info("Web socket connection start");
    webSocket.start();
    webSocket.setOnMessageCallback([this](const ix::WebSocketMessagePtr& msg) {
        if (msg->type == ix::WebSocketMessageType::Open) {
            this->logger->info("Web socket connection opened");
            /// send token
            auto token = tokenHolder.getValue()->value();
            this->webSocket.send(
R"json(
{
"token": ")json" + token + R"json("
}
)json"
            );

        } else if (msg->type == ix::WebSocketMessageType::Message) {
            this->logger->info("Got message: {}", msg->str);
            std::string err;
            nx::kit::Json json = nx::kit::Json::parse(msg->str, err);
            this->wsSubject.get_subscriber().on_next(std::move(json));
        }
    });
}
/* #endregion WS_HANDLER */

/* #region MAINTAIN_HANDLER */
void AiraFaceServer::maintain_handler() {
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    try {
        while (true) {
            auto token = tokenHolder.getFuture();

            /// login failed because of network error
            if (token == nullptr) {
                /// 2nd case, wait for login
                NX_DEBUG_STREAM << PR_HEAD << "maintain status: wait for login" NX_DEBUG_ENDL;
                std::this_thread::sleep_for(std::chrono::milliseconds(1000));
                continue;
            } else {
                auto status = token->wait_for(std::chrono::milliseconds(0));
                if (status == std::future_status::ready) {
                    auto token_result = token->get();
                    if (!token_result.isOk()) {
                        if (token_result.error().errorCode() == val::ErrorCode::networkError) {
                            NX_DEBUG_STREAM << PR_HEAD << "network error. retry again..." NX_DEBUG_ENDL;
                            this->login(true);
                            std::this_thread::sleep_for(std::chrono::milliseconds(2000));
                        } else {
                            /// 2nd case, wait for login
                            NX_DEBUG_STREAM << PR_HEAD << "maintain status: wait for login" NX_DEBUG_ENDL;
                            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
                        }
                        continue;
                    }
                }
            }

            /// do websocket connect
            static std::once_flag onceFlag;
            std::call_once(onceFlag, [this] {
                this->InitWebSocket();
            });

            /// do maintain
            auto res = this->maintain();
            auto mtoken = res->get();
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
}
/* #endregion MAINTAIN_HANDLER */

/* #region LICENSE */
decltype(AiraFaceServer::licenseHolder.getFuture()) AiraFaceServer::getLicense() {
    /// concat fullUrl
    const std::string uri = "/license";
    std::string url = baseUrl(uri);

    /// send request
    decltype(licenseHolder)::FutureMessageType result = std::async(std::launch::async, [this, url]() {
        std::string jsonString, err;
        decltype(licenseHolder)::MessageType res;

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
            auto token = token_future->get();
            if (!token.isOk()) {
                res = val::error(val::ErrorCode::networkError, msg_err_get_token + "[2]");
                break;
            }

            /// actual request
            try {
                //const std::string finalUrl = url + "?token=" + token.value();
                const std::string finalUrl = url;
                http::Request request {finalUrl};
                const auto response = request.send("POST",
                    "",
                    {
                        {"Content-Type", "application/json"},
                        {"token", token.value()}
                    },
                    std::chrono::milliseconds(1000));

                jsonString = std::string {response.body.begin(), response.body.end()};
                nx::kit::Json json = nx::kit::Json::parse(jsonString, err);

                NX_DEBUG_STREAM << PR_HEAD << LICENSE_HEAD << msg_success << jsonString NX_DEBUG_ENDL;
                CLicenseInfo info;
                info.license = "";
                info.count = json["channel_amount"].int_value();
                res = std::move(info);
                break;

            } catch(const std::exception& ex) {
                res = val::error(ex.what() == TIMEOUT ? val::ErrorCode::networkError : val::ErrorCode::otherError, ex.what());
                NX_DEBUG_STREAM << PR_HEAD << LICENSE_HEAD << res NX_DEBUG_ENDL;
                break;
            }

        } while(0);

#ifdef DEBUG
        CLicenseInfo info; info.license = "8OGN-N8YM-B9MH-CP4K"; info.count = 2;
        res = info;
#endif
        pushEvent(res.isOk() ? EventCode::GetLicenseSuccess : EventCode::GetLicenseFailed, res);
        if (res.isOk()) licenseHolder.onNext(res);
        return res;
    });
    
    return licenseHolder.setFuture(std::move(result), true);
}
decltype(AiraFaceServer::licenseHolder.getFuture()) AiraFaceServer::setLicense(const std::string license) {
    /// concat fullUrl
    const std::string uri = "/license";
    std::string url = baseUrl(uri);

    /// send request
    decltype(licenseHolder)::FutureMessageType result = std::async(std::launch::async, [this, url, license]() {
        std::string jsonString, err;
        decltype(licenseHolder)::MessageType res;

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
            auto token = token_future->get();
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
                res = val::error(ex.what() == TIMEOUT ? val::ErrorCode::networkError : val::ErrorCode::otherError, ex.what());
                NX_DEBUG_STREAM << PR_HEAD << LICENSE_HEAD << res NX_DEBUG_ENDL;
                break;
            }

        } while(0);
        pushEvent(res.isOk() ? EventCode::SetLicenseSuccess : EventCode::SetLicenseFailed, res);
        if (res.isOk()) licenseHolder.onNext(res);
        return res;
    });
    
    return licenseHolder.setFuture(std::move(result), true);
}
/* #endregion LICENSE */

/* #region DETECT */
decltype(AiraFaceServer::detectHolder.getFuture()) AiraFaceServer::doDetect(
    std::string base64_image,
    bool enableFacialRecognition, double frMinimumFaceSize, double frRecognitionScore,
    bool enablePersonDetection, double pdMinimumBodySize, double pdDetectionScore
) {
    /// concat fullUrl
    const std::string uri = "/detect";
    std::string url = baseUrl(uri);

    /// send request
    decltype(detectHolder)::FutureMessageType result = std::async(std::launch::async, [
        this, url,
        base64_image,
        enableFacialRecognition, frMinimumFaceSize, frRecognitionScore,
        enablePersonDetection, pdMinimumBodySize, pdDetectionScore
        ]() {
        std::string jsonString, err;
        decltype(detectHolder)::MessageType res;

        /// messages
        const std::string msg_err_get_token = "fetch token timeout@dodetect";
        const std::string msg_success = "success@dodetect";
        const std::string msg_failed = "failed@dodetect";

        do {
            /// get token
            auto token_future = this->login();
            auto status = token_future->wait_for(std::chrono::milliseconds(1000));
            if (status == std::future_status::timeout) {
                res = val::error(val::ErrorCode::networkError, msg_err_get_token);
                NX_DEBUG_STREAM << PR_HEAD << LICENSE_HEAD << res NX_DEBUG_ENDL;
                break;
            }
            auto token = token_future->get();
            if (!token.isOk()) {
                res = val::error(val::ErrorCode::networkError, msg_err_get_token + "[2]");
                break;
            }

            /// actual request
            try {
                const std::string finalUrl = url;
                http::Request request {finalUrl};
                const auto response = request.send("POST",
R"json(
{
    "base64_image": ")json" + base64_image + R"json(",

    "enable_facial_recognition": )json" + (enableFacialRecognition ? "true" : "false") + R"json(,
    "fr_minimum_face_size": )json" + std::to_string(frMinimumFaceSize) + R"json(,
    "fr_recognition_score": )json" + std::to_string(frRecognitionScore) + R"json(,

    "enable_person_detection": )json" + (enablePersonDetection ? "true" : "false") + R"json(,
    "pd_minimum_body_size": )json" + std::to_string(pdMinimumBodySize) + R"json(,
    "pd_detection_score": )json" + std::to_string(pdDetectionScore) + R"json(
}
)json",
                    {
                        {"Content-Type", "application/json"},
                        {"token", token.value()}
                    },
                    std::chrono::milliseconds(1000));

                jsonString = std::string {response.body.begin(), response.body.end()};
                nx::kit::Json json = nx::kit::Json::parse(jsonString, err);

                logger->info("{}{}", msg_success, jsonString);
                CDetectInfo info;
                info.detect_uuid = json["detect_uuid"].string_value();
                res = std::move(info);
                break;

            } catch(const std::exception& ex) {
                res = val::error(ex.what() == TIMEOUT ? val::ErrorCode::networkError : val::ErrorCode::otherError, ex.what());
                logger->info("{}{}", msg_failed, static_cast<std::string>(res));
                break;
            }

        } while(0);

        pushEvent(res.isOk() ? EventCode::DetectSuccess : EventCode::DetectFailed, res);
        if (res.isOk()) detectHolder.onNext(res);
        return res;
    });
    
    return detectHolder.setFuture(std::move(result), true);
}
/* #endregion DETECT */

std::string AiraFaceServer::baseUrl(std::string uri) {
    std::ostringstream strm;
    strm << PROTOCOL << "://";
    strm << this->hostname << ":" << this->port;
    strm << "/airaai" << uri;
    return strm.str();
}

}


/// HTTPRequest:
/// https://github.com/elnormous/HTTPRequest
/// JSON11:
/// https://github.com/dropbox/json11/blob/master/json11.hpp
