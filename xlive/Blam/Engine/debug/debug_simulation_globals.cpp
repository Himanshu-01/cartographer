#include "stdafx.h"
#include "debug_simulation_globals.h"
#include "debug_simulation_constants.h"
#include "debug_gamestate.h"
#include "cseries/cseries_strings.h"
#include "cseries/debug_memory.h"
#include "saved_games/game_state.h"

#include "version_git.h"
#include "Util/filesys.h"




s_simulation_debug_globals g_simulation_debug_globals;

//internal forward declaration
bool debug_simulation_write_file_internal(static_string32* name);
bool debug_simulation_verify_header_internal(s_simulation_debug_file_header* header);
void debug_simulation_generate_default_header_internal(s_simulation_debug_file_header* header);
void debug_simulation_create_folders_internal();



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

bool debug_simulation_read_file(static_string32* name)
{
	return false;
}

bool debug_simulation_write_file(static_string32* name)
{
	if (debug_simulation_active() && debug_simulation_is_recording())
	{
		debug_simulation_stop_recording();
		g_simulation_debug_globals.writing_file = true;
		if (debug_simulation_write_file_internal(name))
		{
			// success
			return true;
		}
		else
		{
			// error occured
			return false;
		}
	}


	if (!debug_simulation_active() || debug_simulation_is_replaying())
	{
		//  bad state
		//  not supported when replaying

	}

	return false;
}



void debug_simulation_initialize()
{
	g_simulation_debug_globals.initialized = true;
	debug_simulation_create_folders_internal();
}

void debug_simulation_clear()
{
	if (g_simulation_debug_globals.initialized)
	{
		debug_simulation_stop_recording();
		debug_simulation_stop_replay();

		g_simulation_debug_globals.writing_file = false;
		g_simulation_debug_globals.reading_file = false;

		if (g_simulation_debug_globals.gamestate_write_buffer)
		{
			debug_free(g_simulation_debug_globals.gamestate_write_buffer);
			g_simulation_debug_globals.gamestate_write_buffer = nullptr;
		}
		g_simulation_debug_globals.update_queue.clear();

		// maybe check save_file is being used and close it ??

	}
}

void debug_simulation_dispose()
{
	if (g_simulation_debug_globals.initialized)
	{
		debug_simulation_clear();
		g_simulation_debug_globals.update_queue.dispose();

		g_simulation_debug_globals.initialized = false;
	}
}

void debug_simulation_start_recording()
{
	g_simulation_debug_globals.recording_started = true;
	g_simulation_debug_globals.record_gamestate = true;
	g_simulation_debug_globals.record_random = true;
	g_simulation_debug_globals.record_update = true;
}

void debug_simulation_stop_recording()
{
	g_simulation_debug_globals.recording_started = false;
	g_simulation_debug_globals.record_gamestate = false;
	g_simulation_debug_globals.record_random = false;
	g_simulation_debug_globals.record_update = false;
	g_simulation_debug_globals.current_recording_tick = NONE;
}

void debug_simulation_stop_replay()
{
	g_simulation_debug_globals.replay_started = false;
	g_simulation_debug_globals.current_replaying_tick = NONE;
	g_simulation_debug_globals.target_replaying_tick = NONE;
}

