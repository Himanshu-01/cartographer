#pragma once
#include "memory/data.h"

void* ui_pool_allocate_space(uint32 pool_size, int a2);
void ui_pool_dellocate(void* object);

s_data_array* ui_list_data_new(char* name, uint32 count, uint32 size);