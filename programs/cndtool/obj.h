#ifndef CNDTOOL_OBJ_H
#define CNDTOOL_OBJ_H
#include <libim/content/asset/material/texture.h>
#include <libim/content/asset/material/texutils.h>
#include <libim/content/asset/world/impl/serialization/cnd/cnd.h>
#include <libim/content/asset/world/impl/serialization/world_ser_common.h>
#include <libim/content/text/text_resource_writer.h>
#include <libim/io/binarystream.h>
#include <libim/io/filestream.h>
#include <libim/utils/utils.h>

#include "config.h"
#include "resource.h"

#include <array>
#include <filesystem>
#include <map>
#include <set>
#include <string>
#include <string_view>
#include <stdexcept>

namespace cndtool {
    constexpr std::string_view kMtlFolder              = "mtl";
    constexpr std::string_view kMtlPngFolder           = "mtl/png";
    constexpr std::string_view kImgTransparentFileName = "transparent.png";
    constexpr std::string_view kNewMtlTransparent      = "newmtl transparent";
    constexpr std::string_view kUseMtlTransparent      = "usemtl transparent";
    constexpr std::string_view kMapKdTransparent       = "map_Kd png/transparent.png";
    constexpr std::string_view kImgDefault             = "dflt";
    constexpr std::size_t kVertexIdxMaxDigits          = 5; // e.g.: 65535
    constexpr std::size_t kUVIdxMaxDigits              = 5; // e.g.: 65535
    constexpr std::size_t kMaxFaceVerticies            = 256;
    constexpr std::size_t kFaceLineMaxChars            = 8 + 64 + // usemtl <name>\n
                                                         4 +      // f\t\n<sp>
                                                         // <max_vert>*<vidx/uvidx/vnidx><sp>
                                                         kMaxFaceVerticies * (kVertexIdxMaxDigits + 1 + kUVIdxMaxDigits + 1 + kVertexIdxMaxDigits + 1);

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

    inline libim::Vector3f getObjCoords(const libim::Vector3f& v) {
        return libim::Vector3f(v.x(), v.z(), -v.y()); // Right-hand coord sys with y as up axis.
    }