void debug_simulation_gamestate_write_test()
{
	c_static_string260 save_file_path;
	save_file_path.set(g_simulation_debug_globals.save_location.get_string());
	save_file_path.append("test.dfilm");


	LOG_ERROR_GAME("{} - starting to create debug simulation file! {}", __FUNCTION__, save_file_path.get_string());

	file_reference_create_from_path(&g_simulation_debug_globals.save_file, save_file_path.get_string(), false);
	e_file_open_error open_file_error_code = _file_open_error_unknown;
	bool create_file_success = file_create(&g_simulation_debug_globals.save_file);

	if (create_file_success)
	{
		if (!file_open(&g_simulation_debug_globals.save_file, _permission_write_bit, &open_file_error_code))
		{
			LOG_ERROR_GAME("{} - failed to open debug simulation file for write , error code: {}", __FUNCTION__, (uint32)open_file_error_code);
		}
	}
	else
	{
		LOG_ERROR_GAME("{} - failed to create debug simulation file!", __FUNCTION__);

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
			LOG_WARNING_GAME("{} - total no of chunks exceed our expectations , count :{}", __FUNCTION__, total_write_chunks);
		}

		debug_file_header.debug_chunks_count = total_write_chunks;

		if (g_simulation_debug_globals.gamestate_write_buffer == nullptr)
		{
			LOG_TRACE_GAME("{} - using runtime gamestate as write_buffer", __FUNCTION__);
			g_simulation_debug_globals.gamestate_write_buffer = (uint8*)game_state_get_buffer_address(nullptr);
		}
		//
		// compress gamestate
		uint32 out_gamestate_chunk_size = NULL;
		uint8* gamestate_temporary_buffer = debug_malloc(K_SIMULATION_DEBUG_GAMESTATE_COMPRESSED_FILE_SIZE);
		if (!debug_gamestate_write_compressed_gamestate_to_buffer(gamestate_temporary_buffer, K_SIMULATION_DEBUG_GAMESTATE_COMPRESSED_FILE_SIZE, &out_gamestate_chunk_size))
		{
			LOG_ERROR_GAME("{} - failed to compress gamestate!", __FUNCTION__);
			//success = false;
		}

		if (g_simulation_debug_globals.gamestate_write_buffer == game_state_get_buffer_address(nullptr))
		{
			LOG_TRACE_GAME("{} - clearing write_buffer", __FUNCTION__);
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
			LOG_ERROR_GAME("{} - failed to write header to simulation debug file!", __FUNCTION__);
			//success = false;
		}
		write_offset += sizeof(s_simulation_debug_file_header);


		if (!file_write(&g_simulation_debug_globals.save_file, sizeof(s_simulation_debug_chunk), &gamestate_chunk))
		{
			LOG_ERROR_GAME("{} - failed to write gamestate debug chunk header to simulation debug file!", __FUNCTION__);
			//success = false;
		}
		write_offset += sizeof(s_simulation_debug_chunk);


		if (!file_write(&g_simulation_debug_globals.save_file, out_gamestate_chunk_size, gamestate_temporary_buffer))
		{
			LOG_ERROR_GAME("{} - failed to write compressed gamestate chunk to simulation debug file!", __FUNCTION__);
			//success = false;
		}
		write_offset += out_gamestate_chunk_size;
		if(gamestate_temporary_buffer)
			debug_free(gamestate_temporary_buffer);



		if (!file_set_eof(&g_simulation_debug_globals.save_file))
		{
			LOG_ERROR_GAME("{} - failed to set simulation debug file size!", __FUNCTION__);
			//success = false;
		}


		file_close(&g_simulation_debug_globals.save_file);
		

	}
}

