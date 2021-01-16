#ifndef LIBIM_VIRTUAL_FILE_SYSTEM_H
#define LIBIM_VIRTUAL_FILE_SYSTEM_H
#include <cstdint>
#include <filesystem>
#include <optional>
#include <stdexcept>
#include <utility>
#include <vector>

#include "stream.h"
#include "vfstream.h"

namespace libim {

    struct VfsError : public std::runtime_error {
        using std::runtime_error::runtime_error;
    };

    class VirtualFileSystem
    {
    public:
        /**
         * Adds new vf container to list of vf containers to look for a file.
         * @param containerPath - system path of container c to tag container
         * @param c             - virtual files container
         * @return true if container was added to the list, otherwise false.
        */
        bool addContainer(const std::filesystem::path& containerPath, const VfContainer& c);

        /**
         * Loads vf container from GOB file and adds it to the list of stored vf containers.
         * @param gobFilePath - system file path to the GOB file.
         * @throw VfsError if gobFilePath is invalid path,
         *                    file at gobFilePath is invalid or corrupted GOB file and
         *                    system already contains GOB container under gobFilePath path.
         */
        void loadGobContainer(const std::filesystem::path& gobFilePath);

        /**
         * Tries to loads vf container from GOB file and adds it to the list of stored vf containers.
         * @param gobFilePath - system file path to the GOB file.
         * @return true if vf container vas successfully loaded and added from GOB file, otherwise false.
         */
        bool tryLoadGobContainer(const std::filesystem::path& gobFilePath) noexcept;

        /**
         * Adds system folder path of system directories to look for a file.
         * @param folder - system folder path to add
         * @throw VfsError in case folder doesn't exist on the system or is not a folder path.
        */
        void addSysFolder(const std::filesystem::path& folder);

        /**
         * Tres find file in the file system.
         * @note First the system folders are searched for the file, if no file is found
         *       then virtual file containers are searched.
         *       Returned file stream is reset
         * @param filePath - relative file path to search for.
         * @return file SharedRef<InputStream> if file is found in the file system, otherwise std::nullopt
        */
        [[nodiscard]] std::optional<SharedRef<InputStream>> findFile(const std::filesystem::path& filePath) const;

        /**
         * Returns file stream
         * @see findFile for more info
         * @param filePath - relative file path
         * @throw VfsError if file couldn't be found.
        */
        [[nodiscard]] SharedRef<InputStream> getFile(const std::filesystem::path& filePath) const
        {
            auto optfs = findFile(filePath);
            if (!optfs) {
                throw VfsError("File doesn't exist");
            }
            return optfs.value();
        }

        /**
         * Checks if file system contains file.
         * @param filePath - path to file to look for
         * @return true if file is found in the file system, otherwise false.
         */
        [[nodiscard]] bool hasFile(const std::filesystem::path& filePath) const // searches the vfs for specific file
        {
            return findFile(filePath).has_value();
        }

    private:
        std::vector<std::filesystem::path> sysDirs_;
        std::vector<std::pair<std::filesystem::path, VfContainer>> vfiles_;
    };
}

#endif // LIBIM_VIRTUAL_FILE_SYSTEM_H