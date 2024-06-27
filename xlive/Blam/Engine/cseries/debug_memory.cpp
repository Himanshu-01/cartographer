#include "stdafx.h"
#include "debug_memory.h"


uint8* __cdecl debug_malloc(uint32 size)
{
	return INVOKE(0x8F50F, 0x0, debug_malloc, size);
}

void __cdecl debug_free(uint8* buffer)
{
	INVOKE(0x8F524, 0x0, debug_free, buffer);
}
