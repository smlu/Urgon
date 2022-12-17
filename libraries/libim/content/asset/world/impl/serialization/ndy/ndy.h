#ifndef LIBIM_NDY_H
#define LIBIM_NDY_H
#include "../cnd/cnd.h"

#include <libim/content/asset/animation/animation.h>
#include <libim/content/asset/cog/cog.h>
#include <libim/content/asset/cog/cogscript.h>
#include <libim/content/asset/material/material.h>
#include <libim/content/asset/world/sector.h>
#include <libim/content/asset/world/georesource.h>
#include <libim/content/audio/sound.h>
#include <libim/content/text/text_resource_reader.h>
#include <libim/content/text/text_resource_writer.h>
#include <libim/io/stream.h>
#include <libim/types/indexmap.h>
#include <libim/types/sharedref.h>

#include <string>
#include <utility>
#include <vector>

namespace libim::content::asset {
    struct NDY final
    {
        static constexpr auto kSectionCopyright   = std::string_view{ "COPYRIGHT"   };
        static constexpr auto kSectionHeader      = std::string_view{ "HEADER"      };
        static constexpr auto kSectionSounds      = std::string_view{ "SOUNDS"      };
        static constexpr auto kSectionMaterials   = std::string_view{ "MATERIALS"   };
        static constexpr auto kSectionGeoresource = std::string_view{ "GEORESOURCE" };
        static constexpr auto kSectionSectors     = std::string_view{ "SECTORS"     };
        static constexpr auto kSectionAIClass     = std::string_view{ "AICLASS"     };
        static constexpr auto kSectionModels      = std::string_view{ "MODELS"      };
        static constexpr auto kSectionSprites     = std::string_view{ "SPRITES"     };
        static constexpr auto kSectionKeyframes   = std::string_view{ "KEYFRAMES"   };
        static constexpr auto kSectionAnimClass   = std::string_view{ "ANIMCLASS"   };
        static constexpr auto kSectionSoundClass  = std::string_view{ "Soundclass"  };
        static constexpr auto kSectionCogScripts  = std::string_view{ "cogscripts"  };
        static constexpr auto kSectionCogs        = std::string_view{ "cogs"        };
        static constexpr auto kSectionTemplates   = std::string_view{ "TEMPLATES"   };
        static constexpr auto kSectionThings      = std::string_view{ "THINGS"   };
        static constexpr auto kSectionPVS         = std::string_view{ "PVS"   };

        /* Parse/Write for section Copyright */
        static bool parseSection_Copyright(text::TextResourceReader& rr); // Retuns true if copyright notice is valid
        static void writeSection_Copyright(text::TextResourceWriter& rw);

        /* Parse/Write for section Header */
        [[nodiscard]] static CndHeader parseSection_Header(text::TextResourceReader& rr);
        static void writeSection_Header(text::TextResourceWriter& rw, const CndHeader& header );

        /* Parse/Write for section Sounds */
        [[nodiscard]] static std::pair<std::size_t, std::vector<std::string>>
                    parseSection_Sounds(text::TextResourceReader& rr); // Returns pair of max no. of world sounds and the list of wav file names
        static void writeSection_Sounds(text::TextResourceWriter& rw, std::size_t maxSounds, const std::vector<std::string>& sounds);
        static void writeSection_Sounds(text::TextResourceWriter& rw, std::size_t maxSounds, const IndexMap<audio::Sound>& track);

        /* Parse/Write for section Materials */
        [[nodiscard]] static std::pair<std::size_t, std::vector<std::string>>
                    parseSection_Materials(text::TextResourceReader& rr); // Returns pair of max no. of world materials and the list of mat file names
        static void writeSection_Materials(text::TextResourceWriter& rw, const std::vector<std::string>& materials);
        static void writeSection_Materials(text::TextResourceWriter& rw, const IndexMap<Material>& materials);

        /* Parse/Write for section Georesource */
        [[nodiscard]] static Georesource parseSection_Georesource(text::TextResourceReader& rr);
        static void writeSection_Georesource(text::TextResourceWriter& rw, const Georesource& geores);

        /* Parse/Write for section Sectors */
        [[nodiscard]] static std::vector<Sector> parseSection_Sectors(text::TextResourceReader& rr);
        static void writeSection_Sectors(text::TextResourceWriter& rw, const std::vector<Sector>& sectors);

        /* Parse/Write for section AIClass */
        [[nodiscard]] static std::pair<std::size_t, std::vector<std::string>>
                    parseSection_AIClass(text::TextResourceReader& rr); // Returns pair of max no. of world ai classes and the list of ai file names
        static void writeSection_AIClass(text::TextResourceWriter& rw, std::size_t maxAIClasses, const std::vector<std::string>& aiclasses);

        /* Parse/Write for section Models */
        [[nodiscard]] static std::pair<std::size_t, std::vector<std::string>>
                    parseSection_Models(text::TextResourceReader& rr); // Returns pair of max no. of world models and the list of 3do file names
        static void writeSection_Models(text::TextResourceWriter& rw, std::size_t maxModels, const std::vector<std::string>& models);

        /* Parse/Write for section Sprites */
        [[nodiscard]] static std::pair<std::size_t, std::vector<std::string>>
                    parseSection_Sprites(text::TextResourceReader& rr); // Returns pair of max no. of world sprites and the list of spr file names
        static void writeSection_Sprites(text::TextResourceWriter& rw, std::size_t maxSprites, const std::vector<std::string>& sprites);

