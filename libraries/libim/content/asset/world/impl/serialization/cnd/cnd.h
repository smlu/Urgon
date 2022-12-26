#ifndef LIBIM_CND_H
#define LIBIM_CND_H
#include <array>
#include <cstdint>
#include <cstdio>
#include <string>
#include <utility>
#include <vector>

#include "cnderror.h"
#include "thing/cnd_thing.h"

#include <libim/common.h>
#include <libim/content/asset/animation/animation.h>
#include <libim/content/asset/cog/cog.h>
#include <libim/content/asset/cog/cogscript.h>
#include <libim/content/asset/material/material.h>
#include <libim/content/asset/world/georesource.h>
#include <libim/content/audio/soundbank.h>
#include <libim/io/stream.h>
#include <libim/math/color.h>
#include <libim/math/vector2.h>
#include <libim/math/vector4.h>
#include <libim/types/flags.h>
#include <libim/types/indexmap.h>

namespace libim::content::asset {

    inline constexpr uint32_t kCndFileVersion = 3;

    /** Represents internal world state*/
    enum class CndWorldState : uint32_t
    {
        Static      = 0x1, // Resource container. e.g. jones3dstatic.cnd
        Initialized = 0x2, // world is initialized
        UpdateFog   = 0x4, // Update fog render state.
        InitHUD     = 0x8, // Initialize HUD.
    };

    struct CndHeader final
    {
        uint32_t fileSize;
        FixedString<1216, /*nullTermination=*/false> copyright;
        CndResourceName filePath;
        Flags<CndWorldState> state; // World stored in cnd file usually has state set to 0x0C. The static world has also flag 0x1 (Static) set and combined with other state results in value 0x0D.
        uint32_t version;
        float    worldGravity;
        float    ceilingSky_Z;
        float    horizonDistance;
        Vector2f horizonSkyOffset; // x,y
        Vector2f ceilingSkyOffset; // x,y
        std::array<float, 4> lodDistances; // list of distances to change level of detail
        struct {
            int32_t enabled;
            LinearColor color;
            float startDepth;
            float endDepth;
        } fog;

        uint32_t numSounds;

        uint32_t numMaterials;
        uint32_t sizeMaterials;
        uint32_t aMaterials;          // 32-bit void*
        uint32_t apMatArray;          // 32-bit pointer to pointer

        uint32_t numVertices;
        uint32_t aVertices;          // 32-bit pointer Vector3f*

        uint32_t unknown28;
        uint32_t unknown29;

        uint32_t numTexVertices;
        uint32_t aTexVertices;       // 32-bit pointer Vector2f*

        uint32_t unknown32;
        uint32_t unknown33;

        uint32_t numAdjoins;
        uint32_t aAdjoins;            // 32-bit pointer

        uint32_t numSurfaces;
        uint32_t aSurfaces;           // 32-bit pointer

        uint32_t numSectors;
        uint32_t aSectors;            // 32-bit pointer

        uint32_t numAIClasses;
        uint32_t sizeAIClasses;
        uint32_t aAIClasses;          // 32-bit pointer

        uint32_t numModels;
        uint32_t sizeModels;
        uint32_t aModels;             // 32-bit pointer

        uint32_t numSprites;
        uint32_t sizeSprites;
        uint32_t aSprites;            // 32-bit pointer

        uint32_t numKeyframes;
        uint32_t sizeKeyframes;
        uint32_t aKeyframes;          // 32-bit pointer

        uint32_t numPuppets;
        uint32_t sizePuppets;
        uint32_t aPuppets;            // 32-bit pointer

        uint32_t numSoundClasses;
        uint32_t sizeSoundClasses;
        uint32_t aSoundClasses;       // 32-bit pointer

        uint32_t numCogScripts;
        uint32_t sizeCogScripts;
        uint32_t aCogScripts;         // 32-bit pointer

        uint32_t numCogs;
        uint32_t sizeCogs;
        uint32_t aCogs;               // 32-bit pointer

        uint32_t numThingTemplates;
        uint32_t sizeThingTemplates;
        uint32_t aThingTemplates;     // 32-bit pointer

