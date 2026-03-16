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
    void startElement(const XMLCh *const URI, const XMLCh *const LocalName,
                              const XMLCh *const QName,
                              const xercesc::Attributes &CurrentAttributes) override;

    void endElement(const XMLCh *const URI, const XMLCh *const LocalName,
                            const XMLCh *const QName) override;

    void characters(const XMLCh *const Chars, const XMLSize_t Length) override;

    virtual void characters(const std::string &LocalCharacters);

    virtual void endCharacters();

    void startPrefixMapping(const XMLCh *const Prefix, const XMLCh *const URI) override;

    void endPrefixMapping(const XMLCh *const Prefix) override;

/** SAX ErrorHandler interface. */
    void warning(const xercesc::SAXParseException &Exception) override;

    void error(const xercesc::SAXParseException &Exception) override;

    void fatalError(const xercesc::SAXParseException &Exception) override;

    void m_errorMessage(const xercesc::SAXParseException &E, const char *Code, const char *Description);

protected:
    bool m_UsingXOP; //TODO: never read, only assigned.
    bool m_CurrentCharactersArePlainBinaryData; //TODO: never read, only assigned.

    /**
     * Parse prepared character buffer
     * @param Buffer Character buffer to parse
     * @param BufferSize How many characters are there in the buffer
     * @return If parsing succeeded
     */
    virtual bool m_Parse(const char *Buffer, size_t BufferSize);

    /**
     * Skip past the XML declaration at the beginning of a buffer
     * @param Buffer
     * @param BufferSize
     * @return A pointer to where the real XML content starts.
     */
    static const char* m_RemoveXMLDeclaration(const char *Buffer, size_t BufferSize = -1 );

public:

    XML_Parser(bool UseValidation = false, bool UseNamespaces = false, bool IgnoreXMLDeclaration = false);

    XML_Parser(const XML_Parser &) = delete;
    XML_Parser& operator=(const XML_Parser &) = delete;
    XML_Parser(XML_Parser &&) = delete;
    XML_Parser& operator=(XML_Parser &&) = delete;

    ~XML_Parser() override;

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
      * See http://xml.apache.org/xerces-c/program-sax2.html#SAX2Features (dead link)
      * @param Name Name of the feature
      * @param Value true or false to enable/disable feature
      */
    void setOption(const char *Name, bool Value);

    /** Set parser property
      * See http://xml.apache.org/xerces-c/program-sax2.html#SAX2Properties (dead link)
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

    xercesc::InputSource *resolveEntity(xercesc::XMLResourceIdentifier *resourceIdentifier) override;

    /**
     * Get the value of an attribute with a given name
     * @param attributes xerces attributes set
     * @param name name of the attribute
     * @return the value of the found attribute <br>
     *      empty string for non-existing attribute or empty attribute value
     * @note this function doesn't differentiate non-existing attribute and empty attribute value
     */
    static ST_String getAttributeValue(const xercesc::Attributes &attributes, std::string_view name);

    static std::optional<long> getAttributeLongOptional(const xercesc::Attributes &attributes, std::string_view name);

    static long getAttributeLong(const xercesc::Attributes &attributes, std::string_view name, bool &exists);

    static long getAttributeLong(const xercesc::Attributes &attributes, std::string_view name);

    /**
     * Get the value of a boolean attribute with a given name
     * @param attributes xerces attributes set
     * @param name name of the attribute
     * @param exists a variable to be overridden by the function (true if exists)
     * @return the value of the boolean attribute
     * @pre The value must be lower-case 'true' or 'false', i.e. '0/1' or uppercase values are not supported
     */
    static bool getAttributeBool(const xercesc::Attributes &attributes, std::string_view name, bool &exists);

    static std::optional<bool>  getAttributeBool(const xercesc::Attributes &attributes, std::string_view name);

};


#endif
