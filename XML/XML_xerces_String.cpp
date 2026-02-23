// XML_xerces_String.cpp
//
// Copyright (c) 2000-2005 e2e technologies ltd

#include "XML_xerces_String.hpp"

#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/XMLString.hpp>
#include "../Misc/Memory.hpp"

XML_xerces_String::XML_xerces_String()
                 : m_Transcoder(nullptr)
                 , m_XMLForm( nullptr )
				 , m_LocalForm( nullptr )
{

	// If we have an input of more than 64k, this might fail badly.
	xercesc::XMLTransService::Codes Result;

	m_Transcoder =
		xercesc::XMLPlatformUtils::fgTransService->makeNewTranscoderFor( "utf-8", Result, 64 * 1024,
		                                                                 xercesc::XMLPlatformUtils::fgMemoryManager);
}

XML_xerces_String::XML_xerces_String( std::string_view localForm )
                 : m_Transcoder( nullptr), m_XMLForm( nullptr ), m_LocalForm( nullptr)
{

	// If we have an input of more than 64k, this might fail badly.
	xercesc::XMLTransService::Codes result {};

	m_Transcoder =
		xercesc::XMLPlatformUtils::fgTransService->makeNewTranscoderFor( "utf-8", result, 64 * 1024,
		                                                                 xercesc::XMLPlatformUtils::fgMemoryManager);

	m_LocalForm = M::Memory::duplicate( localForm.data(), localForm.size());
}

XML_xerces_String::XML_xerces_String( const XMLCh* String)
                 : m_Transcoder( nullptr), m_XMLForm( nullptr), m_LocalForm( nullptr)
{
	// If we have an input of more than 64k, this might fail badly.
	xercesc::XMLTransService::Codes Result;

	m_Transcoder =
		xercesc::XMLPlatformUtils::fgTransService->makeNewTranscoderFor( "utf-8", Result, 64 * 1024,
		                                                                 xercesc::XMLPlatformUtils::fgMemoryManager);

	m_XMLForm = xercesc::XMLString::replicate( String);
}

XML_xerces_String::XML_xerces_String(XML_xerces_String &&other) noexcept
: m_Transcoder( other.m_Transcoder), m_XMLForm( other.m_XMLForm), m_LocalForm( other.m_LocalForm)
{
	other.m_XMLForm = nullptr;
	other.m_LocalForm = nullptr;
	other.m_Transcoder = nullptr;
}

XML_xerces_String & XML_xerces_String::operator=(XML_xerces_String &&other) noexcept {
	if( this == &other) {
		return(*this);
	}
	// release old memory
	M::Memory::release( m_LocalForm );
	xercesc::XMLString::release( &m_XMLForm);
	delete m_Transcoder;

	// steal other's resources
	m_XMLForm = other.m_XMLForm;
	m_LocalForm = other.m_LocalForm;
	m_Transcoder = other.m_Transcoder;

	// remove other's pointers
	other.m_XMLForm = nullptr;
	other.m_LocalForm = nullptr;
	other.m_Transcoder = nullptr;
	return(*this);
}

XML_xerces_String::~XML_xerces_String()
{
	M::Memory::release( m_LocalForm );
	xercesc::XMLString::release( &m_XMLForm);

	delete m_Transcoder;
}

void XML_xerces_String::setLocalForm( std::string_view localForm)
{
    M::Memory::release( m_LocalForm );
    xercesc::XMLString::release( &m_XMLForm);

	m_XMLForm = nullptr;
	m_LocalForm = M::Memory::duplicate( localForm.data(), localForm.length() );
}

void XML_xerces_String::setXMLForm( const XMLCh* XMLForm)
{
    M::Memory::release( m_LocalForm );
	xercesc::XMLString::release( &m_XMLForm);

	m_XMLForm = xercesc::XMLString::replicate( XMLForm);
}

XMLCh* XML_xerces_String::convertToXMLForm( const char* LocalForm)
{
	return( xercesc::XMLString::transcode( LocalForm));
}

char* XML_xerces_String::convertToLocalForm( const XMLCh* XMLForm) const
{
	// If we have an input of more than 64k, this might fail badly.
	if( m_Transcoder)
	{
		const XMLSize_t CharsAvailable = xercesc::XMLString::stringLen( XMLForm);
		char* LocalForm = M::Memory::create( CharsAvailable * 4);

		XMLSize_t CharsEaten;

		const XMLSize_t ResultLength =
			m_Transcoder->transcodeTo( XMLForm, CharsAvailable,
		                               reinterpret_cast< unsigned char*>( LocalForm),
		                               CharsAvailable * 4, CharsEaten,
		                               xercesc::XMLTranscoder::UnRep_RepChar);

		LocalForm[ ResultLength ] = 0;


		return( LocalForm);
	}

	return( nullptr );
}

std::string_view XML_xerces_String::getLocalForm()
{
	if( !m_LocalForm && m_XMLForm )
	{
		m_LocalForm = convertToLocalForm( m_XMLForm);
	}

	return m_LocalForm ? std::string_view { m_LocalForm } : std::string_view{};
}

const XMLCh* XML_xerces_String::getXMLForm()
{
	if( !m_XMLForm)
	{
		if( !m_LocalForm)
		{
			return( nullptr );
		}

		m_XMLForm =	convertToXMLForm( m_LocalForm);
	}

	return( m_XMLForm);
}

int XML_xerces_String::compareNoCase( const char* LocalForm)
{
	// first try lazy generating the local string
	const char* self = getLocalForm().data();

	if (!self) // cannot be generated
		return LocalForm ? -1 : 0;

	if (!LocalForm) // non-null self vs null input
		return 1;

	return( strcasecmp( m_LocalForm, LocalForm));
}

int XML_xerces_String::compareNoCase( const XMLCh* XMLForm)
{
	if (const XMLCh* self = getXMLForm(); !self) // no current XML form and can't generate it
		return XMLForm ? -1:0;

	if (!XMLForm) // self is not nullptr and input XML form is nullptr
		return 1; // greater
	
	return( xercesc::XMLString::compareIString( m_XMLForm, XMLForm));
}

int XML_xerces_String::compare( const char* LocalForm)
{
	if( !getLocalForm().data() )
	{
		if( LocalForm)
		{
			return( -1);
		}
		else
		{
			return( 0);
		}
	}
	
	return( strcmp( m_LocalForm, LocalForm));
}

int XML_xerces_String::compare( const XMLCh* XMLForm)
{
	if( !m_XMLForm && !getXMLForm())
	{
		if( XMLForm)
		{
			return( -1);
		}
		else
		{
			return( 0);
		}
	}
	
	return( xercesc::XMLString::compareString( m_XMLForm, XMLForm));
}
