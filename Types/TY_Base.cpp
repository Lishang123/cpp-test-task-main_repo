// Copyright (c) Scheer PAS Schweiz AG

#include "TY_Base.hpp"


#include "../Misc/Memory.hpp"

void* TY_Base::operator new( size_t Size)
{
	return M::Memory::allocate( Size );
}

void TY_Base::operator delete( void* Old)
{
	M::Memory::release( Old);
}

