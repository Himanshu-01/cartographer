#include "stdafx.h"
#include "debug_simulation_globals.h"
#include "debug_gamestate.h"
#include "debug_update.h"

#include "cseries/debug_memory.h"
#include "game/game.h"
#include "game/game_time.h"
#include "main/main_game.h"
#include "networking/network_event.h"
#include "saved_games/game_state.h"
#include "simulation/simulation.h"
#include "simulation/simulation_world.h"
#include "simulation/simulation_update.h"

#include "version_git.h"

/* constants */

enum
{
	k_simulation_debug_header_signature = 'dhsd',
	k_simulation_debug_header_eof_signature = 'dhed',

	k_simulation_debug_max_gamestates_allowed = 1,
	k_simulation_debug_max_allowed_updates_to_fetch = 30,
	k_simulation_debug_gamestate_compressed_file_size_max = 0x40000
};

/* enums */

/* structures */

/* globals */


static const char* k_simulation_films_save_folder = "mods\\films\\";
static const char* k_simulation_films_file_extension = ".dbgfilm";


s_simulation_debug_globals g_simulation_debug_globals;

/* prototypes */

bool debug_simulation_write_file_internal(void);
bool debug_simulation_read_file_internal(void);
bool debug_simulation_verify_header_internal(s_simulation_debug_file_header* header);
bool debug_simulation_fetch_updates_internal(int32 remaining_updates, int32* updates_read_out);
void debug_simulation_generate_default_header_internal(s_simulation_debug_file_header* header);
void debug_simulation_create_folders_internal();
void debug_simulation_timestamp_internal(c_static_string<32>* timestamp);


/* public code */


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
	return debug_simulation_is_replaying() && g_simulation_debug_globals.update_queue.queued_count() > 0;
}

bool debug_simulation_replay_has_gamesave()
{
	ASSERT(debug_simulation_active());
	return debug_simulation_is_replaying() && g_simulation_debug_globals.replay_has_gamestate;
}

bool debug_simulation_replay_should_reset_time()
{
	ASSERT(debug_simulation_active());
	return debug_simulation_is_replaying() && g_simulation_debug_globals.replay_reset_time;
}

bool debug_simulation_retrieve_updates()
{
	ASSERT(debug_simulation_active());
	ASSERT(debug_simulation_is_replaying());

	int32 updates_left_to_read = debug_simulation_replay_update_queue_length();
	int32 updates_read = 0;
	bool result = debug_simulation_fetch_updates_internal(updates_left_to_read, &updates_read);

	if (result)
	{
		event(_event_status,
			"cartographer:saved_film: debug_simulation_retrieve_updates() , left_to_read/read out : %d/%d ",
			updates_left_to_read,
			updates_read
		);
	}
	else if (updates_left_to_read > 0)
	{
		event(_event_error,
			"cartographer:saved_film: failed to read updates from saved film (should have had %d to read?)",
			updates_left_to_read);

	}
	return result;
}

int32 debug_simulation_replay_update_queue_length()
{
	ASSERT(debug_simulation_active());
	return g_simulation_debug_globals.update_queue.queued_count();
}

void debug_simulation_read_saved_film()
{
	bool result = false;
	debug_simulation_pause(true);
	if (debug_simulation_active())
	{
		debug_simulation_stop_recording();
		debug_simulation_stop_replay();
		debug_update_queue_clear();
		g_simulation_debug_globals.reading_file = true;

		if (debug_simulation_read_file_internal())
		{
			// success
			event(_event_message,
				"cartographer:saved_film: successfully read saved film : %s",
				g_simulation_debug_globals.save_file_name.get_string()
			);
			result = true;
		}
		else
		{
			event(_event_error,
				"cartographer:saved_film: failed to read saved film : %s",
				g_simulation_debug_globals.save_file_name.get_string()
			);
			// error occured
			result = false;
		}
	}


	if (!result)
	{
		//  bad state
		//  not supported when writing
		event(_event_error,
			"cartographer:saved_film: cannot read_saved_film in current state, active/record/replay/reading/writing : %d/%d/%d/%d/%d ",
			debug_simulation_active(),
			debug_simulation_is_recording(),
			debug_simulation_is_replaying(),
			g_simulation_debug_globals.reading_file,
			g_simulation_debug_globals.writing_file
		);
	}
	debug_simulation_pause(false);
}

