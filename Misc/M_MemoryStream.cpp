// M_MemoryStream.cpp
//
// Copyright (c) 2003-2010 e2e technologies ltd

#include "M_MemoryStream.hpp"
#include "../Misc/Memory.hpp"

#include <cstring>

const TY_Blob M::ConstantReadStream::m_placeholder;

#define FRAGMENT_SIZE 2048

namespace M
{

bool IReadStream::setReadPosition( T_int64 offset, int origin )
{
	auto endPos = getSize();
	bool isNegativeOffset = offset < 0;
	T_uint64 positiveOffset = static_cast<T_uint64>( isNegativeOffset ? -1 * offset : offset );
	if( origin == SEEK_SET )
	{
		if( isNegativeOffset )
		{
			return false;
		}
		m_setReadPosition( static_cast<T_uint64>( offset ) );
	}
	else if( origin == SEEK_CUR ) {
		if( isNegativeOffset )
		{
			if( positiveOffset > getReadPosition() )
			{
				return false;
			}
			m_setReadPosition( getReadPosition() - positiveOffset );
		}
		else
		{
			T_uint64 newPos = getReadPosition() + positiveOffset;
			if( newPos > endPos || getReadPosition() > newPos )
			{
				return false;
			}
			m_setReadPosition( newPos );
		}
	}
	else if( origin == SEEK_END ) {
		if( offset > 0 || positiveOffset > endPos ) {
			return false;
		}
		m_setReadPosition(  endPos - positiveOffset );
	}

	return true;
}

} // namespace M

M_MemoryStreamFragment::M_MemoryStreamFragment( char* Content, T_uint64 Size)
    : m_Data( Content)
    , m_UsedSize( Size)
    , m_FreeSize( 0)
{
}

M_MemoryStreamFragment::M_MemoryStreamFragment( const char* Content, T_uint64 Size)
{
	if( Size < FRAGMENT_SIZE)
	{
		m_Data = static_cast< char*>( M::Memory::allocate( FRAGMENT_SIZE * 2));
		memcpy( m_Data, Content, Size);
		m_UsedSize = Size;
		m_FreeSize = (FRAGMENT_SIZE * 2) - Size;
	}
	else
	{
		m_Data = static_cast< char*>( M::Memory::allocate( Size));
		memcpy( m_Data, Content, Size);
		m_UsedSize = Size;
		m_FreeSize = 0;
	}
}

M_MemoryStreamFragment::M_MemoryStreamFragment( M_MemoryStreamFragment&& src)
    : m_Data( src.m_Data)
    , m_UsedSize( src.m_UsedSize)
    , m_FreeSize( src.m_FreeSize)
{
	src.m_Data = nullptr;
}

M_MemoryStreamFragment::~M_MemoryStreamFragment()
{
	M::Memory::release( m_Data);
}

M_MemoryStreamFragment& M_MemoryStreamFragment::operator =( M_MemoryStreamFragment&& src)
{
	m_Data = src.m_Data;
	src.m_Data = nullptr;
	m_UsedSize = src.m_UsedSize;
	m_FreeSize = src.m_FreeSize;
	return *this;
}

void M_MemoryStreamFragment::append( const char* Content, T_uint64 Size)
{
	// you should only call append if there's (enough) space, 
	// I just do the checks to be on th safe side.

	if( !m_Data)
	{
		m_Data = static_cast< char*>( M::Memory::allocate( Size));
		memcpy( m_Data, Content, Size);
		m_UsedSize = Size;
		m_FreeSize = 0;
	}
	else if( m_FreeSize < Size)
	{
		m_Data = static_cast< char*>( M::Memory::reAllocate( m_Data, m_UsedSize + Size));
		memcpy( m_Data + m_UsedSize, Content, Size);
		m_UsedSize += Size;
		m_FreeSize = 0;
	}
	else
	{
		// the most efficent case
		memcpy( m_Data + m_UsedSize, Content, Size);
		m_UsedSize += Size;
		m_FreeSize -= Size;
	}
}

T_uint64 M_MemoryStreamFragment::getSize() const
{
	return m_UsedSize;
}

T_uint64 M_MemoryStreamFragment::getFreeSize() const
{
	return m_FreeSize;
}

T_uint64 M_MemoryStreamFragment::getContent( const char** Content) const
{
	*Content = m_Data;
	return m_UsedSize;
}

M_MemoryStream::M_MemoryStream()
              : m_ReadPosition( 0)
              , m_Size( 0)
{
}

M_MemoryStream::M_MemoryStream( M_MemoryStream&& src)
    : m_UnflushedContent( std::move( src.m_UnflushedContent))
    , m_Content( std::move( src.m_Content))
    , m_ReadPosition( src.m_ReadPosition)
    , m_Size( src.m_Size)
{
}

M_MemoryStream::M_MemoryStream( const char* String)
              : m_Content( String)
              , m_ReadPosition( 0)
              , m_Size( m_Content.getSize())
{
}

M_MemoryStream::M_MemoryStream( const TY_Blob* Content)
              : m_Content( Content)
              , m_ReadPosition( 0)
              , m_Size( m_Content.getSize())
{
}

M_MemoryStream::~M_MemoryStream()
{
}

M_MemoryStream& M_MemoryStream::operator =( M_MemoryStream&& src)
{
	m_UnflushedContent = std::move( src.m_UnflushedContent);
	m_Content = std::move( src.m_Content);
	m_ReadPosition = src.m_ReadPosition;
	m_Size = src.m_Size;
	return *this;
}

