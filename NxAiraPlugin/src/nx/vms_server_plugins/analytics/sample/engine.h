// Copyright 2018-present Network Optix, Inc. Licensed under MPL 2.0: www.mozilla.org/MPL/2.0/

#pragma once

#include <nx/sdk/analytics/helpers/plugin.h>
#include <nx/sdk/analytics/helpers/engine.h>
#include <nx/sdk/analytics/i_uncompressed_video_frame.h>

#include "./../../../../val/AiraFaceServer.hpp"

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
};

} // namespace sample
} // namespace analytics
} // namespace vms_server_plugins
} // namespace nx
