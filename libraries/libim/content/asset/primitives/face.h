#ifndef LIBIM_FACE_H
#define LIBIM_FACE_H

#include <cstdint>
#include <optional>
#include <vector>
#include <tuple>

#include "geomode.h"
#include "light_mode.h"

#include <libim/math/color.h>
#include <libim/math/vector3.h>
#include <libim/types/flags.h>

namespace libim::content::asset {

    struct Face
    {
        enum Flag : uint32_t
        {
            Normal          = 0x00,
            DoubleSided     = 0x01, // "disables" face backface culling
            Transparent     = 0x02, // web surfaces has this flag set to make the transparent web image being correctly rendered
            TexClamp_x      = 0x04, // mapped texture is clamped in x instead of being repeated
            TexClamp_y      = 0x08, // mapped texture is clamped in y instead of being repeated
            TexScale        = 0x10, // 2x scales face texture
            Translucent     = 0x20  // It could be that the depth buffer (ZWrite) is turned off for the polygon face
        };

        struct VertexIdx
        {
            std::size_t vertIdx;
            std::optional<std::size_t> uvIdx;  // index in UV list
        };

        Flags<Flag> flags;
        GeoMode geoMode;
        LightMode lightMode;
        std::optional<std::size_t> matIdx;
        std::size_t matCelIdx = 0; // material texture to use (material mipmap idx)
        LinearColor extraLight;    // face additional light color
        Vector3f normal;
        std::vector<VertexIdx> verts;
    };


    inline bool operator == (const Face::VertexIdx& lhs, const Face::VertexIdx& rhs) {
        return lhs.vertIdx == rhs.vertIdx && lhs.uvIdx == rhs.uvIdx;
    }

    inline bool operator != (const Face::VertexIdx& lhs, const Face::VertexIdx& rhs) {
        return !(lhs == rhs);
    }

    inline bool operator == (const Face& lhs, const Face& rhs)
    {
        return lhs.flags      == rhs.flags      &&
               lhs.geoMode    == rhs.geoMode    &&
               lhs.lightMode  == rhs.lightMode  &&
               lhs.matIdx     == rhs.matIdx     &&
               lhs.matCelIdx  == rhs.matCelIdx  &&
               lhs.extraLight == rhs.extraLight &&
               lhs.normal     == rhs.normal     &&
               lhs.verts      == rhs.verts;
    }

    inline bool operator != (const Face& lhs, const Face& rhs) {
        return !(lhs == rhs);
    }
}
#endif // LIBIM_FACE_H
