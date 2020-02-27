#ifndef LIBIM_FACE_H
#define LIBIM_FACE_H

#include <cstdint>
#include <optional>
#include <vector>
#include <tuple>

#include "geomode.h"
#include "light_mode.h"
#include "../../../math/color.h"
#include "../../../math/vector3.h"

namespace libim::content::asset {

    struct Face
    {
        enum Flag : uint32_t
        {
            Normal                   = 0x00,
            TwoSided                 = 0x01,
            Translucent              = 0x02,
            Unknown_4                = 0x04,
        };

        struct VertexIdx
        {
            std::size_t vertIdx;
            std::optional<std::size_t> texIdx;
        };

        Flag flags;
        GeoMode geoMode;
        LightMode lightMode;
        std::optional<std::size_t> matIdx;
        std::size_t matCelIdx = 0; // material texture to use (material mipmap idx)
        Color extraLight;          // face additional light color
        Vector3f normal;
        std::vector<VertexIdx> verts;
    };



    inline bool operator == (const Face::VertexIdx& lhs, const Face::VertexIdx& rhs) {
        return lhs.vertIdx == rhs.vertIdx && lhs.texIdx == rhs.texIdx;
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
