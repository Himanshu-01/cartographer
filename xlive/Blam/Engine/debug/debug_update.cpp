#include "stdafx.h"
#include "debug_update.h"
#include "debug_simulation_globals.h"
#include "debug_simulation_constants.h"
#include "cseries/debug_memory.h"
#include "simulation/simulation_encoding.h"
#include "Networking/messages/network_messages_simulation_synchronous.h"


//internal functions
bool debug_update_write_to_buffer(s_network_message_synchronous_update* update, uint32 buffer_len, uint8* buffer, uint32* out_size)
{
	return synchronous_update_write_to_buffer(update, buffer_len, buffer, out_size);
}

bool debug_update_read_from_buffer(s_network_message_synchronous_update* message, uint32 data_len, uint8* buffer)
{
	return synchronous_update_read_from_buffer(message, data_len, buffer);
}

// public code

c_debug_update_queue::c_debug_update_queue()
{
	initialize();
}

c_debug_update_queue::~c_debug_update_queue()
{
	dispose();
}

bool c_debug_update_queue::initialized() const
{
	return m_initialized;
}

const c_debug_update_node* c_debug_update_queue::get_head() const
{
	if (initialized())
	{
		return m_head;
	}
	return NULL;
}

const c_debug_update_node* c_debug_update_queue::get_first_element() const
{
	if (initialized())
	{
		return get_head();
	}
	return NULL;
}

const c_debug_update_node* c_debug_update_queue::get_next_element(const c_debug_update_node* element) const
{
	if (initialized())
	{
		return element->next;
	}
	return NULL;
}

int32 c_debug_update_queue::allocated_count() const
{
	if (initialized())
	{
		return m_allocated_count;
	}

	return 0;
}

int32 c_debug_update_queue::allocated_size_in_bytes() const
{
	if (initialized())
	{
		return m_allocated_size_in_bytes;
	}

	return 0;
}

int32 c_debug_update_queue::get_element_size_in_bytes(const c_debug_update_node* element) const
{
	return element->data_size + sizeof(c_debug_update_node);
}

int32 c_debug_update_queue::queued_count() const
{
	if (initialized())
	{
		return m_queued_count;
	}

	return 0;
}

int32 c_debug_update_queue::queued_size() const
{
	if (initialized())
	{
		return m_queued_size;
	}

	return 0;
}

void c_debug_update_queue::initialize()
{
	m_allocated_count = 0;
	m_allocated_size_in_bytes = 0;
	m_queued_count = 0;
	m_queued_size = 0;
	m_head = NULL;
	m_tail = NULL;
	m_initialized = true;
}

//allocate memory for encoded sync-update
void c_debug_update_queue::allocate(int32 data_size, c_debug_update_node** out_allocated_elem)
{
	*out_allocated_elem = NULL;

	if (initialized())
	{
		uint32 required_data_size = sizeof(c_debug_update_node) + data_size;

		//if (allocated_count() + 1 < K_SIMULATION_DEBUG_MAX_UPDATES)
		//{
			uint8* heap_block = debug_malloc(required_data_size);
			if (heap_block)
			{
				csmemset(heap_block, 0, required_data_size);
				c_debug_update_node* allocated_elem = (c_debug_update_node*)heap_block;
				allocated_elem->data = heap_block + sizeof(c_debug_update_node);
				allocated_elem->data_size = data_size;
				allocated_elem->next = nullptr;
				allocated_elem->allocated = true;

				m_allocated_count++;
				m_allocated_size_in_bytes += required_data_size;
				*out_allocated_elem = allocated_elem;
			}
			else
			{
				// DEBUG
			}

		//}
	}
}


void c_debug_update_queue::deallocate(c_debug_update_node* element)
{
	if (initialized())
	{
		m_allocated_size_in_bytes -= get_element_size_in_bytes(element);
		m_allocated_count--;
		debug_free((uint8*)element);
	}
}


void c_debug_update_queue::enqueue(c_debug_update_node* element)
{
	if (initialized())
	{
		if (m_tail)
		{
			m_tail->next = element;
		}
		else
		{
			m_head = element;
		}

		m_tail = element;
		m_queued_count++;
		m_queued_size += get_element_size_in_bytes(element);

	}
}

