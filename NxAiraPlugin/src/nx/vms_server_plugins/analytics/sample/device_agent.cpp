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
#include <nx/sdk/helpers/settings_response.h>

#include "ini.h"
#include "device_agent_manifest.h"

#include "settings_model.h"
#include "./../../../../val/util.h"

namespace nx {
namespace vms_server_plugins {
namespace analytics {
namespace aira {

using namespace nx::sdk;
using namespace nx::sdk::analytics;


DeviceAgent::DeviceAgent(const nx::sdk::IDeviceInfo* deviceInfo, nx::vms_server_plugins::analytics::aira::Engine& engine, std::function<void(void)>&& doUnref):
    ConsumingDeviceAgent(deviceInfo, /*enableOutput*/ true),
    engine(engine),
    doUnref(std::move(doUnref)),
    motionProvider(*this)
    {}

DeviceAgent::~DeviceAgent() {
    if (doUnref != nullptr) doUnref();
}

std::string DeviceAgent::manifestString() const {
    return kDeviceAgentManifest;
}

Result<const ISettingsResponse*> DeviceAgent::settingsReceived() {
    std::map<std::string, std::string> settings = currentSettings();

    /// Load Settings
    /// FR
    nx::kit::utils::fromString(settingValue(kAirafaceEnableFacialRecognitionSetting), &enableFacialRecognition);
    nx::kit::utils::fromString(settingValue(kAirafaceFRMinimumFaceSizeSetting), &frMinimumFaceSize);
    nx::kit::utils::fromString(settingValue(kAirafaceFRRecognitionScoreSetting), &frRecognitionScore);
    nx::kit::utils::fromString(settingValue(kAirafaceFRRecognitionFPSSetting), &frFPS);
    /// FR - Events
    nx::kit::utils::fromString(settingValue(kAirafaceFREventWatchlistSetting), &frEventWatchlist);
    nx::kit::utils::fromString(settingValue(kAirafaceFREventRegisteredSetting), &frEventRegistered);
    nx::kit::utils::fromString(settingValue(kAirafaceFREventVisitorSetting), &frEventVisitor);
    nx::kit::utils::fromString(settingValue(kAirafaceFREventStrangerSetting), &frEventStranger);
    /// PD
    nx::kit::utils::fromString(settingValue(kAirafaceEnablePersonDetectionSetting), &enablePersonDetection);
    nx::kit::utils::fromString(settingValue(kAirafacePDMinimumBodySizeSetting), &pdMinimumBodySize);
    nx::kit::utils::fromString(settingValue(kAirafacePDDetectionScoreSetting), &pdDetectionScore);
    nx::kit::utils::fromString(settingValue(kAirafacePDRecognitionFPSSetting), &pdRecognitionFPS);
    /// PD - Events
    nx::kit::utils::fromString(settingValue(kAirafacePDEventPersonDetectionSetting), &pdEventPersonDetection);

    /// Report
    NX_PRINT << "enable FR?" << enableFacialRecognition;
    NX_PRINT << "FR minimum face size?" << frMinimumFaceSize;
    NX_PRINT << "FR recognition score?" << frRecognitionScore;
    NX_PRINT << "FR recognition fps?" << frFPS;
    NX_PRINT << "FR Event watchlist?" << frEventWatchlist;
    NX_PRINT << "FR Event registered?" << frEventRegistered;
    NX_PRINT << "FR Event visitor?" << frEventVisitor;
    NX_PRINT << "FR Event stranger?" << frEventStranger;
    NX_PRINT << "enable PD?" << enablePersonDetection;
    NX_PRINT << "PD minimum body size?" << pdMinimumBodySize;
    NX_PRINT << "PD detection score?" << pdDetectionScore;
    NX_PRINT << "PD recognition fps?" << pdRecognitionFPS;
    NX_PRINT << "PD Event detection?" << pdEventPersonDetection;

    const auto settingsResponse = new nx::sdk::SettingsResponse();
    settingsResponse->setModel(engine.getManifestModel());
    pushManifest(manifestString());

    return settingsResponse;
}
void DeviceAgent::getPluginSideSettings(nx::sdk::Result<const nx::sdk::ISettingsResponse*>* outResult) const {
    /// updating immediately
    auto settingsResponse = new SettingsResponse();
    settingsResponse->setModel(engine.getManifestModel());
    *outResult = settingsResponse;
}

bool DeviceAgent::pushCompressedVideoFrame(const ICompressedVideoPacket* videoPacket) {
    bool motion_detected = detectMotion(videoPacket);
    // NX_PRINT << "has motion?" << videoPacket->timestampUs();

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
    // NX_OUTPUT << "Received " << metadataPacketCount << " metadata packet(s) with the frame.";
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
