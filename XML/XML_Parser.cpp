// Copyright (c) Scheer PAS Schweiz AG

// SAX has to be first, otherwise it won't compile.
#include <xercesc/framework/MemBufInputSource.hpp>
#include <xercesc/framework/LocalFileInputSource.hpp>
#include <xercesc/util/BinMemInputStream.hpp>
#include <xercesc/util/XMLURL.hpp>

#include "../Misc/M_MemoryStream.hpp"
#include "../Misc/Memory.hpp"
#include "../Misc/M_SystemMessage.hpp"
#include <sys/stat.h>
#include "../Types/TY_Blob.hpp"
#include "XML_Parser.hpp"
#include <array>
#include <sstream>

namespace core::xml
{

    class StreamInputSource : public xercesc::InputSource
    {
    public:
        StreamInputSource( const XMLCh* const systemId, M_MemoryStream&& content )
                : InputSource { systemId, xercesc::XMLPlatformUtils::fgMemoryManager }
                , m_content { std::move( content ) }
        {}

        xercesc::BinInputStream* makeStream() const override
        {
            const char* content {};
            auto size { m_content.getSize() };
            m_content.getContent(&content);
            return new(getMemoryManager()) xercesc::BinMemInputStream {
                    reinterpret_cast<const XMLByte*>( content ),
                    size,
                    xercesc::BinMemInputStream::BufOpt_Copy,
                    getMemoryManager()
            };
        }

    private:
        M_MemoryStream m_content;
    };

}

namespace
{

    static auto transcodedDeleter = []( auto ptr ) { xercesc::XMLString::release( &ptr ); };
    template<typename T>
    using transcoded_ptr = std::unique_ptr<T, decltype( transcodedDeleter )>;


} // namespace

XML_Parser::XML_Parser(bool UseValidation, bool UseNamespaces, bool IgnoreXMLDeclaration)
        : m_UseNamespaces( UseNamespaces)
        , m_IgnoreXMLDeclaration( IgnoreXMLDeclaration)
        , m_UsingXOP( false)
{
    XML_xerces_String Validation( "http://xml.org/sax/features/validation");
    m_Parser.setFeature( Validation.getXMLForm(), UseValidation);

    if( !UseValidation)
    {
        // Disable resolving external DTDs, at least if there's no validation at all!!
        // see http://xerces.apache.org/xerces-c/program-sax2-3.html#load-external-dtd
        XML_xerces_String ExternalDTD( "http://apache.org/xml/features/nonvalidating/load-external-dtd");
        m_Parser.setFeature( ExternalDTD.getXMLForm(), false);
        // Disable schema support, at least if there's no validation at all!!
        // see http://xerces.apache.org/xerces-c/program-sax2-3.html#schema
        XML_xerces_String Schema( "http://apache.org/xml/features/validation/schema");
        m_Parser.setFeature( Schema.getXMLForm(), false);
    }

    // Very important to turn namespaces on.
    XML_xerces_String Namespaces( "http://xml.org/sax/features/namespaces");
    m_Parser.setFeature( Namespaces.getXMLForm(), m_UseNamespaces);

    m_Parser.setContentHandler( this);
    m_Parser.setErrorHandler( this);

    // Add our own resolver to rewrite local file access
    m_Parser.setXMLEntityResolver( this );

    setEntityExpansionLimit( 1);
    XML_xerces_String securityManagerProperty( "http://apache.org/xml/properties/security-manager");
    m_Parser.setProperty( securityManagerProperty.getXMLForm(), &m_securityManager);
}

XML_Parser::~XML_Parser() = default;

void XML_Parser::setOption( const char* Name, bool Value)
{
    XML_xerces_String OptionName( Name);
    m_Parser.setFeature( OptionName.getXMLForm(), Value);
}

void XML_Parser::setOption( const char* Name, const char* Value)
{
    XML_xerces_String OptionName( Name);
    XML_xerces_String OptionValue( Value);
    m_Parser.setProperty( OptionName.getXMLForm(), (void*) OptionValue.getXMLForm());
}

void XML_Parser::setIgnoreXMLDeclaration( bool Ignore)
{
    m_IgnoreXMLDeclaration = Ignore;
}

