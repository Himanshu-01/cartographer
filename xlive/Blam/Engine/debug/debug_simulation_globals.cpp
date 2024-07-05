#include "stdafx.h"
#include "debug_simulation_globals.h"
#include "debug_simulation_constants.h"
#include "debug_gamestate.h"
#include "debug_determinism.h"
#include "cseries/cseries_strings.h"
#include "cseries/debug_memory.h"
#include "game/game_time.h"
#include "main/main_game.h"
#include "saved_games/game_state.h"
#include "simulation/simulation.h"
#include "Networking/messages/network_messages_simulation_synchronous.h"

#include "version_git.h"
#include "Util/filesys.h"




s_simulation_debug_globals g_simulation_debug_globals;

//internal forward declaration
bool debug_simulation_write_file_internal(void);
bool debug_simulation_read_file_internal(void);
bool debug_simulation_verify_header_internal(s_simulation_debug_file_header* header);
bool debug_simulation_fetch_updates_internal(int32 remaining_updates, int32* updates_read_out);
void debug_simulation_generate_default_header_internal(s_simulation_debug_file_header* header);
void debug_simulation_create_folders_internal();
void debug_simulation_timestamp_internal(static_string32* timestamp);


//public code

bool debug_simulation_active()
{
	return g_simulation_debug_globals.initialized;
}

bool debug_simulation_is_recording()
{
	ASSERT(debug_simulation_active());
	return debug_simulation_active() && g_simulation_debug_globals.recording_started;
}

bool debug_simulation_recording_allows_gamestate()
{
	ASSERT(debug_simulation_active());
	return debug_simulation_active() && g_simulation_debug_globals.record_gamestate;
}

bool debug_simulation_recording_allows_random()
{
	ASSERT(debug_simulation_active());
	return debug_simulation_active() && g_simulation_debug_globals.record_random;
}

bool debug_simulation_recording_allows_update()
{
	ASSERT(debug_simulation_active());
	return debug_simulation_active() && g_simulation_debug_globals.record_update;
}

bool debug_simulation_is_replaying()
{
	ASSERT(debug_simulation_active());
	return debug_simulation_active() && g_simulation_debug_globals.replay_started;
}

bool debug_simulation_replay_has_updates()
{
	ASSERT(debug_simulation_active());
	return debug_simulation_active() && g_simulation_debug_globals.update_queue.queued_count() > 0;
}

bool debug_simulation_replay_has_gamesave()
{
	ASSERT(debug_simulation_active());
	return debug_simulation_active() && g_simulation_debug_globals.replay_has_gamestate;
}

bool debug_simulation_retrieve_updates()
{
	ASSERT(debug_simulation_active());
	ASSERT(debug_simulation_is_replaying());

	int32 updates_left_to_read = debug_simulation_replay_update_queue_length();
	int32 updates_read = 0;
	bool result= debug_simulation_fetch_updates_internal(updates_left_to_read, &updates_read);
	LOG_TRACE_SIM("{} - updates_left_to_read : {} , read out : {} , result = {} ", __FUNCTION__, updates_left_to_read, updates_read, result);
	return result;
}

int32 debug_simulation_replay_update_queue_length()
{
	ASSERT(debug_simulation_active());
	return g_simulation_debug_globals.update_queue.queued_count();
}

void debug_simulation_read_debug_file()
{
	bool result = false;
	game_time_set_paused(true);
	if (debug_simulation_active())
	{
		debug_simulation_stop_recording();
		debug_simulation_stop_replay();
		debug_update_queue_clear();
		g_simulation_debug_globals.reading_file = true;
		if (debug_simulation_read_file_internal())
		{
			// success
			result = true;
		}
		else
		{
			// error occured
			result = false;
		}
	}


	if (!result)
	{
		//  bad state
		//  not supported when writing
		LOG_ERROR_SIM("simulation:global:debug cannot read_debug_file in current state, active : {} , record : {} , replay {} , reading {} , writing {} ",
			debug_simulation_active(),
			debug_simulation_is_recording(),
			debug_simulation_is_replaying(),
			g_simulation_debug_globals.reading_file,
			g_simulation_debug_globals.writing_file);
	}
}

void debug_simulation_write_debug_file()
{
	bool result = false;
	game_time_set_paused(true);
	if (debug_simulation_active() && debug_simulation_is_recording())
	{
		debug_simulation_stop_recording();

		g_simulation_debug_globals.writing_file = true;
		if (debug_simulation_write_file_internal())
		{
			// success
			result = true;
		}
		else
		{
			// error occured
			result = false;
		}
	}


	if (!result)
	{
		//  bad state
		//  not supported when replaying or reading
		LOG_ERROR_SIM("simulation:global:debug cannot write_debug_file in current state, active : {} , record : {} , replay {} , reading {} , writing {} ",
			debug_simulation_active(),
			debug_simulation_is_recording(),
			debug_simulation_is_replaying(),
			g_simulation_debug_globals.reading_file,
			g_simulation_debug_globals.writing_file);

	}
}



void debug_simulation_initialize()
{
	LOG_DEBUG_SIM("simulation:global:debug initialized");

	g_simulation_debug_globals.initialized = true;
	debug_simulation_create_folders_internal();

	debug_gamestate_memory_initialize();

	debug_update_queue_initialize_for_load();
}

