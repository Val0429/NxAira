#pragma once
#ifndef DEVICE_AGENT_MANIFEST_H
#define DEVICE_AGENT_MANIFEST_H

#include <string>

namespace nx {
namespace vms_server_plugins {
namespace analytics {
namespace aira {


static const std::string kDeviceAgentManifest = /*suppress newline*/ 1 + (const char*) R"json(
{
    "capabilities": "disableStreamSelection",
    "supportedTypes":
    [
        {
            "objectTypeId": "aira.ai.Person",
            "attributes":
            [
                "Face",
                "Face.Name",
                "Face.Group",
                "Color1",
                "Color2",
                "Color3",
                "Color4",
                "Color5"
            ]
        },
        {
            "objectTypeId": "aira.ai.Face",
            "attributes":
            [
                "Name",
                "Group"
            ]
        }
    ],)json" + /* Workaround for long strings. */ std::string() + R"json(
    "typeLibrary":
    {
        "objectTypes":
        [
            {
                "id": "aira.ai.Person",
                "name": "Person",
                "attributes":
                [
                    {
                        "name": "Face",
                        "type": "Object",
                        "subtype": "aira.ai.Face"
                    },
                    {
                        "name": "Color1",
                        "type": "Color"
                    },
                    {
                        "name": "Color2",
                        "type": "Color"
                    },
                    {
                        "name": "Color3",
                        "type": "Color"
                    },
                    {
                        "name": "Color4",
                        "type": "Color"
                    },
                    {
                        "name": "Color5",
                        "type": "Color"
                    }
                ]
            },
            {
                "id": "aira.ai.Face",
                "name": "Face",
                "attributes":
                [
                    {
                        "name": "Name",
                        "type": "String"
                    },
                    {
                        "name": "Group",
                        "type": "String"
                    }
                ]
            }
        ]
    }
}
)json";

const std::vector<std::vector<std::string>> kObjectAttributes = {
    {
        {
            "nx.base.Person",
                "Gender", "Man",
                "Age", "Adult",
                "Name", "Frank Li",
                "Weapon", "false",
                "Top Clothing Color", "White",
                "Bottom Clothing Color", "Blue"
        },
        {
            "nx.base.Person",
                "Gender", "Man",
                "Age", "Adult",
                "Name", "Tulip Lin",
                "Weapon", "true",
                "Top Clothing Color", "Blue",
                "Bottom Clothing Color", "Black"
        }
    }
};

} // namespace aira
} // namespace analytics
} // namespace vms_server_plugins
} // namespace nx

#endif