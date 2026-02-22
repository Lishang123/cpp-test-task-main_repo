// M_Memory.cpp
//
// Copyright (c) 2002-2014 e2e technologies ltd

#include "Memory.hpp"

#include <iostream>
#ifdef __APPLE__
#include <cstdlib>
#else
#include <malloc.h>
#endif
#include <memory.h>
#include <new>


namespace M::Memory {
	void installOutOfMemoryHandler()
	{
		std::set_new_handler( outOfMemoryHandler );
	}

	[[noreturn]] void outOfMemoryHandler()
	{
		std::cerr<<"Out of memory\n";
		throw std::bad_alloc();
	}

	void* allocate( size_t size)
	{
		if(size == 0)
		{
			size = 1; //malloc can return nullptr if fed with 0, so we avoid this false positive error
		}

		if( void* Buffer = malloc( size))
		{
			return Buffer;
		}
		// allocation failed
		outOfMemoryHandler();
	}

	void* callocate( size_t Size, size_t Count)
	{
		if(void* Buffer = calloc( Count, Size))
			return Buffer;

		outOfMemoryHandler();
	}

	void* reAllocate( void* OldBuffer, size_t Size)
	{
		if( !OldBuffer)
			return allocate( Size);

		if( !Size)
			Size = 1; //avoid false-positive error

		if (void* Buffer = realloc( OldBuffer, Size))
			return Buffer;
		outOfMemoryHandler();
	}

	void release( void* OldBuffer)
	{
		// free does nothing for nullptr
		free( OldBuffer);
	}

	void* duplicate( const void* OldBuffer, size_t Size)
	{
		if( !OldBuffer) return nullptr;
		void* Result = allocate( Size);
		if (Size) memcpy( Result, OldBuffer, Size);
		return Result;
	}

	char* duplicate( const char* Buffer, size_t Size)
	{
		if( !Buffer) return nullptr;
		char* Result = static_cast<char*>( allocate( Size + 1));
		if( Size) memcpy( Result, Buffer, Size);
		Result[Size]= '\0';
		return Result;
	}

	char* create( size_t length )
	{
		char* Result = static_cast< char*>( allocate( length + 1));
		Result[0] = '\0';

		return Result;
	}
}
