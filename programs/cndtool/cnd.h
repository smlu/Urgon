#ifndef CNDTOOL_CND_H
#define CNDTOOL_CND_H
#include <filesystem>
#include <string>

#include <cmdutils/cmdutils.h>

#include <libim/content/asset/material/material.h>
#include <libim/content/asset/world/impl/serialization/cnd/animation/cnd_key_structs.h>
#include <libim/content/asset/world/impl/serialization/cnd/cnd.h>
#include <libim/content/asset/world/impl/serialization/cnd/material/cnd_mat_header.h>
#include <libim/content/audio/soundbank.h>

#include <libim/io/filestream.h>
#include <libim/io/stream.h>
#include <libim/types/flags.h>
#include <libim/types/indexmap.h>
#include <libim/types/safe_cast.h>

#include "ndy.h"
#include "resource.h"

namespace cndtool {
    namespace fs = std::filesystem;
    using namespace libim;
    using namespace libim::content::asset;
    using namespace libim::content::audio;
    using namespace libim::utils;

    bool patchCndMaterials(const fs::path& cndFile, const Table<Material>& materials)
    {
        const fs::path patchedCndFile = cndFile.string() + ".patched";
        try
        {
            InputFileStream ifstream(cndFile);

            /* Read cnd file header (this alos verifies if flie is valid cnd) */
            auto cndHeader = CND::readHeader(ifstream);

            /* Open new output cnd file */
            OutputFileStream ofstream(patchedCndFile, /*truncate=*/true);

            /* Copy input cnd file to output stream until materials section */
            const auto matSectionOffset = CND::getOffset_Materials(ifstream);
            ofstream.write(ifstream, 0, matSectionOffset);
            std::size_t oldSizePixeldata = ifstream.read<uint32_t>();

            /* Write new materials section */
            CND::writeSection_Materials(ofstream, materials);

            /* Write the rest of inputted cnd file to the output */
            std::size_t endOffsMatSection = ifstream.tell() + sizeof(CndMatHeader) * cndHeader.numMaterials + oldSizePixeldata;
            ofstream.write(ifstream, endOffsMatSection);

            /* Write new file size to the beginning of the output cnd file*/
            ofstream.seekBegin();
            ofstream.write(safe_cast<uint32_t>(ofstream.size()));

            /* Update materials info in CND header */
            ofstream.seek(offsetof(CndHeader, numMaterials));
            ofstream.write(safe_cast<uint32_t>(materials.size())); // numMaterials field
            if (cndHeader.sizeMaterials < materials.size()) {
                ofstream.write(safe_cast<uint32_t>(materials.size())); // sizeMaterials field
            }

            /* Close IO file stream */
            ifstream.close();
            ofstream.close();

            /* Rename patched file name to original name */
            renameFile(patchedCndFile, cndFile);
            return true;
        }
        catch(const std::exception& e)
        {
            std::cerr << "ERROR: Failed to patch material section!\n";
            std::cerr << "       Reason: " << e.what() << std::endl;
            deleteFile(patchedCndFile);
            return false;
        }
    }

    bool patchCndAnimations(const fs::path& cndFile, const UniqueTable<Animation>& animations)
    {
        const fs::path patchedCndFile = cndFile.string() + ".patched";
        try
        {
            InputFileStream ifstream(cndFile);

            /* Read cnd file header (this alos verifies if flie is valid cnd) */
            auto cndHeader = CND::readHeader(ifstream);

            /* Open new output cnd file */
            OutputFileStream ofstream(patchedCndFile, /*truncate=*/true);

            /* Copy input cnd file to output stream until materials section */
            const auto keySectionOffset = CND::getOffset_Keyframes(ifstream, cndHeader);
            ofstream.write(ifstream, 0, keySectionOffset);

            /* Write new materials section */
            CND::writeSection_Keyframes(ofstream, animations);

            /* Write the rest of inputted cnd file to the output */
            auto aOldNumEntries = ifstream.read<std::array<uint32_t, 3>>();
            std::size_t endOffsKeySection = ifstream.tell()       +
                    sizeof(CndKeyHeader) * cndHeader.numKeyframes +
                    sizeof(KeyMarker)    * aOldNumEntries.at(0)   +
                    sizeof(CndKeyNode)   * aOldNumEntries.at(1)   +
                    sizeof(KeyNodeEntry) * aOldNumEntries.at(2);

            ofstream.write(ifstream, /*offset=*/endOffsKeySection);

            /* Write new file size to the beginning of the output cnd file*/
            ofstream.seekBegin();
            ofstream.write(safe_cast<uint32_t>(ofstream.size()));

            /* Update materials info in CND header */
            ofstream.seek(offsetof(CndHeader, numKeyframes));
            ofstream.write(safe_cast<uint32_t>(animations.size())); // numKeyframes field
            if(cndHeader.sizeKeyframes < animations.size()) {
                ofstream.write(safe_cast<uint32_t>(animations.size())); // sizeKeyframes field
            }

            /* Close IO file stream */
            ifstream.close();
            ofstream.close();

            /* Rename patched file name to original name */
            renameFile(patchedCndFile, cndFile);
            return true;
        }
        catch(const std::exception& e)
        {
            std::cerr << "ERROR: Failed to patch keyframe section!\n";
            std::cerr << "       Reason: " << e.what() << std::endl;
            deleteFile(patchedCndFile);
            return false;
        }
    }

