// Copyright 2018-present Network Optix, Inc. Licensed under MPL 2.0: www.mozilla.org/MPL/2.0/

#include "device_agent.h"

#include <algorithm>
#include <numeric>

#include <boost/lexical_cast.hpp>
#include <boost/thread/thread.hpp>
#include <boost/algorithm/string/join.hpp>

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
        /// ignore less than interval
        if (elapsedms < periodms) return true;
        lastTime = now;

        logger->info("Uncompressed Thread id: {}", boost::lexical_cast<std::string>(boost::this_thread::get_id()));

        /// 1) take out picture
        Frame frame(videoFrame);
        const int64_t timestamp = frame.timestampUs;

        /// 2) base64 image
        std::string base64_string = frame.getBase64String();
        /// 3) send to server
        std::thread([this, timestamp, base64_string = std::move(base64_string)]() {
            auto res = engine.server.doDetect(
                base64_string,
                enableFacialRecognition,
                frMinimumFaceSize,
                frRecognitionScore,
                enablePersonDetection,
                pdMinimumBodySize,
                pdDetectionScore
            );
            logger->info("Match detect! {}", res->get().toString());
            this->handleDetectionData(res->get().value().json, timestamp);
        }).detach();
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

nx::sdk::Uuid DeviceAgent::getUuidByString(const std::string& key) {
    static std::map<std::string, nx::sdk::Uuid> uuidMapping;
    auto o = uuidMapping.find(key);
    if (o != uuidMapping.end()) return o->second;
    uuidMapping[key] = UuidHelper::randomUuid();
    return uuidMapping[key];
}

