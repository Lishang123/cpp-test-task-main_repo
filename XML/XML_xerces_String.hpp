// XML_xerces_String.hpp
//
// Copyright (c) 2000-2010 e2e technologies ltd

#ifndef XML_XERCES_STRING_HPP
#define XML_XERCES_STRING_HPP

#include <xercesc/util/TransService.hpp>
#include <xercesc/util/XMLString.hpp>
#include <string_view>


class XML_xerces_String
{
	private:
		xercesc::XMLTranscoder* m_Transcoder;

		XMLCh* m_XMLForm;
		char* m_LocalForm;

	public:
		XML_xerces_String();
		explicit XML_xerces_String( std::string_view localForm );
		explicit XML_xerces_String( const XMLCh* XMLForm );
		virtual ~XML_xerces_String();

		/** Set local form.
		  * @param LocalForm The string in local form.
		  */
		void setLocalForm( std::string_view localForm);
		/** Set XML form.
		  * @param XMLForm The string in XML form.
		  */
		void setXMLForm( const XMLCh* XMLForm);

		/** Convert to XML form.
		  * @param LocalForm The string in local form.
		  */
		static XMLCh* convertToXMLForm( const char* LocalForm);
		/** Convert to local form. Use M_String::release() for cleanup.
		  * @param XMLForm The string in XML form.
		  */
		char* convertToLocalForm( const XMLCh* XMLForm);

		/** Convert to local form. */
        std::string_view getLocalForm();
		/** Convert to XML form. */
		const XMLCh* getXMLForm();
		
		/** Compare case insensitive to local form. */
		int compareNoCase( const char* LocalForm);
		/** Compare case insensitive to XML form. */
		int compareNoCase( const XMLCh* XMLForm);

		/** Compare case insensitive to local form. */
		int compare( const char* LocalForm);
		/** Compare case insensitive to XML form. */
		int compare( const XMLCh* XMLForm);
};

#endif