bool XML_Parser::getIgnoreXMLDeclaration() const
{
    return m_IgnoreXMLDeclaration;
}

void XML_Parser::setEntityExpansionLimit( XMLSize_t limit)
{
    m_securityManager.setEntityExpansionLimit( limit);;
}

void XML_Parser::restoreDefaultEntityExpansionLimit()
{
    setEntityExpansionLimit( xercesc::SecurityManager::ENTITY_EXPANSION_LIMIT);
}

void XML_Parser::disableEntityResolver()
{
    m_Parser.setEntityResolver( nullptr );
    setOption( "http://apache.org/xml/features/disable-default-entity-resolution", true);
}

xercesc::InputSource* XML_Parser::resolveEntity( xercesc::XMLResourceIdentifier* resourceIdentifier )
{

    return nullptr;

}

bool XML_Parser::parseBlob( const TY_Blob* Buffer)
{

    m_UsingXOP = false;

    if( !Buffer || !Buffer->getContent() || Buffer->getSize() == 0)
    {
        errorMessage( M_SystemMessage { "XMLLM_DOMAIN", "XMLLM_EMPTY_BUFFER",
                                        "Can't parse empty buffer." } );
        return false;
    }

    m_UsingXOP = false;

    return m_Parse( Buffer->getContent(), Buffer->getSize());
}


bool XML_Parser::m_Parse( const char* Buffer, size_t BufferSize)
{
    if( m_IgnoreXMLDeclaration)
    {
        const char* TmpBuffer = m_RemoveXMLDeclaration( Buffer, BufferSize);
        BufferSize -= ( TmpBuffer - Buffer);
        Buffer = TmpBuffer;
    }

    xercesc::MemBufInputSource
            MemoryBuffer( reinterpret_cast< const unsigned char*>( Buffer),
                          BufferSize, "XML_Buffer", false);

    // Avoid additional copying of the input buffer. The requirement is that the buffer remain valid
    // till the end of parsing. Since we parse synchronously and always have the whole buffer at hand,
    // this is true and safe.
    MemoryBuffer.setCopyBufToStream( false);

    m_CharacterBuffer.reset();
    m_Result = true;

    try
    {
        m_Parser.parse( MemoryBuffer);

        return m_Result;
    }
    catch( const xercesc::XMLException &E)
    {
        XML_xerces_String Error( E.getMessage());

        errorMessage( M_SystemMessage {
                "XMLLM_DOMAIN",
                "XMLLM_XML_PARSER_NATIVE_XML_ERROR",
                fmt::format("XML error: {}", Error.getLocalForm())
        } );
    }
    catch( const xercesc::SAXException& E )
    {
        XML_xerces_String Error( E.getMessage() );

        errorMessage( M_SystemMessage {
                "XMLLM_DOMAIN",
                "XMLLM_XML_PARSER_NATIVE_SAX_ERROR",
                fmt::format("SAX error: {}", Error.getLocalForm())
        } );
    }

    return false;
}

const char *XML_Parser::m_RemoveXMLDeclaration( const char *Buffer, size_t BufferSize)
{
    const char* NoWhitespaceBuffer = Buffer;
    //skipping leading whitespaces
    while( *NoWhitespaceBuffer && std::isspace(*NoWhitespaceBuffer))
    {
        ++NoWhitespaceBuffer;
    }
    if( ( BufferSize -= NoWhitespaceBuffer - Buffer) > 5)
    {
        if( NoWhitespaceBuffer[0] == '<'
            && NoWhitespaceBuffer[1] == '?'
            && NoWhitespaceBuffer[2] == 'x'
            && NoWhitespaceBuffer[3] == 'm'
            && NoWhitespaceBuffer[4] == 'l')
        {
            NoWhitespaceBuffer += 5;
            while( --BufferSize >= 2 && NoWhitespaceBuffer[0] != '?' && NoWhitespaceBuffer[1] != '>')
            {
                ++NoWhitespaceBuffer;
            }
            if( BufferSize >= 2)
            {
                NoWhitespaceBuffer += 2; //we didn't remove "?>" sequence, so it's high time to do this
            }
        }
    }

    return NoWhitespaceBuffer;
}

