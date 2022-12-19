// Copyright 2018-present Network Optix, Inc. Licensed under MPL 2.0: www.mozilla.org/MPL/2.0/

#include "engine.h"
#include "device_agent.h"
#include "engine_manifest.h"
#include "ini.h"

#include <nx/kit/debug.h>
#include <nx/sdk/helpers/settings_response.h>

#include "util.h"
#include "spdlog/spdlog.h"
#include "settings_model.h"

namespace nx {
namespace vms_server_plugins {
namespace analytics {
namespace aira {

using namespace nx::sdk;
using namespace nx::sdk::analytics;

Engine::Engine():
    // Call the DeviceAgent helper class constructor telling it to verbosely report to stderr.
    nx::sdk::analytics::Engine(/*enableOutput*/ true),
    licenseUsed(0),
    server(*this),
    logger(CreateLogger("Engine"))
{
    // server.licenseHolder.getObservable().subscribe([this](auto o) {
    //     pushManifest(manifestString());
    // });
}

Engine::~Engine()
{
}

/**
 * Called when the Server opens a video-connection to the camera if the plugin is enabled for this
 * camera.
 *
 * @param outResult The pointer to the structure which needs to be filled with the resulting value
 *     or the error information.
 * @param deviceInfo Contains various information about the related device such as its id, vendor,
 *     model, etc.
 */
void Engine::doObtainDeviceAgent(Result<IDeviceAgent*>* outResult, const IDeviceInfo* deviceInfo) {
    *outResult = new DeviceAgent(deviceInfo, ++licenseUsed, *this, [this]() {
        licenseUsed--;
    });
    // auto licenseInfo = server.licenseHolder.getValue();
    // bool isNull = licenseInfo == nullptr || !licenseInfo->isOk();
    // if (!isNull) {
    //     auto licenseCount = licenseInfo->value().count;
    //     logger->info("license count: {}", std::to_string(licenseCount));

    //     if (licenseCount > licenseUsed) {
    //         *outResult = new DeviceAgent(deviceInfo, *this, [this]() {
    //             licenseUsed--;
    //         });
    //         licenseUsed++;
    //         return;
    //     }
    //     *outResult = nullptr;

    // } else {
    //     logger->info("is null");
    //     *outResult = nullptr;
    // }
}

/**
 * Val: Build Plugin Capabilities
 */
static std::string buildCapabilities() {
    std::vector<std::string> capabilities;

    capabilities.push_back("needUncompressedVideoFrames_bgr");
    //capabilities.push_back("needUncompressedVideoFrames_yuv420");
    if (ini().deviceDependent) capabilities.push_back("deviceDependent");
    if (ini().usePluginAsSettingsOrigin) capabilities.push_back("usePluginAsSettingsOrigin");

    std::ostringstream join;
    std::copy(capabilities.begin(), capabilities.end(),
        std::ostream_iterator<std::string>(join, "|"));
    
    return join.str();
}

/**
 * @return JSON with the particular structure. Note that it is possible to fill in the values
 *     that are not known at compile time, but should not depend on the Engine settings.
 */
/// metadata_sdk\src\nx\sdk\settings_model.md
std::string Engine::manifestString() const {
    return /*suppress newline*/ 1 + (const char*) R"json(
{
    "version": "1.0.0",
    "vendor": "Aira Corporation",
    "streamTypeFilter": "motion|uncompressedVideo",
    "capabilities": ")json" + buildCapabilities() + R"json(",
    "deviceAgentSettingsModel":
)json" + getManifestModel(0) + R"json(
}
)json";
}

std::string Engine::getManifestModel(int licenseNum) const {
    auto licenseInfo = server.licenseHolder.getValue();
    bool isNull = licenseInfo == nullptr || !licenseInfo->isOk();
    int licenseCount = (isNull ? 0 : licenseInfo->value().count);

    return /*suppress newline*/ 1 + (const char*) R"json(
    {
        "type": "Settings",
        "items":
        [
            {
                "type": "GroupBox",
                "caption": "License Info",
                "items": [
                    {
                        "type": "RadioButtonGroup",
                        "name": "SupportChannels",
                        "caption": "Current / Total Channels",
                        "description": "",
                        "defaultValue": "opt1",
                        "range": ["opt1"],
                        "itemCaptions": {
                            "opt1": ")json" + (isNull ? "No License" : std::to_string(licenseUsed)) + R"json( / )json" + std::to_string(licenseCount) + R"json("
                        }
                    }
                ]
            }
            )json" + ((!isNull && licenseCount>=licenseNum) ? kEngineDetailManifest : "") + R"json(
        ]
    }
)json";
}

Result<const ISettingsResponse*> Engine::settingsReceived() {
    this->logger->info("Handle Plugin Setting...");

    const std::string hostname = settingValue(kAirafaceHostSetting);
    const std::string port = settingValue(kAirafacePortSetting);
    const std::string account = settingValue(kAirafaceAccountSetting);
    const std::string password = settingValue(kAirafacePasswordSetting);

    this->logger->info("Host: {}", hostname);
    this->logger->info("Port: {}", port);
    this->logger->info("Account: {}", account);
    this->logger->info("Password: {}", password);

    /// Val 1) Starting point: Attempt to login when hostname / port / account / password valid
    if (hostname.size() > 0 && account.size() > 0 && password.size() > 0) {
        server.login(hostname, port, account, password);
        auto res = server.getLicense();
        NX_PRINT << "hello!" << res.get();
    }

    // auto response = server.login("211.75.111.228", "82", "Admin", "123456");
    // auto res_maintain = server.maintain();

    return nullptr;
}

void Engine::getPluginSideSettings(nx::sdk::Result<const nx::sdk::ISettingsResponse*>* outResult) const {
    auto licenseInfo = server.licenseHolder.getValue();
    bool isNull = licenseInfo == nullptr || !licenseInfo->isOk();

    auto settingsResponse = new SettingsResponse();
    if (!isNull) settingsResponse->setValue(kAirafaceLicenseSetting, licenseInfo->value().license);

    *outResult = settingsResponse;
}

void Engine::pushEvent(
    nx::sdk::IPluginDiagnosticEvent::Level level,
    std::string caption,
    const std::string& description) {
    pushPluginDiagnosticEvent(level, caption, description);
}

} // namespace sample
} // namespace analytics
} // namespace vms_server_plugins
} // namespace nx
