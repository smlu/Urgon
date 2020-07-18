#ifndef LIBIM_MAT_STRUCTS_H
#define LIBIM_MAT_STRUCTS_H
#include <array>
#include "../../colorformat.h"
#include "../../texture.h"


/* Defines MAT file structures */

namespace libim::content::asset {
    constexpr std::array<char,4> MAT_FILE_SIG    = {{'M', 'A', 'T', ' '}};
    constexpr uint32_t           MAT_VERSION     = 0x32;
    constexpr uint32_t           MAT_TEXTURE_TYPE = 2;


    struct MatHeader
    {
        std::array<char,4> magic;    // should be 'MAT '
        int version;                 // should always be 0x32

        uint32_t type;               // 0 = color, 1 = ?, 2 = texture (Should be 2)
        int32_t recordCount;         // number of MAT records
        int32_t celCount;            // number of textures in mat file

        ColorFormat colorInfo;
    };
    static_assert(sizeof(MatHeader) == 76);

    struct MatRecordHeader
    {
        int32_t recordType;        // 0 = color, 8 = texture
        int32_t transparentColor;
        int32_t Unknown1;
        int32_t Unknown2;
        int32_t Unknown3;

        int32_t Unknown4;          // float(1.0) => 0x803F0000

        int32_t Unknown5;
        int32_t Unknown6;          // 4, 9 (gen_4red.mat), 0x0169F4B4 (jonsescomic_sans_ms14.mat), 5 (splashdemo_6.mat)
        int32_t Unknown7;          // 4, 0x0051F8F8 (gen_4red.mat), 0x0169FC3E (jonsescomic_sans_ms14.mat), 5 (splashdemo_6.mat)
        int32_t texIdx;            // coresponding texture index num
    };
    static_assert(sizeof(MatRecordHeader) == 40);

    struct MatTextureHeader
    {
        int32_t width;
        int32_t height;
        int32_t transparentBool;

        int32_t Unknown1;
        int32_t Unknown2;

        int32_t mipLevels;
    };
    static_assert(sizeof(MatTextureHeader) == 24);

    struct MatTexture
    {
        MatTextureHeader header;
        PixdataPtr ptrPixdta;
    };

    struct MatColorHeader
    {
        int32_t recordType;
        int32_t colorNum;
        float Unknown1;
        float Unknown2;
        float Unknown3;
        float Unknown4;
    };
    static_assert(sizeof(MatColorHeader) == 24);
}
#endif // LIBIM_MAT_STRUCTS_H
