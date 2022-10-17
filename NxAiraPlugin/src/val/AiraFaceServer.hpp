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
    AiraFaceServer() = default;

    std::shared_ptr<std::shared_future<std::string>> login(
        std::string hostname, std::string port,
        std::string username, std::string password
        );
    std::shared_ptr<std::shared_future<std::string>> login();

    std::future<std::string> maintain();

private:
    std::string baseUrl(std::string uri);
};

}