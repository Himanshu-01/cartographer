#pragma once

namespace halo2_memory_functions
{
	//
	unsigned int __cdecl _AllocatePtr(int default_meta_size, char alignment);
	//
	bool __cdecl _DeallocPtr(char *lpMem);
}