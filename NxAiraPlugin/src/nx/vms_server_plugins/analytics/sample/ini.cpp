#include "ini.h"

namespace nx {
namespace vms_server_plugins {
namespace analytics {
namespace aira {

Ini& ini()
{
    static Ini ini;
    return ini;
}

} // namespace aira
} // namespace analytics
} // namespace vms_server_plugins
} // namespace nx
