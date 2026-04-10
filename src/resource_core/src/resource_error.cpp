#include "resource_core/resource_error.hpp"

namespace lab4::resource
{

ResourceError::ResourceError(const std::string& msg) : std::runtime_error(msg) {}

} // namespace lab4::resource
