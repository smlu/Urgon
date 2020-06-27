#ifndef PATCH_H
#define PATCH_H
#include <string>

#include <libim/content/asset/material/material.h>
#include <libim/content/asset/world/impl/serialization/cnd/cnd.h>
#include <libim/content/asset/world/impl/serialization/cnd/animation/cnd_key_structs.h>
#include <libim/content/asset/world/impl/serialization/cnd/material/cnd_mat_header.h>
#include <libim/io/stream.h>
#include <libim/types/hashmap.h>

using namespace libim;
using namespace libim::content::asset;
using namespace libim::utils;

    namespace cndtool {
    bool patchCndMaterials(const std::string& cndFile, const HashMap<Material>& materials)
    {
        try
        {
            InputFileStream ifstream(cndFile);

            /* Read cnd file header (this alos verifies if flie is valid cnd) */
            auto cndHeader = CND::readHeader(ifstream);

            /* Open new output cnd file */
            const std::string patchedCndFile = cndFile + ".patched";
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
            if(cndHeader.sizeMaterials < materials.size()) {
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
            deleteFile(cndFile + ".patched");
            return false;
        }
    }

    bool patchCndAnimations(const std::string& cndFile, const HashMap<Animation>& animations)
    {
        try
        {
            InputFileStream ifstream(cndFile);

            /* Read cnd file header (this alos verifies if flie is valid cnd) */
            auto cndHeader = CND::readHeader(ifstream);

            /* Open new output cnd file */
            const std::string patchedCndFile = cndFile + ".patched";
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
            deleteFile(cndFile + ".patched");
            return false;
        }
    }
}
#endif // PATCH_H
