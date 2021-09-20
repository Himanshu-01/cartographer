#include"h2_memory_funcs.h"
#include"..\H2MOD.h"

namespace halo2_memory_functions
{
	unsigned int __cdecl _AllocatePtr(int default_meta_size, char alignment)
	{
		typedef unsigned int(_cdecl *h2_Allocate_memory)(int size, char arg_4);
		auto pAllocate_memory = h2mod->GetAddress<h2_Allocate_memory>(0x37E69);//TODO
		return pAllocate_memory(default_meta_size, alignment);
	}
	bool __cdecl _DeallocPtr(char *lpMem)
	{
		typedef bool(_cdecl h2_dealloc_ptr)(char *lpMem);
		auto ph2_dealloc_ptr = h2mod->GetAddress<h2_dealloc_ptr*>(0x37EC3);//TODO
		return ph2_dealloc_ptr(lpMem);
	}
}