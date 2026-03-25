// M_Memory.hpp
//
// Copyright (c) 2002-2014 e2e technologies ltd

#ifndef M_MEMORY_HPP
#define M_MEMORY_HPP


#include <memory>

namespace M::Memory
{
	 [[noreturn]] void outOfMemoryHandler();
	 void* allocate( size_t Size);
	 void* callocate( size_t Size, size_t Count);

	template<typename T>
	concept RawAllocatable = std::is_trivially_constructible_v<T> && std::is_trivially_destructible_v<T>;

	/**
	 * Allocates memory for some arbitrary number of elements.
	 *
	 * @param Count Number of elements we are allocating memory for
	 * @tparam ElementType  The type of element we are allocating memory for. This is also
	 *						the resulting pointer type.
	 *
	 * @example M_Memory::allocate<int>( 2) allocates continuous memory fragment for 2 integers.
	 * @returns A pointer to allocated memory
	 */
	template<RawAllocatable ElementType>
	ElementType* allocate( size_t Count)
	{
		//Size * sizeof(ElementType) can overflow size_t
		 if (Count > std::numeric_limits<size_t>::max() / sizeof(ElementType))
		 {
		 	outOfMemoryHandler();
		 }
		// allocate template should only be used for trivial types since the object is not constructed.
		// so if Foo* p is returned, it's dangerous because p->m_func()
		return static_cast<ElementType*>( allocate( Count * sizeof( ElementType)));
	}

	/**
	 * Allocates memory for some arbitrary number of elements and initialize memory with zero.
	 *
	 * @param Count Number of elements we are allocating memory for
	 * @tparam ElementType  The type of element we are allocating memory for. This is also
	 *						the resulting pointer type.
	 *
	 * @example M_Memory::allocate<int>( 2) allocates continuous memory fragment for 2 integers.
	 * @returns A pointer to allocated memory
	 */
	template<RawAllocatable ElementType>
	ElementType* callocate( size_t Count)
	{
		//Size * sizeof(ElementType) can overflow size_t
		if (Count > std::numeric_limits<size_t>::max() / sizeof(ElementType))
		{
			outOfMemoryHandler();
		}
		return static_cast<ElementType*>( callocate( sizeof( ElementType), Count));
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
	unique_ptr<T> as_unique_ptr( T* ptr) { return unique_ptr<T>{ptr}; }

	template<class T>
	unique_ptr<T[]> as_unique_array_ptr( T* ptr) { return unique_ptr<T[]>{ptr}; }

	// /** Copy memory area.
	//   * @param Buffer The buffer to copy.
	//   * @param Size The size of the buffer.
	//   */
	//  void* duplicate( void* Buffer, size_t Size);

	/** Copy memory area.
	  * @param Buffer The buffer to copy. Can have zero bytes which will all be copied up to the Size.
	  * @param Size The size of the buffer.
	  */
	 char* duplicate( const char* Buffer, size_t Size);

	 /**
	  * create an empty C-string with size length, allocating (length + 1) byte.
	  * @param length
	  * @return a pointer to the C-string
	  */
	 char* create( size_t length );

     /**
      * This handler only handles new failures.
      * better: installNewHandler()
      */
     void installOutOfMemoryHandler();

	template<class T>
	unique_ptr<T> allocateUnique()
	{
		return as_unique_ptr(static_cast<T*>(allocate(sizeof(T))));
	}

	template<class T>
	unique_ptr<T[]> allocateUniqueArray(size_t count)
	{
		return as_unique_array_ptr(static_cast<T*>(allocate(sizeof(T) * count)));
	}

	template<class T>
	unique_ptr<T[]> callocateUniqueArray(size_t count)
	{
		return as_unique_array_ptr(static_cast<T*>(callocate(sizeof(T), count)));
	}

	template<class T>
	unique_ptr<T[]> duplicateUniqueArray(const T* src, size_t count)
	{
		return as_unique_array_ptr(static_cast<T*>(duplicate(src, sizeof(T) * count)));
	}

	inline unique_ptr<char[]> duplicateUniqueString(const char* buffer, size_t size)
	{
		return as_unique_array_ptr(duplicate(buffer, size));
	}

	template<class T>
	unique_ptr<T[]> reAllocateUniqueArray(unique_ptr<T[]>&& ptr, size_t newCount)
	{
		T* raw = ptr.release();
		raw = static_cast<T*>(reAllocate(raw, sizeof(T) * newCount));
		return as_unique_array_ptr(raw);
	}

	inline unique_ptr<char[]> createUnique(size_t length)
	{
		auto result = allocateUniqueArray<char>(length + 1);
		result[0] = '\0';
		return result;
	}
} // namespace M::Memory


#endif
