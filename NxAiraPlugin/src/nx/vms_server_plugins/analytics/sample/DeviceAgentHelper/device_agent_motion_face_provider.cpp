#include "device_agent_motion_face_provider.h"

#include <cstdlib>

namespace nx {
namespace vms_server_plugins {
namespace analytics {
namespace aira {

using namespace nx::sdk;
using namespace nx::sdk::analytics;

const int kEllipseFrames = 40;
const std::vector<std::string> kNames = {
    "Frank Li", "Val Liu", "Ken Chan", "Jack Lee", "Tulip Lin",
    "Alex Wu", "Azure Hsieh", "Mr. Chen"
};
const std::vector<std::string> kColors = {
    "Black", "Silver", "Gray", "White", "Red", "Purple", "Green", "Lime",
    "Olive", "Yellow", "Navy", "Blue", "Aqua"
};

static std::string randString(const std::vector<std::string>& list) {
    int pos = static_cast<int>(std::rand()/((RAND_MAX + 1u)/list.size()));
    return list[pos];
}

DeviceAgentMotionFaceProvider::DeviceAgentMotionFaceProvider(
    const ConsumingDeviceAgent& deviceAgent
) : deviceAgent(deviceAgent), m_frameIndex(0), m_targetFrame(0)
{}

Ptr<ObjectMetadata> DeviceAgentMotionFaceProvider::feedWithMotion(bool hasMotion) {
    m_frameIndex++;

    /// detect expired
    if (faceUnit != nullptr && faceUnit->targetFrame < m_frameIndex) {
        faceUnit = nullptr;
    }

    /// make faceUnit
    if (faceUnit == nullptr && hasMotion) {
        faceUnit = std::make_unique<DeviceAgentMotionFaceUnit>(
            randString(kNames),
            randString(kColors),
            randString(kColors),
            m_frameIndex + kEllipseFrames
        );
    }

    /// no motion no new face
    if (faceUnit == nullptr) return nullptr;

    /// make ObjectMetadata
    auto objectMetadata = makePtr<ObjectMetadata>();
    objectMetadata->setTypeId("nx.base.Person");
    objectMetadata->addAttribute(makePtr<Attribute>("Name", faceUnit->name));
    objectMetadata->addAttribute(makePtr<Attribute>("Top Clothing Color", faceUnit->upperClothesColor));
    objectMetadata->addAttribute(makePtr<Attribute>("Bottom Clothing Color", faceUnit->lowerClothesColor));
    objectMetadata->setBoundingBox(Rect(0.1f, 0.1f, 0.8f, 0.8f));
    objectMetadata->setTrackId(faceUnit->trackId);

    return objectMetadata;
}

DeviceAgentMotionFaceUnit::DeviceAgentMotionFaceUnit(std::string name, std::string upperClothesColor, std::string lowerClothesColor, int targetFrame)
: name(name),
  upperClothesColor(upperClothesColor), lowerClothesColor(lowerClothesColor),
  targetFrame(targetFrame), trackId(UuidHelper::randomUuid())
{}

} // namespace aira
} // namespace analytics
} // namespace vms_server_plugins
} // namespace nx
