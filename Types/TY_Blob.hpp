// TY_Blob.hpp
//
// Copyright (c) 2003-2015 e2e technologies ltd

#ifndef TY_BLOB_HPP
#define TY_BLOB_HPP

#include "../Types/TY_Base.hpp"
#include <utility>
#include <memory>

namespace ptr
{

    template<class C>
    struct release_deleter
    {
        void operator()( C *cpx ) { cpx->release(); };
    };

} // namespace ptr

typedef long long T_int64;
typedef unsigned long long T_uint64;

class  TY_Blob final : public TY_Base
{
	private:
		T_uint64 m_size;
		void* m_content;

	public:
		using ptr = std::unique_ptr<TY_Blob, ptr::release_deleter<TY_Blob>>;

		TY_Blob();
		TY_Blob( const TY_Blob& content);
		explicit TY_Blob( const TY_Blob* content);
		TY_Blob( TY_Blob&& content);
		TY_Blob( const char* content);
		TY_Blob( const void* content, T_uint64 size);
		TY_Blob( char* content, bool consume);
		TY_Blob( void* content, T_uint64 size, bool consume);
		~TY_Blob();

		TY_Blob& operator=( const TY_Blob& src);
		TY_Blob& operator=( TY_Blob&& src);

		// Create calls for dynamic allocation.
		template<class... Types>
		static TY_Blob* create( Types&&... args)
		{
			return new TY_Blob( std::forward<Types>( args)...);
		}

		/** Release the object as it's not needed anymore. */
		void release();

		/** Append content of another blob */
		void append( const TY_Blob* content);
		void append( const char* content, T_uint64 size);

		/** Get content and size.
		  * @param Content Store pointer to content here. May be NULL.
		  * @param Size Store pointer to size of content here. May be NULL.
		  */
		void getContent( const char** content, T_uint64* size) const;
		/** Get ownership of internal buffer. The blob will be empty but still usable afterwards.
		  * Make sure to remove the buffer with M_Memory::release().
		  * @param Content Store pointer to content here. May be NULL.
		  * @param Size Store pointer to size of content here. May be NULL.
		  */
		void detachContent( char** content, T_uint64* size);
		/** Get content by copying.
		  * @param Buffer Copy content here.
		  * @param Offset Start here to copy.
		  * @param Size Copy this number of bytes.
		  */
		T_uint64 copyContent( char* buffer, T_uint64 offset, T_uint64 size) const;
		/** Set new content. This makes a copy of the new content.
		  * @param Content New content.
		  */
		void setContent( const char* content);
		/** Set new content. This makes a copy of the new content.
		  * @param Content New content.
		  */
		void setContent( const TY_Blob* content);
		/** Set new content and size. This makes a copy of the new content. The terminating 0 will
		  * not be copied.
		  * @param Content New content.
		  * @param Size Size of new content.
		  */
		void setContent( const void* content, T_uint64 size);
		void setContent( const char* content, T_uint64 size);
		/** Set new content and size. This makes a copy of the new content if Consume is false.
		  * Otherwise the ownership changes to this object. In that case, Content has to be
		  * allocated via M::Memory.
		  * @param Content New content.
		  * @param Size Size of new content.
		  * @param Consume Signals whether a copy should be made.
		  */
		void setContent( void* content, T_uint64 size, bool consume);

		/** Get pointer to content, be aware that this buffer may not be 0-terminated! */
		const char* getContent() const;
		/** Set content size.
		  * @param Size New content size.
		  */
		void setSize( T_uint64 size, char padding = ' ');
		/** Get content size. */
		T_uint64 getSize() const;
		bool isEmpty() const { return getSize() == 0; }

		/** Reset content. */
		void reset();
};

#endif