    void convertCndToObj(const std::filesystem::path& inCndPath, const StaticResourceNames& staticResources, const std::filesystem::path& outFolder, bool extractMat)
    {
        using namespace libim;
        using namespace libim::content::asset;
        using namespace std::string_literals;
        namespace fs = std::filesystem;

        InputFileStream icnds(inCndPath);
        auto mats   = CND::readMaterials(icnds);
        auto geores = CND::readGeoresource(icnds);
        if (geores.vertices.empty()) {
            throw std::runtime_error("CND file has no geometry resources");
        }

        // Load materials from jones3dstatic.cnd
        fs::path scndPath = inCndPath;
        scndPath.replace_filename(kDefaultStaticResourcesFilename);
        Table<Material> smats;
        if (fileExists(scndPath))
        {
            LOG_DEBUG("Loading materials from %", kDefaultStaticResourcesFilename);
            InputFileStream icnds(scndPath);
            smats = CND::readMaterials(icnds);
        }
        else {
            LOG_WARNING("File % was not found. Some surfaces might have incomplete texture information.", kDefaultStaticResourcesFilename);
        }

        UniqueTable<const Material*> usedMats;
        const auto mtlPath = kMtlFolder / fs::path(inCndPath).filename().replace_extension(".mtl");

        // Write obj
        {
            const auto objOutPath = outFolder / fs::path(inCndPath).filename().replace_extension(".obj");
            makePath(objOutPath);

            OutputFileStream objs(objOutPath, /*truncate=*/true);
            TextResourceWriter rw(objs);
            rw.setIndentChar('\t');
            rw.writeCommentLine("% v% OBJ file: %", kProgramName, kVersion, objOutPath.filename().string());
            rw.writeCommentLine(kProgramUrl);
            rw.writeEol();

            rw.writeLine("mtllib " + std::string(kMtlFolder) + "/" + mtlPath.filename().string());
            rw.writeLine("o " + mtlPath.filename().stem().string());

            // Write vertices
            for (const auto& v : geores.vertices)
            {
                rw.write("v");
                rw.writeVector(getObjCoords(v), /*width=*/ 1);
                rw.writeEol();
            }

            for (const auto& uv : geores.texVertices)
            {
                rw.write("vt");
                rw.writeVector(Vector2f(uv.x(), -uv.y()), /*width=*/ 1); // UV.y is flipped as Mat texture is stored top-to-bottom
                rw.writeEol();
            }

            // Write surfaces & vertex normals to buffers
            std::vector<std::vector<Vector3f>> fnormals(geores.vertices.size()); // face normals that share the same vertex
            std::vector<byte_t> fbuffer;
            fbuffer.reserve(geores.surfaces.size() * kFaceLineMaxChars);

            OutputBinaryStream<decltype(fbuffer)> bs(fbuffer);
            TextResourceWriter brw(bs);
            for (const auto& s : geores.surfaces)
            {
                // Write texture to use with face
                auto matIdx = fromOptionalIdx(s.matIdx);
                if (matIdx >= 0)
                {
                    std::string name;
                    const Material* pMat = nullptr;
                    if (matIdx < mats.size()) {
                        const auto& mat = mats.value(matIdx);
                        name = fs::path(mat.name()).stem().string();
                        pMat = &mat;
                    }
                    else // mat in jones3dstatic
                    {
                        matIdx = getStaticResourceIdx(matIdx);
                        if (matIdx < smats.size())
                        {
                            // mat is in jones3dStatic.cnd
                            pMat = &smats.value(matIdx);
                            name = fs::path(pMat->name()).stem().string();
                        }
                        else
                        {
                            // mat is not in jones3dStatic
                            // or jones3dStatic.cnd was not found
                            if (matIdx < staticResources.materials.size()) {
                                // TODO: Try to load mat from specified in staticResources
                                name = getBaseName(staticResources.materials.key(matIdx));
                            }
                            else
                            {
                                LOG_WARNING("Unknown surface % material asset from jones3dstatic at index %. Setting surface material as %.", s.id, matIdx, kImgDefault);
                                name = kImgDefault;
                            }
                        }
                    }

                    usedMats.pushBack(name, pMat);
                    brw.writeLine("usemtl "s + name);
                }
                else {
                    brw.writeLine(kUseMtlTransparent);
                }

                // Write face vertex and UV incices
                brw.write("f\t");
                for (const auto& v : s.vertices)
                {
                    brw.writeNumber(v.vertIdx + 1); // Note: idx in obj starts at 1
                    brw.write("/");
                    auto uvIdx = fromOptionalIdx(v.uvIdx) + 1; // Note: idx in obj starts at 1
                    if (uvIdx > 0) {
                        brw.writeNumber(uvIdx);
                    }
                    brw.write("/");
                    brw.writeNumber(v.vertIdx + 1); // normal idx. Note: idx in obj starts at 1
                    brw.indent(1);

                    // Write vertex normal
                    fnormals[v.vertIdx].push_back(s.normal);
                }
                brw.writeEol();
            }

            // Calculate & write vertex normals
            int i = 0;
            for (const auto& fn : fnormals)
            {
                auto vn = unweightedVertexNormal(fn);
                rw.write("vn");
                rw.writeVector(getObjCoords(vn), /*width=*/ 1);
                rw.writeEol();
                i++;
            }

            // Write object mesh faces
            objs << fbuffer;
        }

        // Write mtl
        {
            const auto mtlOutPath = outFolder / mtlPath;
            makePath(mtlOutPath);

            OutputFileStream mtls(mtlOutPath);
            TextResourceWriter rw(mtls);
            rw.setIndentChar(' ');
            rw.writeCommentLine("% v% MTL file: %", kProgramName, kVersion, mtlOutPath.filename().string());
            rw.writeCommentLine(kProgramUrl);
            rw.writeCommentLine("Material Count: %", usedMats.size() + 1); // +1 = transparent img
            rw.writeEol();

            // Add transparent & default material at the the top of the list
            rw.writeLine(kNewMtlTransparent);
            rw.writeLine("d 0.0");
            rw.writeLine("illum 9");
            rw.writeLine("Ns 0");
            rw.writeLine(kMapKdTransparent);
            rw.writeEol();

            auto writeImage = [&](const auto& imgName, const auto& imgData) {
                auto matPath = outFolder / kMtlPngFolder / imgName;
                if (!fileExists(matPath))
                {
                    makePath(matPath);
                    auto nWritten = OutputFileStream(matPath, /*truncate=*/true)
                        .write(imgData.data(), imgData.size());
                    if (nWritten != imgData.size()) {
                        throw std::runtime_error(
                            "Failed to write file " + std::string(imgName)
                        );
                    }
                }
            };

            // Write transparent & default image to mat folder if it doesn't exists
            writeImage(kImgTransparentFileName, kImgTransparentPng);

            for (const auto& [name, mat] : usedMats.container())
            {
                //const auto name = fs::path(name).stem().string();
                if (extractMat)
                {
                    if (mat)
                    {
                        const auto matOutPath =  outFolder / kMtlPngFolder / (name + ".png");
                        makePath(matOutPath);
                        pngWrite(OutputFileStream(matOutPath, /*truncate=*/true), mat->cells().at(0));
                    }
                    else { // mat in jones3dstatic
                        LOG_WARNING("Couldn't find material: '%'", name);
                    }
                }

                rw.writeLine("newmtl " + name);
                rw.writeLine("d 1.0");
                rw.writeLine("Ns 0"); // No shine
                if (mat && !mat->isEmpty() && mat->format().alphaBPP > 0) {
                    rw.writeLine("illum 9");
                    rw.writeLine("map_d " + std::string("png") + "/" + name + ".png");
                }
                rw.writeLine("map_Kd " + std::string("png") + "/" + name + ".png");
                rw.writeEol();
            }
        }
    }
}

#endif // CNDTOOL_OBJ_H
