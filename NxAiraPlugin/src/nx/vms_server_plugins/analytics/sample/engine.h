#pragma once
#ifndef ENGINE_H
#define ENGINE_H

#define WIN32_LEAN_AND_MEAN

#include <nx/sdk/analytics/helpers/plugin.h>
#include <nx/sdk/analytics/helpers/engine.h>
#include <nx/sdk/analytics/i_uncompressed_video_frame.h>

#include "util.h"
#include "AiraFaceServer.hpp"

namespace nx {
namespace vms_server_plugins {
namespace analytics {
namespace aira {

class Engine: public nx::sdk::analytics::Engine
{
public:
    Engine();
    virtual ~Engine() override;

protected:
    virtual std::string manifestString() const override;
    virtual nx::sdk::Result<const nx::sdk::ISettingsResponse*> settingsReceived() override;
    virtual void getPluginSideSettings(nx::sdk::Result<const nx::sdk::ISettingsResponse*>* outResult) const override;
public:
    std::string getManifestModel() const;

protected:
    virtual void doObtainDeviceAgent(
        nx::sdk::Result<nx::sdk::analytics::IDeviceAgent*>* outResult,
        const nx::sdk::IDeviceInfo* deviceInfo) override;

public:
    void pushEvent(
        nx::sdk::IPluginDiagnosticEvent::Level level,
        std::string caption,
        const std::string& description);
private:
    int licenseUsed;
    val::AiraFaceServer server;

private:
    std::shared_ptr<spdlog::logger> logger;
};

} // namespace sample
} // namespace analytics
} // namespace vms_server_plugins
} // namespace nx

#endif