void debug_simulation_clear()
{
	if (g_simulation_debug_globals.initialized)
	{
		debug_simulation_stop_recording();
		debug_simulation_stop_replay();

		g_simulation_debug_globals.writing_file = false;
		g_simulation_debug_globals.reading_file = false;

		debug_gamestate_memory_clear();
		debug_update_queue_clear();
		debug_random_record_clear();
		// maybe check save_file is being used and close it ??

	}
}

void debug_simulation_dispose()
{
	if (g_simulation_debug_globals.initialized)
	{
		debug_simulation_clear();
		debug_update_queue_dispose();

		g_simulation_debug_globals.initialized = false;
	}
}

void debug_simulation_start_recording()
{
	LOG_INFO_SIM("simulation:global:debug starting simulation recording");
	g_simulation_debug_globals.recording_started = true;
	g_simulation_debug_globals.record_gamestate = true;
	g_simulation_debug_globals.record_random = true;
	g_simulation_debug_globals.record_update = true;
	g_simulation_debug_globals.recorded_gamestate = false;
}

void debug_simulation_stop_recording()
{
	LOG_DEBUG_SIM("simulation:global:debug stopping active recording");
	g_simulation_debug_globals.recording_started = false;
	g_simulation_debug_globals.record_gamestate = false;
	g_simulation_debug_globals.record_random = false;
	g_simulation_debug_globals.record_update = false;
	g_simulation_debug_globals.current_recording_tick = NONE;
}

void debug_simulation_stop_replay()
{
	LOG_DEBUG_SIM("simulation:global:debug stopping active replay");
	g_simulation_debug_globals.replay_started = false;
	g_simulation_debug_globals.current_replaying_tick = NONE;
	g_simulation_debug_globals.target_replaying_tick = NONE;
}

void debug_simulation_pause(bool pause)
{
	char* state = pause ? "pausing" :"un-pausing";
	LOG_DEBUG_SIM("simulation:global:debug {} active game ", state);
	game_time_set_paused(pause);
}

void debug_simulation_launch_replay()
{
	//  simulation_end();
	//	grab header
	//	main_game_change
	if (!debug_simulation_active() || debug_simulation_is_recording())
		return;

	//cache_file_map_clear_all_failures();
	debug_simulation_stop_replay();
	simulation_end();

	LOG_INFO_SIM("simulation:global:debug launching film playback : {}{}",
		g_simulation_debug_globals.save_file_name.get_string(),
		K_SIMULATION_DEBUG_SAVE_FILE_EXTENSION);

	s_game_state_header header;
	s_game_options* launch_options = nullptr;
	if(debug_gamestate_read_header(&header))
	{
		LOG_INFO_SIM("simulation:global:debug  on scenario : {} ",
			header.scenario_name.get_string());
			
		launch_options = &header.options;
	}
	else
	{
		LOG_WARNING_SIM("simulation:global:debug found no gamestate header, using film options..");
		LOG_INFO_SIM(L"simulation:global:debug on scenario : {} ",
			g_simulation_debug_globals.film_options.scenario_path.get_string());


		//g_simulation_debug_globals.film_options.players[0].player_valid = true;
		//g_simulation_debug_globals.film_options.players[0].player_left_game = false;

		launch_options = &g_simulation_debug_globals.film_options;	
	}

	g_simulation_debug_globals.replay_started = true;
	g_simulation_debug_globals.replay_applied_gamestate = false;

	//force synchronous_client to act as synchronous_server
	//doing this so i dont have to rewrite c_simulation_world::update()

	if (launch_options->simulation_type == _game_simulation_synchronous_client)
		launch_options->simulation_type = _game_simulation_synchronous_server;

	main_game_change(launch_options);

}

void debug_simulation_set_name(const char* name)
{
	if (!debug_simulation_active())
	{
		LOG_ERROR_SIM("{} - failed to set debug simulation file name!", __FUNCTION__);
	}

	g_simulation_debug_globals.save_file_name.set(name);
	LOG_DEBUG_SIM("debug simulation file name set to {} ", name);
}

void debug_simulation_notify_oos()
{
	if (!debug_simulation_active() || !debug_simulation_is_recording())
		return;

	LOG_CRITICAL_NETWORK("simulation:global:debug calling dump random seed");
	debug_random_dump_call_stack();


	static_string32 timestamp;
	debug_simulation_timestamp_internal(&timestamp);
	g_simulation_debug_globals.save_file_name.set(timestamp.get_string());

	const char* type = simulation_get_world()->is_authority() ? "host" : "client";
	g_simulation_debug_globals.save_file_name.append(type);
	g_simulation_debug_globals.save_file_name.append("_oos");
	debug_simulation_write_debug_file();
	debug_simulation_stop_recording();
}