void c_debug_update_queue::dequeue(c_debug_update_node** out_deq_elem)
{
	*out_deq_elem = NULL;
	if (initialized())
	{
		if (m_head)
		{
			m_queued_count--;
			m_queued_size -= get_element_size_in_bytes(m_head);
			*out_deq_elem = m_head;
			m_head = m_head->next;
			(*out_deq_elem)->next = NULL;
		}
		if (m_head == NULL)
		{
			m_tail = NULL;
		}
	}
}

void c_debug_update_queue::clear()
{
	if (initialized())
	{
		while (get_head() != NULL)
		{
			c_debug_update_node* element_to_deque = NULL;
			dequeue(&element_to_deque);
			deallocate(element_to_deque);
		}
	}
}

void c_debug_update_queue::dispose()
{
	if (initialized())
	{
		clear();
		m_initialized = false;
	}
}

bool debug_update_record_update(simulation_update* update, c_simulation_queue* bookkeeping, c_simulation_queue* game)
{
	if (!debug_simulation_active()
		|| !debug_simulation_is_recording()
		|| !debug_simulation_recording_allows_update())
	{
		return false;
	}

	s_network_message_synchronous_update message;
	csmemcpy(&message.update, update, sizeof(simulation_update));
	csmemcpy(&message.simulation_bookkeeping_queue, bookkeeping, sizeof(c_simulation_queue));
	csmemcpy(&message.game_simulation_queue, game, sizeof(c_simulation_queue));

	g_simulation_debug_globals.current_recording_tick = update->game_time_ticks;
	return debug_update_record_update(&message);
}

bool debug_update_record_update(s_network_message_synchronous_update* update)
{
	bool result = false;

	if (debug_simulation_active() 
		&& debug_simulation_is_recording()
		&& debug_simulation_recording_allows_update())
	{

		//uint8 buffer[K_SIMULATION_DEBUG_UPDATE_BUFFER_SIZE];
		uint8* buffer = debug_malloc(K_SIMULATION_DEBUG_UPDATE_BUFFER_SIZE);
		uint32 out_size = NULL;
		if (debug_update_write_to_buffer(update, K_SIMULATION_DEBUG_UPDATE_BUFFER_SIZE, buffer, &out_size))
		{
			if (debug_update_record_from_buffer(buffer, out_size))
			{
				g_simulation_debug_globals.current_recording_tick = update->update.game_time_ticks;
				//success
				result = true;
			}
			else
			{
				//failed in recording update
				result = false;
			}
		}
		else
		{
			// some issue in encoding
			result = false;
		}

		debug_free(buffer);
	}
	else
	{
		//cannot record in current state {init,record_allowed,update_allowed}
		result = false;
	}
	return result;
}

bool debug_update_record_from_buffer(uint8* buffer, uint32 buffer_len)
{
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
			result = false;
		}
	}
	return result;
}

bool debug_update_retrieve_latest_update(s_network_message_synchronous_update* out_update)
{
	ASSERT(out_update != nullptr);

	bool result = false;
	if (debug_simulation_active() && debug_simulation_is_replaying() && debug_simulation_replay_has_updates())
	{
		c_debug_update_node* latest_node = nullptr;
		g_simulation_debug_globals.update_queue.dequeue(&latest_node);

		if(latest_node)
		{
			if (debug_update_read_from_buffer(out_update,latest_node->data_size,latest_node->data))
			{
				//success
				result = true;
			}
			else
			{
				// failed to decode update
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
			result = false;
		}		

	}
	else
	{
		//cannot replay updates in current state {init,replay_allowed,update_queued_count}
		result = false;
	}

	return result;
}

void debug_update_read_from_chunk(s_simulation_debug_chunk* chunk)
{
	if (debug_simulation_active())
	{
		uint8* update_buffer = debug_malloc(chunk->chunk_size);
		if (file_read_from_position(&g_simulation_debug_globals.save_file, chunk->file_offset, chunk->chunk_size, false, update_buffer))
		{
			//LOG_TRACE_SIM("{} - successfully read update data from save file ", __FUNCTION__);
			if (!debug_update_record_from_buffer(update_buffer, chunk->chunk_size))
			{
				LOG_ERROR_SIM("{} - failed to record update data for replay!", __FUNCTION__);
			}

		}
		else
		{
			LOG_ERROR_SIM("{} - failed in file_read_from_position!", __FUNCTION__);
		}


		if (update_buffer)
		{
			debug_free(update_buffer);
			//LOG_TRACE_SIM("{} - clearing update buffer ", __FUNCTION__);
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
