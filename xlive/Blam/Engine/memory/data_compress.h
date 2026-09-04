#pragma once


bool __cdecl runtime_data_compress(uint8* in_buffer, uint32 in_buffer_size, uint8* compressed_buffer_out, uint32* compressed_size_out, uint32 a5, int32 level, uint32 scratch_size, uint8* scratch_buffer);
bool __cdecl runtime_data_decompress(uint8* compressed_buffer_in, uint32 compressed_size_in, uint8* dest_buffer, uint32* decompressed_size_out, uint32 a5, uint32 scratch_size, uint8* scratch_buffer);