void XML_Parser::startElement( const XMLCh* const URI, const XMLCh* const Name,
                               const XMLCh* const QName,
                               const xercesc::Attributes &ElementAttributes)
{


    if( !m_Result) return;

    ST_String LocalQName;
    LocalQName.consume( m_xercesString.convertToLocalForm( QName ) );
    ST_String LocalURI;
    LocalURI.consume( m_UseNamespaces ? m_xercesString.convertToLocalForm( URI ) : M::Memory::create( 0 ) );
    ST_String LocalName;
    LocalName.consume( m_UseNamespaces
                       ? m_xercesString.convertToLocalForm( Name )
                       : M::Memory::duplicate( LocalQName.c_str(), strlen( LocalQName.c_str() ) ) );


    endCharacters( );


    m_Callstack.push_back( { std::move( LocalURI ), std::move( LocalName ), std::move( LocalQName ) });


    auto& el = m_Callstack.back();
    m_Result = startElementChar( el.uri.c_str(), el.name.c_str(), el.qName.c_str(), ElementAttributes);

}

void XML_Parser::endElement( const XMLCh* const URI, const XMLCh* const Name,
                             const XMLCh* const QName)
{

    if( !m_Result) return;

    auto& el = m_Callstack.back();
    endCharacters();


    if(  m_Result)
    {
        m_Result = endElementChar( el.uri.c_str(), el.name.c_str(), el.qName.c_str());
    }

    m_Callstack.pop_back();
}

void XML_Parser::characters(const XMLCh* const Characters, const XMLSize_t Length)
{

    if( !m_Result)
    {
        return;
    }

    // This callback tries to store the character part.
    char* LocalCharacters = m_xercesString.convertToLocalForm( Characters);

    m_CharacterBuffer.writeConsume( LocalCharacters);
}

void XML_Parser::characters( const std::string& LocalCharacters)
{


    if( !m_Result)
    {
        return;
    }

    std::string::size_type Length = LocalCharacters.size();

    // LocalCharacters can contain 0 bytes
    m_CharacterBuffer.writeConsume( M::Memory::duplicate( LocalCharacters.c_str(), Length), Length);
}

void XML_Parser::endCharacters()
{
    if( !m_Result)
    {
        return;
    }

    // Only forward call if this character part is part of an element. This filters the call on the
    // start of parsing.
    if( !m_Callstack.empty())
    {
        m_CharacterBuffer.terminate();
        const char* Characters;
        T_uint64 CharactersLength;
        m_CharacterBuffer.getContent( &Characters, &CharactersLength);
        auto& el = m_Callstack.back();
        m_Result = charactersChar( el.uri.c_str(), el.name.c_str(), el.qName.c_str(),
                                   Characters, CharactersLength-1);
    }

    m_CharacterBuffer.reset();
    m_CurrentCharactersArePlainBinaryData = false;
}

void XML_Parser::startPrefixMapping( const XMLCh* const Prefix, const XMLCh* const URI)
{

    if( !m_Result)
    {
        return;
    }

    char* LocalPrefix = m_xercesString.convertToLocalForm( Prefix);
    char* LocalURI = m_xercesString.convertToLocalForm( URI);

    m_Result = startPrefixMappingChar( LocalPrefix, LocalURI);

    M::Memory::release( LocalPrefix);
    M::Memory::release( LocalURI);
}

void XML_Parser::endPrefixMapping( const XMLCh* const Prefix)
{
    if( !m_Result)
    {
        return;
    }

    char* LocalPrefix = m_xercesString.convertToLocalForm( Prefix);

    m_Result = endPrefixMappingChar( LocalPrefix);

    M::Memory::release( LocalPrefix);
}

void XML_Parser::m_errorMessage( const xercesc::SAXParseException& E, const char* Code, const char* Description)
{
    errorMessage( systemMessageFromException( E, Code, Description ) );
    m_Result = false;
}

void XML_Parser::error( const xercesc::SAXParseException& E)
{
    m_errorMessage( E, "XMLLM_XR_REPOSITORYPARSER_NATIVE_PARSE_ERROR", "error");
}

