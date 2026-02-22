#include "FUSReplaceReader.hpp"
#include "utils.hpp"
#include "../XML/XML_Parser.hpp"
#include "specs/ReplaceSpec.hpp"
#include "../Misc/SystemMessageError.hpp"

#include <array>
#include <algorithm>

namespace functions::repository::replace
{

namespace
{

constexpr std::string_view functionsArrayElement = "Functions";
constexpr std::string_view functionElement = "Function";
constexpr std::string_view IDElement = "ID";
constexpr std::string_view sourceElement = "Source";
constexpr std::string_view patternElement = "Pattern";
constexpr std::string_view replacementElement = "Replacement";

class Reader : public XML_Parser
{
public:
    Functions read( const TY_Blob &data , std::string_view repo ) &&;

private:

    enum class Element
    {
        document,
        functions,
        function,
        id,
        source,
        pattern,
        replacement,
        unknown
    };
    static Element toElement(std::string_view name);

    Functions m_functions;
    std::vector<Element> m_elementStack;
    Function *m_currentFunction { nullptr };
    ST_String m_repository;
    void errorMessage( const M_SystemMessage &message ) override;

    bool startElementChar( const char *, const char *localName, const char *, const xercesc::Attributes & ) override;
    bool charactersChar( const char *, const char *, const char *, const char *chars, const unsigned int ) override;
    bool endElementChar( const char *, const char *, const char *) override;

    static constexpr std::string_view toString(Element e)
    {
        switch (e)
        {
            case Element::functions:   return functionsArrayElement;
            case Element::function:    return functionElement;
            case Element::id:          return IDElement;
            case Element::source:      return sourceElement;
            case Element::pattern:     return patternElement;
            case Element::replacement: return replacementElement;
            default:                   return "Unknown";
        }
    }
};

void Reader::errorMessage(const M_SystemMessage &message)
{
    throw SystemMessageError(message);
}

Reader::Element Reader::toElement( std::string_view name )
{
    using Element = Reader::Element;
    auto lowerCaseName = utils::lowercaseUntilCamelBoundary(name);

    #define X(x) {#x, Element::x}
    constexpr std::array<std::pair<std::string_view, Element>, static_cast<size_t>( Element::unknown ) - 1> lookup = { {
      X( functions ),
      X( function ),
      X( id ),
      X( source ),
      X( pattern ),
      X( replacement )
    } };
    #undef X

    const auto it = std::ranges::find_if(lookup,
                                         [&lowerCaseName](const auto& x) { return lowerCaseName == x.first; });
    return it == lookup.end() ? Element::unknown : it->second;
}

Functions Reader::read(const TY_Blob &data, std::string_view repo) &&
{
    m_repository.set( repo );
    m_elementStack.push_back( Element::document );

    parseBlob( &data );

    return std::move( m_functions );
}

bool Reader::startElementChar( const char *, const char *localName, const char *, const xercesc::Attributes & )
{
    auto previousElement = m_elementStack.back();
    auto currentElement = toElement( localName );
    m_elementStack.push_back( currentElement );

    if( currentElement == Element::unknown )
    {
        utils::unexpectedElement(  localName, m_repository.c_str() );
    }

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
                utils::unexpectedElement(   localName, functionElement, m_repository.c_str() );
            }
            break;
        }
        case Element::function:
        {
            if (currentElement != Element::id &&
                currentElement != Element::source &&
                currentElement != Element::pattern &&
                currentElement != Element::replacement)
            {
                utils::unexpectedElement(  localName,
                    {IDElement, sourceElement, patternElement, replacementElement},
                    m_repository.c_str() );
            }
            break;
        }
        case Element::id:
        case Element::source:
        case Element::pattern:
        case Element::replacement:
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

bool Reader::charactersChar( const char *, const char *, const char *, const char *chars, const unsigned int )
{
    auto currentElement = m_elementStack.back();
    if( currentElement == Element::id )
    {
        if (auto it = std::ranges::find_if(m_functions,
                                           [chars](const Function& function) { return !strcmp(chars, function.id.c_str()); });
                it != m_functions.end())
        {
            utils::fatal(  "lm::duplicated_function_id", fmt::format( "Function with ID '{}' duplicated in {}.", chars, m_repository.c_str() ) );
        }
        m_functions.emplace_back();
        m_currentFunction = &m_functions.back();
        m_currentFunction->id.set( chars );
    }
    else if (currentElement == Element::source
        || currentElement == Element::pattern
        || currentElement == Element::replacement) {
        if (!m_currentFunction) {
            utils::fatal("lm::other_function_child_before_id",
                         fmt::format("Found <{}> before <ID> while parsing {}.", toString(currentElement), m_repository.c_str()));
        }
        if ( currentElement == Element::source )
            m_currentFunction->source.set( chars );
        else if( currentElement == Element::pattern )
            m_currentFunction->pattern.set( chars );
        else
            m_currentFunction->replacement.set( chars );
    }
    return true;
}

bool Reader::endElementChar( const char *, const char *, const char * )
{
    m_elementStack.pop_back();
    return true;
}

}//namespace

Functions readRepo( const TY_Blob &data, std::string_view repo )
{
    // You can swap the reader used here.
    // return Reader {}.read( data, repo );
    return FUSReader<ReplaceSpec>{}.read( data, repo );
}


}