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

static constexpr size_t npos = static_cast<size_t>(-1);

ST_String::ST_String()
         : m_String( nullptr)
{
}

ST_String::ST_String( const char* s, size_t len)
    : m_String( duplicate(s, len ) )
{
}

ST_String::ST_String( std::string_view sv )
    : m_String( duplicate( sv.data(), sv.size() ) )
{
}

ST_String::ST_String( const char* s)
    : m_String( s ? duplicate( s, strlen(s) + 1) : nullptr )
{
}

ST_String::ST_String(const ST_String& other)
	: m_String(other.m_String
		? duplicate(other.m_String, std::strlen(other.m_String) + 1)
		: nullptr)
{
}

ST_String::ST_String( ST_String&& other) noexcept
    : m_String( other.m_String)
{
	other.m_String = nullptr;
}

ST_String::~ST_String()
{
    release( m_String );
}

ST_String& ST_String::operator=( const char* s)
{
	set( s);

	return( *this);
}

ST_String& ST_String::operator=( std::string_view sv)
{
	set( sv);
	return *this;
}

ST_String& ST_String::operator=( const ST_String& rhs)
{
	if( this != &rhs)
	{
		assert( !m_String || m_String != rhs.m_String);
		set( rhs.m_String);
	}

	return( *this);
}

ST_String& ST_String::operator=( ST_String&& rhs)
 noexcept {
	if( this != &rhs)
	{
		consume( rhs.m_String);
		rhs.m_String = nullptr;
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

void ST_String::consume( char* s)
{
    release(m_String);
	m_String = s;
}

void ST_String::set( const char* s)
{
	set( s, npos);
}

void ST_String::set( std::string_view str )
{
	set( str.data(), str.size() );
}

void ST_String::set( const char* s, size_t len)
{
	// Since memory allocation is generally costly, try to reuse current allocation.
	// Because we don't store size, we use current length as indicator if reuse is possible.
    if( !s )
    {
        reset();
        return;
    }
	if( s && m_String)
	{
		if( len == npos )
		{
			len = strlen( s);
		}

		if( len <= length())
		{
			std::memcpy( m_String, s, len);
			m_String[len] = '\0';
			return;
		}
	}
    if( len == npos )
    {
        len = strlen( s );
    }
	consume( duplicate(s, len));
}



