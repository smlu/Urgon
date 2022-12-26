 #include "../material.h"
 #include <algorithm>


 using namespace libim;
 using namespace libim::content::asset;


Material& Material::addCel(Texture cel)
{
    if (!isValidCel(cel)) {
        throw MaterialError("Can't add invalid cel texture to material");
    }
    cells_.push_back(std::move(cel));
    return *this;
}

Material& Material::setCells(std::vector<Texture> cells)
{
    const auto validCells = std::all_of(cells.begin(), cells.end(),
        [&](const auto&c) { return isValidCel(c); });
    if (!validCells) {
        throw MaterialError("Can't set material cells, invalid cel texture in list");
    }

    cells_ = std::move(cells);
    return *this;
}

bool Material::isValidCel(const Texture& cel)
{
    if (cells_.empty()) return !cel.isEmpty();
    const Texture& fcel = cells_.at(0);
    return cel.width()  == fcel.width()  &&
           cel.height() == fcel.height() &&
           cel.format() == fcel.format() &&
           fcel.mipLevels() == cel.mipLevels();
}
