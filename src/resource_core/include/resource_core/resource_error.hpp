#pragma once

#include <stdexcept>
#include <string>

namespace lab4::resource
{

// Собственный тип исключений для ошибок при работе с ресурсами
class ResourceError : public std::runtime_error
{
  public:
    explicit ResourceError(const std::string& msg);
};

} // namespace lab4::resource
