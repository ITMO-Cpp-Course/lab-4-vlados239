#pragma once

#include "resource_core/file_handle.hpp"

#include <memory>
#include <string>
#include <unordered_map>

namespace lab4::resource
{

// Предоставляет кешированный доступ к FileHandle через shared_ptr / weak_ptr
class ResourceManager
{
  public:
    // Возвращает shared_ptr на FileHandle для указанного пути.
    // Если ресурс уже есть в кеше и ещё жив — возвращает тот же экземпляр.
    std::shared_ptr<FileHandle> open(const std::string& path, const std::string& mode = "r");

    // Проверить, есть ли в кеше живой FileHandle по заданному пути
    bool contains(const std::string& path) const;

    // Количество записей в кеше (включая истёкшие weak_ptr)
    std::size_t cacheSize() const;

    // Удалить все истёкшие записи из кеша
    void evictExpired();

  private:
    std::unordered_map<std::string, std::weak_ptr<FileHandle>> cache_;
};

} // namespace lab4::resource
