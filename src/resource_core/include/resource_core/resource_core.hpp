#pragma once

#include <cstdio>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>

namespace lab4::resource
{

// Собственный тип исключений для ошибок при работе с ресурсами
class ResourceError : public std::runtime_error
{
  public:
    explicit ResourceError(const std::string& msg) : std::runtime_error(msg) {}
};

// RAII-обёртка над FILE*
class FileHandle
{
  public:
    explicit FileHandle(const std::string& path, const std::string& mode = "r");
    ~FileHandle();

    // Только перемещение — копирование запрещено
    FileHandle(FileHandle&& other) noexcept;
    FileHandle& operator=(FileHandle&& other) noexcept;

    FileHandle(const FileHandle&) = delete;
    FileHandle& operator=(const FileHandle&) = delete;

    bool isOpen() const;
    const std::string& path() const;

    // Записать строку в файл (файл должен быть открыт в режиме записи)
    void write(const std::string& data);

    // Прочитать всё содержимое файла начиная с текущей позиции
    std::string read();

    // Явно закрыть ресурс (также вызывается деструктором)
    void close();

  private:
    std::FILE* file_;
    std::string path_;
};

// Предоставляет кешированный доступ к FileHandle через shared_ptr / weak_ptr
class ResourceManager
{
  public:
    // Возвращает shared_ptr на FileHandle для указанного пути.
    // Если ресурс уже есть в кеше и ещё жив — возвращает тот же экземпляр.
    std::shared_ptr<FileHandle> open(const std::string& path, const std::string& mode = "r");

    // Количество записей в кеше (включая истёкшие weak_ptr)
    std::size_t cacheSize() const;

    // Удалить все истёкшие записи из кеша
    void evictExpired();

  private:
    std::unordered_map<std::string, std::weak_ptr<FileHandle>> cache_;
};

} // namespace lab4::resource
