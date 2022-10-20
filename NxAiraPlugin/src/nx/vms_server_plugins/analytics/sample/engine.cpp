// Copyright 2018-present Network Optix, Inc. Licensed under MPL 2.0: www.mozilla.org/MPL/2.0/

#include "engine.h"
#include "device_agent.h"
#include "ini.h"

#include <nx/kit/debug.h>

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
    server(*this)
{
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
void Engine::doObtainDeviceAgent(Result<IDeviceAgent*>* outResult, const IDeviceInfo* deviceInfo)
{
    *outResult = new DeviceAgent(deviceInfo);
}

/**
 * Val: Build Plugin Capabilities
 */
static std::string buildCapabilities() {
    std::string capabilities = "needUncompressedVideoFrames_yuv420";

    if (ini().deviceDependent)
        capabilities += "|deviceDependent";

    if (ini().usePluginAsSettingsOrigin)
        capabilities += "|usePluginAsSettingsOrigin";

    // Delete first '|', if any.
    if (!capabilities.empty() && capabilities.at(0) == '|')
        capabilities.erase(0, 1);

    return capabilities;
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
    "streamTypeFilter": "motion|compressedVideo",
    "capabilities": ")json" + buildCapabilities() + R"json(",
    "deviceAgentSettingsModel":
    {
        "type": "Settings",
        "items":
        [
            {
                "type": "GroupBox",
                "caption": "Facial Recognition",
                "items": [
                    {
                        "type": "SwitchButton",
                        "name": "EnableFacialRecognition",
                        "caption": "Enable Facial Recognition",
                        "description": "Switch on to enable the facial recognition function",
                        "defaultValue": false
                    },
                    {
                        "type": "GroupBox",
                        "caption": "Facial Recognition Setting",
                        "items": [
                            {
                                "type": "SpinBox",
                                "name": "MinimumFaceSize",
                                "caption": "Minimum FaceSize",
                                "description": "The minimum face size to detect. (0-150)",
                                "defaultValue": 30,
                                "minValue": 0,
                                "maxValue": 150
                            },
                            {
                                "type": "SpinBox",
                                "name": "FRRecognitionScore",
                                "caption": "Recognition Score",
                                "description": "The score to find correct person. The higher the more accurate. (65-100)",
                                "defaultValue": 80,
                                "minValue": 65,
                                "maxValue": 100
                            },
                            {
                                "type": "DoubleSpinBox",
                                "name": "FRRecognitionFPS",
                                "caption": "Recognition FPS",
                                "description": "How many frame per seconds to recognize. (0.5-2)",
                                "defaultValue": 0.5,
                                "minValue": 0.5,
                                "maxValue": 2
                            },
                            {
                                "type": "SpinBox",
                                "name": "FRAntispoofingScore",
                                "caption": "Antispoofing Score",
                                "description": "",
                                "defaultValue": 0,
                                "minValue": 0,
                                "maxValue": 100
                            }
                        ]
                    },
                    {
                        "type": "GroupBox",
                        "caption": "Analytic Event Setting",
                        "items": [
                            {
                                "type": "SwitchButton",
                                "name": "Watchlist",
                                "caption": "Watchlist",
                                "description": "",
                                "defaultValue": true
                            },
                            {
                                "type": "SwitchButton",
                                "name": "Register",
                                "caption": "Register",
                                "description": "",
                                "defaultValue": true
                            },
                            {
                                "type": "SwitchButton",
                                "name": "Stranger",
                                "caption": "Stranger",
                                "description": "",
                                "defaultValue": true
                            }
                        ]
                    }  
                ]               
            },
            {
                "type": "GroupBox",
                "caption": "Person Detection",
                "items":
                [
                    {
                        "type": "SwitchButton",
                        "name": "EnablePersonDetection",
                        "caption": "Enable Person Detection",
                        "description": "Switch on to enable the person detection function",
                        "defaultValue": false
                    },
                    {
                        "type": "GroupBox",
                        "caption": "Person Detection Setting",
                        "items": [
                            {
                                "type": "SpinBox",
                                "name": "MinimumBodySize",
                                "caption": "Minimum Body Size",
                                "description": "The minimum body size to detect. (0-150)",
                                "defaultValue": 30,
                                "minValue": 0,
                                "maxValue": 150
                            },
                            {
                                "type": "SpinBox",
                                "name": "PDRecognitionScore",
                                "caption": "Recognition Score",
                                "description": "The score to detect correct person. The higher the more accurate. (65-100)",
                                "defaultValue": 80,
                                "minValue": 65,
                                "maxValue": 100
                            },
                            {
                                "type": "DoubleSpinBox",
                                "name": "PDRecognitionFPS",
                                "caption": "Recognition FPS",
                                "description": "How many frame per seconds to detect. (0.5-2)",
                                "defaultValue": 0.5,
                                "minValue": 0.5,
                                "maxValue": 2
                            }
                        ]
                    },
                    {
                        "type": "GroupBox",
                        "caption": "Analytic Event Setting",
                        "items": [
                            {
                                "type": "SwitchButton",
                                "name": "PersonDetection",
                                "caption": "Person Detection",
                                "description": "",
                                "defaultValue": true
                            }
                        ]
                    }
                ]
            }            
        ]
    }
}
)json";
}

Result<const ISettingsResponse*> Engine::settingsReceived() {
    NX_PRINT << "Handle Plugin Setting...";

    const std::string license = settingValue(kAirafaceLicenseSetting);
    const std::string hostname = settingValue(kAirafaceHostSetting);
    const std::string port = settingValue(kAirafacePortSetting);
    const std::string account = settingValue(kAirafaceAccountSetting);
    const std::string password = settingValue(kAirafacePasswordSetting);

    NX_PRINT << "License: " << license;
    NX_PRINT << "Host: " << hostname;
    NX_PRINT << "Port: " << port;
    NX_PRINT << "Account: " << account;
    NX_PRINT << "Password: " << password;

    /// Attempt to login when hostname / port / account / password valid
    if (hostname.size() > 0 && account.size() > 0 && password.size() > 0) {
        server.login(hostname, port, account, password);
        // auto res = server.getLicense();
        // NX_PRINT << "hello!" << res.get();
    }


    //settingValue() / m_settings

    // NX_PRINT << "Sent request!";
    // auto response = server.login("211.75.111.228", "82", "Admin", "123456");
    // auto res_maintain = server.maintain();
    // NX_PRINT << "request end!" << res_maintain.get();

    // Val: Todo connect AiraFace
    // settings[kAirafaceAccountSetting];
    // Convert Type
    // nx::kit::utils::fromString(settings[kObjectCountSetting], &objectCount);

    return nullptr;
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
