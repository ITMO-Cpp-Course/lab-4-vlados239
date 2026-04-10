#include "resource_core/file_handle.hpp"
#include "resource_core/resource_error.hpp"

#include <cerrno>
#include <cstring>
#include <utility>

namespace lab4::resource
{

// ─── Конструкторы ────────────────────────────────────────────────────────────

FileHandle::FileHandle(const std::string& path, const std::string& mode) : file_(nullptr), path_(path)
{
    file_ = std::fopen(path.c_str(), mode.c_str());
    if (!file_)
    {
        throw ResourceError("Не удалось открыть файл: " + path + " (" + std::strerror(errno) + ")");
    }
}

FileHandle::FileHandle(std::FILE* file, const std::string& path) : file_(file), path_(path)
{
    if (!file_)
    {
        throw ResourceError("Передан нулевой FILE* для пути: " + path);
    }
}

// ─── Деструктор ──────────────────────────────────────────────────────────────

FileHandle::~FileHandle()
{
    close();
}

// ─── Move-семантика ───────────────────────────────────────────────────────────

FileHandle::FileHandle(FileHandle&& other) noexcept : file_(other.file_), path_(std::move(other.path_))
{
    other.file_ = nullptr; // обнуляем, чтобы деструктор other не закрыл файл
}

FileHandle& FileHandle::operator=(FileHandle&& other) noexcept
{
    if (this != &other)
    {
        close(); // освобождаем текущий ресурс
        file_ = other.file_;
        path_ = std::move(other.path_);
        other.file_ = nullptr; // обнуляем, чтобы деструктор other не закрыл файл
    }
    return *this;
}

// ─── Методы ──────────────────────────────────────────────────────────────────

bool FileHandle::isOpen() const
{
    return file_ != nullptr;
}

const std::string& FileHandle::path() const
{
    return path_;
}

std::FILE* FileHandle::get() const
{
    return file_;
}

void FileHandle::close()
{
    if (file_)
    {
        std::fclose(file_);
        file_ = nullptr; // обнуляем сразу — повторный вызов close() безопасен
    }
}

} // namespace lab4::resource
