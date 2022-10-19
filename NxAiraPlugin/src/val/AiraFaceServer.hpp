#pragma once

#include <set>
#include <thread>
#include <string>
#include <memory>
#include <future>

#include "./../val/result.h"

namespace val {

class AiraFaceServer {
private:
    std::string hostname;
    std::string port;
    std::string username;
    std::string password;

private:
    typedef val::Result<std::string>
            MessageType;
    typedef std::shared_future<MessageType>
            FutureMessageType;

public:
    AiraFaceServer();

    /* #region LOGIN */
private:
    std::shared_ptr<FutureMessageType> shared_token;
public:
    std::shared_ptr<FutureMessageType> login(
        std::string hostname, std::string port,
        std::string username, std::string password
        );
    std::shared_ptr<FutureMessageType> login(bool force);
    std::unique_lock<std::mutex> acquire_login_lock();

    bool getLogined();
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
        friend std::ostream& operator<<(std::ostream& os, const CLicenseInfo& o) {
            os << "license: " << o.license << ", count: " << o.count;
            return os;
        }
    public:
        std::string license;
        int count;
    };
private:
    typedef val::Result<CLicenseInfo>
            LicenseMessageType;
    typedef std::shared_future<LicenseMessageType>
            FutureLicenseMessageType;

private:
    LicenseMessageType licenseInfo;
    LicenseMessageType getLicenseInfo();
public:
    FutureLicenseMessageType getLicense();
    FutureLicenseMessageType setLicense(const std::string license);
    std::unique_lock<std::mutex> acquire_license_lock();
    /* #endregion LICENSE */

private:
    std::string baseUrl(std::string uri);
};

}