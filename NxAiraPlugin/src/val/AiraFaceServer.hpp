#pragma once

#include <set>
#include <thread>
#include <string>
#include <memory>
#include <future>


namespace val {

class AiraFaceServer {
private:
    std::string hostname;
    std::string port;
    std::string username;
    std::string password;

private:
    std::shared_ptr<std::shared_future<std::string>> shared_token;

public:
    AiraFaceServer();

    /* #region LOGIN */
private:
    bool logined = false;
public:
    std::shared_ptr<std::shared_future<std::string>> login(
        std::string hostname, std::string port,
        std::string username, std::string password
        );
    std::shared_ptr<std::shared_future<std::string>> login(bool force);
    std::unique_lock<std::mutex> acquire_login_lock();

    bool getLogined();
    /* #endregion LOGIN */

    /* #region MAINTAIN */
private:
    std::thread token_maintain;
    void maintain_handler();
public:
    std::future<std::string> maintain();
    /* #endregion MAINTAIN */

    /* #region LICENSE */
public:
    class CResponseLicense {
    public:
        bool success;
        std::string license;
        int count;
        std::string message;
    };
private:
    CResponseLicense licenseInfo;
    CResponseLicense getLicenseInfo();
public:
    std::future<CResponseLicense> getLicense();
    std::future<CResponseLicense> setLicense(const std::string license);
    std::unique_lock<std::mutex> acquire_license_lock();
    /* #endregion LICENSE */

private:
    std::string baseUrl(std::string uri);
};

}