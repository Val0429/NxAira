#pragma once
#ifndef ENGINE_MANIFEST_H
#define ENGINE_MANIFEST_H

#include <string>
#include "settings_model.h"

namespace nx {
namespace vms_server_plugins {
namespace analytics {
namespace aira {


static const std::string kEngineDetailManifest =/*suppress newline*/ 1 + (const char*) R"json(
            ,
            {
                "type": "GroupBox",
                "caption": "Facial Recognition",
                "items": [
                    {
                        "type": "SwitchButton",
                        "name": ")json" + kAirafaceEnableFacialRecognitionSetting + R"json(",
                        "caption": "Enable Facial Recognition",
                        "description": "Switch on to enable the facial recognition function",
                        "defaultValue": false
                    },
                    {
                        "type": "GroupBox",
                        "caption": "Facial Recognition Setting",
                        "items": [
                            {
                                "type": "SpinBox",
                                "name": ")json" + kAirafaceFRMinimumFaceSizeSetting + R"json(",
                                "caption": "Minimum Face Size",
                                "description": "The minimum face size to detect. (0-150)",
                                "defaultValue": 0,
                                "minValue": 0,
                                "maxValue": 150
                            },
                            {
                                "type": "DoubleSpinBox",
                                "name": ")json" + kAirafaceFRRecognitionScoreSetting + R"json(",
                                "caption": "Recognition Score",
                                "description": "The score to find correct person. The higher the more accurate. (0-1)",
                                "defaultValue": 0.85,
                                "minValue": 0,
                                "maxValue": 1
                            },
                            {
                                "type": "DoubleSpinBox",
                                "name": ")json" + kAirafaceFRRecognitionFPSSetting + R"json(",
                                "caption": "Recognition FPS",
                                "description": "How many frame per seconds to recognize. (0.5-2)",
                                "defaultValue": 0.5,
                                "minValue": 0.5,
                                "maxValue": 2
                            }
                        ]
                    },
                    {
                        "type": "GroupBox",
                        "caption": "Analytic Event Setting",
                        "items": [
                            {
                                "type": "SwitchButton",
                                "name": ")json" + kAirafaceFREventWatchlistSetting + R"json(",
                                "caption": "Watchlist",
                                "description": "",
                                "defaultValue": true
                            },
                            {
                                "type": "SwitchButton",
                                "name": ")json" + kAirafaceFREventRegisteredSetting + R"json(",
                                "caption": "Registered",
                                "description": "",
                                "defaultValue": true
                            },
                            {
                                "type": "SwitchButton",
                                "name": ")json" + kAirafaceFREventVisitorSetting + R"json(",
                                "caption": "Visitor",
                                "description": "",
                                "defaultValue": true
                            },
                            {
                                "type": "SwitchButton",
                                "name": ")json" + kAirafaceFREventStrangerSetting + R"json(",
                                "caption": "Stranger",
                                "description": "",
                                "defaultValue": true
                            }
                        ]
                    }  
                ]               
            },
            {
                "type": "GroupBox",
                "caption": "Person Detection",
                "items":
                [
                    {
                        "type": "SwitchButton",
                        "name": ")json" + kAirafaceEnablePersonDetectionSetting + R"json(",
                        "caption": "Enable Person Detection",
                        "description": "Switch on to enable the person detection function",
                        "defaultValue": false
                    },
                    {
                        "type": "GroupBox",
                        "caption": "Person Detection Setting",
                        "items": [
                            {
                                "type": "SpinBox",
                                "name": ")json" + kAirafacePDMinimumBodySizeSetting + R"json(",
                                "caption": "Minimum Body Size",
                                "description": "The minimum body size to detect. (0-150)",
                                "defaultValue": 0,
                                "minValue": 0,
                                "maxValue": 150
                            },
                            {
                                "type": "DoubleSpinBox",
                                "name": ")json" + kAirafacePDDetectionScoreSetting + R"json(",
                                "caption": "Detection Score",
                                "description": "The score to detect correct person. The higher the more accurate. (0-1)",
                                "defaultValue": 0.5,
                                "minValue": 0,
                                "maxValue": 1
                            },
                            {
                                "type": "DoubleSpinBox",
                                "name": ")json" + kAirafacePDRecognitionFPSSetting + R"json(",
                                "caption": "Recognition FPS",
                                "description": "How many frame per seconds to detect. (0.5-2)",
                                "defaultValue": 0.5,
                                "minValue": 0.5,
                                "maxValue": 2
                            }
                        ]
                    },
                    {
                        "type": "GroupBox",
                        "caption": "Analytic Event Setting",
                        "items": [
                            {
                                "type": "SwitchButton",
                                "name": ")json" + kAirafacePDEventPersonDetectionSetting + R"json(",
                                "caption": "Person Detection",
                                "description": "",
                                "defaultValue": true
                            }
                        ]
                    }
                ]
            }
)json";


} // namespace aira
} // namespace analytics
} // namespace vms_server_plugins
} // namespace nx

#endif