void debug_simulation_write_saved_film()
{
	bool result = false;
	debug_simulation_pause(true);
	if (debug_simulation_active() && debug_simulation_is_recording())
	{
		debug_simulation_stop_recording();
		g_simulation_debug_globals.writing_file = true;
		if (debug_simulation_write_file_internal())
		{
			// success
			event(_event_message,
				"cartographer:saved_film: successfully written saved film file : %s",
				g_simulation_debug_globals.save_file_name.get_string()
			);
			result = true;
		}
		else
		{
			// error occured
			event(_event_error,
				"cartographer:saved_film: failed to write saved film : %s",
				g_simulation_debug_globals.save_file_name.get_string()
			);
			result = false;
		}
	}


	if (!result)
	{
		//  bad state
		//  not supported when replaying or reading
		event(_event_error,
			"cartographer:saved_film: cannot write_saved_film in current state, active/record/replay/reading/writing : %d/%d/%d/%d/%d ",
			debug_simulation_active(),
			debug_simulation_is_recording(),
			debug_simulation_is_replaying(),
			g_simulation_debug_globals.reading_file,
			g_simulation_debug_globals.writing_file
		);

	}
}



void debug_simulation_initialize()
{
	event(_event_message, "cartographer:saved_film: initializing system");

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

		// maybe check save_file is being used and close it ??
		event(_event_message, "cartographer:saved_film: cleared current state");

	}
}

void debug_simulation_dispose()
{
	if (g_simulation_debug_globals.initialized)
	{
		debug_simulation_clear();
		debug_update_queue_dispose();

		g_simulation_debug_globals.initialized = false;
		event(_event_message, "cartographer:saved_film: disposing");
	}
}

void debug_simulation_start_recording()
{
	event(_event_message, "cartographer:saved_film: starting simulation recording");

	g_simulation_debug_globals.recording_started = true;
	g_simulation_debug_globals.record_gamestate = true;
	g_simulation_debug_globals.record_random = true;
	g_simulation_debug_globals.record_update = true;
	g_simulation_debug_globals.captured_gamestate = false;
}

void debug_simulation_start_recording_on_map_change()
{
	event(_event_message, "cartographer:saved_film: will start recording on map change");
	g_simulation_debug_globals.start_capture_on_map_change = true;
}

void debug_simulation_stop_recording()
{
	event(_event_message, "cartographer:saved_film: stopping recording mode");

	g_simulation_debug_globals.recording_started = false;
	g_simulation_debug_globals.record_gamestate = false;
	g_simulation_debug_globals.record_random = false;
	g_simulation_debug_globals.record_update = false;
	g_simulation_debug_globals.current_recording_tick = NONE;
}

void debug_simulation_stop_replay()
{
	event(_event_message, "cartographer:saved_film: stopping playback mode");

	g_simulation_debug_globals.replay_started = false;
	g_simulation_debug_globals.current_replaying_tick = NONE;
	g_simulation_debug_globals.target_replaying_tick = NONE;
	debug_update_memory_clear();
}

void debug_simulation_pause(bool pause)
{
	const char* state = pause ? "pausing" : "un-pausing";
	event(_event_message, "cartographer:saved_film: %s game state", state);
	game_time_set_paused(pause);
}

void debug_simulation_launch_replay()
{
	//  simulation_end();
	//	grab header
	//	main_game_change
	if (debug_simulation_active()
		&& !debug_simulation_is_recording())
	{
		debug_simulation_stop_replay();
		simulation_end();

		event(_event_message,
			"cartographer:saved_film: launching film playback name : %s%s",
			g_simulation_debug_globals.save_file_name.get_string(),
			k_simulation_films_file_extension
		);

		game_state_header header;
		s_game_options* launch_options = nullptr;

		if (debug_gamestate_read_header(&header))
		{
			event(_event_message,
				"cartographer:saved_film:  on scenario : %s ",
				header.scenario_name
			);
			launch_options = &header.options;
		}
		else
		{
			event(_event_warning, "cartographer:saved_film: found no gamestate header, using saved film options..");
			event(_event_message,
				"cartographer:saved_film:  on scenario : %S ",
				g_simulation_debug_globals.film_options.scenario_path
			);
			launch_options = &g_simulation_debug_globals.film_options;
		}

		g_simulation_debug_globals.replay_started = true;
		g_simulation_debug_globals.replay_applied_gamestate = false;
		debug_update_memory_initialize_for_playback();

		ASSERT(game_options_verify(launch_options));

		main_game_change(launch_options);
	}

}

void debug_simulation_set_name(const char* name)
{
	if (!debug_simulation_active())
	{
		event(_event_error, "cartographer:saved_film: failed to set debug simulation file name! , not initialized");
	}
	else
	{
		ASSERT(debug_simulation_active());
		g_simulation_debug_globals.save_file_name.set(name);
		event(_event_message, "cartographer:saved_film: setting file name set to %s ", name);
	}
}

void debug_simulation_set_options(s_game_options* options)
{
	if (debug_simulation_active()
		&& debug_simulation_is_recording())
	{
		if (game_options_verify(options))
		{
			g_simulation_debug_globals.film_options = *options;
			event(_event_message, "cartographer:saved_film: captured game options");
		}
		else
		{
			event(_event_error, "cartographer:saved_film: tried capturing bad game options!!");
		}
	}
}

