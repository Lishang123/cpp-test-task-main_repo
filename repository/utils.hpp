// Copyright (c) Scheer PAS Schweiz AG

#ifndef SRC_ADDONS_AWSS3_UTILS_UTILS_HPP
#define SRC_ADDONS_AWSS3_UTILS_UTILS_HPP

#include "../Misc/M_SystemMessage.hpp"

namespace utils
{

M_SystemMessage logMsg( std::string_view code, std::string_view message );
M_SystemMessage sysMsg( std::string_view code, std::string_view message );

[[noreturn]]  void fatal( std::string_view code, std::string_view msg );
[[noreturn]] void error( std::string_view code, std::string_view msg );

[[noreturn]]
void unexpectedElement( std::string_view got, std::string_view wanted,
                        std::string_view repo );
[[noreturn]]
void unexpectedElement( std::string_view got, std::string_view repo );

[[noreturn]]
void unexpectedChild( std::string_view element, std::string_view got,
                      std::string_view repo );
[[noreturn]]
void unexpectedChild( std::string_view got, std::string_view repo );

[[noreturn]]
void missingAttribute( std::string_view element, std::string_view attribute,
                       std::string_view repo );

std::string toLower( std::string_view str );

} // namespace utils

#endif // SRC_ADDONS_AWSS3_UTILS_UTILS_HPP