void DeviceAgent::handleDetectionData(nx::kit::Json data, int64_t timestamp) {
    /* #region Data */
    // {
    //     "detect_uuid": "5b33418c-4a69-40c8-9e6a-56c7d36e5e24",
    //     "result": [
    /* #region Person */
    //         {
    //             "body_attributes": {
    //                 "has_backpack": false, "has_bag": false, "has_coat_jacket": false, "has_hat": false, "has_longhair": false, "has_longpants": true, "has_longsleeves": false, "is_male": true,
    //                 "top_5_colors": [{"color": "brown", "count": 4}, {"color": "yellow", "count": 3}, {"color": "red", "count": 2}, {"color": "orange", "count": 1}]
    //             },
    //             "body_position": {"x0": 0.75156199999999995, "x1": 0.87187499999999996, "y0": 0.122222, "y1": 0.70555599999999996},
    //             "body_uuid": "c9a2b84c-7b98-4041-93fd-e4d20baab802",
    //             "type": "person"
    //         },
    /* #endregion Person */
    /* #region Face - Stranger */
    //         {
    //             "face_position": {"x0": 0.24609400000000001, "x1": 0.26171899999999998, "y0": 0.63888900000000004, "y1": 0.66666700000000001},
    //             "face_uuid": "ea6916b9-376d-4a02-b0a9-0a6cbd8ca005",
    //             "is_registered_person": false,
    //             "type": "face"
    //         },
    /* #endregion Face - Stranger */
    /* #region Face - Recognize */
    //         {
    //             "face_position": {"x0": 0.32187500000000002, "x1": 0.45703100000000002, "y0": 0.065277799999999997, "y1": 0.30555599999999999},
    //             "face_uuid": "afa3fc3c-3409-4e7a-93d1-89c0d62c9724",
    //             "is_registered_person": true,
    //             "person_info": {
    //                 "group": ["All Visitor"],
    //                 "is_visitor": true,
    //                 "is_watchlist_1": false,
    //                 "is_watchlist_2": false,
    //                 "is_watchlist_3": true,
    //                 "is_watchlist_4": false,
    //                 "is_watchlist_5": false,
    //                 "person_id": "44103d96-5c5a-4f4b-b10a-a5e64fffd7a5",
    //                 "person_name": "SomeOne"
    //             },
    //             "type": "face"
    //         }
    //     ],
    //     "timestamp": 1671111000016
    // }
    /* #endregion Face - Recognize */
    /* #endregion Data */

    auto& results = data["result"];
    if (results.is_array()) {
        const std::vector<nx::kit::Json>& items = results.array_items();
        if (items.size() > 0) {
            auto objectMetadataPacket = makePtr<ObjectMetadataPacket>();

            for (auto it = items.cbegin(); it != items.cend(); ++it) {
                auto objectMetadata = makePtr<ObjectMetadata>();
                auto& item = *it;

                /// get type: person / face
                std::string type = item["type"].string_value();
                if (type == "person") {
                    objectMetadata->setTypeId("aira.ai.Person");
                    auto& pos = item["body_position"];
                    double x = pos["x0"].number_value();
                    double y = pos["y0"].number_value();
                    double width = pos["x1"].number_value() - x;
                    double height = pos["y1"].number_value() - y;
                    objectMetadata->setBoundingBox(Rect(x, y, width, height));
                    objectMetadata->setTrackId(
                        // getUuidByString(item["body_uuid"].string_value())
                        UuidHelper::randomUuid()
                    );

                    do {
                        auto bodyAttr = item["body_attributes"];
                        if (!bodyAttr.is_object()) break;
                        auto colorAttr = bodyAttr["top_5_colors"];
                        if (!colorAttr.is_array()) break;
                        auto colors = colorAttr.array_items();
                        std::sort(colors.begin(), colors.end(), [](auto a, auto b) {
                            return a["count"].number_value() < b["count"].number_value();
                        });
                        std::vector<std::string> colorNames;
                        std::transform(colors.begin(), colors.end(), std::back_inserter(colorNames), [](auto o) {
                            return o["color"].string_value();
                        });
                        /// add attributes
                        for (int i=1; i<=colorNames.size(); i++) {
                            objectMetadata->addAttribute(
                                makePtr<Attribute>("Color"+std::to_string(i), colorNames[i-1])
                            );
                        }

                    } while(0);

                } else if (type == "face") {
                    objectMetadata->setTypeId("aira.ai.Face");
                    auto& pos = item["face_position"];
                    double x = pos["x0"].number_value();
                    double y = pos["y0"].number_value();
                    double width = pos["x1"].number_value() - x;
                    double height = pos["y1"].number_value() - y;
                    objectMetadata->setBoundingBox(Rect(x, y, width, height));
                    objectMetadata->setTrackId(
                        // getUuidByString(item["face_uuid"].string_value())
                        UuidHelper::randomUuid()
                    );

                } else {
                    logger->error("Unknown type happens: {}", type);
                    return;
                }

                /// Generic Value
                do {
                    /// face not found
                    if (!item["is_registered_person"].is_bool()) break;
                    /// Attribute - Registered
                    bool is_registered = item["is_registered_person"].bool_value();
                    if (!is_registered) {
                        objectMetadata->addAttribute(makePtr<Attribute>("Stranger", "true"));
                        break;
                    }
                    objectMetadata->addAttribute(makePtr<Attribute>("Registered", "true"));

                    auto person_info = item["person_info"];
                    if (!person_info.is_object()) break;
                    /// Attribute - Visitor
                    if (person_info["is_visitor"].bool_value()) {
                        objectMetadata->addAttribute(makePtr<Attribute>("Visitor", "true"));
                    }
                    /// Attribute - Watchlist
                    for (int i=1; ;i++) {
                        auto isw_attr = person_info["is_watchlist_" + std::to_string(i)];
                        if (!isw_attr.is_bool()) break;
                        if (isw_attr.bool_value()) {
                            objectMetadata->addAttribute(makePtr<Attribute>("Watchlist", "true"));
                            break;
                        }
                    }
                    /// Attribute - Name
                    auto name_attr = person_info["person_name"];
                    if (name_attr.is_string()) {
                        objectMetadata->addAttribute(makePtr<Attribute>("Name", name_attr.string_value()));
                    }

                    /// Attribute - Group
                    auto group_attr = person_info["group"];
                    if (group_attr.is_array()) {
                        auto groups = group_attr.array_items();
                        std::vector<std::string> groupNames;
                        std::transform(groups.begin(), groups.end(), std::back_inserter(groupNames), [](auto o) {
                            return o.string_value();
                        });
                        std::string groupName = boost::algorithm::join(groupNames, ", ");
                        objectMetadata->addAttribute(makePtr<Attribute>("Group", groupName));
                    }

                } while(0);

                /// Add Value
                objectMetadataPacket->addItem(objectMetadata.get());
            }

            double fps = std::min(frFPS, pdRecognitionFPS);
            double periodms = fps == 0 ? 0 : (1000/fps);
            objectMetadataPacket->setTimestampUs(timestamp);
            logger->info("timestamp! {} {}", timestamp, periodms);
            objectMetadataPacket->setDurationUs(periodms*1000);
            // objectMetadataPacket->setDurationUs(2000000);
            
            const std::lock_guard<std::mutex> lock(pp_mutex);
            pendingPackets.push_back(objectMetadataPacket);
        }   
    }
}

bool DeviceAgent::pullMetadataPackets(std::vector<IMetadataPacket*>* metadataPackets) {
    const std::lock_guard<std::mutex> lock(pp_mutex);
    
    while (pendingPackets.size() > 0) {
        metadataPackets->push_back( pendingPackets.back().releasePtr() );
        pendingPackets.pop_back();
    }

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
