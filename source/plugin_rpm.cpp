#include <hex/api/content_registry.hpp>
#include <hex/api/localization.hpp>
#include <hex/plugin.hpp>
#include <optional>

#include "rpm_provider.hpp"

IMHEX_PLUGIN_SETUP("RPM", "", "RPM memory provider for ImHex.") {
    using namespace hex::ContentRegistry;
    Provider::add<hex::plugin::rpm::RPMProvider>("hex.rpm.provider");
    Language::addLocalizations("en-US",
                               {
                                   {"hex.rpm.provider", "Read process memory"},
                                   {"hex.rpm.provider.base", "Base"},
                                   {"hex.rpm.provider.size", "Size"},
                                   {"hex.rpm.provider.pid", "PID"},
                                   {"hex.rpm.provider.name", "Name"},
                                   {"hex.rpm.provider.select.write", "Write"},
                                   {"hex.rpm.provider.select.reload", "Reload"},
                                   {"hex.rpm.provider.select.filter", "Filter"},
                               });
}
