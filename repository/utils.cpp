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

std::string toLower( std::string_view str )
{
    if( str == "ID" )
    {
        return "id"; //wouldn't make sense to make it iD
    }
    std::string lowerCaseStr { str };
    lowerCaseStr.at(0) = tolower(lowerCaseStr.at(0));
    for( int i = 1; i < lowerCaseStr.size(); i++ )
    {
        if( isupper( lowerCaseStr.at(i) ) && islower( lowerCaseStr.at(i+1) ) )
        {
            break;
        }
        lowerCaseStr.at(i) = tolower( lowerCaseStr.at(i) );
    }
    return lowerCaseStr;
}

} // namespace utils