void debug_simulation_gamestate_read_test()
{

	c_static_string260 save_file_path;
	save_file_path.set(g_simulation_debug_globals.save_location.get_string());
	save_file_path.append("test.dfilm");


	LOG_ERROR_GAME("{} - start reading debug simulation file! {}", __FUNCTION__, save_file_path.get_string());

	file_reference_create_from_path(&g_simulation_debug_globals.save_file, save_file_path.get_string(), false);
	e_file_open_error open_file_error_code = _file_open_error_unknown;


	if (!file_open(&g_simulation_debug_globals.save_file, _permission_read_bit, &open_file_error_code))
	{
		LOG_ERROR_GAME("{} - failed to open debug simulation file for read , error code: {}", __FUNCTION__, (uint32)open_file_error_code);
	}	



	if (open_file_error_code == _file_open_error_success)
	{

		s_simulation_debug_file_header debug_file_header;

		// first we read the header
		if (!file_read(&g_simulation_debug_globals.save_file, sizeof(s_simulation_debug_file_header), true, &debug_file_header))
		{
			LOG_ERROR_GAME("{} - failed to read debug simulation file header!", __FUNCTION__);
			file_close(&g_simulation_debug_globals.save_file);
			return;
		}

		if (!debug_simulation_verify_header_internal(&debug_file_header))
		{
			LOG_ERROR_GAME("{} - failed to verify debug simulation file header!", __FUNCTION__);
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
				if (file_read_from_position(&g_simulation_debug_globals.save_file, read_offset, sizeof(s_simulation_debug_file_header), true, current_chunk))
				{
					LOG_TRACE_GAME(" - - > reading debug chunk type : {} size : 0x{:08X} offset : 0x{:08X} ", current_chunk->chunk_type, current_chunk->chunk_size, current_chunk->file_offset);
					
					if(current_chunk->chunk_type==_debug_chunk_gamestate)
					{
						LOG_TRACE_GAME(" - - > found gamestate chunk size : 0x{:08X} offset : 0x{:08X}", current_chunk->chunk_size, current_chunk->file_offset);
						gamestate_chunk_count++;
						last_gamestate_chunk_header = current_chunk;
					}
				}
				else
				{
					LOG_TRACE_GAME(" - - > reading chunk failure at count {} ", i);
				}
				read_offset += sizeof(s_simulation_debug_chunk);
				current_chunk++;
			}

			if (gamestate_chunk_count > K_SIMULATION_DEBUG_MAX_GAMESTATES)
			{
				LOG_TRACE_GAME("{} - warning debug simulation file has too many gamestate chunks count :{} , expected : {}", __FUNCTION__, gamestate_chunk_count, K_SIMULATION_DEBUG_MAX_GAMESTATES);
			}

			if (gamestate_chunk_count > 0)
			{

				uint8* gamestate_compressed_buffer = debug_malloc(last_gamestate_chunk_header->chunk_size);
				if (file_read_from_position(&g_simulation_debug_globals.save_file, last_gamestate_chunk_header->file_offset, last_gamestate_chunk_header->chunk_size, true, gamestate_compressed_buffer))
				{
					LOG_TRACE_GAME("{} - successfully read gamestate data from save file ", __FUNCTION__);

					if (g_simulation_debug_globals.gamestate_write_buffer == nullptr)
					{
						uint32 gamestate_buffer_size = NULL;
						game_state_get_buffer_address(&gamestate_buffer_size);

						LOG_TRACE_GAME("{} - simulation:debug:gamestate initializing save memory for reading ", __FUNCTION__);
						g_simulation_debug_globals.gamestate_write_buffer = debug_malloc(gamestate_buffer_size);
					}

					uint32 out_decompressed_size = NULL;
					if (!debug_gamestate_read_compressed_gamestate_from_buffer(gamestate_compressed_buffer, last_gamestate_chunk_header->chunk_size, &out_decompressed_size))
					{
						LOG_ERROR_GAME("{} - failed to decompress gamestate!", __FUNCTION__);
						//success = false;
					}

					s_game_state_header saved_header;
					debug_gamestate_read_header(&saved_header);
					debug_gamestate_compare_header_with_runtime(&saved_header);

					if(g_simulation_debug_globals.gamestate_write_buffer)
					{
						LOG_TRACE_GAME("{} - clearing write_buffer 0x{:X} ", __FUNCTION__ , (uint32)g_simulation_debug_globals.gamestate_write_buffer);
						debug_free(g_simulation_debug_globals.gamestate_write_buffer);
						g_simulation_debug_globals.gamestate_write_buffer = nullptr;
					}
				}
				if(gamestate_compressed_buffer)
				{
					debug_free(gamestate_compressed_buffer);
					LOG_TRACE_GAME("{} - clearing compressed buffer ", __FUNCTION__);
				}
			}
			if(debug_chunks)
			{
				debug_free((uint8*)debug_chunks);
				LOG_TRACE_GAME("{} - clearing header chunks buffer ", __FUNCTION__);
			}
			
		}
		else
		{
			LOG_TRACE_GAME("{} - warning debug simulation file has no chunk entries!", __FUNCTION__);
		}

		file_close(&g_simulation_debug_globals.save_file);


	}


}






// internal function definitions



