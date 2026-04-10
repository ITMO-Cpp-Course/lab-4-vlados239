#pragma once

#include "resource_core/file_handle.hpp"

#include <string>

namespace lab4::resource
{

// Записать строку в файл (файл должен быть открыт в режиме записи)
void writeToFile(FileHandle& fh, const std::string& data);

// Прочитать всё содержимое файла начиная с текущей позиции
std::string readFromFile(FileHandle& fh);

} // namespace lab4::resource
