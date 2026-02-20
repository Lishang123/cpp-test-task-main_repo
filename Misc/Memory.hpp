// M_Memory.hpp
//
// Copyright (c) 2002-2014 e2e technologies ltd

#ifndef M_MEMORY_HPP
#define M_MEMORY_HPP


#include <stddef.h>
#include <memory>

namespace M
{
namespace Memory
{
	 void outOfMemoryHandler();
	 void* allocate( size_t Size);
	 void* callocate( size_t Size, size_t Count);

	/**
	 * Allocates memory for some arbitrary number of elements.
	 *
	 * @param Size Number of elements we are allocating memory for
	 * @tparam ElementType  The type of element we are allocating memory for. This is also
	 *						the resulting pointer type.
	 *
	 * @example M_Memory::allocate<int>( 2) allocates continous memory fragment for 2 integers.
	 * @returns A pointer to allocated memory
	 */
	template<typename ElementType>
	ElementType* allocate( size_t Size)
	{
		return static_cast<ElementType*>( allocate( Size * sizeof( ElementType)));
	}

	/**
	 * Allocates memory for some arbitrary number of elements and initialize memory with zero.
	 *
	 * @param Size Number of elements we are allocating memory for
	 * @tparam ElementType  The type of element we are allocating memory for. This is also
	 *						the resulting pointer type.
	 *
	 * @example M_Memory::allocate<int>( 2) allocates continous memory fragment for 2 integers.
	 * @returns A pointer to allocated memory
	 */
	template<typename ElementType>
	ElementType* callocate( size_t Size)
	{
		return static_cast<ElementType*>( callocate( Size, sizeof( ElementType)));
	}

	 void* reAllocate( void* OldBuffer, size_t Size);
	 void* duplicate( const void* OldBuffer, size_t Size);
	 void release( void* OldBuffer);

	struct Releaser
	{
	    void operator()( void *p) const
	    {
	        release( p);
	    }
	};

	template<class T>
	using unique_ptr = std::unique_ptr<T, Releaser>;

	template<class T>
	unique_ptr<T> as_unique_ptr( T* ptr) { return unique_ptr<T>( ptr, Releaser()); }

	template<class T>
	unique_ptr<T[]> as_unique_array_ptr( T* ptr) { return unique_ptr<T[]>( ptr, Releaser()); }

	/** Copy memory area.
	  * @param Buffer The buffer to copy.
	  * @param Size The size of the buffer.
	  */
	 void* duplicate( void* Buffer, size_t Size);

	/** Copy memory area.
	  * @param Buffer The buffer to copy. Can have zero bytes which will all be copied up to the Size.
	  * @param Size The size of the buffer.
	  */
	 char* duplicate( const char* Buffer, size_t Size);

     char* create( size_t length );

	 void installOutOfMemoryHandler();

} // namespace Memory
} // namespace M

#endif