        uint32_t numThings;
        uint32_t lastThingIdx;
        uint32_t aThings;             // 32-bit pointer

        uint32_t sizePVS;
        uint32_t aPVS;                // 32-bit pointer (struct of 16 * int)
    };

    static_assert(sizeof(CndHeader) == 1568);


    struct CND final
    {
        static CndHeader readHeader(const InputStream& istream);

        /**
         * Returns the offset to the sounds section.
         *
         * @return std::size_t
         */
        static std::size_t getOffset_Sounds();

        /**
        * Parses sounds section and imports sounds to soundbank.
        * Offset of the istream has to be at the beginning of sound section.
        *
        * @param istream     - Const reference to the InputStream
        * @param bank        - Reference to the SoundBank to import the sounds into.
        * @param trackIdx    - The index of the track to import the sounds into.
        *
        * @throw CndError    - If the sound section contains errors or IO error occurs.
        */
        static void parseSection_Sounds(const InputStream& istream, audio::SoundBank& bank, std::size_t trackIdx);

        /**
         * Reads sounds section and imports sounds to soundbank.
         * The istream does not have to be at the beginning of the sound section.
         *
         * @param istream     - Const reference to the InputStream
         * @param bank        - Reference to the SoundBank to import the sounds into.
         * @param trackIdx    - The index of the track to import the sounds into.
         *
         * @throw CndError    - If the sound section contains errors or IO error occurs.
         */
        static void readSounds(const InputStream& istream, audio::SoundBank& bank, std::size_t trackIdx);
        static void writeSection_Sounds(OutputStream& ostream, audio::SoundBank& bank, std::size_t trackIdx);

        [[nodiscard]] static std::size_t getOffset_Materials(const InputStream& istream);
        [[nodiscard]] static IndexMap<Material> parseSection_Materials(const InputStream& istream, const CndHeader& header); // Reads materials section. Offset of istream hast to be at beginning of material section.
        [[nodiscard]] static IndexMap<Material> readMaterials(const InputStream& istream);
        static void writeSection_Materials(OutputStream& ostream, const IndexMap<Material>& materials);

        [[nodiscard]] static std::size_t getOffset_Georesource(const InputStream& istream, const CndHeader& header);
        [[nodiscard]] static Georesource parseSection_Georesource(const InputStream& istream, const CndHeader& cndHeader);
        [[nodiscard]] static Georesource readGeoresource(const InputStream& istream);
        static void writeSection_Georesource(OutputStream& ostream, const Georesource& geores);

        [[nodiscard]] static std::size_t getOffset_Sectors(const InputStream& istream, const CndHeader& header);
        [[nodiscard]] static std::vector<Sector> parseSection_Sectors(const InputStream& istream, const CndHeader& header);
        [[nodiscard]] static std::vector<Sector> readSectors(const InputStream& istream);
        static void writeSection_Sectors(OutputStream& ostream, const std::vector<Sector>& sectors);

        [[nodiscard]] static std::size_t getOffset_AIClasses(const InputStream& istream, const CndHeader& header);
        [[nodiscard]] static std::vector<std::string> parseSection_AIClasses(const InputStream& istream, const CndHeader& header);
        [[nodiscard]] static std::vector<std::string> readAIClasses(const InputStream& istream);
        static void writeSection_AIClasses(OutputStream& ostream, const std::vector<std::string>& aiclasses);

        [[nodiscard]] static std::size_t getOffset_Models(const InputStream& istream, const CndHeader& header);
        [[nodiscard]] static std::vector<std::string> parseSection_Models(const InputStream& istream, const CndHeader& header);
        [[nodiscard]] static std::vector<std::string> readModels(const InputStream& istream);
        static void writeSection_Models(OutputStream& ostream, const std::vector<std::string>& models);

        [[nodiscard]] static std::size_t getOffset_Sprites(const InputStream& istream, const CndHeader& header);
        [[nodiscard]] static std::vector<std::string> parseSection_Sprites(const InputStream& istream, const CndHeader& header);
        [[nodiscard]] static std::vector<std::string> readSprites(const InputStream& istream);
        static void writeSection_Sprites(OutputStream& ostream, const std::vector<std::string>& sprites);

