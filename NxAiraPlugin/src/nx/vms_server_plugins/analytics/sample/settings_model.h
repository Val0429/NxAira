#pragma once
#ifndef SETTINGS_MODEL_H
#define SETTINGS_MODEL_H

#include <string>

#define SET_PARSE std::string("Airaface")
#define DEF_HOST ""
#define DEF_PORT "8080"
#define DEF_ACCOUNT "Admin"

namespace nx {
namespace vms_server_plugins {
namespace analytics {
namespace aira {

/// settings - plugin
static const std::string kAirafaceHostSetting = SET_PARSE+"Host";
static const std::string kAirafacePortSetting = SET_PARSE+"Port";
static const std::string kAirafaceAccountSetting = SET_PARSE+"Account";
static const std::string kAirafacePasswordSetting = SET_PARSE+"Password";

static const std::string kAirafaceLicenseSetting = SET_PARSE+"License";

/// settings - engine
/// FR
static const std::string kAirafaceEnableFacialRecognitionSetting = SET_PARSE+"EnableFacialRecognition";
static const std::string kAirafaceFRMinimumFaceSizeSetting = SET_PARSE+"MinimumFaceSize";
static const std::string kAirafaceFRRecognitionScoreSetting = SET_PARSE+"FRRecognitionScore";
static const std::string kAirafaceFRRecognitionFPSSetting = SET_PARSE+"FRRecognitionFPS";
/// FR - event
static const std::string kAirafaceFREventWatchlistSetting = SET_PARSE+"FREventWatchlist";
static const std::string kAirafaceFREventRegisteredSetting = SET_PARSE+"FREventRegistered";
static const std::string kAirafaceFREventVisitorSetting = SET_PARSE+"FREventVisitor";
static const std::string kAirafaceFREventStrangerSetting = SET_PARSE+"FREventStranger";
/// PD
static const std::string kAirafaceEnablePersonDetectionSetting = SET_PARSE+"EnablePersonDetection";
static const std::string kAirafacePDMinimumBodySizeSetting = SET_PARSE+"PDMinimumBodySize";
static const std::string kAirafacePDDetectionScoreSetting = SET_PARSE+"PDDetectionScore";
static const std::string kAirafacePDRecognitionFPSSetting = SET_PARSE+"PDRecognitionFPS";
/// PD - event
static const std::string kAirafacePDEventPersonDetectionSetting = SET_PARSE+"PDEventPersonDetection";


/// models
// static const std::string kPluginBasicSettingsModel = /*suppress newline*/ 1 + (const char*) R"json(
// {
//     "type": "GroupBox",
//     "caption": "AiraFace Configuration",
//     "items":
//     [
//         {
//             "type": "TextField",
//             "name": ")json" + kAirafaceHostSetting + R"json(",
//             "caption": "Hostname",
//             "defaultValue": )json" + DEF_HOST + R"json(,
//         },
//         {
//             "type": "SpinBox",
//             "name": ")json" + kAirafacePortSetting + R"json(",
//             "caption": "Port",
//             "defaultValue": )json" + DEF_PORT + R"json(,
//             "minValue": 0,
//             "maxValue": 65535
//         },
//         {
//             "type": "TextField",
//             "name": ")json" + kAirafaceAccountSetting + R"json(",
//             "caption": "Account",
//             "defaultValue": )json" + DEF_ACCOUNT + R"json(
//         },
//         {
//             "type": "TextField",
//             "name": ")json" + kAirafacePasswordSetting + R"json(",
//             "caption": "Password",
//             "defaultValue": "",
//             "validationErrorMessage": "Password is required.",
//             "validationRegex": "^.+$",
//             "validationRegexFlags": "i"
//         }
//     ]
// }
// )json";

// static const std::string kPluginLicenseSettingsModel = /*suppress newline*/ 1 + (const char*) R"json(
// {
//     "type": "GroupBox",
//     "caption": "AiraFace License",
//     "items":
//     [
//         {
//             "type": "TextField",
//             "name": ")json" + kAirafaceLicenseSetting + R"json(",
//             "caption": "License",
//             "defaultValue": ""
//         }
//     ]
// }
// )json";

} // namespace sample
} // namespace analytics
} // namespace vms_server_plugins
} // namespace nx

#endif