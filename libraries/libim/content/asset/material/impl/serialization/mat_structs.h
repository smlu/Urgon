#ifndef LIBIM_MAT_STRUCTS_H
#define LIBIM_MAT_STRUCTS_H
#include <array>
#include "../../colorformat.h"
#include "../../../../../common.h"


/* Defines MAT file structures */

namespace libim::content::asset {
    constexpr std::array<char,4> MAT_FILE_SIG    = {{'M', 'A', 'T', ' '}};
    constexpr uint32_t           MAT_VERSION     = 0x32;
    constexpr uint32_t           MAT_MIPMAP_TYPE = 2;


    struct MatHeader
    {
        std::array<char,4> magic;     // Should be 'MAT '
        int version;                  // Should 0x32

        uint32_t type;               // 0 = color, 1 = ?, 2 = texture (Should be 2)
        int32_t recordCount;         // Number of MAT records
        int32_t mipmapCount;         // Number of mipmaps

        ColorFormat colorInfo;
    };

    struct MatRecordHeader
    {
        int32_t recordType;       // 0 = color, 8 = texture
        int32_t transparentColor;
        int32_t Unknown1;
        int32_t Unknown2;
        int32_t Unknown3;

        int32_t Unknown4;        // float(1.0) => 0x803F0000

        int32_t Unknown5;
        int32_t Unknown6;       // 4, 9 (gen_4red.mat), 0x0169F4B4 (jonsescomic_sans_ms14.mat), 5 (splashdemo_6.mat)
        int32_t Unknown7;       // 4, 0x0051F8F8 (gen_4red.mat), 0x0169FC3E (jonsescomic_sans_ms14.mat), 5 (splashdemo_6.mat)
        int32_t mipmapIdx;      // Coresponding mipmap index num
    };

    struct MatMipmapHeader
    {
        int32_t width;
        int32_t height;
        int32_t transparentBool;

        int32_t Unknown1;
        int32_t Unknown2;

        int32_t textureCount;
    };

    struct MatTexture
    {
        MatMipmapHeader header;
        BitmapPtr bitmap;
    };

    struct MatColorHeader
    {
        int32_t RecordType;
        int32_t ColorNum;
        float Unknown1;
        float Unknown2;
        float Unknown3;
        float Unknown4;
    };
}
#endif // LIBIM_MAT_STRUCTS_H
