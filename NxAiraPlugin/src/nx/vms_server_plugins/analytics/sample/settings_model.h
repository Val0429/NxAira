#pragma once

#include <string>

#define SET_PARSE std::string("Airaface")
#define DEF_HOST "211.75.111.228"
#define DEF_PORT "82"
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