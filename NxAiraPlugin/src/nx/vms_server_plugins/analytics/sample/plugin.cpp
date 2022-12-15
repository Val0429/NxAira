// Copyright 2018-present Network Optix, Inc. Licensed under MPL 2.0: www.mozilla.org/MPL/2.0/

#include "plugin.h"

#include <nx/kit/debug.h>
#include <nx/kit/utils.h>

#include <ixwebsocket/IXNetSystem.h>

#include "engine.h"
#include "settings_model.h"

namespace nx {
namespace vms_server_plugins {
namespace analytics {
namespace aira {

using namespace nx::sdk;
using namespace nx::sdk::analytics;

Plugin::Plugin() {
    ix::initNetSystem();
}
Plugin::~Plugin() {
    ix::uninitNetSystem();
}

Result<IEngine*> Plugin::doObtainEngine()
{
    return new Engine();
}

std::string Plugin::manifestString() const
{
    return /*suppress newline*/ 1 + (const char*) R"json(
{
    "id": ")json" + instanceId() + R"json(",
    "name": "Aira x NX metadata",
    "description": "Aira Plugin",
    "version": "1.0.0",
    "vendor": "Aira Corporation",
    "engineSettingsModel":
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
                        "type": "TextField",
                        "name": ")json" + kAirafaceHostSetting + R"json(",
                        "caption": "Hostname",
                        "defaultValue": ")json" + DEF_HOST + R"json("
                    },
                    {
                        "type": "SpinBox",
                        "name": ")json" + kAirafacePortSetting + R"json(",
                        "caption": "Port",
                        "defaultValue": )json" + DEF_PORT + R"json(,
                        "minValue": 0,
                        "maxValue": 65535
                    },
                    {
                        "type": "TextField",
                        "name": ")json" + kAirafaceAccountSetting + R"json(",
                        "caption": ")json" + DEF_ACCOUNT + R"json(",
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
                    },
                    {
                        "type": "TextField",
                        "name": "NTP",
                        "caption": "NTP Server",
                        "defaultValue": ""
                    }
                ]
            }
        ]
    }
}
)json";
}

/**
 * Called by the Server to instantiate the Plugin object.
 *
 * The Server requires the function to have C linkage, which leads to no C++ name mangling in the
 * export table of the plugin dynamic library, so that makes it possible to write plugins in any
 * language and compiler.
 *
 * NX_PLUGIN_API is the macro defined by CMake scripts for exporting the function.
 */
extern "C" NX_PLUGIN_API nx::sdk::IPlugin* createNxPlugin()
{
    // The object will be freed when the Server calls releaseRef().
    return new Plugin();
}

} // namespace aira
} // namespace analytics
} // namespace vms_server_plugins
} // namespace nx
