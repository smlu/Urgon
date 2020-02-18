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
        std::size_t matCelIdx; // material texture to use (material mipmap idx)
        Color color;
        Vector3f normal;
        std::vector<VertexIdx> verts;
    };
}
#endif // LIBIM_FACE_H
