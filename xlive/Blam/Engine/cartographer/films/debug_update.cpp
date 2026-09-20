#include "stdafx.h"
#include "debug_update.h"
#include "debug_simulation_globals.h"
#include "debug_update_queue.h"

#include "cseries/cseries_system_memory.h"
#include "networking/network_event.h"
#include "simulation/simulation.h"
#include "simulation/simulation_update.h"


/* constants */

enum
{
	k_simulation_debug_update_buffer_size = 0xFFFF
};

/* enums */

/* structures */

/* globals */

/* prototypes */

bool debug_update_write_to_buffer(struct simulation_update* update, uint32 buffer_len, uint8* buffer, int32* out_size);
bool debug_update_read_from_buffer(struct simulation_update* message, uint32 data_len, uint8* buffer);
bool debug_update_record_from_buffer(uint8* buffer, uint32 buffer_len);


/* public code */

bool debug_update_record_update(struct simulation_update* update)
{
	bool result = false;

	if (debug_simulation_active()
		&& debug_simulation_is_recording()
		&& debug_simulation_recording_allows_update())
	{

		uint8* buffer = (uint8*)CSERIES_MALLOC(k_simulation_debug_update_buffer_size);
		ASSERT(buffer);

		int32 out_size = 0;
		if (debug_update_write_to_buffer(update, k_simulation_debug_update_buffer_size, buffer, &out_size))
		{
			if (debug_update_record_from_buffer(buffer, out_size))
			{
				//success
				g_simulation_debug_globals.current_recording_tick = update->verify_game_time;
				event(_event_verbose,
					"cartographer:saved_film:update: successfully recorded update #%d",
					update->update_number
				);
				result = true;
			}
			else
			{
				//failed in recording update
				event(_event_error,
					"cartographer:saved_film:update: failed to record update from buffer #%d",
					update->update_number
				);

				result = false;
			}
		}
		else
		{
			// some issue in encoding
			event(_event_error,
				"cartographer:saved_film:update: failed to encode simulation_update to buffer #%d",
				update->update_number
			);
			result = false;
		}

		if (buffer)
			CSERIES_FREE(buffer);
	}
	else
	{
		//cannot record in current state {init,record_allowed,update_allowed}
		event(_event_status, "cartographer:saved_film:update: not allowed to record update");
		result = false;
	}
	return result;
}

bool debug_update_retrieve_latest_update(struct simulation_update* out_update)
{
	ASSERT(out_update);

	bool result = false;
	if (debug_simulation_active()
		&& debug_simulation_is_replaying())
	{
		if (debug_simulation_replay_has_updates()) 
		{
			c_debug_update_node* latest_node = nullptr;
			g_simulation_debug_globals.update_queue.dequeue(&latest_node);

			if (latest_node)
			{
				if (debug_update_read_from_buffer(out_update, latest_node->data_size, latest_node->data))
				{
					//success
					event(_event_verbose,
						"cartographer:saved_film:update: successfully decoded simulation_update #%d",
						out_update->update_number);
					result = true;
				}
				else
				{
					// failed to decode update
					event(_event_error, "cartographer:saved_film:update: failed to decode fetched update");
					result = false;
				}

				if (latest_node->allocated)
				{
					// free the allocated data
					g_simulation_debug_globals.update_queue.deallocate(latest_node);
				}
			}
			else
			{
				//failed to fetch update
				event(_event_error, "cartographer:saved_film:update: failed to dequeue saved update");
				result = false;
			}
		}
		else
		{
			event(_event_message, "cartographer:saved_film:update: no more updates left to retreive");
			result = false;
		}

	}
	else
	{
		//cannot replay updates in current state {init,replay_allowed}
		int32 leftover_updates = g_simulation_debug_globals.update_queue.queued_count();
		if (leftover_updates > 0)
		{
			event(_event_error,
				"cartographer:saved_film:update: not allowed to retrieve updates from queue , leftover count : %d",
				leftover_updates
			);

		}
		result = false;
	}

	return result;
}