void debug_simulation_start_recording_for_oos()
{
	if (!game_is_ui_shell()
		&& game_is_synchronous_networking()
		&& !game_is_playback())
	{
		//always start recording in synchronous so that we can capture oos
		event(_event_message, "cartographer:saved_film: starting for oos capture");
		debug_simulation_start_recording();
	}
}

void debug_simulation_notify_oos()
{
	if (debug_simulation_active()
		&& debug_simulation_is_recording())
	{
		event(_event_fatal, "cartographer:saved_film: has encountered oos , terminating saved_film recording");
		//event(_event_error, "cartographer:saved_film: calling dump random seed");
		//debug_random_dump_call_stack();


		c_static_string<32> timestamp;
		debug_simulation_timestamp_internal(&timestamp);
		g_simulation_debug_globals.save_file_name.set(timestamp.get_string());

		const char* type = game_is_server() ? "host" : "client";
		g_simulation_debug_globals.save_file_name.append(type);
		g_simulation_debug_globals.save_file_name.append("_oos");
		debug_simulation_write_saved_film();
		debug_simulation_stop_recording();
	}
}

void debug_simulation_gamestate_write_test()
{
	if (debug_simulation_active())
	{
		c_static_string<MAX_PATH> save_file_path;
		save_file_path.set(k_simulation_films_save_folder);
		save_file_path.append("test");
		save_file_path.append(k_simulation_films_file_extension);


		event(_event_status, "cartographer:saved_film_write: starting to create test simulation file %s", save_file_path.get_string());
		file_reference_create_from_path(&g_simulation_debug_globals.save_file, save_file_path.get_string(), false);
		e_file_open_error open_file_error_code = _file_open_error_unknown;
		bool create_file_success = file_create(&g_simulation_debug_globals.save_file);

		if (create_file_success)
		{
			if (!file_open(&g_simulation_debug_globals.save_file, _permission_write_bit, &open_file_error_code))
			{
				event(_event_error,
					"cartographer:saved_film_write: failed to open test saved_film file for write , error code: %d",
					open_file_error_code
				);
			}
		}
		else
		{
			event(_event_error, "cartographer:saved_film_write: failed to create test saved_film file!");
		}

		if (open_file_error_code == _file_open_error_success)
		{

			uint32 total_write_chunks = 1;
			uint32 chunk_headers_end = total_write_chunks * sizeof(s_simulation_debug_chunk) + sizeof(s_simulation_debug_file_header);

			s_simulation_debug_file_header debug_file_header;
			debug_simulation_generate_default_header_internal(&debug_file_header);


			debug_file_header.debug_chunks_count = total_write_chunks;
			uint32 gamestate_size = 0;
			if (g_simulation_debug_globals.gamestate_write_buffer == nullptr)
			{
				event(_event_status, "cartographer:saved_film_write: using runtime gamestate as write_buffer");
				g_simulation_debug_globals.gamestate_write_buffer = (uint8*)game_state_get_buffer_address(&gamestate_size);
			}
			//
			// compress gamestate
			uint32 out_gamestate_chunk_size = NULL;
			uint8* gamestate_temporary_buffer = (uint8*)CSERIES_MALLOC(k_simulation_debug_gamestate_compressed_file_size_max);
			if (!debug_gamestate_write_compressed_gamestate_to_buffer(
				gamestate_temporary_buffer,
				k_simulation_debug_gamestate_compressed_file_size_max,
				&out_gamestate_chunk_size))
			{
				event(_event_error, "cartographer:saved_film_write: failed to compress gamestate!");
				//success = false;
			}

			if (g_simulation_debug_globals.gamestate_write_buffer == game_state_get_buffer_address(&gamestate_size))
			{
				g_simulation_debug_globals.gamestate_write_buffer = nullptr;
			}
			debug_file_header.game_saves_count++;

			s_simulation_debug_chunk gamestate_chunk;
			gamestate_chunk.chunk_type = _debug_chunk_gamestate;
			gamestate_chunk.file_offset = chunk_headers_end;
			gamestate_chunk.chunk_size = out_gamestate_chunk_size;



			debug_file_header.chunk_size = out_gamestate_chunk_size + 0;
			debug_file_header.file_size = chunk_headers_end + out_gamestate_chunk_size + 0;

			uint32 write_offset = 0;
			if (!file_write(&g_simulation_debug_globals.save_file, sizeof(s_simulation_debug_file_header), &debug_file_header))
			{
				event(_event_error, "cartographer:saved_film_write: failed to write header to simulation film file!");
				//success = false;
			}
			write_offset += sizeof(s_simulation_debug_file_header);


			if (!file_write(&g_simulation_debug_globals.save_file, sizeof(s_simulation_debug_chunk), &gamestate_chunk))
			{
				event(_event_error, "cartographer:saved_film_write: failed to write gamestate chunk header to simulation film file!");
				//success = false;
			}
			write_offset += sizeof(s_simulation_debug_chunk);


			if (!file_write(&g_simulation_debug_globals.save_file, out_gamestate_chunk_size, gamestate_temporary_buffer))
			{
				event(_event_error, "cartographer:saved_film_write: failed to write compressed gamestate chunk to simulation film file!");
				//success = false;
			}
			write_offset += out_gamestate_chunk_size;

			ASSERT(write_offset == debug_file_header.file_size);

			if (gamestate_temporary_buffer)
				CSERIES_FREE(gamestate_temporary_buffer);


			if (!file_set_eof(&g_simulation_debug_globals.save_file, write_offset))
			{
				event(_event_error, "cartographer:saved_film_write: failed to set simulation debug file size!");
				//success = false;
			}

			file_close(&g_simulation_debug_globals.save_file);
		}
	}
}