void XML_Parser::fatalError( const xercesc::SAXParseException& E)
{
    m_errorMessage( E, "XMLLM_XR_REPOSITORYPARSER_NATIVE_FATAL_PARSE_ERROR", "fatal error");
}

void XML_Parser::warning( const xercesc::SAXParseException& E)
{
    m_errorMessage( E, "XMLLM_XR_REPOSITORYPARSER_NATIVE_PARSE_WARNING", "warning");
}

bool XML_Parser::startElementChar( const char* URI, const char* LocalName, const char* QName,
                                   const xercesc::Attributes &ElementAttributes)
{
    return true;
}

bool XML_Parser::endElementChar( const char* URI, const char* LocalName, const char* QName)
{
    return true;
}

bool XML_Parser::startPrefixMappingChar( const char* Prefix, const char* URI)
{
    return true;
}

bool XML_Parser::endPrefixMappingChar( const char* Prefix)
{
    return true;
}

bool XML_Parser::charactersChar( const char* URI, const char* LocalName, const char* QName,
                                 const char* Chars, const unsigned int Length)
{
    return true;
}

void XML_Parser::errorMessage( const M_SystemMessage& message )
{}

M_SystemMessage
XML_Parser::systemMessageFromException( const xercesc::SAXParseException& E, const char* Code,
                                        const char* errorLevel )
{
    // Generate XPath location
    M_MemoryStream Location;
    for( const auto& el : m_Callstack )
    {
        Location.write( "/");
        Location.write( el.qName);
    }
    Location.terminate();
    const char* LocationString;
    Location.getContent( &LocationString);

    return M_SystemMessage {
            "XMLLM_DOMAIN", Code,
            fmt::format( R"(Native XML parser {} in "{}" line {}, column {}, location {}: "{}")",
                         errorLevel,
                         XML_xerces_String { E.getSystemId() }.getLocalForm(),
                         static_cast<unsigned long>(E.getLineNumber()),
                         static_cast<unsigned long>(E.getColumnNumber()),
                         LocationString,
                         XML_xerces_String { E.getMessage() }.getLocalForm() )
    };
}

ST_String XML_Parser::getAttributeValue( const xercesc::Attributes &attributes, std::string_view name )
{
    XML_xerces_String  nam2 = XML_xerces_String( name );
    const auto *value2 = attributes.getValue( nam2.getXMLForm() );
    if( const auto *value = attributes.getValue( XML_xerces_String { name }.getXMLForm() ) )
    {
        return ST_String { XML_xerces_String { value }.getLocalForm() };
    }

    return {};
}

long XML_Parser::getAttributeLong( const xercesc::Attributes &attributes, std::string_view name, bool& exists)
{
    if( const auto *xmlValue = attributes.getValue( XML_xerces_String { name }.getXMLForm() ) )
    {
        std::istringstream iss(XML_xerces_String{ xmlValue }.getLocalForm().data());
        long result {};
        iss >> result;
        if( iss.fail() || iss.eof() )
        {
            exists = true;
            return result;
        }
    }

    exists = false;
    return {};
}

long XML_Parser::getAttributeLong( const xercesc::Attributes &attributes, std::string_view name )
{
    bool dummy{};
    return getAttributeLong( attributes, name, dummy);
}

bool XML_Parser::getAttributeBool( const xercesc::Attributes &attributes, std::string_view name, bool& exists)
{
    if( const auto *xmlValue = attributes.getValue( XML_xerces_String { name }.getXMLForm() ) )
    {
        static constexpr std::array<XMLCh, 5> true_ {{ 't', 'r', 'u', 'e', '\0' }};
        if( xercesc::XMLString::compareString( xmlValue, true_.data() ) == 0 )
        {
            exists = true;
            return true;
        }

        static constexpr std::array<XMLCh, 6> false_ {{ 'f', 'a', 'l', 's', 'e', '\0' }};
        if( xercesc::XMLString::compareString( xmlValue, false_.data() ) == 0 )
        {
            exists = true;
            return false;
        }
    }

    exists = false;
    return {};
}

bool XML_Parser::getAttributeBool( const xercesc::Attributes &attributes, std::string_view name )
{
    bool dummy{};
    return getAttributeBool( attributes, name, dummy );
}

