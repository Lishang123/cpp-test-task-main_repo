// ST_String.cpp
//
// Copyright (c) 2002-2015 e2e technologies ltd

#include "ST_String.hpp"
#include "../Misc/Memory.hpp"

using namespace M::Memory;

#include <cstring>
#include <cstdarg>
#include <cassert>

using namespace M::String;

ST_String::ST_String()
         : m_String( nullptr)
{
}

ST_String::ST_String( const char* String, size_t Length)
    : m_String( duplicate(String, Length ) )
{
}

ST_String::ST_String( std::string_view sv )
    : m_String( duplicate( sv.data(), sv.size() ) )
{
}

ST_String::ST_String( const char* String)
    : m_String( duplicate( String, strlen(String) + 1) )
{
}

ST_String::ST_String( const ST_String& String)
    : m_String(duplicate( String.m_String , strlen(String.m_String) + 1 ))
{
}

ST_String::ST_String( ST_String&& String) noexcept
    : m_String( String.m_String)
{
	String.m_String = nullptr;
}

ST_String::~ST_String()
{
    release( m_String );
}

ST_String& ST_String::operator=( const char* String)
{
	set( String);

	return( *this);
}

ST_String& ST_String::operator=( std::string_view String)
{
	set( String);
	return *this;
}

ST_String& ST_String::operator=( const ST_String& String)
{
	if( this != &String)
	{
		assert( !m_String || m_String != String.m_String);
		set( String.m_String);
	}

	return( *this);
}

ST_String& ST_String::operator=( ST_String&& String)
 noexcept {
	if( this != &String)
	{
		consume( String.m_String);
		String.m_String = nullptr;
	}

	return *this;
}

const char* ST_String::c_str() const noexcept
{
	return m_String;
}

std::string_view ST_String::view() const noexcept
{
	return c_str() ? c_str() : std::string_view{};
}

bool ST_String::isEmpty() const
{
	return !m_String || !m_String[0];
}

size_t ST_String::length() const
{
	return m_String ? strlen( m_String) : 0;
}

void ST_String::reset()
{
    release(m_String);
	m_String = nullptr;
}

void ST_String::consume( char* String)
{
    release(m_String);
	m_String = String;
}

void ST_String::set( const char* String)
{
	set( String, -1);
}

void ST_String::set( std::string_view str )
{
	set( str.data(), str.size() );
}

void ST_String::set( const char* String, size_t Length)
{
	// Since memory allocation is generally costly, try to reuse current allocation.
	// Because we don't store size, we use current length as indicator if reuse is possible.
    if( !String )
    {
        reset();
        return;
    }
	if( String && m_String)
	{
		if( Length == -1 )
		{
			Length = strlen( String);
		}

		if( Length <= length())
		{
			strncpy( m_String, String, Length);
			m_String[Length] = '\0';
			return;
		}
	}
    if( Length == -1 )
    {
        Length = strlen( String );
    }
	consume( duplicate(String, Length));
}



