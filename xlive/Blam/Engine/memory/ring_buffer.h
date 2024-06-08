#pragma once

class _ring_buffer_base
{
protected:
	uint32 ring_size;
	uint8* storage;
	uint32 element_count;
	uint32 data_size;
};
ASSERT_STRUCT_SIZE(_ring_buffer_base, 16);



class c_ring_buffer : _ring_buffer_base
{
public:
	bool empty();
	bool full();

};
ASSERT_STRUCT_SIZE(c_ring_buffer, 16);


// in h2v and h2x c_ring_stream && c_ring_buffer have same structure
// this changed in later games and includes a signature field
class c_ring_stream : _ring_buffer_base
{
public:
	bool attached();
	void attach(uint32 storage_size, void* storage_buffer);
	void detach();
	uint32 add_block(uint32 block_data_size, void* block_data);
	void remove_block(uint32 block_data_size, void* out_block_data);
};
ASSERT_STRUCT_SIZE(c_ring_stream, 16);