void debug_simulation_gamestate_write_test()
{
	c_static_string260 save_file_path;
	save_file_path.set(g_simulation_debug_globals.save_directory.get_string());
	save_file_path.append("test");
	save_file_path.append(K_SIMULATION_DEBUG_SAVE_FILE_EXTENSION);


	LOG_ERROR_SIM("{} - starting to create debug simulation file! {}", __FUNCTION__, save_file_path.get_string());

	file_reference_create_from_path(&g_simulation_debug_globals.save_file, save_file_path.get_string(), false);
	e_file_open_error open_file_error_code = _file_open_error_unknown;
	bool create_file_success = file_create(&g_simulation_debug_globals.save_file);

	if (create_file_success)
	{
		if (!file_open(&g_simulation_debug_globals.save_file, _permission_write_bit, &open_file_error_code))
		{
			LOG_ERROR_SIM("{} - failed to open debug simulation file for write , error code: {}", __FUNCTION__, (uint32)open_file_error_code);
		}
	}
	else
	{
		LOG_ERROR_SIM("{} - failed to create debug simulation file!", __FUNCTION__);

	}


	if (open_file_error_code == _file_open_error_success)
	{
		uint32 write_offset = NULL;
		uint32 total_write_chunks = 1;
		uint32 chunk_headers_end = total_write_chunks * sizeof(s_simulation_debug_chunk) + sizeof(s_simulation_debug_file_header);

		s_simulation_debug_file_header debug_file_header;
		debug_simulation_generate_default_header_internal(&debug_file_header);

		if (total_write_chunks > K_SIMULATION_DEBUG_MAX_GAMESTATES + K_SIMULATION_DEBUG_MAX_UPDATES)
		{
			//warning
			LOG_WARNING_SIM("{} - total no of chunks exceed our expectations , count :{}", __FUNCTION__, total_write_chunks);
		}

		debug_file_header.debug_chunks_count = total_write_chunks;

		if (g_simulation_debug_globals.gamestate_write_buffer == nullptr)
		{
			LOG_TRACE_SIM("{} - using runtime gamestate as write_buffer", __FUNCTION__);
			g_simulation_debug_globals.gamestate_write_buffer = (uint8*)game_state_get_buffer_address(nullptr);
		}
		//
		// compress gamestate
		uint32 out_gamestate_chunk_size = NULL;
		uint8* gamestate_temporary_buffer = debug_malloc(K_SIMULATION_DEBUG_GAMESTATE_COMPRESSED_FILE_SIZE);
		if (!debug_gamestate_write_compressed_gamestate_to_buffer(gamestate_temporary_buffer, K_SIMULATION_DEBUG_GAMESTATE_COMPRESSED_FILE_SIZE, &out_gamestate_chunk_size))
		{
			LOG_ERROR_SIM("{} - failed to compress gamestate!", __FUNCTION__);
			//success = false;
		}

		if (g_simulation_debug_globals.gamestate_write_buffer == game_state_get_buffer_address(nullptr))
		{
			LOG_TRACE_SIM("{} - clearing write_buffer", __FUNCTION__);
			g_simulation_debug_globals.gamestate_write_buffer = nullptr;
		}
		debug_file_header.game_saves_count++;


		s_simulation_debug_chunk gamestate_chunk;
		gamestate_chunk.chunk_type = _debug_chunk_gamestate;
		gamestate_chunk.file_offset = chunk_headers_end;
		gamestate_chunk.chunk_size = out_gamestate_chunk_size;



		debug_file_header.chunk_size = out_gamestate_chunk_size + 0;
		debug_file_header.file_size = chunk_headers_end + out_gamestate_chunk_size + 0;


		if (!file_write(&g_simulation_debug_globals.save_file, sizeof(s_simulation_debug_file_header), &debug_file_header))
		{
			LOG_ERROR_SIM("{} - failed to write header to simulation debug file!", __FUNCTION__);
			//success = false;
		}
		write_offset += sizeof(s_simulation_debug_file_header);


		if (!file_write(&g_simulation_debug_globals.save_file, sizeof(s_simulation_debug_chunk), &gamestate_chunk))
		{
			LOG_ERROR_SIM("{} - failed to write gamestate debug chunk header to simulation debug file!", __FUNCTION__);
			//success = false;
		}
		write_offset += sizeof(s_simulation_debug_chunk);


		if (!file_write(&g_simulation_debug_globals.save_file, out_gamestate_chunk_size, gamestate_temporary_buffer))
		{
			LOG_ERROR_SIM("{} - failed to write compressed gamestate chunk to simulation debug file!", __FUNCTION__);
			//success = false;
		}
		write_offset += out_gamestate_chunk_size;
		if(gamestate_temporary_buffer)
			debug_free(gamestate_temporary_buffer);



		if (!file_set_eof(&g_simulation_debug_globals.save_file))
		{
			LOG_ERROR_SIM("{} - failed to set simulation debug file size!", __FUNCTION__);
			//success = false;
		}


		file_close(&g_simulation_debug_globals.save_file);
		

	}
}

