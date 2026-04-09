#include "resource_core/resource_core.hpp"

#include <cerrno>
#include <cstring>
#include <utility>

namespace lab4::resource
{

// ─── FileHandle ──────────────────────────────────────────────────────────────

FileHandle::FileHandle(const std::string& path, const std::string& mode) : file_(nullptr), path_(path)
{
    file_ = std::fopen(path.c_str(), mode.c_str());
    if (!file_)
    {
        throw ResourceError("Failed to open file: " + path + " (" + std::strerror(errno) + ")");
    }
}

FileHandle::~FileHandle()
{
    close();
}

FileHandle::FileHandle(FileHandle&& other) noexcept : file_(other.file_), path_(std::move(other.path_))
{
    other.file_ = nullptr;
}

FileHandle& FileHandle::operator=(FileHandle&& other) noexcept
{
    if (this != &other)
    {
        close();
        file_ = other.file_;
        path_ = std::move(other.path_);
        other.file_ = nullptr;
    }
    return *this;
}

bool FileHandle::isOpen() const
{
    return file_ != nullptr;
}

const std::string& FileHandle::path() const
{
    return path_;
}

void FileHandle::write(const std::string& data)
{
    if (!file_)
    {
        throw ResourceError("Write failed: file is not open");
    }
    if (std::fwrite(data.data(), 1, data.size(), file_) != data.size())
    {
        throw ResourceError("Write failed: incomplete write to " + path_);
    }
    std::fflush(file_);
}

std::string FileHandle::read()
{
    if (!file_)
    {
        throw ResourceError("Read failed: file is not open");
    }
    std::string result;
    char buf[4096];
    std::size_t n;
    while ((n = std::fread(buf, 1, sizeof(buf), file_)) > 0)
    {
        result.append(buf, n);
    }
    return result;
}

void FileHandle::close()
{
    if (file_)
    {
        std::fclose(file_);
        file_ = nullptr;
    }
}

// ─── ResourceManager ─────────────────────────────────────────────────────────

std::shared_ptr<FileHandle> ResourceManager::open(const std::string& path, const std::string& mode)
{
    auto it = cache_.find(path);
    if (it != cache_.end())
    {
        if (auto handle = it->second.lock())
        {
            return handle; // ресурс жив — возвращаем из кеша
        }
    }
    auto handle = std::make_shared<FileHandle>(path, mode);
    cache_[path] = handle;
    return handle;
}

std::size_t ResourceManager::cacheSize() const
{
    return cache_.size();
}

void ResourceManager::evictExpired()
{
    for (auto it = cache_.begin(); it != cache_.end();)
    {
        if (it->second.expired())
        {
            it = cache_.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

} // namespace lab4::resource
