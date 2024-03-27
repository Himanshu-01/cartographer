#include "stdafx.h"


#include "user_interface_memory.h"

void* ui_pool_allocate_space(uint32 pool_size, int a2)
{
	return INVOKE_TYPE(0x20D2D8, 0x0, void* (__cdecl*)(uint32, int), pool_size, a2);
}

void ui_pool_dellocate(void* object)
{
	return INVOKE_TYPE(0x20D2EA, 0x0, void(__cdecl*)(void*), object);
}