void debug_simulation_gamestate_read_test()
{

	c_static_string260 save_file_path;
	save_file_path.set(g_simulation_debug_globals.save_directory.get_string());
	save_file_path.append("test");
	save_file_path.append(K_SIMULATION_DEBUG_SAVE_FILE_EXTENSION);


	LOG_ERROR_SIM("{} - start reading debug simulation file! {}", __FUNCTION__, save_file_path.get_string());

	file_reference_create_from_path(&g_simulation_debug_globals.save_file, save_file_path.get_string(), false);
	e_file_open_error open_file_error_code = _file_open_error_unknown;


	if (!file_open(&g_simulation_debug_globals.save_file, _permission_read_bit, &open_file_error_code))
	{
		LOG_ERROR_SIM("{} - failed to open debug simulation file for read , error code: {}", __FUNCTION__, (uint32)open_file_error_code);
	}	



	if (open_file_error_code == _file_open_error_success)
	{

		s_simulation_debug_file_header debug_file_header;

		// first we read the header
		if (!file_read(&g_simulation_debug_globals.save_file, sizeof(s_simulation_debug_file_header), true, &debug_file_header))
		{
			LOG_ERROR_SIM("{} - failed to read debug simulation file header!", __FUNCTION__);
			file_close(&g_simulation_debug_globals.save_file);
			return;
		}

		if (!debug_simulation_verify_header_internal(&debug_file_header))
		{
			LOG_ERROR_SIM("{} - failed to verify debug simulation file header!", __FUNCTION__);
			file_close(&g_simulation_debug_globals.save_file);
			return;
		}

		if (debug_file_header.debug_chunks_count > 0)
		{
			uint8 gamestate_chunk_count = NULL;
			uint32 read_offset = sizeof(s_simulation_debug_file_header);
			uint32 debug_chunk_headers_size = sizeof(s_simulation_debug_chunk) * debug_file_header.debug_chunks_count;
			s_simulation_debug_chunk* debug_chunks = (s_simulation_debug_chunk*)debug_malloc(debug_chunk_headers_size);
			s_simulation_debug_chunk* last_gamestate_chunk_header = nullptr;

			for (uint8 i = 0; i < debug_file_header.debug_chunks_count; i++)
			{
				s_simulation_debug_chunk* current_chunk = debug_chunks;
				if (file_read_from_position(&g_simulation_debug_globals.save_file, read_offset, sizeof(s_simulation_debug_chunk), true, current_chunk))
				{
					LOG_TRACE_SIM(" - - > reading debug chunk type : {} size : 0x{:08X} offset : 0x{:08X} ", current_chunk->chunk_type, current_chunk->chunk_size, current_chunk->file_offset);
					
					if(current_chunk->chunk_type==_debug_chunk_gamestate)
					{
						LOG_TRACE_SIM(" - - > found gamestate chunk size : 0x{:08X} offset : 0x{:08X}", current_chunk->chunk_size, current_chunk->file_offset);
						gamestate_chunk_count++;
						last_gamestate_chunk_header = current_chunk;
					}
				}
				else
				{
					LOG_TRACE_SIM(" - - > reading chunk failure at count {} ", i);
				}
				read_offset += sizeof(s_simulation_debug_chunk);
				current_chunk++;
			}

			if (gamestate_chunk_count > K_SIMULATION_DEBUG_MAX_GAMESTATES)
			{
				LOG_TRACE_SIM("{} - warning debug simulation file has too many gamestate chunks count :{} , expected : {}", __FUNCTION__, gamestate_chunk_count, K_SIMULATION_DEBUG_MAX_GAMESTATES);
			}

			if (gamestate_chunk_count > 0)
			{

				uint8* gamestate_compressed_buffer = debug_malloc(last_gamestate_chunk_header->chunk_size);
				if (file_read_from_position(&g_simulation_debug_globals.save_file, last_gamestate_chunk_header->file_offset, last_gamestate_chunk_header->chunk_size, true, gamestate_compressed_buffer))
				{
					LOG_TRACE_SIM("{} - successfully read gamestate data from save file ", __FUNCTION__);

					if (g_simulation_debug_globals.gamestate_write_buffer == nullptr)
					{
						uint32 gamestate_buffer_size = NULL;
						game_state_get_buffer_address(&gamestate_buffer_size);

						LOG_TRACE_SIM("{} - simulation:debug:gamestate initializing save memory for reading ", __FUNCTION__);
						g_simulation_debug_globals.gamestate_write_buffer = debug_malloc(gamestate_buffer_size);
					}

					uint32 out_decompressed_size = NULL;
					if (!debug_gamestate_read_compressed_gamestate_from_buffer(gamestate_compressed_buffer, last_gamestate_chunk_header->chunk_size, &out_decompressed_size))
					{
						LOG_ERROR_SIM("{} - failed to decompress gamestate!", __FUNCTION__);
						//success = false;
					}

					s_game_state_header saved_header;
					debug_gamestate_read_header(&saved_header);
					debug_gamestate_compare_header_with_runtime(&saved_header);

					if(g_simulation_debug_globals.gamestate_write_buffer)
					{
						LOG_TRACE_SIM("{} - clearing write_buffer 0x{:X} ", __FUNCTION__ , (uint32)g_simulation_debug_globals.gamestate_write_buffer);
						debug_free(g_simulation_debug_globals.gamestate_write_buffer);
						g_simulation_debug_globals.gamestate_write_buffer = nullptr;
					}
				}
				if(gamestate_compressed_buffer)
				{
					debug_free(gamestate_compressed_buffer);
					LOG_TRACE_SIM("{} - clearing compressed buffer ", __FUNCTION__);
				}
			}
			if(debug_chunks)
			{
				debug_free((uint8*)debug_chunks);
				LOG_TRACE_SIM("{} - clearing header chunks buffer ", __FUNCTION__);
			}
			
		}
		else
		{
			LOG_TRACE_SIM("{} - warning debug simulation file has no chunk entries!", __FUNCTION__);
		}

		file_close(&g_simulation_debug_globals.save_file);


	}


}






// internal function definitions



