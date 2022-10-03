// Copyright 2018-present Network Optix, Inc. Licensed under MPL 2.0: www.mozilla.org/MPL/2.0/

#include "device_agent.h"

#define NX_PRINT_PREFIX (this->logUtils.printPrefix)
// #define NX_DEBUG_ENABLE_OUTPUT true
#include <nx/kit/debug.h>
#include <nx/kit/utils.h>

#include <chrono>

#include <nx/sdk/analytics/helpers/event_metadata.h>
#include <nx/sdk/analytics/helpers/event_metadata_packet.h>
#include <nx/sdk/analytics/helpers/object_metadata_packet.h>

#include "device_agent_manifest.h"

namespace nx {
namespace vms_server_plugins {
namespace analytics {
namespace aira {

using namespace nx::sdk;
using namespace nx::sdk::analytics;


/**
 * @param deviceInfo Various information about the related device, such as its id, vendor, model,
 *     etc.
 */
DeviceAgent::DeviceAgent(const nx::sdk::IDeviceInfo* deviceInfo):
    // Call the DeviceAgent helper class constructor telling it to verbosely report to stderr.
    ConsumingDeviceAgent(deviceInfo, /*enableOutput*/ true)
{
    // NX_PRINT << "I should have gotten something here!";
}

DeviceAgent::~DeviceAgent()
{
}

/**
 *  @return JSON with the particular structure. Note that it is possible to fill in the values
 * that are not known at compile time, but should not depend on the DeviceAgent settings.
 */
std::string DeviceAgent::manifestString() const {
    return kDeviceAgentManifest;
}

/**
 * Val: For Test
 */
static constexpr int kTrackLength = 200;
static constexpr float kMaxBoundingBoxWidth = 0.5F;
static constexpr float kMaxBoundingBoxHeight = 0.5F;
static constexpr float kFreeSpace = 0.1F;

static Rect generateBoundingBox(int frameIndex, int trackIndex, int trackCount) {
    Rect boundingBox;
    boundingBox.width = std::min((1.0F - kFreeSpace) / trackCount, kMaxBoundingBoxWidth);
    boundingBox.height = std::min(boundingBox.width, kMaxBoundingBoxHeight);
    boundingBox.x = 1.0F / trackCount * trackIndex + kFreeSpace / (trackCount + 1);
    boundingBox.y = 1.0F - boundingBox.height - (1.0F / kTrackLength) * (frameIndex % kTrackLength);

    return boundingBox;
}

nx::sdk::Uuid DeviceAgent::trackIdByTrackIndex(int trackIndex) {
    while (trackIndex >= m_trackIds.size())
        m_trackIds.push_back(UuidHelper::randomUuid());

    return m_trackIds[trackIndex];
}

std::vector<Ptr<ObjectMetadata>> DeviceAgent::generateTestObject(
    const std::vector<std::vector<std::string>>& attributesByObjectType
) {
    std::vector<Ptr<ObjectMetadata>> result;

    for (const auto& element: attributesByObjectType) {
        const std::string& objectTypeId = element[0];
        auto objectMetadata = makePtr<ObjectMetadata>();
        objectMetadata->setTypeId(objectTypeId);

        for (size_t i=1; i<element.size(); i+=2) {
            objectMetadata->addAttribute(
                makePtr<Attribute>(element[i], element[i+1])
            );
        }
        result.push_back(std::move(objectMetadata));
    }

    return result;
}

Ptr<IMetadataPacket> DeviceAgent::generateTestObjectMetadataPacket(int64_t frameTimestampUs) {
    auto metadataPacket = makePtr<ObjectMetadataPacket>();
    metadataPacket->setTimestampUs(frameTimestampUs);

    std::vector<Ptr<ObjectMetadata>> objects;
    {
        const std::lock_guard<std::mutex> lock(m_mutex);
        objects = generateTestObject(kObjectAttributes);
    }

    for (int i = 0; i < objects.size(); ++i) {
        objects[i]->setBoundingBox(generateBoundingBox(m_frameIndex, i, objects.size()));
        objects[i]->setTrackId(trackIdByTrackIndex(i));

        metadataPacket->addItem(objects[i].get());
    }

    return metadataPacket;
}
/**
 * Val: For Test End
 */

// bool DeviceAgent::pushUncompressedVideoFrame(const IUncompressedVideoFrame* videoFrame)
bool DeviceAgent::pushCompressedVideoFrame(const ICompressedVideoPacket* videoPacket) {
    ++m_frameIndex;
    if ((m_frameIndex % kTrackLength) == 0) {
        m_trackIds.clear();
    }

    Ptr<IMetadataPacket> objectMetadataPacket = generateTestObjectMetadataPacket(
        videoPacket->timestampUs()
    );

    pushMetadataPacket(objectMetadataPacket.releasePtr());

    return true; //< There were no errors while processing the video frame.
}

/**
 * Serves the similar purpose as pushMetadataPacket(). The differences are:
 * - pushMetadataPacket() is called by the plugin, while pullMetadataPackets() is called by Server.
 * - pushMetadataPacket() expects one metadata packet, while pullMetadataPacket expects the
 *     std::vector of them.
 *
 * There are no strict rules for deciding which method is "better". A rule of thumb is to use
 * pushMetadataPacket() when you generate one metadata packet and do not want to store it in the
 * class field, and use pullMetadataPackets otherwise.
 */
bool DeviceAgent::pullMetadataPackets(std::vector<IMetadataPacket*>* metadataPackets)
{
    NX_PRINT << "Val Pull!";
    // metadataPackets->push_back(generateObjectMetadataPacket().releasePtr());

    return true; //< There were no errors while filling metadataPackets.
}

void DeviceAgent::doSetNeededMetadataTypes(
    nx::sdk::Result<void>* /*outValue*/,
    const nx::sdk::analytics::IMetadataTypes* /*neededMetadataTypes*/)
{
    /// initialization
}

//-------------------------------------------------------------------------------------------------
// private

// Ptr<IMetadataPacket> DeviceAgent::generateEventMetadataPacket()
// {
//     // Generate event every kTrackFrameCount'th frame.
//     if (m_frameIndex % kTrackFrameCount != 0)
//         return nullptr;

//     // EventMetadataPacket contains arbitrary number of EventMetadata.
//     const auto eventMetadataPacket = makePtr<EventMetadataPacket>();
//     // Bind event metadata packet to the last video frame using a timestamp.
//     eventMetadataPacket->setTimestampUs(m_lastVideoFrameTimestampUs);
//     // Zero duration means that the event is not sustained, but momental.
//     eventMetadataPacket->setDurationUs(0);

//     // EventMetadata contains an information about event.
//     const auto eventMetadata = makePtr<EventMetadata>();
//     // Set all required fields.
//     eventMetadata->setTypeId(kNewTrackEventType);
//     eventMetadata->setIsActive(true);
//     eventMetadata->setCaption("New sample plugin track started");
//     eventMetadata->setDescription("New track #" + std::to_string(m_trackIndex) + " started");

//     eventMetadataPacket->addItem(eventMetadata.get());

//     // Generate index and track id for the next track.
//     ++m_trackIndex;
//     m_trackId = nx::sdk::UuidHelper::randomUuid();

//     return eventMetadataPacket;
// }

// Ptr<IMetadataPacket> DeviceAgent::generateObjectMetadataPacket()
// {
//     // ObjectMetadataPacket contains arbitrary number of ObjectMetadata.
//     const auto objectMetadataPacket = makePtr<ObjectMetadataPacket>();

//     // Bind the object metadata to the last video frame using a timestamp.
//     objectMetadataPacket->setTimestampUs(m_lastVideoFrameTimestampUs);
//     objectMetadataPacket->setDurationUs(0);

//     // ObjectMetadata contains information about an object on the frame.
//     const auto objectMetadata = makePtr<ObjectMetadata>();
//     // Set all required fields.
//     objectMetadata->setTypeId(kPersonObjectType);
//     objectMetadata->setTrackId(m_trackId);

//     // Calculate bounding box coordinates each frame so that it moves from the top left corner
//     // to the bottom right corner during kTrackFrameCount frames.
//     static constexpr float d = 0.5F / kTrackFrameCount;
//     static constexpr float width = 0.5F;
//     static constexpr float height = 0.5F;
//     const int frameIndexInsideTrack = m_frameIndex % kTrackFrameCount;
//     const float x = d * frameIndexInsideTrack;
//     const float y = d * frameIndexInsideTrack;
//     objectMetadata->setBoundingBox(Rect(x, y, width, height));

//     objectMetadataPacket->addItem(objectMetadata.get());

//     return objectMetadataPacket;
// }

} // namespace sample
} // namespace analytics
} // namespace vms_server_plugins
} // namespace nx