bool debug_simulation_write_file_internal(static_string32* name)
{
	bool success = true;
	if (!g_simulation_debug_globals.writing_file)
		// cannot write without starting write
		return false;

	c_static_string260 save_file_path;
	save_file_path.set(g_simulation_debug_globals.save_location.get_string());
	save_file_path.append(name->get_string());

	file_reference_create_from_path(&g_simulation_debug_globals.save_file, save_file_path.get_string(), false);
	//  file_create
	//  file_open
	//  file_write
	//  verify
	//  s_simulation_debug_file_header
	//  debug_simulation_generate_default_header_internal

	e_file_open_error open_file_error_code = _file_open_error_unknown;
	bool create_file_success = file_create(&g_simulation_debug_globals.save_file);

	if(create_file_success)
	{
		if (!file_open(&g_simulation_debug_globals.save_file, _permission_write_bit, &open_file_error_code))
		{
			LOG_ERROR_GAME("{} - failed to open debug simulation file for write , error code: {}", __FUNCTION__, (uint32)open_file_error_code);
			success = false;
		}
	}
	else
	{
		LOG_ERROR_GAME("{} - failed to create debug simulation file!", __FUNCTION__);
		success = false;
	}


	if (open_file_error_code == _file_open_error_success)
	{
		uint32 write_offset = NULL;
		uint32 total_write_chunks = K_SIMULATION_DEBUG_MAX_GAMESTATES + g_simulation_debug_globals.update_queue.queued_count();
		uint32 chunk_headers_end = total_write_chunks * sizeof(s_simulation_debug_chunk) + sizeof(s_simulation_debug_file_header);

		s_simulation_debug_file_header debug_file_header;
		debug_simulation_generate_default_header_internal(&debug_file_header);

		if (total_write_chunks > K_SIMULATION_DEBUG_MAX_GAMESTATES + K_SIMULATION_DEBUG_MAX_UPDATES)
		{
			//warning
			LOG_WARNING_GAME("{} - total no of chunks exceed our expectations update chunks :{}", __FUNCTION__, total_write_chunks - K_SIMULATION_DEBUG_MAX_GAMESTATES);
		}

		debug_file_header.debug_chunks_count = total_write_chunks;

		//
		// compress gamestate
		uint32 out_gamestate_chunk_size = NULL;
		uint8* gamestate_temporary_buffer = debug_malloc(K_SIMULATION_DEBUG_GAMESTATE_COMPRESSED_FILE_SIZE);
		if (!debug_gamestate_write_compressed_gamestate_to_buffer(gamestate_temporary_buffer, K_SIMULATION_DEBUG_GAMESTATE_COMPRESSED_FILE_SIZE, &out_gamestate_chunk_size))
		{
			LOG_ERROR_GAME("{} - failed to compress gamestate!", __FUNCTION__);
			success = false;
		}

		debug_file_header.game_saves_count++;


		if (success)
		{
			//LOG_INFO_GAME("{} - writing simulation debug file data...", __FUNCTION__);

			s_simulation_debug_chunk gamestate_chunk;
			gamestate_chunk.chunk_type = _debug_chunk_gamestate;
			gamestate_chunk.file_offset = chunk_headers_end;
			gamestate_chunk.chunk_size = out_gamestate_chunk_size;


			uint32 update_queue_header_bucket_size = sizeof(s_simulation_debug_chunk) * g_simulation_debug_globals.update_queue.queued_count();
			s_simulation_debug_chunk* update_queue_header_bucket = (s_simulation_debug_chunk*)debug_malloc(update_queue_header_bucket_size);
			uint32 update_chunks_size = NULL;



			for (const c_debug_update_node* update_node = g_simulation_debug_globals.update_queue.get_first_element();
				update_node != nullptr;
				update_node = g_simulation_debug_globals.update_queue.get_next_element(update_node)
				)
			{
				update_queue_header_bucket->chunk_type = _debug_chunk_update;
				update_queue_header_bucket->file_offset = chunk_headers_end + out_gamestate_chunk_size + update_chunks_size;
				update_queue_header_bucket->chunk_size = update_node->data_size;
				update_chunks_size += update_node->data_size;

				update_queue_header_bucket++;
				debug_file_header.game_updates_count++;
			}

			debug_file_header.chunk_size = out_gamestate_chunk_size + update_chunks_size;
			debug_file_header.file_size = chunk_headers_end + out_gamestate_chunk_size + update_chunks_size;


			if (!file_write(&g_simulation_debug_globals.save_file, sizeof(s_simulation_debug_file_header), &debug_file_header))
			{
				LOG_ERROR_GAME("{} - failed to write header to simulation debug file!", __FUNCTION__);
				success = false;
			}
			write_offset += sizeof(s_simulation_debug_file_header);


			if (!file_write(&g_simulation_debug_globals.save_file, sizeof(gamestate_chunk), &gamestate_chunk))
			{
				LOG_ERROR_GAME("{} - failed to write gamestate debug chunk header to simulation debug file!", __FUNCTION__);
				success = false;
			}
			write_offset += sizeof(gamestate_chunk);


			if (!file_write(&g_simulation_debug_globals.save_file, update_queue_header_bucket_size, update_queue_header_bucket))
			{
				LOG_ERROR_GAME("{} - failed to write update debug chunk headers to simulation debug file!", __FUNCTION__);
				success = false;
			}
			write_offset += update_queue_header_bucket_size;
			debug_free((uint8*)update_queue_header_bucket);


			//check expected offset with actual offset
			ASSERT(write_offset == chunk_headers_end);


			if (!file_write(&g_simulation_debug_globals.save_file, out_gamestate_chunk_size, gamestate_temporary_buffer))
			{
				LOG_ERROR_GAME("{} - failed to write compressed gamestate chunk to simulation debug file!", __FUNCTION__);
				success = false;
			}
			write_offset += out_gamestate_chunk_size;
			debug_free(gamestate_temporary_buffer);

			//check expected offset with actual offset
			ASSERT(write_offset == chunk_headers_end + out_gamestate_chunk_size);



			for (const c_debug_update_node* update_node = g_simulation_debug_globals.update_queue.get_first_element();
				update_node != nullptr;
				update_node = g_simulation_debug_globals.update_queue.get_next_element(update_node)
				)
			{
				if (!file_write(&g_simulation_debug_globals.save_file, update_node->data_size, update_node->data))
				{
					LOG_ERROR_GAME("{} - failed to write update debug chunk to simulation debug file!", __FUNCTION__);
					success = false;
				}
				write_offset += update_node->data_size;
			}

			//check expected offset with actual offset
			ASSERT(write_offset == chunk_headers_end + out_gamestate_chunk_size + update_chunks_size);

		}
		if (!file_set_eof(&g_simulation_debug_globals.save_file))
		{
			LOG_ERROR_GAME("{} - failed to set simulation debug file size!", __FUNCTION__);
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

bool debug_simulation_verify_header_internal(s_simulation_debug_file_header* header)
{
	if (header->signature != K_SIMULATION_DEBUG_HEADER_SIGNATURE
		|| header->eof_signature != K_SIMULATION_DEBUG_HEADER_EOF_SIGNATURE)
	{
		LOG_ERROR_GAME("{} - invalid debug simulation file signature! ", __FUNCTION__);
		return false;
	}


	if (header->header_size != sizeof(s_simulation_debug_file_header))
	{
		LOG_TRACE_GAME("{} - warning debug simulation file is old or outdated! (header-size-mismatch)", __FUNCTION__);
		return false;
	}

	if (header->file_size != (header->header_size
		+ header->chunk_size
		+ header->debug_chunks_count * sizeof(s_simulation_debug_chunk))
		)
	{
		LOG_ERROR_GAME("{} - invalid debug simulation file (file-size-mismatch) ", __FUNCTION__);
		return false;
	}

	if (header->debug_chunks_count != header->game_saves_count + header->game_updates_count)
	{
		LOG_ERROR_GAME("{} - invalid debug chunks count detected", __FUNCTION__);
		return false;
	}

	return true;
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


	header->debug_chunks_count = NULL;


	header->eof_signature = K_SIMULATION_DEBUG_HEADER_EOF_SIGNATURE;
}

void debug_simulation_create_folders_internal()
{
	g_simulation_debug_globals.save_location.set(GetExeDirectoryNarrow().c_str());
	g_simulation_debug_globals.save_location.append(K_SIMULATION_DEBUG_SAVE_FOLDER_APPEND);

	s_file_reference save_folder;
	file_reference_create_from_path(&save_folder, g_simulation_debug_globals.save_location.get_string(), true);
	file_create_parent_directories_if_not_present(&save_folder);
	file_close(&save_folder);
}