bool debug_simulation_write_file_internal(void)
{
	bool success = true;
	if (!g_simulation_debug_globals.writing_file)
		// cannot write without starting write
		return false;

	c_static_string260 save_file_path;
	save_file_path.set(g_simulation_debug_globals.save_directory.get_string());

	if (g_simulation_debug_globals.save_file_name.length() == NULL)
	{
		static_string32 timestamp;
		debug_simulation_timestamp_internal(&timestamp);
		g_simulation_debug_globals.save_file_name.set(timestamp.get_string());
	}

	save_file_path.append(g_simulation_debug_globals.save_file_name.get_string());	
	save_file_path.append(K_SIMULATION_DEBUG_SAVE_FILE_EXTENSION);

	LOG_INFO_SIM("{} - writing simulation debug file : {}", __FUNCTION__, save_file_path.get_string());
	
	file_reference_create_from_path(&g_simulation_debug_globals.save_file, save_file_path.get_string(), false);

	e_file_open_error open_file_error_code = _file_open_error_unknown;
	bool create_file_success = file_create(&g_simulation_debug_globals.save_file);

	if(create_file_success)
	{
		if (!file_open(&g_simulation_debug_globals.save_file, _permission_write_bit, &open_file_error_code))
		{
			LOG_ERROR_SIM("{} - failed to open debug simulation file for write , error code: {}", __FUNCTION__, (uint32)open_file_error_code);
			success = false;
		}
	}
	else
	{
		LOG_ERROR_SIM("{} - failed to create debug simulation file!", __FUNCTION__);
		success = false;
	}


	if (open_file_error_code == _file_open_error_success)
	{
		uint32 write_offset = NULL;
		uint32 total_write_chunks = (g_simulation_debug_globals.recorded_gamestate?1:0) + g_simulation_debug_globals.update_queue.queued_count();
		uint32 chunk_headers_end = total_write_chunks * sizeof(s_simulation_debug_chunk) + sizeof(s_simulation_debug_file_header);

		s_simulation_debug_file_header debug_file_header;
		debug_simulation_generate_default_header_internal(&debug_file_header);
		debug_file_header.debug_chunks_count = total_write_chunks;

		if (total_write_chunks > K_SIMULATION_DEBUG_MAX_GAMESTATES + K_SIMULATION_DEBUG_MAX_UPDATES)
		{
			//warning
			LOG_WARNING_SIM("{} - no of chunks exceed our expectations , chunks count :{}", __FUNCTION__, total_write_chunks);
		}



		//
		// compress gamestate and write
		//
		uint32 out_gamestate_chunk_size = NULL;
		uint8* gamestate_temporary_buffer = debug_malloc(K_SIMULATION_DEBUG_GAMESTATE_COMPRESSED_FILE_SIZE);
		
		if(g_simulation_debug_globals.recorded_gamestate)
		{
			if (debug_gamestate_write_compressed_gamestate_to_buffer(gamestate_temporary_buffer, K_SIMULATION_DEBUG_GAMESTATE_COMPRESSED_FILE_SIZE, &out_gamestate_chunk_size))
			{
				LOG_TRACE_SIM("{} - successfully written gamestate data to save file ", __FUNCTION__);
				debug_file_header.game_saves_count++;
			}
			else
			{
				LOG_ERROR_SIM("{} - failed to compress gamestate!", __FUNCTION__);
				success = false;
			}
		}


		if (success)
		{
			s_simulation_debug_chunk gamestate_chunk;
			gamestate_chunk.chunk_type = _debug_chunk_gamestate;
			gamestate_chunk.file_offset = chunk_headers_end;
			gamestate_chunk.chunk_size = out_gamestate_chunk_size;


			uint32 update_chunks_header_size = sizeof(s_simulation_debug_chunk) * g_simulation_debug_globals.update_queue.queued_count();
			s_simulation_debug_chunk* update_header_bucket = (s_simulation_debug_chunk*)debug_malloc(update_chunks_header_size);
			uint32 update_chunks_size = NULL;

			s_simulation_debug_chunk* current_chunk = update_header_bucket;
			for (const c_debug_update_node* update_node = g_simulation_debug_globals.update_queue.get_first_element();
				update_node != nullptr;
				update_node = g_simulation_debug_globals.update_queue.get_next_element(update_node)
				)
			{
				current_chunk->chunk_type = _debug_chunk_update;
				current_chunk->file_offset = chunk_headers_end + out_gamestate_chunk_size + update_chunks_size;
				current_chunk->chunk_size = update_node->data_size;
				update_chunks_size += update_node->data_size;

				current_chunk++;
				debug_file_header.game_updates_count++;
			}

			debug_file_header.chunk_size = out_gamestate_chunk_size + update_chunks_size;
			debug_file_header.file_size = chunk_headers_end + out_gamestate_chunk_size + update_chunks_size;


			if (success && file_write(&g_simulation_debug_globals.save_file, sizeof(s_simulation_debug_file_header), &debug_file_header))
			{
				write_offset += sizeof(s_simulation_debug_file_header);
			}
			else
			{
				LOG_ERROR_SIM("{} - failed to write header to simulation debug file!", __FUNCTION__);
				success = false;
			}

			if(g_simulation_debug_globals.recorded_gamestate)
			{
				if (success && file_write(&g_simulation_debug_globals.save_file, sizeof(s_simulation_debug_chunk), &gamestate_chunk))
				{
					write_offset += sizeof(s_simulation_debug_chunk);
				}
				else
				{
					LOG_ERROR_SIM("{} - failed to write gamestate debug chunk header to simulation debug file!", __FUNCTION__);
					success = false;
				}
			}



			if (success && file_write(&g_simulation_debug_globals.save_file, update_chunks_header_size, update_header_bucket))
			{
				write_offset += update_chunks_header_size;
			}
			else
			{
				LOG_ERROR_SIM("{} - failed to write update debug chunk headers to simulation debug file!", __FUNCTION__);
				success = false;
			}


			//check expected offset with actual offset
			ASSERT(write_offset == chunk_headers_end);

			if (g_simulation_debug_globals.recorded_gamestate)
			{
				if (success && file_write(&g_simulation_debug_globals.save_file, out_gamestate_chunk_size, gamestate_temporary_buffer))
				{
					write_offset += out_gamestate_chunk_size;
				}
				else
				{
					LOG_ERROR_SIM("{} - failed to write compressed gamestate chunk to simulation debug file!", __FUNCTION__);
					success = false;
				}
			}

			if(gamestate_temporary_buffer)
			{
				LOG_TRACE_SIM("{} - clearing gamestate_temporary_buffer ", __FUNCTION__);
				debug_free(gamestate_temporary_buffer);
			}

			//check expected offset with actual offset
			ASSERT(write_offset == chunk_headers_end + out_gamestate_chunk_size);


			current_chunk = update_header_bucket;
			for (const c_debug_update_node* update_node = g_simulation_debug_globals.update_queue.get_first_element();
				update_node != nullptr;
				update_node = g_simulation_debug_globals.update_queue.get_next_element(update_node)
				)
			{
				ASSERT(write_offset == current_chunk->file_offset);

				if (success && file_write(&g_simulation_debug_globals.save_file, update_node->data_size, update_node->data))
				{
					write_offset += update_node->data_size;
				}
				else
				{
					LOG_ERROR_SIM("{} - failed to write update debug chunk to simulation debug file!", __FUNCTION__);
					success = false;
				}
				current_chunk++;
			}

			//check expected offset with actual offset
			ASSERT(write_offset == chunk_headers_end + out_gamestate_chunk_size + update_chunks_size);

			if (update_header_bucket)
			{
				LOG_TRACE_SIM("{} - clearing update_header chunks buffer ", __FUNCTION__);
				debug_free((uint8*)update_header_bucket);
			}


		}
		if (!file_set_eof(&g_simulation_debug_globals.save_file))
		{
			LOG_ERROR_SIM("{} - failed to set simulation debug file size!", __FUNCTION__);
			success = false;
		}
	}


	//  allocate gamestate_header
	//  debug_gamestate_get_header();
	//  write_header to file_header
	//  dispose
	//  file_write s_simulation_debug_file_header
	//  write_offset+=sizeof(s_simulation_debug_file_header)
	//
	// 
	//  gamestate_chunk_size=0
	//  chunk_headers_end = sizeof(debug_chunk_gamestate)*(MAX_GAMESAVES+queed_count)
	//  allocate gamestate_chunk_compressed (heap)
	//  debug_gamestate_write_gamestate_to_buffer(out_size);
	//  debug_chunk_gamestate (stack)
	//  debug_chunk_gamestate.chunk_size = out_size
	//  debug_chunk_gamestate.file_offset = chunk_headers_end
	//  gamestate_chunk_size=out_size;
	//  
	// 
	//  update_queue_header_bucket (heap)
	//  update_chunks_size = 0
	//  loop queue
	//  save_file_offset,size
	//  update_queue_header_bucket[i].file_offset = chunk_headers_end + gamestate_chunk_size +update_chunks_size
	//  update_queue_header_bucket[i].chunk_size  
	//  update_chunks_size+=data_size;
	//  update_count
	// 
	//
	//  file_write debug_chunk_gamestate
	//  write_offset+=sizeof(debug_chunk_gamestate)
	//  loop update_queue_header_bucket
	//  file_write update_queue_header_bucket
	//  write_offset+=sizeof(update_queue_header_bucket)
	//  dispose update_queue_header_bucket 
	//  verify (write_offset == chunk_headers_end)
	// 
	//  file_write gamestate_chunk_compressed
	//  write_offset+= gamestate_chunk_size
	//  dispose gamestate_chunk_compressed
	//  verify (write_offset == chunk_headers_end + gamestate_chunk_size) 
	// 
	//  loop queue
	//  file_write update_queue.data
	//  write_offset+= data_size
	//  loop_end
	//  verify (write_offset == chunk_headers_end + gamestate_chunk_size + update_chunks_size) 
	// 
	//  file_set_eof
	//  file_close

	//  file_open
	//  file_get_size()
	//  debug_simulation_post_write_header_fields
	// 
	//  file_set_read_only
	//  file_close

	if (open_file_error_code == _file_open_error_success)
	{
		file_close(&g_simulation_debug_globals.save_file);
	}

	//finished writing
	g_simulation_debug_globals.writing_file = false;
	return success;
}

