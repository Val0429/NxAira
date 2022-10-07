// Copyright 2018-present Network Optix, Inc. Licensed under MPL 2.0: www.mozilla.org/MPL/2.0/

#include "device_agent.h"

#define NX_PRINT_PREFIX (this->logUtils.printPrefix)
// #define NX_DEBUG_ENABLE_OUTPUT true
#include <nx/kit/debug.h>
#include <nx/kit/utils.h>

#include <chrono>

#include <nx/sdk/analytics/helpers/event_metadata.h>
#include <nx/sdk/analytics/helpers/event_metadata_packet.h>
#include <nx/sdk/analytics/helpers/object_metadata.h>
#include <nx/sdk/analytics/helpers/object_metadata_packet.h>
#include <nx/sdk/analytics/i_motion_metadata_packet.h>

#include "ini.h"
#include "device_agent_manifest.h"

namespace nx {
namespace vms_server_plugins {
namespace analytics {
namespace aira {

using namespace nx::sdk;
using namespace nx::sdk::analytics;


DeviceAgent::DeviceAgent(const nx::sdk::IDeviceInfo* deviceInfo):
    ConsumingDeviceAgent(deviceInfo, /*enableOutput*/ true),
    motionProvider(*this)
    {}

DeviceAgent::~DeviceAgent() {}

std::string DeviceAgent::manifestString() const {
    return kDeviceAgentManifest;
}

Result<const ISettingsResponse*> DeviceAgent::settingsReceived() {
    std::map<std::string, std::string> settings = currentSettings();

    // Val: Todo connect AiraFace
    // settings[kAirafaceAccountSetting];
    // Convert Type
    // nx::kit::utils::fromString(settings[kObjectCountSetting], &objectCount);

    return nullptr;
}

bool DeviceAgent::pushCompressedVideoFrame(const ICompressedVideoPacket* videoPacket) {
    bool motion_detected = detectMotion(videoPacket);
    NX_PRINT << "has motion?" << motion_detected;

    Ptr<ObjectMetadata> metadata = motionProvider.feedWithMotion(motion_detected);
    if (metadata) {
        auto metadataPacket = makePtr<ObjectMetadataPacket>();
        metadataPacket->setTimestampUs(videoPacket->timestampUs());
        metadataPacket->addItem(metadata.get());
        pushMetadataPacket(metadataPacket.releasePtr());
    }

    return true; // no errors
}

bool DeviceAgent::pullMetadataPackets(std::vector<IMetadataPacket*>* metadataPackets) {
    return true; // no errors
}

void DeviceAgent::doSetNeededMetadataTypes(
    nx::sdk::Result<void>* /*outValue*/,
    const nx::sdk::analytics::IMetadataTypes* /*neededMetadataTypes*/) {
    /// initialization
}

/// Private Helpers
bool DeviceAgent::detectMotion(const ICompressedVideoPacket* videoPacket) {
    Ptr<IList<IMetadataPacket>> metadataPacketList = videoPacket->metadataList();
    if (!metadataPacketList) return false;

    const int metadataPacketCount = metadataPacketList->count();
    NX_OUTPUT << "Received " << metadataPacketCount << " metadata packet(s) with the frame.";
    /// no packets
    if (metadataPacketCount == 0) return false;

    for (int i=0; i<metadataPacketCount; i++) {
        const auto metadataPacket = metadataPacketList->at(i);
        /// sanity check
        if (!NX_KIT_ASSERT(metadataPacket)) continue;

        const auto motionPacket = metadataPacket->queryInterface<IMotionMetadataPacket>();
        /// not motion
        if (!motionPacket) continue;

        /// no motion
        if (motionPacket->isEmpty()) continue;

        // int columns = motionPacket->columnCount();
        // int rows = motionPacket->rowCount();
        // NX_PRINT << "column:" << columns << ", " << rows;
        return true;
    }
    return false;
}

} // namespace aira
} // namespace analytics
} // namespace vms_server_plugins
} // namespace nx
