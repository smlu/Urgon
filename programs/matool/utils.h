#ifndef MATOOL_UTILS_H
#define MATOOL_UTILS_H
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <optional>
#include <regex>
#include <string>
#include <string_view>

#include <cmdutils/cmdutils.h>

#include <libim/content/asset/material/colorformat.h>
#include <libim/content/asset/material/material.h>
#include <libim/content/asset/material/texture.h>
#include <libim/content/asset/material/texture_view.h>
#include <libim/io/filestream.h>
#include <libim/math/math.h>
#include <libim/utils/utils.h>

#define MATOOL_SET_DOT_LW(n) CMDUTILS_SETW(32 + n, '.')

namespace matool {
    static constexpr std::string_view kExtBmp = ".bmp";
    static constexpr std::string_view kExtMat = ".mat";
    static constexpr std::string_view kExtPng = ".png";

    namespace detail {
        static const auto kLodSeqSufRegex = std::regex("__lod_[0-9]+$");
        static const auto kSeqSufRegex    = std::regex("__cel_[0-9]+$");
        inline const std::string makeLodSeqSuf(std::size_t idx) {
            return "__lod_" + std::to_string(idx);
        }
        inline const std::string makeSeqSuf(std::size_t idx) {
            return "__cel_" + std::to_string(idx);
        }

        inline std::string_view removeSuffix(const std::string_view& name, const std::regex& regex)
        {
            std::match_results<std::string_view::const_iterator> match;
            if (std::regex_search(name.begin(), name.end(), match, regex)) {
                return name.substr(0, match.position());
            }
            return name;
        }
    }

    [[nodiscard]] inline std::string_view removeSeqSuffix(const std::string_view& name) {
        return detail::removeSuffix(name, detail::kSeqSufRegex);
    }

    [[nodiscard]] inline std::string_view removeLodSeqSuffix(const std::string_view& name) {
        return detail::removeSuffix(name, detail::kLodSeqSufRegex);
    }

    [[nodiscard]] inline bool hasLodSeqSuf(const std::string_view& name) {
        return std::regex_search(name.begin(), name.end(), detail::kLodSeqSufRegex);
    }

    inline void appendSeqSuffix(std::string& name, std::size_t idx) {
        name += detail::makeSeqSuf(idx);
    }

    inline void appendLodSeqSuffix(std::string& name, std::size_t idx) {
        name += detail::makeLodSeqSuf(idx);
    }

