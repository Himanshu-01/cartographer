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

s_data_array* ui_list_data_new(char* name, uint32 count, uint32 size)
{
	return INVOKE(0x20D1FD, 0x0, ui_list_data_new, name, count, size);
}