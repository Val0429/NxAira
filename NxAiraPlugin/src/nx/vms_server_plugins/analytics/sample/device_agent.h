#pragma once
#ifndef DEVICE_AGENT_H
#define DEVICE_AGENT_H

#include <set>
#include <thread>
#include <memory>
#include <chrono>

#include <nx/sdk/analytics/helpers/consuming_device_agent.h>
#include <nx/sdk/helpers/uuid_helper.h>
#include <nx/kit/json.h>

#include "util.h"
#include "engine.h"
#include "DeviceAgentHelper/device_agent_motion_face_provider.h"
#include "DeviceAgentHelper/frame.h"

namespace nx {
namespace vms_server_plugins {
namespace analytics {
namespace aira {

class Engine;

class DeviceAgent: public nx::sdk::analytics::ConsumingDeviceAgent {
private:
    nx::vms_server_plugins::analytics::aira::Engine& engine;
    std::function<void(void)> doUnref;
public:
    DeviceAgent(const nx::sdk::IDeviceInfo* deviceInfo, nx::vms_server_plugins::analytics::aira::Engine& engine, std::function<void(void)>&& doUnref);
    virtual ~DeviceAgent() override;

protected:
    virtual std::string manifestString() const override;

    virtual nx::sdk::Result<const nx::sdk::ISettingsResponse*> settingsReceived() override;
    virtual void getPluginSideSettings(nx::sdk::Result<const nx::sdk::ISettingsResponse*>* outResult) const override;

    // virtual bool pushCompressedVideoFrame(
    //     const nx::sdk::analytics::ICompressedVideoPacket* videoFrame) override;

    virtual bool pushUncompressedVideoFrame(
        const nx::sdk::analytics::IUncompressedVideoFrame* videoFrame) override;

    virtual bool pullMetadataPackets(
        std::vector<nx::sdk::analytics::IMetadataPacket*>* metadataPackets) override;

    virtual void doSetNeededMetadataTypes(
        nx::sdk::Result<void>* outValue,
        const nx::sdk::analytics::IMetadataTypes* neededMetadataTypes) override;

private:
    std::mutex pp_mutex;
    std::vector<nx::sdk::analytics::IMetadataPacket*> pendingPackets;
    bool detectMotion(const nx::sdk::analytics::IUncompressedVideoFrame* videoPacket);
    nx::sdk::Uuid getUuidByString(const std::string& key);
    void handleDetectionData(nx::kit::Json data, int64_t timestamp);

private:
    bool enableFacialRecognition;
    double frMinimumFaceSize;
    double frRecognitionScore;
    double frFPS;
    bool frEventWatchlist;
    bool frEventRegistered;
    bool frEventVisitor;
    bool frEventStranger;
    bool enablePersonDetection;
    double pdMinimumBodySize;
    double pdDetectionScore;
    double pdRecognitionFPS;
    bool pdEventPersonDetection;

private:
    std::shared_ptr<spdlog::logger> logger;

private:
    DeviceAgentMotionFaceProvider motionProvider;
    std::chrono::steady_clock::time_point lastTime;
};

} // namespace sample
} // namespace analytics
} // namespace vms_server_plugins
} // namespace nx

#endif