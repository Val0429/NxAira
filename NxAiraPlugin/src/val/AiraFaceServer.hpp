#pragma once

#include <set>
#include <thread>
#include <string>
#include <memory>
#include <future>

#include "./../lib/rxcpp/rx.hpp"

#include "./../val/result.h"

namespace nx {
    namespace vms_server_plugins {
        namespace analytics {
        namespace aira {
            class Engine;
}}}}

namespace val {

class AiraFaceServer {
private:
    std::string hostname;
    std::string port;
    std::string username;
    std::string password;

private:
    typedef std::string
            Message;
    typedef val::Result<Message>
            MessageType;
    typedef std::shared_future<MessageType>
            FutureMessageType;

private:
    nx::vms_server_plugins::analytics::aira::Engine& engine;
public:
    AiraFaceServer(nx::vms_server_plugins::analytics::aira::Engine& engine);

    /* #region LOGIN */
private:
    rxcpp::subjects::behavior<std::shared_ptr<Message>> sj_shared_token;
    std::shared_ptr<FutureMessageType> shared_token;
public:
    std::shared_ptr<FutureMessageType> login(
        std::string hostname, std::string port,
        std::string username, std::string password
        );
    std::shared_ptr<FutureMessageType> login(bool force);
    std::unique_lock<std::mutex> acquire_login_lock();

    bool getLogined();
private:
    void set_shared_token(std::shared_ptr<FutureMessageType> o);
public:
    std::shared_ptr<FutureMessageType> get_shared_token();
    /* #endregion LOGIN */

    /* #region MAINTAIN */
private:
    std::thread token_maintain;
    void maintain_handler();
public:
    FutureMessageType maintain();
    /* #endregion MAINTAIN */

    /* #region LICENSE */
public:
    class CLicenseInfo {
    public:
        /// string conversion
        std::string toString() const {
            return std::string("license: ") + license + ", count: " + std::to_string(count);
        }
        operator std::string() { return toString(); }
        friend std::ostream& operator<<(std::ostream& os, const CLicenseInfo& o) {
            os << o.toString();
            return os;
        }

    public:
        std::string license;
        int count;
    };
private:
    typedef CLicenseInfo
            LicenseMessage;
    typedef val::Result<LicenseMessage>
            LicenseMessageType;
    typedef std::shared_future<LicenseMessageType>
            FutureLicenseMessageType;

private:
    LicenseMessageType licenseInfo;
    void setLicenseInfo(LicenseMessageType o);
public:
    LicenseMessageType getLicenseInfo();
public:
    FutureLicenseMessageType getLicense();
    FutureLicenseMessageType setLicense(const std::string license);
    std::unique_lock<std::mutex> acquire_license_lock();
    /* #endregion LICENSE */

private:
    std::string baseUrl(std::string uri);

/// Event Handling
private:
    enum class EventCode: int {
        LoginSuccess,
        LoginFailed,
        MaintainSuccess,
        MaintainFailed,
        GetLicenseSuccess,
        GetLicenseFailed,
        SetLicenseSuccess,
        SetLicenseFailed
    };

    template<typename Value>
    void pushEvent(EventCode code, val::Result<Value>& o) {
        switch (code) {
            case EventCode::LoginSuccess: { engine.pushEvent(IPluginDiagnosticEvent::Level::info, "Login", "AiraFace Server successfully."); break; }
            case EventCode::LoginFailed: { engine.pushEvent(IPluginDiagnosticEvent::Level::info, "Login Failed", o); break; }
            case EventCode::MaintainSuccess:
            case EventCode::MaintainFailed:
                break;
            case EventCode::GetLicenseSuccess: { engine.pushEvent(IPluginDiagnosticEvent::Level::info, "Get License", o); break; }
            case EventCode::GetLicenseFailed: { engine.pushEvent(IPluginDiagnosticEvent::Level::info, "Get License Failed", o); break; }
            case EventCode::SetLicenseSuccess:
            case EventCode::SetLicenseFailed:
                break;
            default: { engine.pushEvent(IPluginDiagnosticEvent::Level::error, "Unknown Error", o); break; }
        }
    }
};

}