// M_Memory.cpp
//
// Copyright (c) 2002-2014 e2e technologies ltd

#include "Memory.hpp"

#include <iostream>
#include <malloc.h>
#include <memory.h>
#include <new>


namespace M
{
namespace Memory
{
void installOutOfMemoryHandler()
{
    std::set_new_handler( outOfMemoryHandler );
}

void outOfMemoryHandler()
{
    std::cerr<<"Out of memory"<<std::endl;
	exit( 2);
}

void* allocate( size_t Size)
{
	void* Buffer = NULL;

	if( !Size)
	{
		Size = 1; //malloc can return NULL if feeded with 0, so we avoid this false-positiv error
	}

	if( !( Buffer = malloc( Size)))
	{
		// allocation failed
		outOfMemoryHandler();
		return( NULL);
	}
	else
	{
		// allocation went fine
		return( Buffer);
	}
}

void* callocate( size_t Size, size_t Count)
{
	void* Buffer = calloc( Size, Count);
	if( !Buffer)
	{
		outOfMemoryHandler();
	}

	return Buffer;
}

void* reAllocate( void* OldBuffer, size_t Size)
{
	if( !OldBuffer)
	{
		return( allocate( Size));
	}

	void* Buffer = NULL;

	if( !Size)
	{
		Size = 1; //avoid false-positive error
	}

	if( !( Buffer = realloc( OldBuffer, Size)))
	{
		// realloc failed
		outOfMemoryHandler();
		return( NULL);
	}
	else
	{
		return( Buffer);
	}
}

void* duplicate( const void* OldBuffer, size_t Size)
{
	if( !OldBuffer)
	{
		return( NULL);
	}

	void* Result;

	if(( Result = allocate( Size)))
	{
		if( Size)
		{
			memcpy( Result, OldBuffer, Size);
		}
	}

	return( Result);
}

void release( void* OldBuffer)
{
	if( !OldBuffer)
	{
		return;
	}

	free( OldBuffer);
}

void* duplicate( void* Buffer, size_t Size)
{
	if( !Buffer)
	{
		return( NULL);
	}

	void* Result;

	if( !( Result = allocate( Size)))
	{
		return( NULL);
	}

	if( Size)
	{
		memcpy( Result, Buffer, Size);
	}

	return( Result);
}

char* duplicate( const char* Buffer, size_t Size)
{
	if( !Buffer)
	{
		return NULL;
	}

	char* Result = static_cast<char*>( allocate( Size + 1));

	if( !Result)
	{
		return NULL;
	}

	if( Size)
	{
		memcpy( Result, Buffer, Size);
	}
    Result[Size]= '\0';

	return Result;
}

char* create( size_t length )
{
    char* Result;

    if(( Result = static_cast< char*>( allocate( length + 1))))
    {
        *Result = 0;
    }

    return Result;
}
} // namespace Memory
} // namespace M


