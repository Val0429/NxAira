// Copyright 2018-present Network Optix, Inc. Licensed under MPL 2.0: www.mozilla.org/MPL/2.0/

#include "engine.h"
#include "device_agent.h"
#include "ini.h"

#include <nx/kit/debug.h>

namespace nx {
namespace vms_server_plugins {
namespace analytics {
namespace aira {

using namespace nx::sdk;
using namespace nx::sdk::analytics;

Engine::Engine():
    // Call the DeviceAgent helper class constructor telling it to verbosely report to stderr.
    nx::sdk::analytics::Engine(/*enableOutput*/ true)
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
    "streamTypeFilter": "motion|compressedVideo",
    "capabilities": ")json" + buildCapabilities() + R"json(",
    "deviceAgentSettingsModel":
    {
        "type": "Settings",
        "items":
        [
            {
                "type": "GroupBox",
                "caption": "AiraFace License",
                "items":
                [
                    {
                        "type": "TextField",
                        "name": ")json" + kAirafaceLicenseSetting + R"json(",
                        "caption": "License",
                        "defaultValue": ""
                    }
                ]
            },
            {
                "type": "GroupBox",
                "caption": "AiraFace Configuration",
                "items":
                [
                    {
                        "type": "ComboBox",
                        "name": ")json" + kAirafaceProtocolSetting + R"json(",
                        "caption": "Protocol",
                        "defaultValue": "HTTPS",
                        "range": [
                            "HTTP",
                            "HTTPS"
                        ]
                    },
                    {
                        "type": "TextField",
                        "name": ")json" + kAirafaceIPSetting + R"json(",
                        "caption": "IP",
                        "defaultValue": "localhost"
                    },
                    {
                        "type": "SpinBox",
                        "name": ")json" + kAirafacePortSetting + R"json(",
                        "caption": "Port",
                        "defaultValue": 8888,
                        "minValue": 0,
                        "maxValue": 65535
                    },
                    {
                        "type": "TextField",
                        "name": ")json" + kAirafaceAccountSetting + R"json(",
                        "caption": "Account",
                        "defaultValue": "Admin"
                    },
                    {
                        "type": "TextField",
                        "name": ")json" + kAirafacePasswordSetting + R"json(",
                        "caption": "Password",
                        "defaultValue": "",
                        "validationErrorMessage": "Password is required.",
                        "validationRegex": "^.+$",
                        "validationRegexFlags": "i"
                    }
                ]
            },
            {
                "type": "GroupBox",
                "caption": "AiraFace ROI",
                "items":
                [
                    {
                        "type": "PolygonFigure",
                        "name": "polygon1",
                        "caption": "ROI",
                        "description": "Select a region of interest",
                        "minPoints": 4,
                        "maxPoints": 8
                    }
                ]
            },
            {
                "type": "GroupBox",
                "caption": "AiraFace Object Size",
                "items":
                [
                    {
                        "type": "ObjectSizeConstraints",
                        "name": "sizeConstraints1",
                        "caption": "Person size constraints",
                        "description": "Size range a person should fit into to be detected",
                        "defaultValue": {
                            "minimum": [0.1, 0.3],
                            "maximum": [0.3, 0.9]
                        }
                    }
                ]
            }            
        ]
    }
}
)json";
}

Result<const ISettingsResponse*> Engine::settingsReceived() {
    //settingValue() / m_settings

    NX_PRINT << "Sent request!";
    auto response = server.login("211.75.111.228", "82", "Admin", "123456");
    auto res_maintain = server.maintain();
    NX_PRINT << "request end!" << res_maintain.get();
    // NX_PRINT << "request end!" << response->get();

    // NX_PRINT << "try another!" << response->get();

    // Val: Todo connect AiraFace
    // settings[kAirafaceAccountSetting];
    // Convert Type
    // nx::kit::utils::fromString(settings[kObjectCountSetting], &objectCount);

    return nullptr;
}

} // namespace sample
} // namespace analytics
} // namespace vms_server_plugins
} // namespace nx