void debug_simulation_gamestate_read_test()
{
	c_static_string<MAX_PATH> save_file_path;
	save_file_path.set(k_simulation_films_save_folder);
	save_file_path.append("test");
	save_file_path.append(k_simulation_films_file_extension);


	event(_event_status, "cartographer:saved_film_read: starting to read test simulation file %s", save_file_path.get_string());
	file_reference_create_from_path(&g_simulation_debug_globals.save_file, save_file_path.get_string(), false);
	e_file_open_error open_file_error_code = _file_open_error_unknown;


	if (!file_open(&g_simulation_debug_globals.save_file, _permission_read_bit, &open_file_error_code))
	{
		event(_event_error,
			"cartographer:saved_film_read: failed to open debug simulation file for read , error code: %d",
			open_file_error_code
		);
	}

	if (open_file_error_code == _file_open_error_success)
	{
		s_simulation_debug_file_header debug_file_header;
		// first we read the header
		if (!file_read(&g_simulation_debug_globals.save_file, sizeof(s_simulation_debug_file_header), true, &debug_file_header))
		{
			event(_event_error, "cartographer:saved_film_read: failed to read test simulation file header!");
			file_close(&g_simulation_debug_globals.save_file);
			return;
		}

		if (!debug_simulation_verify_header_internal(&debug_file_header))
		{
			event(_event_error, "cartographer:saved_film_read: failed to verify test simulation file header!");
			file_close(&g_simulation_debug_globals.save_file);
			return;
		}

		if (debug_file_header.debug_chunks_count > 0)
		{
			uint8 gamestate_chunk_count = NULL;
			uint32 read_offset = sizeof(s_simulation_debug_file_header);
			uint32 debug_chunk_headers_size = sizeof(s_simulation_debug_chunk) * debug_file_header.debug_chunks_count;
			s_simulation_debug_chunk* debug_chunks = (s_simulation_debug_chunk*)CSERIES_MALLOC(debug_chunk_headers_size);
			s_simulation_debug_chunk* last_gamestate_chunk_header = nullptr;

			ASSERT(debug_chunks);
			for (uint8 i = 0; i < debug_file_header.debug_chunks_count; i++)
			{
				s_simulation_debug_chunk* current_chunk = debug_chunks;
				if (file_read_from_position(&g_simulation_debug_globals.save_file, read_offset, sizeof(s_simulation_debug_chunk), true, current_chunk))
				{
					event(_event_verbose,
						"- - > reading debug chunk type/size/offset %d/%d/0x%08X",
						current_chunk->chunk_type,
						current_chunk->chunk_size,
						current_chunk->file_offset
					);

					if (current_chunk->chunk_type == _debug_chunk_gamestate)
					{
						event(_event_verbose,
							"- - > found gamestate chunk size 0x%08X at offset: 0x%08X",
							current_chunk->chunk_size,
							current_chunk->file_offset
						);

						gamestate_chunk_count++;
						last_gamestate_chunk_header = current_chunk;
					}
				}
				else
				{
					event(_event_verbose, " - - > reading chunk failure at count: %d ", i);
				}
				read_offset += sizeof(s_simulation_debug_chunk);
				current_chunk++;
			}

			if (gamestate_chunk_count > k_simulation_debug_max_gamestates_allowed)
			{
				event(_event_error,
					"cartographer:saved_film_read: warning test simulation file has too many gamestate chunks count/expected ",
					gamestate_chunk_count,
					k_simulation_debug_max_gamestates_allowed
				);
			}

			if (gamestate_chunk_count > 0)
			{
				uint8* gamestate_compressed_buffer = (uint8*)CSERIES_MALLOC(last_gamestate_chunk_header->chunk_size);
				if (file_read_from_position(&g_simulation_debug_globals.save_file, last_gamestate_chunk_header->file_offset, last_gamestate_chunk_header->chunk_size, true, gamestate_compressed_buffer))
				{
					event(_event_status, "cartographer:saved_film_read: successfully read gamestate chunk from test file ");
					if (g_simulation_debug_globals.gamestate_write_buffer == nullptr)
					{
						uint32 gamestate_buffer_size = NULL;
						game_state_get_buffer_address(&gamestate_buffer_size);

						event(_event_status, "cartographer:saved_film_read: initializing gamestate memory for reading test file");
						g_simulation_debug_globals.gamestate_write_buffer = (uint8*)CSERIES_MALLOC(gamestate_buffer_size);
						ASSERT(g_simulation_debug_globals.gamestate_write_buffer);
					}

					uint32 out_decompressed_size = NULL;
					if (!debug_gamestate_read_compressed_gamestate_from_buffer(gamestate_compressed_buffer, last_gamestate_chunk_header->chunk_size, &out_decompressed_size))
					{
						event(_event_error, "cartographer:saved_film_read: failed to decompress gamestate chunk!");
						//success = false;
					}

					game_state_header saved_header;
					debug_gamestate_read_header(&saved_header);
					debug_gamestate_compare_header_with_runtime(&saved_header);

					if (g_simulation_debug_globals.gamestate_write_buffer)
					{
						event(_event_status,
							"cartographer:saved_film_read: clearing gamestate memory 0x%08X",
							(uint32)g_simulation_debug_globals.gamestate_write_buffer
						);
						CSERIES_FREE(g_simulation_debug_globals.gamestate_write_buffer);
						g_simulation_debug_globals.gamestate_write_buffer = nullptr;
					}
				}
				if (gamestate_compressed_buffer)
				{
					CSERIES_FREE(gamestate_compressed_buffer);
				}
			}
			if (debug_chunks)
			{
				CSERIES_FREE((uint8*)debug_chunks);
			}

		}
		else
		{
			event(_event_error, "cartographer:saved_film_read: test simulation file has no chunk entries!");
		}

		file_close(&g_simulation_debug_globals.save_file);
	}
}