        /* Parse/Write for section Keyframes */
        [[nodiscard]] static std::pair<std::size_t, std::vector<std::string>>
                    parseSection_Keyframes(text::TextResourceReader& rr); // Returns pair of max no. of world keyframes and the list of key file names
        static void writeSection_Keyframes(text::TextResourceWriter& rw, std::size_t maxKeyframes, const std::vector<std::string>& keyframes);
        static void writeSection_Keyframes(text::TextResourceWriter& rw, std::size_t maxKeyframes, const IndexMap<Animation>& keyframes);

        /* Parse/Write for section AnimClass */
        [[nodiscard]] static std::pair<std::size_t, std::vector<std::string>>
                    parseSection_AnimClass(text::TextResourceReader& rr); // Returns pair of max no. of world puppets and the list of pup file names
        static void writeSection_AnimClass(text::TextResourceWriter& rw, std::size_t maxAnimClasses, const std::vector<std::string>& puppets);

        /* Parse/Write for section SoundClass */
        [[nodiscard]] static std::pair<std::size_t, std::vector<std::string>>
                    parseSection_SoundClass(text::TextResourceReader& rr); // Returns pair of max no. of world sound classes and the list of snd file names
        static void writeSection_SoundClass(text::TextResourceWriter& rw, std::size_t maxSoundClasses, const std::vector<std::string>& sndclasses);

        /* Parse/Write for section CogScripts */
        [[nodiscard]] static std::pair<std::size_t, std::vector<std::string>>
                    parseSection_CogScripts(text::TextResourceReader& rr); // Returns pair of max no. of world cog scripts and the list of cog file names
        static void writeSection_CogScripts(text::TextResourceWriter& rw, std::size_t maxCogScripts, const std::vector<std::string>& scripts);

        /* Parse/Write for section Cogs */
        [[nodiscard]] static std::pair<std::size_t, std::vector<SharedRef<Cog>>>
                    parseSection_Cogs(text::TextResourceReader& rr, const IndexMap<SharedRef<CogScript>>& scripts); // Returns pair of max no. of world cogs and list of world cogs
        static void writeSection_Cogs(text::TextResourceWriter& rw, std::size_t maxCogs, const std::vector<SharedRef<Cog>>& cogs);

        /* Parse/Write for section Templates */
        [[nodiscard]] static IndexMap<CndThing> parseTemplateList(text::TextResourceReader& rr);
        [[nodiscard]] static IndexMap<CndThing> parseTemplateList(text::TextResourceReader&& rr);

        [[nodiscard]] static std::pair<std::size_t, IndexMap<CndThing>>
        parseSection_Templates(text::TextResourceReader& rr);

        static void writeSection_Templates(text::TextResourceWriter& rw, std::size_t maxTemplates, const IndexMap<CndThing>& templates);

        /* Parse/Write for section Things */
        [[nodiscard]] static std::vector<CndThing> parseSection_Things(text::TextResourceReader& r, const IndexMap<CndThing>& templates);
        static void writeSection_Things(text::TextResourceWriter& rw, const std::vector<CndThing>& things, const IndexMap<CndThing>& templates);

        /* Parse/Write for section PVS */
        /* Note: Section PVS is optional and it doesn't need to be written. */
        [[nodiscard]] static ByteArray parseSection_PVS(TextResourceReader& rr, std::vector<Sector>& sectors);
        static void writeSection_PVS(text::TextResourceWriter& rw, const ByteArray& pvs, const std::vector<Sector>& sectors);

    private:
        template<bool hasRowIdx, typename Container, typename Lambda>
        static void writeResourceSection(text::TextResourceWriter& rw,
                                         std::string_view sectionDescription,
                                         std::string_view sectionName,
                                         std::string_view listName,
                                         std::size_t maxResources,
                                         const Container& c,
                                         Lambda&& nameExtractor,
                                         std::string_view listHeader = std::string_view())
        {
            using namespace std::string_view_literals;

            rw.writeLine(sectionDescription);
            rw.writeSection(sectionName, /*overline=*/ false);
            rw.writeEol();

            rw.writeKeyValue(listName, maxResources);

            if(listHeader.empty()) {
                rw.writeEol();
            }
            else {
                rw.writeCommentLine(listHeader);
            }

            AT_SCOPE_EXIT([&rw, ich = rw.indentChar()](){
                rw.setIndentChar(ich);
            });

            rw.setIndentChar('\t');

            rw.writeList(c, [&](auto& rw, [[maybe_unused]]auto idx, auto& v){
                if constexpr (hasRowIdx)
                {
                    rw.writeRowIdx(idx, 0);
                    rw.indent(1);
                }
                rw.write(nameExtractor(v));
            });

            rw.writeLine("################################"sv);
            rw.writeEol();
            rw.writeEol();
        }

        template<bool hasRowIdxs>
        static std::pair<std::size_t, std::vector<std::string>>
        parseResourceSection(text::TextResourceReader& rr, std::string_view listName)
        {
            std::size_t maxResources = rr.readKey<std::size_t>(listName);
            auto resources = rr.readList<std::vector<std::string>, hasRowIdxs>(
            [](auto& rr, auto /*rowIdx*/, auto& r) {
                r = rr.getDelimitedString([](char c) { return c == '\r' || c == '\n'; });
            });

            return { maxResources, std::move(resources) };
        }
    };
}

#endif // LIBIM_NDY_H
