// Copyright (c) Scheer PAS Schweiz AG

#include "utils.hpp"


namespace utils
{

M_SystemMessage logMsg( std::string_view code, std::string_view message )
{
    return M_SystemMessage { "LM::domain", code, message };
}

M_SystemMessage sysMsg( std::string_view code, std::string_view message )
{
    return M_SystemMessage { "SM::domain", code, message };
}

[[noreturn]]
void fatal( std::string_view code, std::string_view msg )
{
    throw logMsg( code, msg );
}

[[noreturn]]
void error( std::string_view code, std::string_view msg )
{
    throw sysMsg( code, msg );
}

namespace
{

void unexpected( std::string_view code, std::string msg )
{
    auto message = logMsg( code, msg );

    throw message;
}

void unexpected( std::string msg )
{
    unexpected( "lm::unexpected_element", msg );
}

} // namespace

std::string listToString(const std::vector<std::string_view>& list) {
    std::string res {"["};
    bool first = true;
    for (auto wanted : list) {
        if (!first)
            res += ", ";
        res += fmt::format("'{}'", wanted);
        first = false;
    }
    return res;
}

void unexpectedElement( std::string_view got, const std::vector<std::string_view>& wanted_list,
                            std::string_view repo )
{
    std::string expected;

    if (wanted_list.size() == 1) {
        expected = fmt::format("'{}'", *wanted_list.begin());
    } else {
        expected = "one of " + listToString( wanted_list ) + "]";
    }
    unexpected( fmt::format( "Expected {} while parsing {}, got '{}'.",
                             expected, repo, got ) ) ;
}

void unexpectedElement( std::string_view got, std::string_view wanted,
                        std::string_view repo )
{
    unexpected( fmt::format( "Expected '{}' while parsing {}, got '{}'.",
                             wanted, repo, got ) ) ;
}

void unexpectedElement( std::string_view got, std::string_view repo )
{
    unexpected( fmt::format( "Found unexpected element '{}' while parsing  {}.",
                             got, repo ) ) ;
}

void unexpectedChild( std::string_view element, std::string_view got,
                      std::string_view repo )
{
    unexpected( "lm::unexpected_child_element",
                fmt::format( "Expected '{}' child element, got '{}' on parsing {}.",
                             element, got, repo ) );
}

void unexpectedChild( std::string_view got, std::string_view repo )
{
    unexpected( "lm::unexpected_child_element",
                fmt::format( "Found unexpected child element '{}' while parsing {}.",
                             got, repo ) );
}

[[noreturn]]
void missingAttribute( std::string_view element, std::string_view attribute,
                       std::string_view repo )
{
    fatal( "lm::missing_attribute",
           fmt::format( "Missing '{}' attribute for '{}' on parsing  {}.",
                        attribute, element, repo ) );
}

std::string lowercaseUntilCamelBoundary( std::string_view str )
{
    if (str.empty()) {
        fatal("lm::invalid_argument", "utils::toLower: input must not be empty");
    }

    std::string s { str };

    // Cast every char to unsigned char to avoid undefined behavior.
    auto toLowerChar = [](char c) {
        return static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    };
    auto isUpper = [](char c) {
        return std::isupper(static_cast<unsigned char>(c)) != 0;
    };
    auto isLower = [](char c) {
        return std::islower(static_cast<unsigned char>(c)) != 0;
    };

    // Lowercase first character
    s[0] = toLowerChar(s[0]);

    for (std::size_t i = 1; i < s.size(); ++i)
    {
        // Check if i+1 exists
        if (i + 1 < s.size() && isUpper(s[i]) && isLower(s[i + 1]))
            break;

        s[i] = toLowerChar(s[i]);
    }

    return s;
}

} // namespace utils