    /**
     * Converts NDY file to CND file.
     */
    bool convertNdyToCnd(const fs::path& ndyPath, const libim::VirtualFileSystem& vfs, const StaticResourceNames& staticResources, const fs::path& outDir, SoundHandle soundHandleSeed, bool staticCnd, bool verify, bool cleanUp, bool verbose)
    {
        fs::path cndPath;
        using namespace cmdutils;
        try
        {
            cleanUp = cleanUp && !staticCnd;
            verify  = verify  && !staticCnd;

            const std::size_t total = (cleanUp ? 21U : 20U) + (verify ? 1U : 0U);
            constexpr auto progressTitle = "Converting to CND ... "sv;
            std::size_t progress = 0;
            if (!verbose) printProgress(progressTitle, progress++, total);

            LOG_DEBUG("Reading NDY file %", ndyPath);
            auto world = ndyReadFile(ndyPath, vfs);
            LOG_DEBUG("NDY file was successfully read.");
            if (!verbose) printProgress(progressTitle, progress++, total);

            /* Clean up resources */
            if (cleanUp)
            {
                LOG_DEBUG("Removing static game resources ...");
                /* Map surface material idxs to mat names */
                auto oldMats = world.materials.second;
                auto nOldSounds  = world.sounds.second.size();
                auto nOldModels  = world.models.second.size();
                auto nOldSprites = world.sprites.second.size();
                auto nOldKeys    = world.keyframes.second.size();
                auto nOldScripts = world.cogScripts.second.size();

                // Filter out all static resources
                filterStringList(world.sounds.second     , staticResources.sounds);
                filterStringList(world.materials.second  , staticResources.materials);
                filterStringList(world.models.second     , staticResources.models);
                filterStringList(world.sprites.second    , staticResources.sprites);
                filterStringList(world.keyframes.second  , staticResources.keyframes);
                filterStringList(world.cogScripts.second , staticResources.scripts);

                if (world.sounds.second.size() != nOldSounds) {
                    LOG_DEBUG("Removed % static sounds", nOldSounds - world.sounds.second.size());
                }
                if (world.materials.second.size() != oldMats.size()) {
                    LOG_DEBUG("Removed % static materials", oldMats.size() - world.materials.second.size());
                }
                if (world.models.second.size() != nOldModels) {
                    LOG_DEBUG("Removed % static models", nOldModels - world.models.second.size());
                }
                if (world.sprites.second.size() != nOldSprites) {
                    LOG_DEBUG("Removed % static sprites", nOldSprites - world.sprites.second.size());
                }
                if (world.keyframes.second.size() != nOldKeys) {
                    LOG_DEBUG("Removed % static keyframes", nOldKeys - world.keyframes.second.size());
                }
                if (world.cogScripts.second.size() != nOldScripts) {
                    LOG_DEBUG("Removed % static scripts", nOldScripts - world.cogScripts.second.size());
                }

                /* Fix surface indices */
                if (world.materials.second.size() != oldMats.size())
                {
                    LOG_DEBUG("Fixing surface material indices ...");
                    auto newMats = world.materials.second;
                    for (auto& surf : world.georesource.surfaces)
                    {
                        int32_t matIdx = fromOptionalIdx(surf.matIdx);
                        if (matIdx > -1 && !isStaticResource(matIdx))
                        {
                            if (auto sit = staticResources.materials.find(oldMats[matIdx]);
                                sit != staticResources.materials.end()) {
                                surf.matIdx = makeStaticResourceIdx(*sit);
                            }
                            else
                            {
                                auto it = std::find(newMats.begin(), newMats.end(), oldMats[matIdx]);
                                check(it != newMats.end(), "Failed to find material '%' in filtered material list!", oldMats[matIdx]);
                                surf.matIdx = static_cast<std::size_t>(std::distance(newMats.begin(), it));
                            }
                        }
                    }
                }

                LOG_DEBUG("Finished cleaning up static game resources.");
                if (!verbose) printProgress(progressTitle, progress++, total);
            }

            /* Verify resources */
            if (verify)
            {
                LOG_DEBUG("Verifying loaded NDY file ...");
                checkNdyWorld(world, staticResources);
                LOG_DEBUG("Finished verifying NDY file.");
                if (!verbose) printProgress(progressTitle, progress++, total);
            }

            /* Load resources */
            LOG_DEBUG("Loading required CND resources ...");
            if (!verbose) printProgress(progressTitle, progress++, total);

            std::size_t sndbankIdx = getSoundBankTrackIdx(staticCnd);
            SoundBank bank(sndbankIdx + 1);
            bank.setHandleSeed(soundHandleSeed);
            bank.setStaticTrack(sndbankIdx, staticCnd); // Don't forget for this one!
            loadSounds(vfs, bank, sndbankIdx, world.sounds.second); // Always import to track 1 the normal world bank and to 0 the static world.

            if (!verbose) printProgress(progressTitle, progress++, total);
            auto mats  = loadMaterials(vfs, world.materials.second);
            auto anims = loadAnimations(vfs, world.keyframes.second);

            LOG_DEBUG("Loading resources succeed!");
            if (!verbose) printProgress(progressTitle, progress++, total);

            /* Write CND file */
            cndPath = outDir / "ndy" / ndyPath.filename().replace_extension("cnd");
            LOG_DEBUG("Creating output CND file path %", cndPath);
            makePath(cndPath);
            OutputFileStream cnds(cndPath, /*truncate=*/true);

            /* Move beyond file header */
            cnds.seek(sizeof(CndHeader));

            /* Write sections */
            LOG_DEBUG("Writing CND section 'Sounds' at offset: %", utils::to_string<16>(cnds.tell()));
            CND::writeSection_Sounds(cnds, bank, sndbankIdx);
            if (!verbose) printProgress(progressTitle, progress++, total);

            LOG_DEBUG("Writing CND section 'Materials' at offset: %", utils::to_string<16>(cnds.tell()));
            CND::writeSection_Materials(cnds, mats);
            if (!verbose) printProgress(progressTitle, progress++, total);

            LOG_DEBUG("Writing CND section 'GeoResource' at offset: %", utils::to_string<16>(cnds.tell()));
            CND::writeSection_Georesource(cnds, world.georesource);
            if (!verbose) printProgress(progressTitle, progress++, total);

            LOG_DEBUG("Writing CND section 'Sectors' at offset: %", utils::to_string<16>(cnds.tell()));
            CND::writeSection_Sectors(cnds, world.sectors);
            if (!verbose) printProgress(progressTitle, progress++, total);

            LOG_DEBUG("Writing CND section 'AIClasses' at offset: %", utils::to_string<16>(cnds.tell()));
            CND::writeSection_AIClasses(cnds, world.aiClasses.second);
            if (!verbose) printProgress(progressTitle, progress++, total);

            LOG_DEBUG("Writing CND section 'Models' at offset: %", utils::to_string<16>(cnds.tell()));
            CND::writeSection_Models(cnds, world.models.second);
            if (!verbose) printProgress(progressTitle, progress++, total);

            LOG_DEBUG("Writing CND section 'Sprites' at offset: %", utils::to_string<16>(cnds.tell()));
            CND::writeSection_Sprites(cnds, world.sprites.second);
            if (!verbose) printProgress(progressTitle, progress++, total);

            LOG_DEBUG("Writing CND section 'Keyframes' at offset: %", utils::to_string<16>(cnds.tell()));
            CND::writeSection_Keyframes(cnds, anims);
            if (!verbose) printProgress(progressTitle, progress++, total);

            LOG_DEBUG("Writing CND section 'AnimClasses' at offset: %", utils::to_string<16>(cnds.tell()));
            CND::writeSection_AnimClasses(cnds, world.animClasses.second);
            if (!verbose) printProgress(progressTitle, progress++, total);

            LOG_DEBUG("Writing CND section 'SoundClasses' at offset: %", utils::to_string<16>(cnds.tell()));
            CND::writeSection_SoundClasses(cnds, world.soundClasses.second);
            if (!verbose) printProgress(progressTitle, progress++, total);

            LOG_DEBUG("Writing CND section 'COGScripts' at offset: %", utils::to_string<16>(cnds.tell()));
            CND::writeSection_CogScripts(cnds, world.cogScripts.second);
            if (!verbose) printProgress(progressTitle, progress++, total);

            LOG_DEBUG("Writing CND section 'COGs' at offset: %", utils::to_string<16>(cnds.tell()));
            CND::writeSection_Cogs(cnds, world.cogs.second);
            if (!verbose) printProgress(progressTitle, progress++, total);

            LOG_DEBUG("Writing CND section 'Templates' at offset: %", utils::to_string<16>(cnds.tell()));
            CND::writeSection_Templates(cnds, world.templates.second);
            if (!verbose) printProgress(progressTitle, progress++, total);

            LOG_DEBUG("Writing CND section 'Things' at offset: %", utils::to_string<16>(cnds.tell()));
            CND::writeSection_Things(cnds, world.things, world.templates.second);
            if (!verbose) printProgress(progressTitle, progress++, total);

            LOG_DEBUG("Writing CND section 'PVS' at offset: %", utils::to_string<16>(cnds.tell()));
            CND::writeSection_PVS(cnds, world.pvs);
            if (!verbose) printProgress(progressTitle, progress++, total);

            // Init Header and write it to file
            LOG_DEBUG("Initializing CND file header ...");
            CndHeader header = world.header;
            header.fileSize  = cnds.tell();
            header.copyright = kWorldFileCopyright;
            header.filePath  = CndResourceName(cnds.name());
            header.version   = kCndFileVersion;

            header.state |= Flags(CndWorldState::UpdateFog) | CndWorldState::InitHUD;
            if (staticCnd) {
                header.state |= CndWorldState::Static;
            }

            header.numSounds = safe_cast<uint32_t>(world.sounds.second.size());

            header.numMaterials = safe_cast<uint32_t>(world.materials.second.size());
            if (header.sizeMaterials < world.materials.first) {
                header.sizeMaterials = safe_cast<uint32_t>(world.materials.first);
            }

            header.numVertices    = safe_cast<uint32_t>(world.georesource.vertices.size());
            header.numTexVertices = safe_cast<uint32_t>(world.georesource.texVertices.size());
            header.numAdjoins     = safe_cast<uint32_t>(world.georesource.adjoins.size());
            header.numSurfaces    = safe_cast<uint32_t>(world.georesource.surfaces.size());

            header.numSectors = world.sectors.size();
            header.numAIClasses = world.aiClasses.second.size();
            if (header.sizeAIClasses < world.aiClasses.first) {
                header.sizeAIClasses = safe_cast<uint32_t>(world.aiClasses.first);
            }

            header.numModels = safe_cast<uint32_t>(world.models.second.size());
            if (header.sizeModels < world.models.first) {
                header.sizeModels = safe_cast<uint32_t>(world.models.first);
            }

            header.numSprites = safe_cast<uint32_t>(world.sprites.second.size());
            if (header.sizeSprites < world.sprites.first) {
                header.sizeSprites = safe_cast<uint32_t>(world.sprites.first);
            }

            header.numKeyframes = safe_cast<uint32_t>(world.keyframes.second.size());
            if (header.sizeKeyframes < world.keyframes.first) {
                header.sizeKeyframes = world.keyframes.first;
            }

            header.numPuppets = safe_cast<uint32_t>(world.animClasses.second.size());
            if (header.sizePuppets < world.animClasses.first) {
                header.sizePuppets = safe_cast<uint32_t>(world.animClasses.first);
            }

            header.numSoundClasses = safe_cast<uint32_t>(world.soundClasses.second.size());
            if (header.sizeSoundClasses < world.soundClasses.first) {
                header.sizeSoundClasses = safe_cast<uint32_t>(world.soundClasses.first);
            }

            header.numCogScripts = safe_cast<uint32_t>(world.cogScripts.second.size());
            if (header.sizeCogScripts < world.cogScripts.first) {
                header.sizeCogScripts = safe_cast<uint32_t>(world.cogScripts.first);
            }

            header.numCogs = safe_cast<uint32_t>(world.cogs.first);// Note, this is hack, because engine takes this field to allocate memory for cogs instead of sizeCogs
            if (header.sizeCogs < world.cogs.first) {
                header.sizeCogs = safe_cast<uint32_t>(world.cogs.first);
            }

            header.numThingTemplates = safe_cast<uint32_t>(world.templates.second.size());
            if (header.sizeThingTemplates < world.templates.first) {
                header.sizeThingTemplates = safe_cast<uint32_t>(world.templates.first);
            }

            if (header.numThings < world.things.size()) {
                header.numThings = safe_cast<uint32_t>(world.things.size());
            }
            header.lastThingIdx = 0;

            if (header.sizePVS < world.pvs.size()) {
                header.sizePVS = safe_cast<uint32_t>(world.pvs.size());
            }

            // Write header to stream
            LOG_DEBUG("Writing CND file header to stream");
            cnds.seek(0);
            cnds.write(header);
            LOG_DEBUG("Finish converting NDY to CND.");

            if (!verbose) printProgress(progressTitle, progress++, total);
            if (!verbose) std::cout << "\r" << progressTitle << kSuccess << std::endl;
            return true;
        }
        catch (const std::exception& e)
        {
            std::cerr << std::endl;
            printError(e.what());
            deleteFile(cndPath);
            return false;
        }
    }
}
#endif // CNDTOOL_CND_H