bool debug_simulation_read_file_internal(void)
{
	bool success = true;
	if (!g_simulation_debug_globals.reading_file)
		// cannot write without starting write
		return false;

	if (g_simulation_debug_globals.save_file_name.length() == NULL)
	{
		LOG_ERROR_SIM("{} - no name set for debug simulation file!", __FUNCTION__);
		g_simulation_debug_globals.reading_file = false;
		return false;
	}

	c_static_string260 save_file_path;
	save_file_path.set(g_simulation_debug_globals.save_directory.get_string());
	save_file_path.append(g_simulation_debug_globals.save_file_name.get_string());
	save_file_path.append(K_SIMULATION_DEBUG_SAVE_FILE_EXTENSION);


	LOG_INFO_SIM("{} - start reading debug simulation file! {}", __FUNCTION__, save_file_path.get_string());

	file_reference_create_from_path(&g_simulation_debug_globals.save_file, save_file_path.get_string(), false);
	e_file_open_error open_file_error_code = _file_open_error_unknown;
	if (!file_open(&g_simulation_debug_globals.save_file, _permission_read_bit, &open_file_error_code))
	{
		LOG_ERROR_SIM("{} - failed to open debug simulation file for read , error code: {}", __FUNCTION__, (uint32)open_file_error_code);
		success = false;
	}

	if (open_file_error_code == _file_open_error_success)
	{

		s_simulation_debug_file_header debug_file_header;

		// first we read the header
		if (!success || !file_read(&g_simulation_debug_globals.save_file, sizeof(s_simulation_debug_file_header), true, &debug_file_header))
		{
			LOG_ERROR_SIM("{} - failed to read debug simulation file header!", __FUNCTION__);
			success = false;
		}


		if (success)
		{
			uint32 file_size_disk = NULL;
			if (!file_get_size(&g_simulation_debug_globals.save_file, &file_size_disk))
			{
				LOG_ERROR_SIM("{} - file_get_size failed", __FUNCTION__);
				success = false;
			}

			if (debug_file_header.file_size != file_size_disk)
			{
				LOG_ERROR_SIM("{} - stored size does not match size on disk! (file-size-mismatch)", __FUNCTION__);
				success = false;
			}
		}

		if (!success || !debug_simulation_verify_header_internal(&debug_file_header))
		{
			LOG_ERROR_SIM("{} - failed to verify debug simulation file header!", __FUNCTION__);
			success = false;
		}

		if (success)
		{
			csmemcpy(&g_simulation_debug_globals.film_options, &debug_file_header.debug_game_options, sizeof(s_game_options));

			if (debug_file_header.debug_chunks_count > 0)
			{
				uint8 gamestate_chunk_count = NULL;
				uint32 update_chunk_count = NULL;
				uint32 read_offset = sizeof(s_simulation_debug_file_header);
				uint32 debug_chunk_headers_size = sizeof(s_simulation_debug_chunk) * debug_file_header.debug_chunks_count;
				s_simulation_debug_chunk* debug_chunks = (s_simulation_debug_chunk*)debug_malloc(debug_chunk_headers_size);
				
				if (file_read(&g_simulation_debug_globals.save_file, debug_chunk_headers_size, false, debug_chunks))
				{
					LOG_TRACE_SIM(" - - > successfully read debug chunk headers , total count {}", debug_file_header.debug_chunks_count);
					read_offset += debug_chunk_headers_size;
				}
				else
				{
					LOG_ERROR_SIM("{} - failed to read debug chunk headers from simulation file!", __FUNCTION__);
					success = false;
				}

				//check expected offset with actual offset
				ASSERT(read_offset == debug_file_header.header_size + debug_chunk_headers_size);

				s_simulation_debug_chunk* current_chunk = debug_chunks;
				for (uint32 i = 0; i < debug_file_header.debug_chunks_count; i++)
				{
					//too much spam
					//LOG_TRACE_SIM(" - - > reading debug chunk type : {} size : 0x{:08X} offset : 0x{:08X} ", current_chunk->chunk_type, current_chunk->chunk_size, current_chunk->file_offset);

					if (VALID_COUNT(current_chunk->chunk_type, k_simulation_debug_chunk_types))
					{
						if (current_chunk->chunk_type == _debug_chunk_gamestate)
						{
							LOG_TRACE_SIM(" - - > found gamestate chunk size : 0x{:08X} offset : 0x{:08X}", current_chunk->chunk_size, current_chunk->file_offset);
							gamestate_chunk_count++;
							debug_gamestate_read_from_chunk(current_chunk);

						}
						else if (current_chunk->chunk_type == _debug_chunk_update)
						{
							//too much spam
							//LOG_TRACE_SIM(" - - > found update chunk size : 0x{:08X} offset : 0x{:08X}", current_chunk->chunk_size, current_chunk->file_offset);
							update_chunk_count++;
							debug_update_read_from_chunk(current_chunk);
						}

						read_offset += current_chunk->chunk_size;
						current_chunk++;
					}
					else
					{
						LOG_WARNING_SIM(" - - > got a bad chunk at count {} , skipping!!", i);
					}				
				}

				if (debug_chunks)
				{
					debug_free((uint8*)debug_chunks);
					LOG_TRACE_SIM("{} - clearing header chunks buffer ", __FUNCTION__);
				}

				//check expected offset with actual offset
				ASSERT(read_offset == debug_file_header.header_size + debug_chunk_headers_size + debug_file_header.chunk_size);

				if (gamestate_chunk_count > K_SIMULATION_DEBUG_MAX_GAMESTATES)
				{
					LOG_WARNING_SIM("{} - warning debug simulation file has too many gamestate chunks count :{} , expected : {}", __FUNCTION__, gamestate_chunk_count, K_SIMULATION_DEBUG_MAX_GAMESTATES);
				}
				if (update_chunk_count > K_SIMULATION_DEBUG_MAX_UPDATES)
				{
					LOG_WARNING_SIM("{} - warning debug simulation file has too many update chunks count :{} , expected : {}", __FUNCTION__, update_chunk_count, K_SIMULATION_DEBUG_MAX_UPDATES);
				}

			}
			else
			{
				LOG_WARNING_SIM("{} - warning debug simulation file has no chunk entries!", __FUNCTION__);
			}
		}

		file_close(&g_simulation_debug_globals.save_file);
	}

	g_simulation_debug_globals.reading_file = false;
	return success;
}

