#include "resource_core/resource_manager.hpp"
#include "resource_core/resource_error.hpp"

namespace lab4::resource
{

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
    cache_[path] = handle; // сохраняем weak_ptr — не удерживает объект от удаления
    return handle;
}

bool ResourceManager::contains(const std::string& path) const
{
    auto it = cache_.find(path);
    if (it == cache_.end())
    {
        return false; // пути нет в кеше вовсе
    }
    return !it->second.expired(); // есть в кеше, но жив ли ещё?
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
            it = cache_.erase(it); // erase возвращает итератор на следующий элемент
        }
        else
        {
            ++it;
        }
    }
}

} // namespace lab4::resource
