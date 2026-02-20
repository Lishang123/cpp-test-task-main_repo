// XML_Parser.hpp
//
// Copyright (c) 2001-2020 e2e technologies ltd

#ifndef XML_PARSER_HPP
#define XML_PARSER_HPP

#include "../String/ST_String.hpp"
#include "XML_xerces_String.hpp"
#include "../Misc/M_MemoryStream.hpp"

#include <xercesc/sax2/DefaultHandler.hpp>
#include <xercesc/parsers/SAX2XMLReaderImpl.hpp>
#include <xercesc/sax2/Attributes.hpp>
#include <xercesc/util/SecurityManager.hpp>
#include <xercesc/util/XMLEntityResolver.hpp>
#include <vector>
#include <sstream>


class TY_Blob;

class M_SystemMessage;


class XML_Parser: public xercesc::DefaultHandler, public xercesc::XMLEntityResolver {
private:
    bool m_Result;
    bool m_UseNamespaces;
    bool m_IgnoreXMLDeclaration;
    xercesc::SAX2XMLReaderImpl m_Parser;
    xercesc::SecurityManager m_securityManager;
    XML_xerces_String m_xercesString;

    struct StackElement {
        ST_String uri;
        ST_String name;
        ST_String qName;
    };

    std::vector<StackElement> m_Callstack;

    M_MemoryStream m_CharacterBuffer;

/** xerces DefaultHandler interface. */
    virtual void startElement(const XMLCh *const URI, const XMLCh *const LocalName,
                              const XMLCh *const QName,
                              const xercesc::Attributes &CurrentAttributes);

    virtual void endElement(const XMLCh *const URI, const XMLCh *const LocalName,
                            const XMLCh *const QName);

    virtual void characters(const XMLCh *const Chars, const XMLSize_t Length);

    virtual void characters(const std::string &LocalCharacters);

    virtual void endCharacters();

    virtual void startPrefixMapping(const XMLCh *const Prefix, const XMLCh *const URI);

    virtual void endPrefixMapping(const XMLCh *const Prefix);

/** SAX ErrorHandler interface. */
    virtual void warning(const xercesc::SAXParseException &Exception);

    virtual void error(const xercesc::SAXParseException &Exception);

    virtual void fatalError(const xercesc::SAXParseException &Exception);

    void m_errorMessage(const xercesc::SAXParseException &E, const char *Code, const char *Description);

protected:
    bool m_UsingXOP;
    bool m_CurrentCharactersArePlainBinaryData;

/**
 * Parse prepared character buffer
 * @param Buffer Character buffer to parse
 * @param BufferSize How many characters are there in the buffer
 * @return If parsing succeeded
 */
    virtual bool m_Parse(const char *Buffer, size_t BufferSize);

    const char *m_RemoveXMLDeclaration(const char *Buffer, size_t BufferSize = -1 );

public:

    XML_Parser(bool UseValidation = false, bool UseNamespaces = false, bool IgnoreXMLDeclaration = false);

    virtual ~

    XML_Parser();

/** Start of a new element. */
    virtual bool startElementChar(const char *URI, const char *LocalName, const char *QName,
                                  const xercesc::Attributes &CurrentAttributes);

/** End of the current element. */
    virtual bool endElementChar(const char *URI, const char *LocalName, const char *QName);

/** Character part of the current element. */
    virtual bool charactersChar(const char *URI, const char *LocalName, const char *QName,
                                const char *Chars, const unsigned int Length);

/** A new prefix mapping occurs. */
    virtual bool startPrefixMappingChar(const char *Prefix, const char *URI);

/** An old prefix mapping vanishes. */
    virtual bool endPrefixMappingChar(const char *Prefix);

/** The parser want's to tell us that an error ocurred.
  * @param message The message describing what happened. It's yours, so delete it when
  * you're done.
  */
    virtual void errorMessage(const M_SystemMessage &message);

    M_SystemMessage systemMessageFromException(const xercesc::SAXParseException &E,
                                               const char *Code,
                                               const char *errorLevel);

/** Parse buffer.
  * @param Buffer Buffer to parse.
  */
    virtual bool parseBlob(const TY_Blob *Buffer);

/** Set parser feature
  * See http://xml.apache.org/xerces-c/program-sax2.html#SAX2Features
  * @param Name Name of the feature
  * @param Value true or false to enable/disable feature
  */
    void setOption(const char *Name, bool Value);

/** Set parser property
  * See http://xml.apache.org/xerces-c/program-sax2.html#SAX2Properties
  * @param Name Name of the property
  * @param Value String to set the property
  */
    void setOption(const char *Name, const char *Value);

    void setIgnoreXMLDeclaration(bool Ignore);

    bool getIgnoreXMLDeclaration() const;

    void setEntityExpansionLimit(XMLSize_t limit);

    void restoreDefaultEntityExpansionLimit();

/** Disable entity/DTD/schema resolver.
 * If you don't trust the XML (like SOAP service) it is a good idea
 * to disable it because of security vulnerability
 * https://www.owasp.org/index.php/XML_External_Entity_(XXE)_Processing
 */
    void disableEntityResolver();

    using xercesc::DefaultHandler::resolveEntity;

    xercesc::InputSource *resolveEntity(xercesc::XMLResourceIdentifier *resourceIdentifier)

    override;

    static ST_String getAttributeValue(const xercesc::Attributes &attributes, std::string_view name);

    static long getAttributeLong(const xercesc::Attributes &attributes, std::string_view name, bool &exists);

    static long getAttributeLong(const xercesc::Attributes &attributes, std::string_view name);

    static bool getAttributeBool(const xercesc::Attributes &attributes, std::string_view name, bool &exists);

    static bool getAttributeBool(const xercesc::Attributes &attributes, std::string_view name);

};


#endif
