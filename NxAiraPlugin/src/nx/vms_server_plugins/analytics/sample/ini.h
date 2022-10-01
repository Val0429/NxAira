#pragma once

#include <nx/kit/ini_config.h>
#include <nx/sdk/analytics/helpers/pixel_format.h>

namespace nx {
namespace vms_server_plugins {
namespace analytics {
namespace aira {

struct Ini: public nx::kit::IniConfig
{
    Ini(): IniConfig("nxaira.ini") { reload(); }

    NX_INI_FLAG(0, enableOutput, "");

    NX_INI_FLAG(0, deviceDependent, "Respective capability in the manifest.");
};

Ini& ini();

} // namespace aira
} // namespace analytics
} // namespace vms_server_plugins
} // namespace nx
