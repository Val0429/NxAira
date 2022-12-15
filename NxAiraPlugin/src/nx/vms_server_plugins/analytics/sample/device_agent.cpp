// Copyright 2018-present Network Optix, Inc. Licensed under MPL 2.0: www.mozilla.org/MPL/2.0/

#include "device_agent.h"

#include <boost/lexical_cast.hpp>
#include <boost/thread/thread.hpp>

#include <nx/kit/debug.h>
#include <nx/kit/utils.h>

#include "util.h"
#include "spdlog/spdlog.h"

#include <nx/sdk/analytics/helpers/event_metadata.h>
#include <nx/sdk/analytics/helpers/event_metadata_packet.h>
#include <nx/sdk/analytics/helpers/object_metadata.h>
#include <nx/sdk/analytics/helpers/object_metadata_packet.h>
#include <nx/sdk/analytics/i_motion_metadata_packet.h>
#include <nx/sdk/helpers/settings_response.h>

#include "ini.h"
#include "device_agent_manifest.h"

#include "settings_model.h"

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
    logger(CreateLogger("DeviceAgent")),
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
    logger->info("Setting Thread id: {}", boost::lexical_cast<std::string>(boost::this_thread::get_id()));

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
    logger->info(__func__);
    logger->info("enable FR? {}", enableFacialRecognition);
    logger->info("FR minimum face size? {}", frMinimumFaceSize);
    logger->info("FR recognition score? {}", frRecognitionScore);
    logger->info("FR recognition fps? {}", frFPS);
    logger->info("FR Event watchlist? {}", frEventWatchlist);
    logger->info("FR Event registered? {}", frEventRegistered);
    logger->info("FR Event visitor? {}", frEventVisitor);
    logger->info("FR Event stranger? {}", frEventStranger);
    logger->info("enable PD? {}", enablePersonDetection);
    logger->info("PD minimum body size? {}", pdMinimumBodySize);
    logger->info("PD detection score? {}", pdDetectionScore);
    logger->info("PD recognition fps? {}", pdRecognitionFPS);
    logger->info("PD Event detection? {}", pdEventPersonDetection);

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

// bool DeviceAgent::pushCompressedVideoFrame(const ICompressedVideoPacket* videoPacket) {
//     return true;
// }

bool DeviceAgent::pushUncompressedVideoFrame(const IUncompressedVideoFrame* videoFrame) {    
    /// one should be enabled to detect
    if (!enableFacialRecognition && !enablePersonDetection) return true;
    /// determine FPS
    double fps = std::min(frFPS, pdRecognitionFPS);
    double periodms = fps == 0 ? 0 : (1000/fps);

    bool motion_detected = detectMotion(videoFrame);
    if (motion_detected) {
        /// calculate time
        auto now = std::chrono::high_resolution_clock::now();
        double elapsedms = std::chrono::duration<double, std::milli>(now-lastTime).count();
        lastTime = now;
        /// ignore less than interval
        if (elapsedms < periodms) return true;

        logger->info("Uncompressed Thread id: {}", boost::lexical_cast<std::string>(boost::this_thread::get_id()));

        /// 1) take out picture
        Frame frame(videoFrame);

        /// 2) base64 image
        std::string base64_string = frame.getBase64String();
        auto res = engine.server.doDetect(
            base64_string,
            enableFacialRecognition,
            frMinimumFaceSize,
            frRecognitionScore,
            enablePersonDetection,
            pdMinimumBodySize,
            pdDetectionScore
        );
        // logger->info("Hello! {}", res->get().toString());

        // logger->info("encode! {}", base64_string);
        // logger->info("haha {} / {}", elapsedms, periodms);

        /// 4) send to server

    }

    // Ptr<ObjectMetadata> metadata = motionProvider.feedWithMotion(motion_detected);
    // if (metadata) {
    //     this->logger->info(__func__);
    //     this->logger->info("has motion? {}", videoFrame->timestampUs());

    //     auto metadataPacket = makePtr<ObjectMetadataPacket>();
    //     metadataPacket->setTimestampUs(videoFrame->timestampUs());
    //     metadataPacket->addItem(metadata.get());
    //     pushMetadataPacket(metadataPacket.releasePtr());
    // }

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
bool DeviceAgent::detectMotion(const IUncompressedVideoFrame* videoPacket) {
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
