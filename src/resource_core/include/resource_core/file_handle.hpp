#pragma once

#include <string>

namespace lab4::resource
{

// RAII-обёртка над FILE* — захватывает ресурс при создании,
// автоматически освобождает при уничтожении объекта
class FileHandle
{
  public:
    // Открыть файл по пути и режиму ("r", "w", "a" и т.д.)
    explicit FileHandle(const std::string& path, const std::string& mode = "r");

    // Обернуть уже открытый FILE* (например, stdin или файл из сторонней библиотеки)
    explicit FileHandle(std::FILE* file, const std::string& path = "");

    // Явно закрыть ресурс (также вызывается деструктором)
    void close();

    ~FileHandle();

    // Только перемещение — копирование запрещено
    FileHandle(FileHandle&& other) noexcept;
    FileHandle& operator=(FileHandle&& other) noexcept;
    FileHandle(const FileHandle&) = delete;
    FileHandle& operator=(const FileHandle&) = delete;

    bool isOpen() const;
    const std::string& path() const;

    // Доступ к сырому FILE* — используется в file_io для операций чтения/записи
    std::FILE* get() const;

  private:
    std::FILE* file_;
    std::string path_;
};

} // namespace lab4::resource