/* private code */

bool debug_simulation_write_file_internal(void)
{
	bool success = true;

	if (debug_simulation_active()
		&& g_simulation_debug_globals.writing_file)
	{
		c_static_string<MAX_PATH> save_file_path;
		save_file_path.set(k_simulation_films_save_folder);

		if (g_simulation_debug_globals.save_file_name.length() == NULL)
		{
			c_static_string<32> timestamp;
			debug_simulation_timestamp_internal(&timestamp);
			g_simulation_debug_globals.save_file_name.set(timestamp.get_string());
		}

		save_file_path.append(g_simulation_debug_globals.save_file_name.get_string());
		save_file_path.append(k_simulation_films_file_extension);

		event(_event_message,
			"cartographer:saved_film_write: writing simulation saved film : %s",
			save_file_path.get_string()
		);

		file_reference_create_from_path(&g_simulation_debug_globals.save_file, save_file_path.get_string(), false);
		e_file_open_error open_file_error_code = _file_open_error_unknown;
		bool create_file_success = file_create(&g_simulation_debug_globals.save_file);

		if (create_file_success)
		{
			if (!file_open(&g_simulation_debug_globals.save_file, _permission_write_bit, &open_file_error_code))
			{
				event(_event_error,
					"cartographer:saved_film_write: failed to open film for write , error code: %d",
					open_file_error_code
				);
				success = false;
			}
		}
		else
		{
			event(_event_error, "cartographer:saved_film_write: failed to create simulation saved film!");
			success = false;
		}


		if (open_file_error_code == _file_open_error_success)
		{
			uint32 write_offset = NULL;
			uint32 total_write_chunks = g_simulation_debug_globals.captured_gamestate + g_simulation_debug_globals.update_queue.queued_count();
			uint32 chunk_headers_end = total_write_chunks * sizeof(s_simulation_debug_chunk) + sizeof(s_simulation_debug_file_header);

			s_simulation_debug_file_header debug_file_header;
			debug_simulation_generate_default_header_internal(&debug_file_header);
			debug_file_header.debug_chunks_count = total_write_chunks;

			//
			// compress gamestate and write
			//
			uint32 out_gamestate_chunk_size = NULL;
			uint8* gamestate_temporary_buffer = (uint8*)CSERIES_MALLOC(k_simulation_debug_gamestate_compressed_file_size_max);
			ASSERT(gamestate_temporary_buffer != NULL);

			if (g_simulation_debug_globals.captured_gamestate)
			{
				if (debug_gamestate_write_compressed_gamestate_to_buffer(
					gamestate_temporary_buffer,
					k_simulation_debug_gamestate_compressed_file_size_max,
					&out_gamestate_chunk_size))
				{
					event(_event_message, "cartographer:saved_film_write: successfully written gamestate data to save file ");
					debug_file_header.game_saves_count++;
				}
				else
				{
					event(_event_error, "cartographer:saved_film_write: failed to compress gamestate!");
					success = false;
				}
			}


			if (success)
			{
				s_simulation_debug_chunk gamestate_chunk;
				gamestate_chunk.chunk_type = _debug_chunk_gamestate;
				gamestate_chunk.file_offset = chunk_headers_end;
				gamestate_chunk.chunk_size = out_gamestate_chunk_size;

				//
				// setup update chunks header
				//
				uint32 update_chunks_header_size = sizeof(s_simulation_debug_chunk) * g_simulation_debug_globals.update_queue.queued_count();
				s_simulation_debug_chunk* update_header_bucket = (s_simulation_debug_chunk*)CSERIES_MALLOC(update_chunks_header_size);
				uint32 update_chunks_size = NULL;

				s_simulation_debug_chunk* current_chunk = update_header_bucket;

				for (const c_debug_update_node* update_node = g_simulation_debug_globals.update_queue.get_first_element();
					update_node != nullptr;
					update_node = g_simulation_debug_globals.update_queue.get_next_element(update_node)
					)
				{
					ASSERT(current_chunk && update_node);

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
					event(_event_error, "cartographer:saved_film_write: failed to write header to saved film!");
					success = false;
				}

				if (g_simulation_debug_globals.captured_gamestate)
				{
					if (success && file_write(&g_simulation_debug_globals.save_file, sizeof(s_simulation_debug_chunk), &gamestate_chunk))
					{
						write_offset += sizeof(s_simulation_debug_chunk);
					}
					else
					{
						event(_event_error, "cartographer:saved_film_write: failed to write gamestate debug chunk header to film!");
						success = false;
					}
				}



				if (success && file_write(&g_simulation_debug_globals.save_file, update_chunks_header_size, update_header_bucket))
				{
					write_offset += update_chunks_header_size;
				}
				else
				{
					event(_event_error, "cartographer:saved_film_write: failed to write update chunks header to film!");
					success = false;
				}


				//check expected offset with actual offset
				ASSERT(write_offset == chunk_headers_end);

				if (g_simulation_debug_globals.captured_gamestate)
				{
					if (success && file_write(&g_simulation_debug_globals.save_file, out_gamestate_chunk_size, gamestate_temporary_buffer))
					{
						write_offset += out_gamestate_chunk_size;
					}
					else
					{
						event(_event_error, "cartographer:saved_film_write: failed to write compressed gamestate chunk to simulation file!");
						success = false;
					}
				}

				if (gamestate_temporary_buffer)
				{
					event(_event_status, "cartographer:saved_film_write: clearing gamestate_temporary_buffer ");
					CSERIES_FREE(gamestate_temporary_buffer);
				}

				//check expected offset with actual offset
				ASSERT(write_offset == chunk_headers_end + out_gamestate_chunk_size);


				current_chunk = update_header_bucket;
				for (const c_debug_update_node* update_node = g_simulation_debug_globals.update_queue.get_first_element();
					update_node != nullptr;
					update_node = g_simulation_debug_globals.update_queue.get_next_element(update_node)
					)
				{
					ASSERT(update_node != NULL);
					ASSERT(write_offset == current_chunk->file_offset);

					if (success && file_write(&g_simulation_debug_globals.save_file, update_node->data_size, update_node->data))
					{
						write_offset += update_node->data_size;
					}
					else
					{
						event(_event_error, "cartographer:saved_film_write: failed to write update debug chunk to simulation film!");
						success = false;
					}
					current_chunk++;
				}

				//check expected offset with actual offset
				ASSERT(write_offset == chunk_headers_end + out_gamestate_chunk_size + update_chunks_size);

				if (update_header_bucket)
				{
					event(_event_status, "cartographer:saved_film_write: clearing update_header chunks buffer ", __FUNCTION__);
					CSERIES_FREE(update_header_bucket);
				}


			}
			if (!file_set_eof(&g_simulation_debug_globals.save_file, write_offset))
			{
				event(_event_error, "cartographer:saved_film_write: failed to set saved_film size!", __FUNCTION__);
				success = false;
			}
		}

		if (open_file_error_code == _file_open_error_success)
		{
			file_close(&g_simulation_debug_globals.save_file);
		}

		//finished writing
		g_simulation_debug_globals.writing_file = false;
	}
	else
	{
		// cannot write without starting write
		event(_event_error, "cartographer:saved_film_write: has failed, not allowed to start writing");
		success = false;
	}

	return success;
}

