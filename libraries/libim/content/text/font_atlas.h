#ifndef LIBIM_FONTATLAS_H
#define LIBIM_FONTATLAS_H
#include <array>
#include <cmath>
#include <cstdint>
#include <limits>
#include <map>
#include <string>
#include <utility>

#include <libim/content/asset/material/texture.h>
#include <libim/math/vector2.h>
#include <libim/utils/utils.h>

namespace libim::content::text {
    struct GlyphMetrics
    {
        float left;              // left x of glyph rectangle, multiply it by font texture width to get x position in texture
        float top;               // top y of glyph rectangle, multiply it by font texture height to get y position in texture
        float right;             // right x of glyph rectangle, multiply it by font texture width to get x1 position in texture
        float bottom;            // bottom y of glyph rectangle, multiply it by font texture width to get x1 position in texture
        int32_t baselineOriginY; // glyph origin point y (from glyph rect top y)
        int32_t baselineOriginX; // glyph origin point x (from glyph rect left x)

        float width() const
        {
            return right - left;
        }

        float height() const
        {
            return bottom - top;
        }

        Point2f centerPoint() const
        {
            auto cx = left + width()  * 0.5;
            auto cy = top  + height() * 0.5;
            return { float(cx), float(cy) };
        }
    };

    template<typename CharT>
    class FontAtlas
    {
    public:
        using CharType = CharT;
        using MapType = std::map<CharT, GlyphMetrics>;

        FontAtlas() = default;
        FontAtlas(std::string name, uint32_t fontSize, uint32_t lineSpacing) :
            name_(std::move(name)),
            fontSize_(fontSize),
            lineSpacing_(lineSpacing)
        {}

        FontAtlas(std::string name, uint32_t fontSize, uint32_t lineSpacing, asset::Texture atlasImage) :
            name_(std::move(name)),
            fontSize_(fontSize),
            lineSpacing_(lineSpacing),
            image_(std::move(atlasImage))
        {}

        FontAtlas(std::string name, uint32_t fontSize, uint32_t lineSpacing, asset::Texture atlasImage, std::map<CharT, GlyphMetrics> glyphs) :
            name_(std::move(name)),
            fontSize_(fontSize),
            lineSpacing_(lineSpacing),
            image_(std::move(atlasImage)),
            glyphs_(std::move(glyphs))
        {}

        FontAtlas(const FontAtlas&) = default;
        FontAtlas(FontAtlas&&) noexcept = default;
        FontAtlas& operator=(const FontAtlas&) = default;
        FontAtlas& operator=(FontAtlas&&) noexcept = default;

        void setName(std::string name)
        {
            name_ = std::move(name);
        }

        const std::string& name() const
        {
            return name_;
        }

        void setFontSize(uint32_t fontSize)
        {
            fontSize_ = fontSize;
        }

        uint32_t fontSize() const
        {
            return fontSize_;
        }

        void setLineSpacing(uint32_t lineSpacing)
        {
            lineSpacing_ = lineSpacing;
        }

        uint32_t lineSpacing() const
        {
            return lineSpacing_;
        }

        void setImage(asset::Texture img)
        {
            image_ = std::move(img);
        }

        const asset::Texture& image() const
        {
            return image_;
        }

        GlyphMetrics& at(CharT glyph)
        {
            return glyphs_.at(glyph);
        }

        const GlyphMetrics& at(CharT glyph) const
        {
            return glyphs_.at(glyph);
        }

        GlyphMetrics& operator[](CharT glyph)
        {
            return glyphs_[glyph];
        }

        // Adds or replace existing glyph
        void insert(CharT glyph, const GlyphMetrics& metrics)
        {
            this->operator[](glyph) = metrics;
        }

        bool contains(CharT glyph) const
        {
            return glyphs_.find(glyph) != glyphs_.end();
        }

        const std::map<CharT, GlyphMetrics>& glyphs() const &
        {
            return glyphs_;
        }

        std::map<CharT, GlyphMetrics>& glyphs() &
        {
            return glyphs_;
        }

        std::map<CharT, GlyphMetrics>&& glyphs() &&
        {
            return std::move(glyphs_);
        }

        static std::size_t maxGlyphs()
        {
            return (std::numeric_limits<CharT>::max)() + 1;
        }

    private:
        std::string name_;
        uint32_t fontSize_ = 0;
        uint32_t lineSpacing_ = 0;
        asset::Texture image_;
        MapType glyphs_;
    };


    struct GcfFile
    {
        int32_t lineSpacing;
        std::array<char, 256> fontName;
        int32_t fontSize;
        std::array<GlyphMetrics, 256> glyphs;
    };

    struct AsciiFontAtlas : public FontAtlas<unsigned char>
    {
        using FontAtlas<unsigned char>::FontAtlas;
        static AsciiFontAtlas fromGcf(const GcfFile& gcf)
        {
            auto font = AsciiFontAtlas(utils::trim(gcf.fontName), gcf.fontSize, gcf.lineSpacing);
            for (std::size_t g = 0; g < gcf.glyphs.size(); g++) {
                if (!std::isnan(gcf.glyphs[g].left)) {
                    font.glyphs().emplace(static_cast<CharType>(g), gcf.glyphs[g]);
                }
            }
            return font;
        }

        GcfFile toGcf() const
        {
            GcfFile gcf{};
            if (name().size() > gcf.fontName.size()) {
                throw std::runtime_error("Gcf font name too big");
            }

            using GT = decltype(gcf.glyphs)::value_type;
            memset(gcf.glyphs.data(), -1, sizeof(GT) * gcf.glyphs.size());

            std::copy(name().begin(), name().end(), gcf.fontName.data());
            gcf.fontSize = fontSize();
            gcf.lineSpacing = lineSpacing();
            for (const auto [g, m] : glyphs()) {
                gcf.glyphs[static_cast<CharType>(g)] = m;
            }
            return gcf;
        }
    };
}
#endif // LIBIM_FONTATLAS_H
