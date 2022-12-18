#ifndef CNDTOOL_RESOURCE_H
#define CNDTOOL_RESOURCE_H
#include <exception>
#include <filesystem>

#include <libim/content/asset/cog/cog.h>
#include <libim/content/asset/cog/cogscript.h>
#include <libim/content/asset/cog/impl/grammer/parse_utils.h>
#include <libim/io/vfs.h>
#include <libim/log/log.h>
#include <libim/types/indexmap.h>
#include <libim/types/sharedref.h>
#include <libim/utils/utils.h>

namespace cndtool {
    [[nodiscard]] libim::IndexMap<libim::SharedRef<libim::content::asset::CogScript>> loadCogScripts(const libim::VirtualFileSystem& vfs, const std::vector<std::string>& scripts, bool bFixCogScripts)
    {
        using namespace libim;
        using namespace libim::content::asset;
        namespace fs = std::filesystem;


        IndexMap<SharedRef<CogScript>> stable;
        stable.reserve(scripts.size());

        for(const auto& sname : scripts)
        {
            auto optfs = vfs.findFile(fs::path("cog") / sname);
            if (!optfs) {
                if(optfs = vfs.findFile(sname); !optfs) {
                    throw std::runtime_error(std::string("Couldn't find cog script '" + sname + "'"));
                }
            }

            stable.emplaceBack(
                sname,
                loadCogScript(optfs->get(), /*load description*/true)
            );

            if(bFixCogScripts) {
                imfixes::fixCogScript((--stable.end())->get());
            }
        }
        return stable;
    }

    void verifyCogsNonLocalRawInitValues(const std::vector<libim::SharedRef<libim::content::asset::Cog>>& cogs)
    {
        using namespace libim;
        using namespace libim::content::asset::impl;
        for (const auto [cidx, srCog]  : utils::enumerate(cogs)) {
            for (const auto& sym : srCog->script->symbols) {
                if (!sym.isLocal
                 && sym.vtable.contains(srCog->vtid)
                 && !is_valid_raw_init_value(sym.type, sym.vtable.at(srCog->vtid))) {
                    LOG_WARNING("verifyCogsNonLocalRawInitValues: In COG % invalid initial value for symbol '%' of type '%'", cidx, sym.name, sym.type);
                    throw std::runtime_error("Invalid initial COG symbol value of non-local symbol");
                }
            }
        }
    }
}

#endif // CNDTOOL_RESOURCE_H