bool debug_simulation_read_file_internal(void)
{
	bool success = true;

	if (debug_simulation_active()
		&& g_simulation_debug_globals.reading_file)
	{
		if (g_simulation_debug_globals.save_file_name.length() == NULL)
		{
			event(_event_error, "cartographer:saved_film_read: no name set for saved film!");
			success = false;
		}
		else
		{
			//
			// reading saved_film starts
			// 

			c_static_string<MAX_PATH> save_file_path;
			save_file_path.set(k_simulation_films_save_folder);
			save_file_path.append(g_simulation_debug_globals.save_file_name.get_string());
			save_file_path.append(k_simulation_films_file_extension);

			event(_event_message,
				"cartographer:saved_film_read:  reading saved film %s",
				save_file_path.get_string()
			);

			file_reference_create_from_path(&g_simulation_debug_globals.save_file, save_file_path.get_string(), false);
			e_file_open_error open_file_error_code = _file_open_error_unknown;
			if (!file_open(&g_simulation_debug_globals.save_file, _permission_read_bit, &open_file_error_code))
			{
				event(_event_error,
					"cartographer:saved_film_read: failed to open file for reading , error code: %d",
					open_file_error_code
				);
				success = false;
			}

			if (open_file_error_code == _file_open_error_success)
			{
				s_simulation_debug_file_header debug_file_header;
				// first we read the header
				if (!success || !file_read(&g_simulation_debug_globals.save_file, sizeof(s_simulation_debug_file_header), true, &debug_file_header))
				{
					event(_event_error, "cartographer:saved_film_read: failed to read film header!");
					success = false;
				}


				if (success)
				{
					uint32 file_size_disk = NULL;
					if (!file_get_size(&g_simulation_debug_globals.save_file, &file_size_disk))
					{
						event(_event_error, "cartographer:saved_film_read: file_get_size failed");
						success = false;
					}

					if (debug_file_header.file_size != file_size_disk)
					{
						event(_event_error, "cartographer:saved_film_read: stored size does not match size on disk! (file-size-mismatch)");
						success = false;
					}
				}

				if (!success || !debug_simulation_verify_header_internal(&debug_file_header))
				{
					event(_event_error, "cartographer:saved_film_read: failed to verify film header! (bad-header)");
					success = false;
				}

				if (success)
				{
					csmemcpy(&g_simulation_debug_globals.film_options, &debug_file_header.debug_game_options, sizeof(s_game_options));

					if (debug_file_header.debug_chunks_count > 0)
					{
						uint8 gamestate_chunk_count = NULL;
						uint32 update_chunk_count = NULL;
						//uint32 read_offset = sizeof(s_simulation_debug_file_header);
						uint32 read_offset = debug_file_header.header_size;//use read header size instead of latest
						uint32 debug_chunk_headers_size = sizeof(s_simulation_debug_chunk) * debug_file_header.debug_chunks_count;
						s_simulation_debug_chunk* debug_chunks = (s_simulation_debug_chunk*)CSERIES_MALLOC(debug_chunk_headers_size);

						ASSERT(debug_chunks != NULL);
						if (file_read(&g_simulation_debug_globals.save_file, debug_chunk_headers_size, false, debug_chunks))
						{
							event(_event_verbose, "- - > successfully read debug chunk headers , total count : %d", debug_file_header.debug_chunks_count);
							read_offset += debug_chunk_headers_size;
						}
						else
						{
							event(_event_error, "cartographer:saved_film_read: failed to read debug chunk headers from simulation film!");
							success = false;
						}

						//check expected offset with actual offset
						ASSERT(read_offset == debug_file_header.header_size + debug_chunk_headers_size);

						s_simulation_debug_chunk* current_chunk = debug_chunks;
						for (uint32 i = 0; i < debug_file_header.debug_chunks_count; i++)
						{
							//too much spam
							event(_event_verbose,
								"- - > reading debug chunk type/size/offset %d/%0x%08X/0x%08X",
								current_chunk->chunk_type,
								current_chunk->chunk_size,
								current_chunk->file_offset
							);

							if (VALID_INDEX(current_chunk->chunk_type, k_simulation_debug_chunk_types))
							{
								if (current_chunk->chunk_type == _debug_chunk_gamestate)
								{
									event(_event_verbose,
										" - - > found gamestate chunk size :0x%08X at offset :0x%08X",
										current_chunk->chunk_size,
										current_chunk->file_offset
									);
									gamestate_chunk_count++;
									debug_gamestate_read_from_chunk(current_chunk);

								}
								else if (current_chunk->chunk_type == _debug_chunk_update)
								{
									//too much spam
									event(_event_verbose,
										" - - > found update chunk size :0x%08X at offset :0x%08X",
										current_chunk->chunk_size,
										current_chunk->file_offset
									);
									update_chunk_count++;
									debug_update_read_from_chunk(current_chunk);
								}

								read_offset += current_chunk->chunk_size;
								current_chunk++;
							}
							else
							{
								event(_event_error, "cartographer:saved_film_read: found bad chunk at count %d , skipping!!", i);
							}
						}

						if (debug_chunks)
						{
							CSERIES_FREE(debug_chunks);
							event(_event_status, "cartographer:saved_film_read: clearing header chunks");
						}

						//check expected offset with actual offset
						ASSERT(read_offset == debug_file_header.header_size + debug_chunk_headers_size + debug_file_header.chunk_size);

						if (gamestate_chunk_count > k_simulation_debug_max_gamestates_allowed)
						{
							event(_event_warning,
								" warning saved film has too many gamestate chunks count/expected : %d/%d",
								gamestate_chunk_count,
								k_simulation_debug_max_gamestates_allowed
							);
						}

					}
					else
					{
						event(_event_warning, " saved film has no chunk entries!");
					}
				}

				file_close(&g_simulation_debug_globals.save_file);
			}

			// 
			// read_saved_film ends 
			//
		}

		//reading finished
		g_simulation_debug_globals.reading_file = false;
	}
	else
	{
		// cannot read without starting reading
		event(_event_error, "cartographer:saved_film_read: has failed, not allowed to start reading");
		success = false;
		return false;

	}
	return success;
}

