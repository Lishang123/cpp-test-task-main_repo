// Copyright (c) Scheer PAS Schweiz AG

#ifndef TY_BASE_HPP
#define TY_BASE_HPP

#include <cstddef>

class TY_Base
{

	public:
		void* operator new( size_t Size);
		void operator delete( void* Old) noexcept;

};

#endif
