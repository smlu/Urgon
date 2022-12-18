#ifndef CNDTOOL_RESOURCE_H
#define CNDTOOL_RESOURCE_H
#include <exception>
#include <filesystem>

#include <libim/content/asset/animation/animation.h>
#include <libim/content/asset/cog/cog.h>
#include <libim/content/asset/cog/cogscript.h>
#include <libim/content/asset/cog/impl/grammer/parse_utils.h>
#include <libim/content/asset/cog/impl/grammer/parser.h>
#include <libim/content/audio/soundbank.h>

#include <libim/io/vfs.h>
#include <libim/log/log.h>
#include <libim/types/indexmap.h>
#include <libim/types/sharedref.h>
#include <libim/utils/utils.h>

namespace cndtool {
     namespace fs = std::filesystem;
    using namespace libim;
    using namespace libim::content::asset;
    using namespace libim::content::audio;

    constexpr std::string_view   kDefaultStaticResourcesFilename = "jones3dstatic.cnd";

    constexpr inline SoundHandle kDefaultStartSoundHandle  = SoundHandle(349); // 349 is the next sound handle seed stored in original jones3dstatic.cnd
    constexpr inline std::size_t kSoundbankStaticTrackIdx  = 0;
    constexpr inline std::size_t kSoundbankNormalTrackIdx  = 1;

    constexpr static std::string_view kAnimationDir1 = "3do/key";
    constexpr static std::string_view kAnimationDir2 = "key";
    constexpr static std::string_view kCogScriptDir  = "cog";
    constexpr static std::string_view kMaterialDir   = "mat";
    constexpr static std::string_view kSoundDir1     = "sound";
    constexpr static std::string_view kSoundDir2     = "wv";
    constexpr static std::string_view kSoundDir3     = "wav";

    constexpr inline std::size_t getSoundBankTrackIdx(const bool isStatic) {
        return isStatic ? kSoundbankStaticTrackIdx : kSoundbankNormalTrackIdx;
    }

    constexpr inline SoundHandle getDefaultStartSoundHandle(const bool isStatic) {
        return isStatic ? SoundHandle(0) : kDefaultStartSoundHandle;
    }

    SharedRef<InputStream> searchFile(const VirtualFileSystem& vfs, const std::vector<fs::path>& folders, std::string_view filename)
    {
        for (const auto& dir : folders)
        {
            if( auto optfile = vfs.findFile(dir / filename)){
                return optfile.value();
            }
        }
        // No file was found in dirs try to get file from root or throw an exception
        return vfs.getFile(filename);
    }

    IndexMap<Animation> loadAnimations(const VirtualFileSystem& vfs, const std::vector<std::string>& animFilenames)
    {
        IndexMap<Animation> animations;
        for (const auto& animFilename : animFilenames)
        {

            auto file = searchFile(vfs, { kAnimationDir1, kAnimationDir2 }, animFilename);
            auto anim = keyLoad(TextResourceReader(file.get()));
            animations.pushBack(animFilename, std::move(anim));
        }
        return animations;
    }

    [[nodiscard]] IndexMap<Material> loadMaterials(const VirtualFileSystem& vfs, const std::vector<std::string>& materialFilenames)
    {
        IndexMap<Material> materials;
        for (const auto& matFilename : materialFilenames)
        {
            auto file = searchFile(vfs, { kMaterialDir }, matFilename);
            auto mat  = matLoad(file.get());
            materials.pushBack(matFilename, std::move(mat));
        }
        return materials;
    }

    void loadSounds(const VirtualFileSystem& vfs, SoundBank& bank, std::size_t trackIdx, const std::vector<std::string>& soundFilenames)
    {
        for (const auto& sndFilename : soundFilenames)
        {
            auto file = searchFile(vfs, { kSoundDir1, kSoundDir2, kSoundDir3 }, sndFilename);
            bank.loadSound(file.get(), trackIdx);
        }
    }

    [[nodiscard]] libim::IndexMap<SharedRef<CogScript>> loadCogScripts(const VirtualFileSystem& vfs, const std::vector<std::string>& scripts, bool bFixCogScripts)
    {
        using namespace libim;
        using namespace libim::content::asset;
        namespace fs = std::filesystem;

        IndexMap<SharedRef<CogScript>> stable;
        stable.reserve(scripts.size());

        for(const auto& sname : scripts)
        {
            auto file = searchFile(vfs, { kCogScriptDir }, sname);
            stable.emplaceBack(
                sname,
                loadCogScript(file.get(), /*load description*/true)
            );

            if(bFixCogScripts) {
                imfixes::fixCogScript((--stable.end())->get());
            }
        }
        return stable;
    }

    void verifyCogs(const std::vector<SharedRef<Cog>>& cogs)
    {
        using namespace libim::content::asset;
        for (const auto [cidx, cog]  : utils::enumerate(cogs))
        {
            for (const auto& sym : cog->script->symbols)
            {
                if (sym.isLocal || sym.type == CogSymbol::Message) {
                    continue;
                }

                const auto getValue = [&]() -> const CogSymbolValue&
                {
                    try {
                        return sym.valueOrDefault(cog->vtid);
                    }
                    catch (...) {
                        throw std::runtime_error(
                            utils::format("COG at index % is missing value for non-local symbol '%'", cidx, sym.name)
                        );
                    }
                };

                if (!is_valid_raw_init_value(sym.type, getValue())) {
                    throw std::runtime_error(
                        utils::format("COG at index % has invalid value for non-local symbol '%' of type %",
                            cidx, sym.name, cogGetSymbolTypeName(sym.type)
                        )
                    );
                }
            }
        }
    }
}

#endif // CNDTOOL_RESOURCE_H
