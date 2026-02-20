#include "FUSReplaceReader.hpp"
#include "utils.hpp"
#include "../XML/XML_Parser.hpp"
#include <array>
#include <algorithm>


namespace functions::repository::replace
{

namespace
{

constexpr const std::string_view functionsArrayElement = "Functions";
constexpr const std::string_view functionElement = "Function";
constexpr const std::string_view IDElement = "ID";
constexpr const std::string_view sourceElement = "Source";
constexpr const std::string_view patternElement = "Pattern";
constexpr const std::string_view replacementElement = "Replacement";

class Reader : public XML_Parser
{
public:
    bool startElementChar( const char *, const char *localName, const char *, const xercesc::Attributes & ) override;
    bool charactersChar( const char *, const char *, const char *, const char *chars, const unsigned int ) override;
    bool endElementChar( const char *, const char *, const char *) override;

    Functions read( const TY_Blob &data , std::string_view repo ) &&;

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

private:
    Functions m_functions;
    std::vector<Element> m_elementStack;
    Function *m_currentFunction { nullptr };
    void errorMessage( const M_SystemMessage &message ) override;
    ST_String m_repository;
};

void Reader::errorMessage(const M_SystemMessage &message)
{
    throw message;
}

Reader::Element toElement( std::string_view name )
{
    using Element = Reader::Element;
    auto lowerCaseName = utils::toLower(name);

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
                                         [&lowerCaseName](auto x) { return !strcmp(lowerCaseName.c_str(), x.first.data()); });
    return it == lookup.end() ? Element::unknown : it->second;
}

Functions Reader::read(const TY_Blob &data, std::string_view repo) &&
{
    m_repository.set( repo );
    m_functions.clear();
    m_elementStack.clear();
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
            if( currentElement < Element::id || currentElement > Element::replacement )
            {
                utils::unexpectedElement(  localName, m_repository.c_str() );
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
    }
    return true;
}

bool Reader::charactersChar( const char *, const char *, const char *, const char *chars, const unsigned int )
{
    auto currentElement = m_elementStack.back();
    if( currentElement == Element::id )
    {
        if (auto it = std::ranges::find_if(m_functions,
                                           [&chars](auto function) { return !strcmp(chars, function.id.c_str()); });
                it != m_functions.end())
        {
            utils::fatal(  "lm::duplicated_function_id", fmt::format( "Function with ID '{}' duplicated in {}.", chars, m_repository.c_str() ) );
        }
        m_functions.emplace_back( Function() );
        m_currentFunction = &m_functions.back();
        m_currentFunction->id.set( chars );
    }
    else if ( currentElement == Element::source )
    {
        m_currentFunction->source.set( chars );
    }
    else if( currentElement == Element::pattern )
    {
        m_currentFunction->pattern.set( chars );
    }
    else if( currentElement == Element::replacement )
    {
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
    return Reader {}.read( data, repo );
}


}//namespace functions::repository::fus::replace