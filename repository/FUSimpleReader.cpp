// This file implements a XML reader for  using SAX-style parsing.
// The parser enforces a strict structure:
//   document -> Functions -> Function -> (ID, Source)
// Any unexpected element results in an error.

#include "FUSimpleReader.hpp"
#include "../XML/XML_Parser.hpp"
#include "utils.hpp"
#include <array>
#include <algorithm>

namespace functions::repository::simple
{
namespace
{

constexpr const std::string_view functionsArrayElement = "Functions";
constexpr const std::string_view functionElement = "Function";
constexpr const std::string_view IDElement = "ID";
constexpr const std::string_view sourceElement = "Source";

class Reader : public XML_Parser
{
public:
    bool startElementChar( const char *, const char *localName, const char *, const xercesc::Attributes & ) override;
    bool endElementChar( const char *, const char *, const char * ) override;
    // character part of the current element.
    bool charactersChar( const char *, const char *, const char *, const char *chars, const unsigned int ) override;

    Functions read ( const TY_Blob &data, std::string_view repo ) &&;

    enum class Element
    {
        document,
        functions,
        function,
        id,
        source,
        unknown
    };
private:
    Functions m_functions;
    // tracks the current position
    std::vector<Element> m_elementStack;
    // stores the current function being built, TODO: a raw pointer?
    Function *m_currentFunction {nullptr};
    void errorMessage( const M_SystemMessage &message )override;
    ST_String m_repository;
};

void Reader::errorMessage( const M_SystemMessage &message )
{
    // Any XML parse error becomes an exception.
    throw message;
}

/**
 * Mapping tag names to enums.
 * @param name: The name of the element.
 * @return: The element enum defined in the reader.
 */
Reader::Element toElement( std::string_view name )
{
    using Element = Reader::Element;
    auto lowerCaseName = utils::lowercaseUntilCamelBoundary( name );

    #define X(x) {#x, Element::x}
    constexpr std::array<std::pair<std::string_view, Element>, static_cast<size_t>( Element::unknown ) - 1> lookup = {{
        X(functions),
        X(function),
        X(id),
        X(source)
    }};
    #undef X
    // const auto it = std::ranges::find(lookup, lowerCaseName, &std::pair<std::string_view, Element>::first);
    const auto it = std::ranges::find_if( lookup,
                                   [&lowerCaseName]( const auto& x ){return lowerCaseName == x.first;});
    return it == lookup.end() ? Element::unknown : it->second;
}

/**
 * Parse the Blob data to functions.
 * @param data blob data being parsed
 * @param repo the repo label for error messages
 * @return functions parsed
 */
Functions Reader::read( const TY_Blob &data, std::string_view repo ) &&
{
    // stores repo label for diagnostics
    m_repository.set( repo );
    // seeds the stack with document
    m_elementStack.push_back( Element::document );

    //SAX parsing
    parseBlob( &data );

    return std::move( m_functions );
}

/**
 * Structural validation of the element and push back to element stack.
    It does not enforce order of <ID> vs <Source>.
    It does not enforce that <Function> actually contains both fields.
    It does not prevent multiple <Source> tags etc.
 * @param localName the element name
 * @return true
 */
bool Reader::startElementChar( const char *, const char *localName, const char *, const xercesc::Attributes & )
{
    auto previousElement = m_elementStack.back();
    auto currentElement = toElement( localName );
    m_elementStack.push_back( currentElement );

    // Reject unknown tags
    if( currentElement == Element::unknown )
    {
        utils::unexpectedElement(  localName, m_repository.c_str() );
    }

    // validates parent -> child relationships
    switch( previousElement )
    {
        case Element::document:
        {
            if( currentElement != Element::functions )
            {
                utils::unexpectedElement(   localName, functionsArrayElement, m_repository.c_str() );
            }
            break;
        }
        case Element::functions:
        {
            if( currentElement != Element::function )
            {
                utils::unexpectedElement(   localName, functionElement, m_repository.c_str());
            }
            break;
        }
        case Element::function:
        {
            if( currentElement != Element::id && currentElement != Element::source )
            {
                utils::unexpectedElement(  localName, {IDElement, sourceElement},m_repository.c_str());
            }
            break;
        }
        case Element::id:
        case Element::source:
        {
            utils::unexpectedChild(  localName, m_repository.c_str() );
        }
        case Element::unknown:
            utils::fatal("lm::internal_error",
                         fmt::format("Invalid parser state while parsing {}.",
                                     m_repository.c_str()));
    }
    return true;
}

/**
 * Handle character data inside <ID> and <Source> elements.
 * When parsing <ID>, a new Function object is created and its ID is set.
 * When parsing <Source>, the source text is assigned to the current Function.
 * @param chars
 * @return true if build succeeds. Throws for duplicate function ids  lm::duplicated_function_id.
 */
bool Reader::charactersChar( const char *, const char *, const char *, const char *chars, const unsigned int )
{
    auto currentElement = m_elementStack.back();
    if( currentElement == Element::id )
    {
        // Checks duplicate IDs against all previously read functions.
        // Make 'function' a const reference.
        if (auto it = std::ranges::find_if(
                                            m_functions,
                                           [chars](const Function& function) { return !strcmp(chars, function.id.c_str()); }
                                           );
                it != m_functions.end())
        {
            utils::fatal(  "lm::duplicated_function_id",
                fmt::format( "Function with ID '{}' duplicated in {}.", chars, m_repository.c_str() ) );
        }
        m_functions.emplace_back( Function() );
        m_currentFunction = &m_functions.back();
        m_currentFunction->id.set( chars );
    }
    else if( currentElement == Element::source )
    {
        if (!m_currentFunction) {
            utils::fatal("lm::source_before_id",
                         fmt::format("Found <Source> before <ID> while parsing {}.", m_repository.c_str()));
        }
        m_currentFunction->source.set( chars );
    }
    return true;
}

/**
 * Pop the m_elementStack
 * @return
 */
bool Reader::endElementChar( const char *, const char *, const char * )
{
    m_elementStack.pop_back();
    return true;
}

}//namespace

Functions readRepo( const TY_Blob &data, std::string_view repo )
{
    return Reader {}.read( data, repo );
}

}
