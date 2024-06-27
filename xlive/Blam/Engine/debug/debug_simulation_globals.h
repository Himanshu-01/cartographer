#pragma once

#include "debug_update.h"
#include "cseries/cseries_strings.h"
#include "tag_files/files_windows.h"

enum e_simulation_debug_chunk_type
{
	_debug_chunk_gamestate,
	_debug_chunk_update,
	_debug_chunk_random, //maybe someday
};



struct s_simulation_debug_globals
{
	bool initialized;
	bool recording_started;
	bool record_random;
	bool record_gamestate;
	bool record_update;

	bool replay_started;

	bool writing_file;
	bool reading_file;

	int32 current_recording_tick;
	int32 current_replaying_tick;
	int32 target_replaying_tick;

	uint8* gamestate_write_buffer;
	c_debug_update_queue update_queue;

	c_static_string260 save_location;
	s_file_reference save_file;
};



struct s_simulation_debug_chunk
{
	//	type
	//	file_offset
	//	block_size (for memcpy)

	e_simulation_debug_chunk_type chunk_type;
	uint32 file_offset;
	uint32 chunk_size;
};

struct s_simulation_debug_file_header
{
	//	signature
	//	dll_build
	//	total_size
	//	total_block_size
	//	no of ticks
	//	start_tick
	//	no of game_saves
	//	no of game_updates
	//	game_options/ game_state header
	//	no of debug_chunks
	//	eof_signature


	uint32 signature;
	uint32 header_size;
	c_static_wchar_string128 build;
	c_static_wchar_string32 build_time;
	uint32 file_size;
	uint32 chunk_size;
	//uint32 ticks_count;
	//uint32 start_tick;
	uint32 game_saves_count;
	uint32 game_updates_count;

	
	//dont need this
	//game saves already contain this data
	//s_game_state_header game_state_header;

	uint32 debug_chunks_count;

	uint32 eof_signature;

};


struct s_simulation_debug_file
{
	//s_simulation_debug_file_header
	//	chunk_headers[debug_chunks_count]

	//	gamestate chunk 
	//  (and or)
	//	synchronous_update 
};

bool debug_simulation_active();
bool debug_simulation_is_recording();
bool debug_simulation_recording_allows_gamestate();
bool debug_simulation_recording_allows_random();
bool debug_simulation_recording_allows_update();
bool debug_simulation_is_replaying();
bool debug_simulation_replay_has_updates();
bool debug_simulation_read_file(static_string32* name);
bool debug_simulation_write_file(static_string32* name);


void debug_simulation_initialize();
void debug_simulation_clear();
void debug_simulation_dispose();
void debug_simulation_start_recording();
void debug_simulation_stop_recording();
void debug_simulation_stop_replay();
extern s_simulation_debug_globals g_simulation_debug_globals;

void debug_simulation_gamestate_write_test();
void debug_simulation_gamestate_read_test();