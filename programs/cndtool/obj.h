#ifndef CNDTOOL_OBJ_H
#define CNDTOOL_OBJ_H
#include <libim/content/asset/material/texture.h>
#include <libim/content/asset/material/texutils.h>
#include <libim/content/asset/world/impl/serialization/cnd/cnd.h>
#include <libim/content/asset/world/impl/serialization/world_ser_common.h>
#include <libim/content/text/text_resource_writer.h>
#include <libim/io/filestream.h>
#include <libim/utils/utils.h>

#include <array>
#include <filesystem>
#include <string>
#include <string_view>
#include <stdexcept>

namespace cndtool {
    constexpr std::string_view kMtlFolder               = "mtl";
    constexpr std::string_view kMtlPngFolder            = "mtl/png";
    constexpr std::string_view kImgTransparentFileName  = "transparent.png";
    constexpr std::string_view kNewMtlTransparent       = "newmtl transparent";
    constexpr std::string_view kUseMtlTransparent       = "usemtl transparent";
    constexpr std::string_view kMapKdTransparent        = "map_Kd png/transparent.png";

    constexpr std::array<unsigned char, 334> kImgTransparentPng = {
        0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00, 0x0d, 0x49, 0x48, 0x44, 0x52,
        0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x08, 0x06, 0x00, 0x00, 0x00, 0x5c, 0x72, 0xa8,
        0x66, 0x00, 0x00, 0x01, 0x15, 0x49, 0x44, 0x41, 0x54, 0x78, 0xda, 0xed, 0xc1, 0x31, 0x01, 0x00,
        0x00, 0x00, 0xc2, 0xa0, 0xf5, 0x4f, 0xed, 0x6b, 0x08, 0xa0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x78, 0x03, 0x01, 0x3c, 0x00, 0x01, 0xd8, 0x29,
        0x43, 0x04, 0x00, 0x00, 0x00, 0x00, 0x49, 0x45, 0x4e, 0x44, 0xae, 0x42, 0x60, 0x82
    };


    void convertCndToObj(const std::filesystem::path& inCndPath, const std::filesystem::path& outFolder, bool extractMat)
    {
        using namespace libim;
        using namespace libim::content::asset;
        using namespace std::string_literals;
        namespace fs = std::filesystem;

        InputFileStream icnds(inCndPath);
        auto mats   = CND::readMaterials(icnds);
        auto geores = CND::readGeoresource(icnds);

        // Write mtl
        const auto mtlPath = kMtlFolder / fs::path(inCndPath).filename().replace_extension(".mtl");
        {
            const auto mtlOutPath = outFolder / mtlPath;
            makePath(mtlOutPath);

            OutputFileStream mtls(mtlOutPath);
            TextResourceWriter rw(mtls);
            rw.setIndentCh(' ');

            // Add transparent material at the the top of the list
            rw.writeLine(kNewMtlTransparent);
            rw.writeLine("Kd 1.00000 0.000000 0.000000");
            rw.writeLine("d 0.1"); // make transparent
            rw.writeLine(kMapKdTransparent);

            // Write transparent image to mat folder if it doesn't exists
            {
                auto matTransPath = outFolder / kMtlPngFolder / kImgTransparentFileName;
                if (!fileExists(matTransPath))
                {
                    makePath(matTransPath);
                    auto nWritten = OutputFileStream(matTransPath, /*truncate=*/true)
                        .write(kImgTransparentPng.data(), kImgTransparentPng.size());
                    if (nWritten != kImgTransparentPng.size()) {
                        throw std::runtime_error(
                            "Failed to write file " + std::string(kImgTransparentFileName)
                        );
                    }
                }
            }

            for (const auto& m : mats)
            {
                const auto name =fs::path(m.name()).stem().string();
                if (extractMat)
                {
                    const auto matOutPath =  outFolder / kMtlPngFolder / (name + ".png");
                    makePath(matOutPath);
                    pngWriteTexture(OutputFileStream(matOutPath, /*truncate=*/true), m.cells().at(0));
                }

                rw.writeLine("newmtl " + name);
                rw.writeLine("d 1.0");
                rw.writeLine("map_Kd " + std::string(kMtlFolder) + "/" + name + ".png");
                rw.writeEol();
            }
        }

        // Write obj
        {
            const auto objOutPath = outFolder / fs::path(inCndPath).filename().replace_extension(".obj");
            makePath(objOutPath);

            OutputFileStream objs(objOutPath, /*truncate=*/true);
            TextResourceWriter rw(objs);
            rw.setIndentCh('\t');

            rw.writeLine("mtllib " + std::string(kMtlFolder) + "/" + mtlPath.filename().string());
            rw.writeLine("o " + mtlPath.filename().stem().string());

            // Write vertices
            for (const auto& v : geores.verts)
            {
                rw.write("v");
                rw.writeVector(v, /*indent=*/ 1);
                rw.writeEol();
            }

            rw.writeEol();
            for (const auto& uv : geores.texVerts)
            {
                rw.write("vt");
                rw.writeVector(Vector2f(uv.x(), -uv.y()), /*indent=*/ 1); // UV.y is flipped (blender)
                rw.writeEol();
            }

            rw.writeEol();
            for (const auto& s : geores.surfaces)
            {
                // Write texture to use with face
                const auto matIdx = fromOptionalIdx(s.matIdx);
                if (matIdx != -1 && matIdx < mats.size()) {
                    const auto name =
                        fs::path(mats.at(matIdx).name()).stem().string();
                    rw.writeLine("usemtl " + name);
                }
                else {
                    rw.writeLine(kUseMtlTransparent);
                }

                // Write face normal
                rw.write("vn");
                rw.writeVector(s.normal, /*indent=*/ 1);
                rw.writeEol();

                // Write face vertex and UV incices
                rw.write("f\t");
                for (const auto& v : s.verts)
                {
                    rw.writeNumber(v.vertIdx + 1); // Note: idx in obj starts at 1
                    rw.write("/");
                    auto uvIdx = fromOptionalIdx(v.uvIdx) + 1; // Note: idx in obj starts at 1
                    if (uvIdx != 0) {
                        rw.writeNumber(uvIdx);
                    }
                    rw.write("/");
                    rw.writeNumber(s.id + 1); // normal idx. Note: idx in obj starts at 1
                    rw.indent(1);
                }
                rw.writeEol();
            }
        }
    }
}

#endif // CNDTOOL_OBJ_H