bool debug_simulation_verify_header_internal(s_simulation_debug_file_header* header)
{
	bool result = true;
	if (header->signature != k_simulation_debug_header_signature
		|| header->eof_signature != k_simulation_debug_header_eof_signature)
	{
		event(_event_error, "cartographer:saved_film_read: invalid file signature! ");
		result = false;
	}

	if (header->header_size != sizeof(s_simulation_debug_file_header))
	{
		event(_event_warning, "cartographer:saved_film_read: simulation film is bad or outdated! (version-mismatch) , expected %d", sizeof(s_simulation_debug_file_header));
		result = true;
	}

	if (header->file_size !=
		(header->header_size + header->chunk_size + header->debug_chunks_count * sizeof(s_simulation_debug_chunk)))
	{
		event(_event_error, "cartographer:saved_film_read: invalid file (file-size-mismatch) ");
		result = false;
	}

	if (header->debug_chunks_count != header->game_saves_count + header->game_updates_count)
	{
		event(_event_error, "cartographer:saved_film_read: invalid saved chunks count detected");
		result = false;
	}

	if (header->game_saves_count < 1)
	{
		event(_event_warning, "cartographer:saved_film_read: warning no gamesaves found in this film");
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

	real32 seconds = game_time_get_speed() * 0.25f; // figure out why h3 does this multiplier
	int32 updates_required = game_seconds_to_ticks_round(seconds);

	c_simulation_world* world = simulation_get_world();
	bool match_remote_time = false;
	if (!world->time_get_available(&match_remote_time))
	{
		updates_required = MAX(updates_required, 1);
	}
	updates_required = MIN(remaining_updates, updates_required);

	bool succesfully_fetched = true;
	int32 updates_fetched = NULL;

	ASSERT(g_simulation_debug_globals.playback_buffer != nullptr);
	struct simulation_update* message = (struct simulation_update*)g_simulation_debug_globals.playback_buffer;
	do
	{
		if (updates_fetched >= k_simulation_debug_max_allowed_updates_to_fetch)
			break;

		int32 available_updates = world->time_get_available(&match_remote_time);
		if (available_updates >= updates_required)
			break;

		// fetch updates and push to queue
		if (debug_update_retrieve_latest_update(message))
		{
			if (debug_simulation_replay_should_reset_time())
			{
				event(_event_message,
					"cartographer:saved_film: world time is being reset to update :[#%d] , game_time : %d",
					message->update_number,
					message->verify_game_time
				);
				world->time_stop();
				world->time_start(message->update_number);

				g_simulation_debug_globals.replay_reset_time = false;
			}

			updates_fetched++;
			if (!simulation_get_world()->update_queue_handle_server_update(message))
			{
				succesfully_fetched = false;
				event(_event_error,
					"cartographer:saved_film: failed to insert update [#%d] into world at time %d , debugging required!!",
					message->update_number,
					world->get_time()
				);
			}
		}

	} while (succesfully_fetched);

	*updates_read_out = updates_fetched;
	return succesfully_fetched;
}


void debug_simulation_generate_default_header_internal(s_simulation_debug_file_header* header)
{
	csmemset(header, 0, sizeof(s_simulation_debug_file_header));
	header->signature = k_simulation_debug_header_signature;
	header->header_size = sizeof(s_simulation_debug_file_header);
	header->build.clear();
	header->build_time.clear();

#if defined(GEN_GIT_VER_VERSION_STRING) 
	{
		swprintf(header->build.get_buffer(), header->build.max_length(), L"%S.%S.%S", GEN_GIT_VER_VERSION_STRING, GET_GIT_VER_USERNAME, GET_GIT_VER_BRANCH);
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


	header->eof_signature = k_simulation_debug_header_eof_signature;
}

void debug_simulation_create_folders_internal()
{
	s_file_reference save_folder;
	file_reference_create_from_path(&save_folder, k_simulation_films_save_folder, true);
	file_create_parent_directories_if_not_present(&save_folder);
	file_close(&save_folder);
}

void debug_simulation_timestamp_internal(c_static_string<32>* timestamp)
{
	time_t timer = time(NULL);
	tm tm_info;
	localtime_s(&tm_info, &timer);
	strftime(timestamp->get_buffer(), timestamp->max_length(), "%Y%m%d-%H%M%S", &tm_info);
}