// Copyright (c) Scheer PAS Schweiz AG

#ifndef TY_BASE_HPP
#define TY_BASE_HPP

#include <cstddef>

class TY_Base
{

	public:
		static void* operator new( size_t Size);
		static void operator delete( void* Old);	
		

};

#endif