void debug_update_read_from_chunk(s_simulation_debug_chunk* chunk)
{
	if (debug_simulation_active())
	{
		uint8* update_buffer = (uint8*)CSERIES_MALLOC(chunk->chunk_size);
		ASSERT(update_buffer);

		if (file_read_from_position(&g_simulation_debug_globals.save_file, chunk->file_offset, chunk->chunk_size, false, update_buffer))
		{
			if (debug_update_record_from_buffer(update_buffer, chunk->chunk_size))
			{
				event(_event_verbose, "cartographer:saved_film:update: successfully inserted update from saved chunk");
				
			}
			else
			{
				event(_event_error, "cartographer:saved_film:update: failed to queue update data for replay!");
			}

		}
		else
		{
			event(_event_error, "cartographer:saved_film:update: debug_update_read_from_chunk failed in file_read_from_position!");
		}


		if (update_buffer)
		{
			CSERIES_FREE(update_buffer);
		}
	}
}

void debug_update_queue_initialize_for_load()
{
	if (debug_simulation_active())
	{
		if (!g_simulation_debug_globals.update_queue.initialized())
		{
			g_simulation_debug_globals.update_queue.initialize();
		}

		if (debug_simulation_is_recording())
		{
			debug_update_queue_clear();
		}
		else
		{
			// we are the replay , so do nothing?
		}
	}
}

void debug_update_queue_clear()
{
	g_simulation_debug_globals.update_queue.clear();
}

void debug_update_queue_dispose()
{
	g_simulation_debug_globals.update_queue.dispose();
}

void debug_update_memory_initialize_for_playback()
{
	if (debug_simulation_active() 
		&& debug_simulation_is_replaying()
		&& g_simulation_debug_globals.playback_buffer == nullptr)
	{
		uint8* heap = (uint8*)CSERIES_MALLOC(sizeof(struct simulation_update));
		if (heap)
		{
			g_simulation_debug_globals.playback_buffer = heap;
			event(_event_status, "cartographer:saved_film:update: allocated playback update buffer");
		}
		else
		{
			event(_event_error, "cartographer:saved_film:update: failed to allocate playback update buffer");
		}
	}
}

void debug_update_memory_clear()
{
	if (g_simulation_debug_globals.playback_buffer != nullptr)
	{
		event(_event_status, "cartographer:saved_film:update: clearing playback update storage");
		CSERIES_FREE(g_simulation_debug_globals.playback_buffer);
		g_simulation_debug_globals.playback_buffer = nullptr;
	}
}


/* private code */

bool debug_update_write_to_buffer(struct simulation_update* update, uint32 buffer_len, uint8* buffer, int32* out_size)
{
	ASSERT(update);
	ASSERT(buffer_len > 0);
	ASSERT(buffer);
	ASSERT(out_size);

	return simulation_update_write_to_buffer(update, buffer_len, buffer, out_size);
}

bool debug_update_read_from_buffer(struct simulation_update* message, uint32 data_len, uint8* buffer)
{
	ASSERT(message);
	ASSERT(data_len > 0);
	ASSERT(buffer);

	return simulation_update_read_from_buffer(message, data_len, buffer);
}

bool debug_update_record_from_buffer(uint8* buffer, uint32 buffer_len)
{
	ASSERT(buffer);
	bool result = false;

	if (debug_simulation_active())
	{
		c_debug_update_node* new_element = nullptr;
		g_simulation_debug_globals.update_queue.allocate(buffer_len, &new_element);
		if (new_element)
		{
			csmemcpy(new_element->data, buffer, buffer_len);
			g_simulation_debug_globals.update_queue.enqueue(new_element);
			result = true;
		}
		else
		{
			// error in allocation
			event(_event_error, "cartographer:saved_film:update: failed to allocate new element");
			result = false;
		}
	}
	return result;
}
