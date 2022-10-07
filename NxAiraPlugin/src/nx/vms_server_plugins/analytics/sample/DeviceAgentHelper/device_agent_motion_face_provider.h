#pragma once

#include <string>
#include <memory>

#include <nx/sdk/analytics/helpers/consuming_device_agent.h>
#include <nx/sdk/helpers/uuid_helper.h>
#include <nx/sdk/analytics/helpers/object_metadata.h>

namespace nx {
namespace vms_server_plugins {
namespace analytics {
namespace aira {

class DeviceAgentMotionFaceUnit {
public:
    DeviceAgentMotionFaceUnit(std::string name, std::string upperClothesColor, std::string lowerClothesColor, int targetFrame);
public:
    const std::string name;
    const std::string upperClothesColor;
    const std::string lowerClothesColor;
public:
    const int targetFrame;
    const nx::sdk::Uuid trackId;
};

class DeviceAgentMotionFaceProvider {
public:
    DeviceAgentMotionFaceProvider(const nx::sdk::analytics::ConsumingDeviceAgent& deviceAgent);
    nx::sdk::Ptr<nx::sdk::analytics::ObjectMetadata> feedWithMotion(bool hasMotion);

private:
    const nx::sdk::analytics::ConsumingDeviceAgent& deviceAgent;

private:
    std::unique_ptr<DeviceAgentMotionFaceUnit> faceUnit;
    int m_frameIndex;
    int m_targetFrame;
};

} // namespace aira
} // namespace analytics
} // namespace vms_server_plugins
} // namespace nx