void M_MemoryStream::flush() const
{

	if( !m_UnflushedContent.size())
	{
		// No new data. Bye bye.
		return;
	}

	auto size = std::accumulate( m_UnflushedContent.begin(), m_UnflushedContent.end(), m_Content.getSize(),
	                             []( T_uint64 sum, const M_MemoryStreamFragment& f) { return sum + f.getSize(); });

	auto newContent = M::Memory::allocate<char>( size);

	const char* startContent;
	T_uint64 startSize;
	m_Content.getContent( &startContent, &startSize);

	if( startContent)
	{
		memcpy( newContent, startContent, startSize);
	}

	T_uint64 position = startSize;

	for( const auto& fragment : m_UnflushedContent)
	{
		const char* content;
		T_uint64 size = fragment.getContent( &content);

		if( content)
		{
			memcpy( newContent + position, content, size);
		}

		position += size;
	}

	m_UnflushedContent.clear();

	m_Content.setContent( newContent, size, true);
	m_Size = m_Content.getSize();
}

void M_MemoryStream::write( long Content)
{
	char buffer[21];
	int size = snprintf( buffer, 21, "%ld", Content);
	write( buffer, size);
}

void M_MemoryStream::write(const std::string &String)
{
	write( String.c_str());
}

void M_MemoryStream::write( const ST_String &String)
{
	write( String.c_str());
}

void M_MemoryStream::write( const char* String)
{
	if( !String)
	{
		return;
	}

	T_uint64 Size64 = strlen( String);
	write( String, Size64);
}

void M_MemoryStream::writeConsume( char* String)
{
	if( !String)
	{
		return;
	}

	T_uint64 Size64 = strlen( String);
	writeConsume( String, Size64);
}

void M_MemoryStream::writeConsume( M_MemoryStream& Stream)
{
	writeConsume( Stream.detachContent());
}

void M_MemoryStream::write( const TY_Blob* Content)
{
	write( Content->getContent(), Content->getSize());
}

void M_MemoryStream::writeConsume( TY_Blob* Content)
{
	T_uint64 Size;
	char* ContentChar;
	Content->detachContent( &ContentChar, &Size);
	writeConsume( ContentChar, Size);
	Content->release();
}

void M_MemoryStream::writeConsume( TY_Blob&& Content)
{
	T_uint64 Size;
	char* ContentChar;
	Content.detachContent( &ContentChar, &Size);
	writeConsume( ContentChar, Size);
}

void M_MemoryStream::write( const char* Content, T_uint64 Size)
{
	if( !Content || Size == 0)
	{
		return;
	}

	if( hasUnflushedContent() && m_lastUnflushed().getFreeSize() > Size)
	{
		m_lastUnflushed().append( Content, Size);
	}
	else
	{
		m_UnflushedContent.emplace_back( Content, Size);
	}
	
	m_Size += Size;
}

void M_MemoryStream::writeConsume( char* Content, T_uint64 Size)
{
	if( !Content)
	{
		return;
	}

	if( !Size)
	{
		// nothing to write... simply release buffer and we're done
		M::Memory::release( Content);
		return;
	}

	if( hasUnflushedContent() && m_lastUnflushed().getFreeSize() > Size)
	{
		m_lastUnflushed().append( Content, Size);
		M::Memory::release( Content);
	}
	else
	{
		m_UnflushedContent.emplace_back( Content, Size);
	}

	m_Size += Size;
}


void M_MemoryStream::terminate()
{
	const char Termite[] = "";
	write( Termite, 1L);
}

void M_MemoryStream::getContent( const char** Content, T_uint64* Size) const
{
	flush();

	m_Content.getContent( Content, Size);
}

void M_MemoryStream::detachContent( char** Content, T_uint64* Size)
{
	flush();

	m_ReadPosition = 0;
	m_Size = 0;

	m_Content.detachContent( Content, Size);
}

const TY_Blob* M_MemoryStream::getContent() const
{
	flush();

	return &m_Content;
}

TY_Blob M_MemoryStream::detachContent()
{
	flush();

	m_ReadPosition = 0;
	m_Size = 0;

	TY_Blob result = std::move( m_Content);

	m_Content.reset(); // like new! ;)

	return result;
}


T_uint64 M_MemoryStream::getSize() const
{
	return m_Size;
}

void M_MemoryStream::reset()
{
	m_Content.reset();
	m_UnflushedContent.clear();
	m_ReadPosition = 0;	
	m_Size = 0;
}

T_uint64 M_MemoryStream::read( char* Content, T_uint64 Size) const
{
	flush();

	T_uint64 ReadSize = m_Content.copyContent( Content, m_ReadPosition, Size);

	m_ReadPosition += ReadSize;

	return ReadSize;
}

T_uint64 M_MemoryStream::getReadPosition() const
{
	return m_ReadPosition;
}

void M_MemoryStream::m_setReadPosition( T_uint64 newPos )
{
	m_ReadPosition = newPos;
}

T_uint64 M::ConstantReadStream::read( char* content, T_uint64 size) const
{
	T_uint64 readSize = m_content->copyContent( content, m_readPosition, size);
	m_readPosition += readSize;
	return readSize;
}

void M::ConstantReadStream::getContent( const char** content, T_uint64* size) const
{
	if( m_content)
	{
		m_content->getContent( content, size);
	}
	else
	{
		content = nullptr;
		if( size) *size = 0;
	}
}

T_uint64 M::ConstantReadStream::getReadPosition() const
{
	return m_readPosition;
}

void M::ConstantReadStream::m_setReadPosition( T_uint64 newPos )
{
	m_readPosition = newPos;
}
