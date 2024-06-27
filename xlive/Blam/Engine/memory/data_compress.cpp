#include "stdafx.h"
#include "data_compress.h"

bool __cdecl runtime_data_compress(uint8* in_buffer, uint32 in_buffer_size, uint8* compressed_buffer_out, uint32* compressed_size_out, uint32 a5, int32 level, uint32 scratch_size, uint8* scratch_buffer)
{
	return INVOKE(0x381ADD, 0x0, runtime_data_compress, in_buffer, in_buffer_size, compressed_buffer_out, compressed_size_out, a5, level, scratch_size, scratch_buffer);
}

bool __cdecl runtime_data_decompress(uint8* compressed_buffer_in, uint32 compressed_size_in, uint8* dest_buffer, uint32* decompressed_size_out, uint32 a5, uint32 scratch_size, uint8* scratch_buffer)
{
	return INVOKE(0x381BA4, 0x0, runtime_data_decompress, compressed_buffer_in, compressed_size_in, dest_buffer, decompressed_size_out, a5, scratch_size, scratch_buffer);
}