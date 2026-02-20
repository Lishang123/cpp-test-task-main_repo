// TY_Blob.cpp
//
// Copyright (c) 2003-2015 e2e technologies ltd

#include "TY_Blob.hpp"

#include "../Misc/Memory.hpp"

#include <ctype.h>
#include <string.h>
#include <cassert>

using namespace M::Memory;

TY_Blob::TY_Blob()
       : m_size( 0)
       , m_content( nullptr)
{
}

TY_Blob::TY_Blob( const TY_Blob& content)
	: TY_Blob()
{
	setContent( &content);
}

TY_Blob::TY_Blob( const TY_Blob* content)
    : TY_Blob()
{
	setContent( content);
}

TY_Blob::TY_Blob( TY_Blob&& content)
	: m_size( content.m_size)
    , m_content( content.m_content)
{
	content.m_content = nullptr;
}

TY_Blob::TY_Blob( const char* content)
    : TY_Blob()
{
	setContent( content);
}

TY_Blob::TY_Blob( const void* content, T_uint64 size)
       : TY_Blob()
{
	setContent( content, size);
}

TY_Blob::TY_Blob( char* content, bool consume)
    : TY_Blob()
{
	if( content)
	{
		setContent( content, strlen( content), consume);
	}
}

TY_Blob::TY_Blob( void* content, T_uint64 size, bool consume)
       : TY_Blob()
{
	setContent( static_cast<char*>( content), size, consume);
}

TY_Blob::~TY_Blob()
{
	reset();
}

TY_Blob& TY_Blob::operator=( const TY_Blob& src)
{
	if( this != &src)
	{
		setContent( &src);
	}
	return *this;
}

TY_Blob& TY_Blob::operator=( TY_Blob&& src)
{
	if( this != &src)
	{
		reset();
		m_size = src.m_size;
		m_content = src.m_content;
		src.m_content = nullptr;
	}
	return *this;
}

void TY_Blob::release()
{
	delete this;
}


void TY_Blob::append( const TY_Blob* content)
{
	if( content)
	{
		append( content->getContent(), content->getSize());
	}
}

void TY_Blob::append( const char* content, T_uint64 size)
{
	if( !content || size == 0)
	{
		// nothing to append
		return;
	}

	// resize
	m_content = reAllocate( m_content, m_size + size);

	if( !m_content)
	{
		// out of memory!?
		m_size = 0;
		return;
	}

	// copy content
	memcpy( static_cast< char*>( m_content) + m_size, content, size);
	m_size += size;
}

void TY_Blob::getContent( const char** content, T_uint64* size) const
{
	if( content)
	{
		*content = static_cast< char*>( m_content);
	}

	if( size)
	{
		*size = m_size;
	}
}

void TY_Blob::detachContent( char** content, T_uint64* size)
{
	if( content)
	{
		*content = static_cast< char*>( m_content);
		m_content = nullptr;
	}

	if( size)
	{
		*size = m_size;
	}

	reset();
}

T_uint64 TY_Blob::copyContent( char* buffer, T_uint64 offset, T_uint64 size) const
{
	if( !m_content || !buffer)
	{
		return 0;
	}

	T_uint64 sizeLeft = m_size - offset;

	if( sizeLeft <= 0)
	{
		return 0;
	}

	if( sizeLeft < size)
	{
		size = sizeLeft;
	}

	memcpy( buffer, static_cast< char*>( m_content) + offset, size);

	return size;
}

void TY_Blob::setContent( const char* content)
{
	if( !content)
	{
		reset();
	}
	else
	{
		setContent( content, strlen( content));
	}
}

void TY_Blob::setContent( const TY_Blob* content)
{
	if( !content)
	{
		reset();
	}
	else
	{
		setContent( content->getContent(), content->getSize());
	}
}

void TY_Blob::setContent( const char* content, T_uint64 size)
{
	if( !content)
	{
		reset();
	}
	else
	{
		setContent( static_cast< const void*>( content), size);
	}
}

void TY_Blob::setContent( const void* content, T_uint64 size)
{
	reset();
	m_content = duplicate( content, size);

	if( m_content)
	{
		m_size = size;
	}
}

void TY_Blob::setContent( void* content, T_uint64 size, bool consume)
{
	reset();

	m_size = size;

	if( consume)
	{
		m_content = content;
		assert( (m_content || m_size == 0) && "Content is null but size > 0");
	}
	else
	{
		m_content = duplicate( content, size);

		if( !m_content)
		{
			m_size = 0;
		}
	}
}

const char* TY_Blob::getContent() const
{
	return static_cast< char*>( m_content);
}

void TY_Blob::setSize( T_uint64 size, char padding)
{
	if( size > m_size)
	{
		// Resize.
		m_content = reAllocate( m_content, size);

		memset( static_cast< char*>( m_content) + m_size, padding, size - m_size);
	}

	m_size = size;
}

T_uint64 TY_Blob::getSize() const
{
	return m_size;
}

void TY_Blob::reset()
{
	M::Memory::release( m_content);
	m_content = nullptr;
	m_size = 0;
}
