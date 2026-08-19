#include "stdafx.h"
#include "ring_buffer.h"


/* public code */

void c_ring_stream::attach(uint32 storage_size, void* storage_buffer)
{
	INVOKE_TYPE(0x3814EF, 0x0, void(__thiscall*)(c_ring_stream*, uint32, void*), this, storage_size, storage_buffer);
}
bool c_ring_stream::attached()
{
	return storage == nullptr;
}
void c_ring_stream::detach()
{
	storage = nullptr;
	ring_size = NULL;
}
uint32 c_ring_stream::add_block(uint32 block_data_size, void* block_data)
{
	return INVOKE_TYPE(0x381507, 0x0, uint32(__thiscall*)(c_ring_stream*, uint32, void*), this, block_data_size, block_data);
}
void c_ring_stream::remove_block(uint32 block_data_size, void* out_block_data)
{
	return INVOKE_TYPE(0x381546, 0x0, void(__thiscall*)(c_ring_stream*, uint32, void*), this, block_data_size, out_block_data);
}