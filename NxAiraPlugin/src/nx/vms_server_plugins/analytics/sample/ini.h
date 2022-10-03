#pragma once

#include <nx/kit/ini_config.h>
#include <nx/sdk/analytics/helpers/pixel_format.h>

namespace nx {
namespace vms_server_plugins {
namespace analytics {
namespace aira {

/// settings
const std::string kAirafaceProtocolSetting = "AirafaceProtocol";
const std::string kAirafaceIPSetting = "AirafaceIP";
const std::string kAirafacePortSetting = "AirafacePort";
const std::string kAirafaceAccountSetting = "Account";
const std::string kAirafacePasswordSetting = "Password";


struct Ini: public nx::kit::IniConfig
{
    Ini(): IniConfig("nxaira.ini") { reload(); }

    NX_INI_FLAG(0, enableOutput, "");

    NX_INI_FLAG(0, deviceDependent, "Respective capability in the manifest.");

    NX_INI_FLAG(0, usePluginAsSettingsOrigin,
        "If set, Engine will declare the corresponding capability in the manifest.");

    NX_INI_FLAG(1, sendSettingsModelWithValues,
        "If set, Settings Model is being sent along with setting values when\n"
        "setSettings() or getPluginSideSettings() are called.");
};

Ini& ini();

} // namespace aira
} // namespace analytics
} // namespace vms_server_plugins
} // namespace nx
