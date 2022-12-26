#ifndef LIBIM_ASSET_H
#define LIBIM_ASSET_H
#include <string>
#include <utility>

namespace libim::content::asset {
    class Asset
    {
    public:
        Asset() = default;
        explicit Asset(std::string name) : name_(std::move(name)) {}
        Asset(const Asset&) = default;
        Asset(Asset&&) noexcept = default;
        Asset& operator=(const Asset&) = default;
        Asset& operator=(Asset&&) noexcept = default;
        virtual ~Asset() = default;

        inline void setName(std::string name)
        {
            name_ = std::move(name);
        }

        inline const std::string& name() const
        {
            return name_;
        }

    private:
        std::string name_;
    };
}
#endif // LIBIM_ASSET_H
