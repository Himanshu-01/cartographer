#pragma once

struct s_game_state_header;
struct s_simulation_debug_chunk;

bool debug_gamestate_read_header(s_game_state_header* out_header);
bool debug_gamestate_read_header_runtime(s_game_state_header* out_header);

bool debug_gamestate_write_compressed_gamestate_to_buffer(uint8* temporary_buffer, uint32 temporary_buffer_size, uint32* compressed_size_out);
bool debug_gamestate_read_compressed_gamestate_from_buffer(uint8* temporary_buffer, uint32 temporary_buffer_size, uint32* compressed_size_out);

void debug_gamestate_read_from_chunk(s_simulation_debug_chunk* chunk);
void debug_gamestate_compare_headers(s_game_state_header* first,s_game_state_header* second);
void debug_gamestate_compare_header_with_runtime(s_game_state_header* saved);
void debug_gamestate_record_current_state();
void debug_gamestate_memory_initialize();
void debug_gamestate_memory_clear();
void debug_gamestate_apply_saved_state();
