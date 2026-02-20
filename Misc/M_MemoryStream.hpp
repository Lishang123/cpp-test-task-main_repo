// M_MemoryStream.hpp
//
// Copyright (c) 2003-2012 e2e technologies ltd

#ifndef M_MEMORYSTREAM_HPP
#define M_MEMORYSTREAM_HPP

#include "../Types/TY_Blob.hpp"
#include "M_SystemMessage.hpp"
#include <string>
#include <deque>
#include <variant>


class ST_String;

class M_MemoryStreamFragment
{
	private:
		char*		m_Data;
		T_uint64	m_UsedSize;
		T_uint64	m_FreeSize;

	public:
		M_MemoryStreamFragment( char* Content, T_uint64 Size);
		M_MemoryStreamFragment( const char* Content, T_uint64 Size);
		M_MemoryStreamFragment( const M_MemoryStreamFragment& src) = delete;
		M_MemoryStreamFragment( M_MemoryStreamFragment&& src);
		~M_MemoryStreamFragment();

		M_MemoryStreamFragment& operator =( const M_MemoryStreamFragment& src) = delete;
		M_MemoryStreamFragment& operator =( M_MemoryStreamFragment&& src);

		void append( const char* Content, T_uint64 Size);

		T_uint64 getSize() const;
		T_uint64 getFreeSize() const;

		T_uint64 getContent( const char** Content) const;
};

namespace M
{

class  IReadStream
{
	public:
		virtual ~IReadStream() {}

		/** Read from buffer from current read position. Returns number of bytes read.
		  * @param Buffer Put stuff here.
		  * @param Size How much you want to read.
		  */
		virtual T_uint64 read( char* Content, T_uint64 Size) const = 0;

		/** Get access to internal content.
		  * @param Buffer Store pointer to buffer here.
		  * @param Size Store size of buffer here.
		  */
		virtual void getContent( const char** content, T_uint64* size = nullptr) const = 0;

		/** Get internal buffer. */
		virtual const TY_Blob* getContent() const = 0;

		/** Return size of stream. */
		virtual T_uint64 getSize() const = 0;

		/** Sets the read pointer.
		 * @param offset The new offset in the buffer. Counted from origin.
		 * @param origin The meaning is analogous to the origin parameter of fseek()
		 * @return Whether the pointer has been reset.
		 */
		bool setReadPosition( T_int64 offset, int origin = SEEK_SET );
		virtual T_uint64 getReadPosition() const = 0;

	private:
		virtual void m_setReadPosition( T_uint64 newPos ) = 0;

};

class  ConstantReadStream : public M::IReadStream
{
	public:
		ConstantReadStream() = delete;
		ConstantReadStream( const TY_Blob* buffer) : m_content( buffer ? buffer : &m_placeholder), m_readPosition( 0) {}

		T_uint64 read( char* Content, T_uint64 Size) const;
		void getContent( const char** content, T_uint64* size = nullptr) const;
		const TY_Blob* getContent() const { return m_content; }
		T_uint64 getSize() const { return m_content ? m_content->getSize() : 0; }

		T_uint64 getReadPosition() const;
	private:
		const TY_Blob* m_content;
		mutable T_uint64 m_readPosition;
		static const TY_Blob m_placeholder;
		void m_setReadPosition( T_uint64 newPos ) override;
};

}

class  M_MemoryStream : public M::IReadStream
{
	public:
		M_MemoryStream();
		M_MemoryStream( const M_MemoryStream& src) = delete;
		M_MemoryStream( M_MemoryStream&& src);
		M_MemoryStream( const char* String);
		M_MemoryStream( const TY_Blob* Buffer);
		virtual ~M_MemoryStream();

		M_MemoryStream& operator =( const M_MemoryStream& src) = delete;
		M_MemoryStream& operator =( M_MemoryStream&& src);

		/** Flush flushes the fresh and compactifies the internal data structures. */
		void flush() const;

		/** Write a null terminated string into the stream. The null will NOT be written into the
		  * stream, only the string content.
		  * @param String The string to add to the stream.
		  */
		void write( const char* String);
		/** Write a null terminated string into the stream. The null will NOT be written into the
		  * stream, only the string content. The string will not be copied and the stream takes
		  * over the ownership about it. The string HAS to be allocated via M_String.	
	  	  * @param String The string to add to the stream.
		  */
		void writeConsume( char* String);
		/** Write the content of another stream into this stream.
		  * @param Stream Stream to add to this stream. This stream will be reseted.
		  */
		void writeConsume( M_MemoryStream &Stream);
		/** Write the content of this Buffer into the stream.
		  * @param Buffer Buffer to add to this stream.
		  */
		void write( const TY_Blob* Content);
		void write( long Content);
		void write( const std::string& String);
		void write( const ST_String& String);
		void writeConsume( TY_Blob* Content);		
		void writeConsume( TY_Blob&& Content);

		/** Write the content into this stream.
		  * @param Content The content to add.
		  * @param Size The size of the content.
		  */
		void write( const char* Content, T_uint64 Size);
		/** Write the content into this stream and consume it
		  * @param Content The content to add.
		  * @param Size The size of the content.
		  */
		void writeConsume( char* Content, T_uint64 Size);

		/** Write a null terminated string into the stream at the given position. The null will NOT
		  * be written into the stream, only the string content. The offset is measered in bytes,
		  * not characters.
		  * @param String The content to add.
		  * @param Offset The offset of the content.
		  */

		/** Terminate the content with 0. This is necessary when M_MemoryStream is used to build
		  * strings. */
		void terminate();

		/** Read from buffer from current read position. Returns number of bytes read.
		  * @param Buffer Put stuff here.
		  * @param Size How much you want to read.
		  */
		T_uint64 read( char* Content, T_uint64 Size) const;

		/** Get access to internal content.
		  * @param Buffer Store pointer to buffer here.
		  * @param Size Store size of buffer here.
		  */
		void getContent( const char** Content, T_uint64* Size = NULL) const;

		/** Get ownership of internal content. The memory stream will be empty but still usable
		  * afterwards. Make sure to remove the content with M_Memory::release().
		  * @param Buffer Store pointer to buffer here.
		  * @param Size Store size of buffer here.
		  */
		void detachContent( char** Content, T_uint64* Size = NULL);
		/** Get internal buffer. */
		const TY_Blob* getContent() const;
		/** Get ownership of internal content. The memory stream will be empty but still usable
		  * afterwards. Make sure to remove the content on your own. */
		TY_Blob detachContent();

		/** Set size of stream.
		  * @param Size New stream size.
		  * @param Padding Value to pad with if size grows.
		  */
		/** Return size of stream. */
		T_uint64 getSize() const;

		/** Reset stream. */
		void reset();

        T_uint64 getReadPosition() const;

		bool hasUnflushedContent() { return m_UnflushedContent.size() > 0; }
private:
		typedef std::deque<M_MemoryStreamFragment> Unflusched_t;
		mutable Unflusched_t m_UnflushedContent;
		mutable TY_Blob m_Content;

		mutable T_uint64 m_ReadPosition;
		mutable T_uint64 m_Size;

		M_MemoryStreamFragment& m_lastUnflushed() { return m_UnflushedContent.back(); }
		void m_setReadPosition( T_uint64 newPos ) override;
};

#endif
