#include "resource_core/file_io.hpp"
#include "resource_core/resource_error.hpp"

#include <cstdio>

namespace lab4::resource
{

void writeToFile(FileHandle& fh, const std::string& data)
{
    if (!fh.isOpen())
    {
        throw ResourceError("Запись невозможна: файл не открыт");
    }
    if (std::fwrite(data.data(), 1, data.size(), fh.get()) != data.size())
    {
        throw ResourceError("Ошибка записи: данные записаны не полностью в " + fh.path());
    }
    std::fflush(fh.get()); // сбрасываем буфер на диск
}

std::string readFromFile(FileHandle& fh)
{
    if (!fh.isOpen())
    {
        throw ResourceError("Чтение невозможно: файл не открыт");
    }
    std::string result;
    char buf[4096];
    std::size_t n;
    // читаем файл кусками по 4096 байт до конца
    while ((n = std::fread(buf, 1, sizeof(buf), fh.get())) > 0)
    {
        result.append(buf, n);
    }
    return result;
}

} // namespace lab4::resource
