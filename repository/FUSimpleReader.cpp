// This file implements an XML reader for  using SAX-style parsing.
// The parser enforces a strict structure:
//   document -> Functions -> Function -> (ID, Source)
// Any unexpected element results in an error.

#include "FUSimpleReader.hpp"
#include "../XML/XML_Parser.hpp"
#include "specs/SimpleSpec.hpp"
#include "utils.hpp"
#include "../Misc/SystemMessageError.hpp"

#include <array>
#include <algorithm>

namespace functions::repository::simple
{
namespace
{

constexpr std::string_view functionsArrayElement = "Functions";
constexpr std::string_view functionElement = "Function";
constexpr std::string_view IDElement = "ID";
constexpr std::string_view sourceElement = "Source";

class Reader : public XML_Parser
{
public:
    Functions read ( const TY_Blob &data, std::string_view repo ) &&;

private:
    enum class Element
    {
        document,
        functions,
        function,
        id,
        source,
        unknown
    };
    static Element toElement(std::string_view name);

    Functions m_functions;
    // tracks the current position
    std::vector<Element> m_elementStack;
    std::optional<std::size_t> m_currentFuncIndex{};
    void errorMessage( const M_SystemMessage &message )override;
    ST_String m_repository;

    bool startElementChar( const char *, const char *localName, const char *, const xercesc::Attributes & ) override;
    bool endElementChar( const char *, const char *, const char * ) override;
    bool charactersChar( const char *, const char *, const char *, const char *text, unsigned int ) override;

};

void Reader::errorMessage( const M_SystemMessage &message )
{
    // Any XML parse error becomes an exception.
    throw SystemMessageError(message);
}

/**
 * Mapping tag names to enums.
 * @param name: The name of the element.
 * @return: The element enum defined in the reader.
 */
Reader::Element Reader::toElement(const std::string_view name )
{
    auto lowerCaseName = utils::lowercaseUntilCamelBoundary( name );

    static constexpr std::array<std::pair<std::string_view, Element>, 4> lookup{{
        {"functions", Element::functions},
        {"function",  Element::function},
        {"id",        Element::id},
        {"source",    Element::source},
    }};
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
Functions Reader::read( const TY_Blob &data, const std::string_view repo ) && // ref-value qualifier
{
    // stores repo label for diagnostics
    m_repository.set( repo );
    // add these lines to prevent deliberate reuse of this function like: std::move(reader).read()
    // m_functions.clear();
    // m_elementStack.clear();
    // m_currentFuncIndex = std::nullopt;
    // seeds the stack with document
    m_elementStack.push_back( Element::document );

    //SAX parsing
    parseBlob( &data );

    return std::move( m_functions );
}

/**
 * Structural validation of the element and push back to element stack.<br>
    It does not enforce order of <ID> vs <Source>. <br>
    It does not enforce that <Function> actually contains both fields. <br>
    It does not prevent multiple <Source> tags etc. <br>
    FIXME?? It does not create the Function object when seeing Function.
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
    * Handle character data inside <ID> and <Source> elements. <br>
    * When parsing <ID>, a new Function object is created and its ID is set. <br>
    * When parsing <Source>, the source text is assigned to the current Function. <br>
    * @param text the text content. An empty string will be given if the content is empty.
    * @return true if build succeeds.
    * Throws lm::duplicated_function_id for duplicate function ids.
    * Throws lm::source_before_id if ID is not the first child element.
*/
bool Reader::charactersChar( const char *, const char *, const char *, const char *text, const unsigned int )
{
    if(const auto currentElement = m_elementStack.back(); currentElement == Element::id )
    {
        // Checks duplicate IDs against all previously read functions.
        // Made 'function' a const reference.
        // text can never be nullptr, same as function.id.c_str() since it's copied from char*, so this comp is null-safe.
        if (const auto it = std::ranges::find_if(
                                            m_functions,
                                           [text](const Function& function) { return !strcmp(text, function.id.c_str()); }
                                           );
                it != m_functions.end())
        {
            utils::fatal(  "lm::duplicated_function_id",
                fmt::format( "Function with ID '{}' duplicated in {}.", text, m_repository.c_str() ) );
        }
        m_functions.emplace_back();
        m_currentFuncIndex = m_functions.size() -1;
        m_functions[*m_currentFuncIndex].id.set( text );
    }
    else if( currentElement == Element::source )
    {
        if (!m_currentFuncIndex) {
            utils::fatal("lm::source_before_id",
                         fmt::format("Found <Source> before <ID> while parsing {}.", m_repository.c_str()));
        }
        m_functions[*m_currentFuncIndex].source.set( text );
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

Functions readRepo_legacy( const TY_Blob &data, std::string_view repo )
{
    return Reader {}.read( data, repo );
}

Functions readRepo( const TY_Blob &data, std::string_view repo )
{
    // You can swap the reader used here.
    // return Reader {}.read( data, repo );
    return FUSReader<SimpleSpec>{}.read( data, repo );
}

}