        [[nodiscard]] static std::size_t getOffset_Keyframes(const InputStream& istream, const CndHeader& header);
        [[nodiscard]] static IndexMap<Animation> parseSection_Keyframes(const InputStream& istream, const CndHeader& header); // Reads keyframes section. Offset of istream hast to be at beginning of keyframe section.
        [[nodiscard]] static IndexMap<Animation> readKeyframes(const InputStream& istream);
        static void writeSection_Keyframes(OutputStream& ostream, const IndexMap<Animation>& animations);

        [[nodiscard]] static std::size_t getOffset_AnimClasses(const InputStream& istream, const CndHeader& header);
        [[nodiscard]] static std::vector<std::string> parseSection_AnimClasses(const InputStream& istream, const CndHeader& header);
        [[nodiscard]] static std::vector<std::string> readAnimClasses(const InputStream& istream);
        static void writeSection_AnimClasses(OutputStream& ostream, const std::vector<std::string>& animclasses);

        [[nodiscard]] static std::size_t getOffset_SoundClasses(const InputStream& istream, const CndHeader& header);
        [[nodiscard]] static std::vector<std::string> parseSection_SoundClasses(const InputStream& istream, const CndHeader& header);
        [[nodiscard]] static std::vector<std::string> readSoundClasses(const InputStream& istream);
        static void writeSection_SoundClasses(OutputStream& ostream, const std::vector<std::string>& sndclasses);

        [[nodiscard]] static std::size_t getOffset_CogScripts(const InputStream& istream, const CndHeader& header);
        [[nodiscard]] static std::vector<std::string> parseSection_CogScripts(const InputStream& istream, const CndHeader& header);
        [[nodiscard]] static std::vector<std::string> readCogScripts(const InputStream& istream);
        static void writeSection_CogScripts(OutputStream& ostream, const std::vector<std::string>& scripts);

        [[nodiscard]] static std::size_t getOffset_Cogs(const InputStream& istream, const CndHeader& header);
        [[nodiscard]] static std::vector<SharedRef<Cog>> parseSection_Cogs(const InputStream& istream, const CndHeader& header, const IndexMap<SharedRef<CogScript>>& scripts);
        [[nodiscard]] static std::vector<SharedRef<Cog>> readCogs(const InputStream& istream, const IndexMap<SharedRef<CogScript>>& scripts);
        static void writeSection_Cogs(OutputStream& ostream, const std::vector<SharedRef<Cog>>& cogs);

        [[nodiscard]] static std::size_t getOffset_Templates(const InputStream& istream, const CndHeader& header);
        [[nodiscard]] static IndexMap<CndThing> parseSection_Templates(const InputStream& istream, const CndHeader& header);
        [[nodiscard]] static IndexMap<CndThing> readTemplates(const InputStream& istream);
        static void writeSection_Templates(OutputStream& ostream, const IndexMap<CndThing>& templates);

        [[nodiscard]] static std::size_t getOffset_Things(const InputStream& istream, const CndHeader& header);
        [[nodiscard]] static std::vector<CndThing> parseSection_Things(const InputStream& istream, const CndHeader& header, const IndexMap<CndThing>& templates);
        [[nodiscard]] static std::vector<CndThing> readThings(const InputStream& istream, const IndexMap<CndThing>& templates);
        static void writeSection_Things(OutputStream& ostream, const std::vector<CndThing>& things, const IndexMap<CndThing>& templates);

        // Note: Section PVS is optional and it doesn't need to be written but performance will be degraded.
        //       Also if this section is not written, the sectors in sector section should have pvsIdx member set to -1
        [[nodiscard]] static std::size_t getOffset_PVS(const InputStream& istream, const CndHeader& header);
        [[nodiscard]] static ByteArray parseSection_PVS(const InputStream& istream);
        [[nodiscard]] static ByteArray readPVS(const InputStream& istream);
        static void writeSection_PVS(OutputStream& ostream, const ByteArray& pvs);
    };
}
#endif // LIBIM_CND_H
