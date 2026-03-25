// XML_xerces_String.hpp
//
// Copyright (c) 2000-2010 e2e technologies ltd

#ifndef XML_XERCES_STRING_HPP
#define XML_XERCES_STRING_HPP

#include <xercesc/util/TransService.hpp>
#include <xercesc/util/XMLString.hpp>
#include <string_view>
#include <Misc/Memory.hpp>

namespace
{

    static auto transcodedDeleter = []( auto ptr ) { xercesc::XMLString::release( &ptr ); };
    template<typename T>
    using transcoded_ptr = std::unique_ptr<T, decltype( transcodedDeleter )>;


} // namespace
class XML_xerces_String
{
	private:
		std::unique_ptr<xercesc::XMLTranscoder> m_Transcoder;

		transcoded_ptr<XMLCh> m_XMLForm;
		M::Memory::unique_ptr<char[]> m_LocalForm;

	public:
		XML_xerces_String();
		explicit XML_xerces_String( std::string_view localForm );
		explicit XML_xerces_String( const XMLCh* XMLForm );
		// prevent double free/ double delete problem for shallow copying in generated copy constructor
		XML_xerces_String(XML_xerces_String const&) = delete;
		XML_xerces_String(XML_xerces_String&& other) noexcept = default;

		XML_xerces_String& operator=(XML_xerces_String const&) = delete;
		XML_xerces_String& operator=(XML_xerces_String&& other) noexcept = default;

		virtual ~XML_xerces_String() = default;

		/** Set local form.
		  * @param localForm The string in local form.
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
		char* convertToLocalForm( const XMLCh* XMLForm) const;

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