bool debug_simulation_verify_header_internal(s_simulation_debug_file_header* header)
{
	bool result = true;
	if (header->signature != K_SIMULATION_DEBUG_HEADER_SIGNATURE
		|| header->eof_signature != K_SIMULATION_DEBUG_HEADER_EOF_SIGNATURE)
	{
		LOG_ERROR_SIM("{} - invalid debug simulation file signature! ", __FUNCTION__);
		result = false;
	}


	if (header->header_size != sizeof(s_simulation_debug_file_header))
	{
		LOG_WARNING_SIM("{} - warning debug simulation file is old or outdated! (header-size-mismatch)", __FUNCTION__);
		result = true;
	}

	if (header->file_size != (header->header_size
		+ header->chunk_size
		+ header->debug_chunks_count * sizeof(s_simulation_debug_chunk))
		)
	{
		LOG_ERROR_SIM("{} - invalid debug simulation file (file-size-mismatch) ", __FUNCTION__);
		result = false;
	}

	if (header->debug_chunks_count != header->game_saves_count + header->game_updates_count)
	{
		LOG_ERROR_SIM("{} - invalid debug chunks count detected", __FUNCTION__);
		result = false;
	}	
	
	if (header->game_saves_count <1)
	{
		LOG_WARNING_SIM("{} - warning no gamesaves found in this file", __FUNCTION__);
		result = true;
	}

	return result;
}

