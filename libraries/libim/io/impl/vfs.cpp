#include "../vfs.h"
#include "../filestream.h"
#include <libim/log/log.h>

using namespace libim;
namespace fs = std::filesystem;

bool VirtualFileSystem::addContainer(const fs::path& containerPath, const VfContainer& c)
{
    auto it = std::find_if(vfiles_.begin(), vfiles_.end(),
        [&](const std::pair<fs::path, VfContainer>& p){ return p.first == containerPath; }
    );
    if (it != vfiles_.end())
    {
        LOG_DEBUG("VFS::addContainer: vf container % already in the file system", containerPath);
        return false;
    }

    vfiles_.emplace_back(containerPath, c);
    LOG_INFO("VFS: Added vf container %", containerPath);
    return true;
}

void VirtualFileSystem::loadGobContainer(const fs::path& gobFilePath)
{
    try
    {
        if (!fileExists(gobFilePath)) {
            throw VfsError("GOB file doesn't exist");
        }

        auto c = gobLoad(gobFilePath);
        if (!addContainer(gobFilePath, c)) {
            throw VfsError("GOB vf container already in the file system");
        }
    }
    catch(const VfsError&) { throw; }
    catch(const std::exception& e)
    {
        throw VfsError(e.what());
    }
}

bool VirtualFileSystem::tryLoadGobContainer(const fs::path& gobFilePath) noexcept
{
    try
    {
        loadGobContainer(gobFilePath);
        return true;
    }
    catch(const std::exception& e)
    {
        LOG_INFO("VFS: Failed to load GOB vf container %, e='%'", gobFilePath, e.what());
        return false;
    }
}

void VirtualFileSystem::addSysFolder(const fs::path& folder)
{
    if (!isDirPath(folder) || !dirExists(folder)) {
        throw VfsError("Trying to add non-existing or invalid system folder path");
    }
    auto it = std::find(sysDirs_.begin(), sysDirs_.end(), folder );
    if (it != sysDirs_.end())
    {
        LOG_DEBUG("VFS::addSysFolder: system folder % already in the file system", folder);
        return;
    }
    sysDirs_.push_back(folder);
    LOG_INFO("VFS: Added system folder %", folder);
}

std::optional<SharedRef<InputStream>> VirtualFileSystem::findFile(const fs::path& filePath) const
{
    for (const auto& sysFolder : sysDirs_)
    {
        auto path = sysFolder / filePath;
        if (fileExists(path))
        {
            LOG_DEBUG("VFS: Found file % in system folder %", filePath, sysFolder);
            return makeSharedRef<InputFileStream>(path);
        }
    }

    // No file was found in the system folders,
    // let's now search virtual containers
    for (const auto& [cpath, c] : vfiles_)
    {
        if (const auto& fs = c.get(filePath); fs.has_value())
        {
            LOG_DEBUG("VFS: Found file % in vf container %", filePath, cpath);
            fs.value()->seekBegin();
            return fs.value();
        }
    }
    LOG_DEBUG("VFS: Couldn't find file %", filePath);
    return std::nullopt;
}