    static void texPrintInfo(const libim::content::asset::Texture& tex)
    {
        using namespace libim;
        using namespace libim::content::asset;
        if(tex.isEmpty()) return;

        std::string colorMode;
        switch (tex.format().mode)
        {
            case ColorMode::Indexed:
                colorMode = "Indexed";
                break;
            case ColorMode::RGB:
                colorMode = "RGB";
                break;
            case ColorMode::RGBA:
                colorMode = "RGBA";
                break;
            default:
                colorMode = "Unknown";
                break;
        }

        std::cout << "    ------------------ Texture Info -----------------\n";
        std::cout << "    Width:"           << MATOOL_SET_DOT_LW(10) << tex.width()           << std::endl;
        std::cout << "    Height:"          << MATOOL_SET_DOT_LW(9)  << tex.height()          << std::endl;
        std::cout << "    MimMap levels:"   << MATOOL_SET_DOT_LW(2)  << tex.mipLevels()       << std::endl;
        std::cout << "    Pixel data size:" << MATOOL_SET_DOT_LW(0)  << tex.pixdata()->size() << std::endl;
        std::cout << "    Color info:\n";
        std::cout << "      Color mode:" << MATOOL_SET_DOT_LW(3) << colorMode        << std::endl;
        std::cout << "      Bit depth:"  << MATOOL_SET_DOT_LW(4) << tex.format().bpp << std::endl;

        if (tex.format() == RGB555) {
            std::cout << "      Encoding:" << MATOOL_SET_DOT_LW(5) << "RGB-555" << std::endl;
        }
        else if (tex.format() == RGB555be) {
            std::cout << "      Encoding:" << MATOOL_SET_DOT_LW(5) << "RGB-555be" << std::endl;
        }
        else if (tex.format() == RGB565) {
            std::cout << "      Encoding:" << MATOOL_SET_DOT_LW(5) << "RGB-565" << std::endl;
        }
        else if (tex.format() == RGB565be) {
            std::cout << "      Encoding:" << MATOOL_SET_DOT_LW(5) << "RGB-565be" << std::endl;
        }
        else if(tex.format() == RGB24) {
            std::cout << "      Encoding:" << MATOOL_SET_DOT_LW(5) << "RGB-888" << std::endl;
        }
        else if  (tex.format() == RGB24be) {
            std::cout << "      Encoding:" << MATOOL_SET_DOT_LW(5) << "RGB-888be" << std::endl;
        }
        else if (tex.format() == RGBA4444) {
            std::cout << "      Encoding:" << MATOOL_SET_DOT_LW(5) << "RGBA-4444" << std::endl;
        }
        else if (tex.format() == RGBA4444be) {
            std::cout << "      Encoding:" << MATOOL_SET_DOT_LW(5) << "RGBA-4444be" << std::endl;
        }
        else if (tex.format() == ARGB4444) {
            std::cout << "      Encoding:" << MATOOL_SET_DOT_LW(5) << "ARGB-4444" << std::endl;
        }
        else if (tex.format() == ARGB4444be) {
            std::cout << "      Encoding:" << MATOOL_SET_DOT_LW(5) << "ARGB-4444be" << std::endl;
        }
        else if (tex.format() == RGBA5551) {
            std::cout << "      Encoding:" << MATOOL_SET_DOT_LW(5) << "RGBA-5551" << std::endl;
        }
        else if (tex.format() == RGBA5551be) {
            std::cout << "      Encoding:" << MATOOL_SET_DOT_LW(5) << "RGBA-5551be" << std::endl;
        }
        else if (tex.format() == ARGB1555) {
            std::cout << "      Encoding:" << MATOOL_SET_DOT_LW(5) << "ARGB-5551" << std::endl;
        }
        else if (tex.format() == ARGB1555be) {
            std::cout << "      Encoding:" << MATOOL_SET_DOT_LW(5) << "ARGB-5551be" << std::endl;
        }
        else if (tex.format() == RGBA32) {
            std::cout << "      Encoding:" << MATOOL_SET_DOT_LW(5) << "RGBA-8888" << std::endl;
        }
        else if (tex.format() == RGBA32be) {
            std::cout << "      Encoding:" << MATOOL_SET_DOT_LW(5) << "RGBA-8888be" << std::endl;
        }
        else if (tex.format() == ARGB32) {
            std::cout << "      Encoding:" << MATOOL_SET_DOT_LW(5) << "ARGB-8888" << std::endl;
        }
        else if (tex.format() == ARGB32be) {
            std::cout << "      Encoding:" << MATOOL_SET_DOT_LW(5) << "ARGB-8888be" << std::endl;
        }
        else
        {
            std::cout << "      Bit depth per channel:" << std::endl;
            std::cout << "        Red:"   << MATOOL_SET_DOT_LW(8) << tex.format().redBPP   << std::endl;
            std::cout << "        Green:" << MATOOL_SET_DOT_LW(6) << tex.format().greenBPP << std::endl;
            std::cout << "        Blue:"  << MATOOL_SET_DOT_LW(7) << tex.format().blueBPP  << std::endl;
            std::cout << "        Alpha:" << MATOOL_SET_DOT_LW(6) << tex.format().alphaBPP << std::endl;
            std::cout << "      Left shift per channel:" << std::endl;
            std::cout << "        Red:"   << MATOOL_SET_DOT_LW(8) << tex.format().redShl   << std::endl;
            std::cout << "        Green:" << MATOOL_SET_DOT_LW(6) << tex.format().greenShl << std::endl;
            std::cout << "        Blue:"  << MATOOL_SET_DOT_LW(7) << tex.format().blueShl  << std::endl;
            std::cout << "        Alpha:" << MATOOL_SET_DOT_LW(6) << tex.format().alphaShl << std::endl;
            std::cout << "      Right shift per channel:" << std::endl;
            std::cout << "        Red:"   << MATOOL_SET_DOT_LW(8) << tex.format().redShr   << std::endl;
            std::cout << "        Green:" << MATOOL_SET_DOT_LW(6) << tex.format().greenShr << std::endl;
            std::cout << "        Blue:"  << MATOOL_SET_DOT_LW(7) << tex.format().blueShr  << std::endl;
            std::cout << "        Alpha:" << MATOOL_SET_DOT_LW(6) << tex.format().alphaShr << std::endl;
        }
        std::cout << "    -------------------------------------------------\n";
    }

    inline void matPrintInfo(const libim::content::asset::Material& mat)
    {
        std::cout << "\n  ================== Material Info ===================\n";
        std::cout << "    Total textures:" << MATOOL_SET_DOT_LW(1) << mat.count() << std::endl;
        if (mat.count()) texPrintInfo(mat.cel(0));
        std::cout << "  ===================================================\n\n\n";
    }

    /**
     * Extract images from Material to file.
     *
     * @param mat          - const reference to MAterial object.
     * @param outDir       - directory to save extracted images.
     * @param optMaxCel    - (Optional) max number of images to extract from Material object.
     * @param extractLod   - extract also LOD images of each Texture.
     * @param extractAsBmp - extract images in BMP file format. Default is PNG.
     */
    static void
    matExtractImages(const libim::content::asset::Material& mat,
        const std::filesystem::path& outDir,
        const std::optional<uint64_t> optMaxCel = std::nullopt,
        const bool extractLod   = false,
        const bool extractAsBmp = false)
    {
        using namespace libim;
        const auto maxCelCount = min(mat.count(), optMaxCel.value_or(mat.count()));
        for (const auto [celNum, tex] : utils::enumerate(mat.cells()))
        {
            if(celNum >= maxCelCount) break;

            const std::size_t mipLevels = extractLod ? tex.mipLevels() : 1;
            for (uint32_t lod = 0; lod < mipLevels; lod++)
            {
                std::string fileName = getBaseName(mat.name());
                if (maxCelCount > 1) appendSeqSuffix(fileName, celNum);
                if (mipLevels > 1 && lod > 0)  appendLodSeqSuffix(fileName, lod);

                const auto outPath = (outDir / fileName).replace_extension(extractAsBmp ? kExtBmp : kExtPng);
                OutputFileStream ofs(outPath, /*truncate=*/true);
                if (extractAsBmp) {
                    bmpWriteTexture(ofs, tex.mipmap(lod));
                }
                else {
                    pngWriteTexture(ofs, tex.mipmap(lod));
                }
            }
        }
    }
}

#endif // MATOOL_UTILS_H