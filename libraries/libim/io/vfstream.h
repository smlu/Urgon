#include <cstdint>
#include <filesystem>
#include <string_view>

#include "stream.h"
#include <libim/types/indexmap.h>
#include <libim/utils/utils.h>

namespace libim {
    class VirtualFile : public virtual InputStream
    {
    public:
        /**
         * Creates new VirtualFile from input stream, virtual offset and size in the input stream.
         * @param istream - input stream ref
         * @param offset  - offset in the istream to the beginning of file
         * @param size    - the size of file
         * @throw VirtualFileError if istream is null or params offset and size are invalid
         */
        VirtualFile(SharedRef<InputStream> istream, std::size_t offset, std::size_t size) :
            istream_(std::move(istream)),
            offset_(offset),
            size_(size)
        {
            if ((offset + size) > istream_->size()) {
                throw VirtualFileError("Invalid input stream or invalid offset parameters to the virtual file in stream");
            }
            seekBegin(); // set the offset in the istream
        }

        virtual void seek(std::size_t offset) const override
        {
            if (offset >= size_) {
                throw VirtualFileError("Seek beyond EOF");
            }
            istream_->seek(offset_ + offset);
            pos_ = offset;
        }

        virtual std::size_t size() const override
        {
            return size_;
        }

        virtual std::size_t tell() const override
        {
            return pos_;
        }

        virtual bool canRead() const override
        {
            return true;
        }

        virtual bool canWrite() const override
        {
            return false;
        }

        virtual void flush() override
        {
            throw VirtualFileError("Flush not supported");
        }

    protected:
        virtual std::size_t readsome(byte_t* data, std::size_t length) const override
        {
            if((tell() + length) > size_) {
                 throw VirtualFileError("Read beyond EOF");
            }
            seek(pos_);
            auto nRead = istream_->read(data, length);
            pos_ += nRead;
            return nRead;
        }

        virtual std::size_t writesome(const byte_t* data, std::size_t length) override
        {
            throw VirtualFileError("Can't write into virtual file");
        }

    private:
        SharedRef<InputStream> istream_;
        std::size_t offset_;
        std::size_t size_;
        mutable std::size_t pos_ = 0;
    };

    /* Virtual files container which maps file path to VirtualFile */
    class VfContainer
    {
    public:
        using MapType = IndexMap<SharedRef<VirtualFile>>;
        using ConstIterator = typename MapType::ContainerType::const_iterator;

        ConstIterator begin() const
        {
            return std::cbegin(files_.container());
        }

        ConstIterator end() const
        {
            return std::cend(files_.container());
        }

        bool contains(const std::string& filePath) const noexcept
        {
            return files_.find(getKey(filePath)) != files_.end();
        }

        bool contains(const std::filesystem::path& filePath) const noexcept
        {
            return contains(filePath.string());
        }

        /**
         * Adds virtual file to container.
         * The file path is convert to lower case.
         * @param filePath - the file path of the virtual file
         * @param vf       - virtual file to add
         * @return true if file was added to container,
         *         otherwise false mening the file with the same path already exists.
         */
        bool add(const std::string& filePath, SharedRef<VirtualFile> vf)
        {
            auto key = getKey(filePath);
            auto [it, success] = files_.emplaceBack(key, std::move(vf));
            if (success) {
                it->get().setName(getFilename(key));
            }
            return success;
        }

        bool add(const std::filesystem::path& filePath, SharedRef<VirtualFile> vf)
        {
            return add(filePath.string(), std::move(vf));
        }

        /**
         * Returns pointer to stored virtual file.
         * @param filePath the path to file to retrieve
         * @return SharedRef<VirtualFile> or std::nullopt if file doesn't exists in the container.
        */
        std::optional<SharedRef<VirtualFile>> get(const std::string& filePath) const
        {
            auto it = files_.find(getKey(filePath));
            if (it == files_.end()) {
                return std::nullopt;
            }
            return *it;
        }

        std::optional<SharedRef<VirtualFile>> get(const std::filesystem::path& filePath) const
        {
            return get(filePath.string());
        }

        void remove(const std::string& filePath)
        {
            files_.erase(getKey(filePath));
        }

        void remove(const std::filesystem::path& filePath)
        {
            remove(filePath.string());
        }

        std::size_t size() const
        {
            return files_.size();
        }

    private:
        const std::string getKey(std::string_view key) const
        {
            static std::string tmp;
            tmp.clear();
            tmp.reserve(key.size());
            std::transform(key.begin(), key.end(), std::back_inserter(tmp), [](char c){
                return static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
            });
            return tmp;
        }

    private:
        MapType files_;
    };


    /**
     * Loads VfContainer from GOB file
     * @param is - pointer to GOB file input stream
     * @throw StreamError
     */
    VfContainer gobLoad(SharedRef<InputStream> is);

    /**
     * Loads VfContainer from GOB file
     * @param filePath- path to GOB file
     * @throw StreamError
     */
    VfContainer gobLoad(const std::filesystem::path& filePath);
}