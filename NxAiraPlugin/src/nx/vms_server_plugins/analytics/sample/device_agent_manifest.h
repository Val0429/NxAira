// Copyright 2018-present Network Optix, Inc. Licensed under MPL 2.0: www.mozilla.org/MPL/2.0/

#pragma once

#include <string>

namespace nx {
namespace vms_server_plugins {
namespace analytics {
namespace aira {

static const std::string kDeviceAgentManifest = /*suppress newline*/ 1 + (const char*) R"json(
{
    "supportedTypes":
    [
        {
            "objectTypeId": "nx.base.Person",
            "attributes":
            [
                "Gender",
                "Race",
                "Age",
                "Height",
                "Activity",
                "Hat",
                "Hat.Color",
                "Hat.Type",
                "Scarf",
                "Scarf.Color",
                "Body Shape",
                "Top Clothing Color",
                "Top Clothing Length",
                "Top Clothing Grain",
                "Top Clothing Type",
                "Bottom Clothing Color",
                "Bottom Clothing Length",
                "Bottom Clothing Grain",
                "Bottom Clothing Type",
                "Gloves",
                "Gloves.Color",
                "Shoes",
                "Shoes.Color",
                "Shoes.Type",
                "Name",
                "Temperature",
                "Tattoo",
                "Bag",
                "Bag.Size",
                "Bag.Color",
                "Bag.Type",
                "Weapon",
                "Cigarette",
                "Cigarette.Type",
                "Mobile Phone",
                "Mobile Phone.Position",
                "Cart",
                "Cart.Type",
                "Bottle",
                "Umbrella",
                "Umbrella.Color",
                "Umbrella.Open",
                "Box",
                "Box.Color",
                "Box.Lug",
                "Mask",
                "Glasses",
                "Glasses.Type",
                "Helmet"
            ]
        }
    ]
}
)json";

// const std::map<std::string, std::map<std::string, std::string>> kObjectAttributes = {
//     {
//         "nx.base.Person",
//         {
//             {"Gender", "Man"},
//             {"Age", "Adult"},
//             {"Name", "Frank Li"},
//             {"Weapon", "false"},
//             {"Top Clothing Color", "White"},
//             {"Bottom Clothing Color", "Blue"}
//         }
//     }
// };

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
