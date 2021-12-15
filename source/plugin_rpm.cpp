#include <hex/helpers/lang.hpp>
#include <hex/pattern_language/evaluator.hpp>
#include <hex/pattern_language/token.hpp>
#include <hex/plugin.hpp>
#include <optional>

#include "rpm_provider.hpp"

static std::optional<hex::pl::Token::Literal> get_base(hex::pl::Evaluator* ctx,
                                                       std::vector<hex::pl::Token::Literal> const& params) {
    if (auto const p = dynamic_cast<hex::plugin::rpm::RPMProvider const*>(ctx->getProvider())) {
        return u128(p->getBase());
    }
    return u128(0);
};

IMHEX_PLUGIN_SETUP("RPM", "", "RPM memory provider for ImHex.") {
    using namespace ContentRegistry;
    Provider::add<hex::plugin::rpm::RPMProvider>("hex.rpm.provider");
    PatternLanguageFunctions::add({}, "RPM_EXE_BASE", PatternLanguageFunctions::NoParameters, get_base);
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