bool debug_simulation_fetch_updates_internal(int32 remaining_updates, int32* updates_read_out)
{
	// apply initial gamestate if not done
	//
	// still buggy sadly 
	//debug_gamestate_apply_saved_state();



	//
	// fetch required no of updates
	//

	real32 game_speed = time_globals::get()->game_speed;
	int32 updates_required = time_globals::seconds_to_ticks_round(game_speed);

	c_simulation_world* world = simulation_get_world();
	bool match_remote_time = false;
	if (!world->time_get_available(&match_remote_time))
	{
		updates_required = MAX(updates_required, 1);
	}

	updates_required = MIN(remaining_updates, updates_required);
	int32 updates_fetched = NULL;

	bool succesfully_fetched = true;

	do
	{
		if (updates_fetched >= K_SIMULATION_DEBUG_MAX_ALLOWED_UPDATES_TO_FETCH)
			break;
		int32 available_updates = world->time_get_available(&match_remote_time);
		if (available_updates >= updates_required)
			break;

		// fetch updates and push to queue
		s_network_message_synchronous_update message; // maybe allocate in heap?
		if (debug_update_retrieve_latest_update(&message))
		{
			updates_fetched++;
			//LOG_INFO_SIM("simulation:global:debug inserting film playback update: tick/time {}/[{}]", message.update.game_time_ticks, message.update.simulation_time);
			if (!simulation_get_globals()->world->update_queue_handle_server_update(&message))
			{
				succesfully_fetched = false;
				LOG_WARNING_SIM("simulation:global:debug failed to insert film playback update.tick = {} , maybe out of memory", message.update.game_time_ticks);
			}
		}

	} while (succesfully_fetched);

	*updates_read_out = updates_fetched;

	return succesfully_fetched;
}


void debug_simulation_generate_default_header_internal(s_simulation_debug_file_header* header)
{
	csmemset(header, 0, sizeof(s_simulation_debug_file_header));
	header->signature = K_SIMULATION_DEBUG_HEADER_SIGNATURE;
	header->header_size = sizeof(s_simulation_debug_file_header);
	header->build.clear();
	header->build_time.clear();

#if defined(GEN_GIT_VER_VERSION_STRING) 
	{
		swprintf(header->build.get_buffer(), header->build.max_length(), L"%S\.%S\.%S", GEN_GIT_VER_VERSION_STRING, GET_GIT_VER_USERNAME, GET_GIT_VER_BRANCH);
#if defined(_DEBUG)
		header->build.append(L".debug");
#else
		header->build.append(L".release");
#endif
	}
#else
	{
		header->build.set(L"untracked carto build");
	}
#endif


	swprintf(header->build_time.get_buffer(), header->build_time.max_length(), L"%S %S", __DATE__, __TIME__);
	header->file_size = NULL;
	header->chunk_size = NULL;
	//header->start_tick = NULL;
	header->game_saves_count = NULL;
	header->game_updates_count = NULL;

	//copy current game_options
	csmemcpy(&header->debug_game_options, &g_simulation_debug_globals.film_options, sizeof(s_game_options));

	header->debug_chunks_count = NULL;


	header->eof_signature = K_SIMULATION_DEBUG_HEADER_EOF_SIGNATURE;
}

void debug_simulation_create_folders_internal()
{
	g_simulation_debug_globals.save_directory.set(GetExeDirectoryNarrow().c_str());
	g_simulation_debug_globals.save_directory.append(K_SIMULATION_DEBUG_SAVE_FOLDER_APPEND);

	s_file_reference save_folder;
	file_reference_create_from_path(&save_folder, g_simulation_debug_globals.save_directory.get_string(), true);
	file_create_parent_directories_if_not_present(&save_folder);
	file_close(&save_folder);
}

void debug_simulation_timestamp_internal(static_string32* timestamp)
{
	time_t timer = time(NULL);
	tm* tm_info = localtime(&timer);
	strftime(timestamp->get_buffer(), timestamp->max_length(), "%Y%m%d-%H%M%S_", tm_info);
}
