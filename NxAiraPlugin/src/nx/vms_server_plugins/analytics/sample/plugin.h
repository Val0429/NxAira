#pragma once
#ifndef PLUGIN_H
#define PLUGIN_H

#include <nx/sdk/analytics/helpers/plugin.h>
#include <nx/sdk/analytics/i_engine.h>
#include "fwd/spdlog.h"

namespace nx {
namespace vms_server_plugins {
namespace analytics {
namespace aira {

class Plugin: public nx::sdk::analytics::Plugin
{
public:
    Plugin() = default;

protected:
    virtual nx::sdk::Result<nx::sdk::analytics::IEngine*> doObtainEngine() override;
    virtual std::string instanceId() const override { return "nx.aira3"; }
    virtual std::string manifestString() const override;
};

} // namespace sample
} // namespace analytics
} // namespace vms_server_plugins
} // namespace nx

#endif