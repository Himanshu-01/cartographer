#include "stdafx.h"
#include "simulation_world.h"
#include "simulation.h"

#include "simulation_queue_events.h"
#include "simulation_queue_entities.h"
#include "simulation_queue_global_events.h"

#include "game/game.h"
#include "game/game_time.h"
#include "math/random_math.h"
#include "saved_games/game_state_procs.h"
#include "shell/shell_windows.h"

// TODO verify if these buffers get saturated quickly
// if that's the case, increse the buffer size
c_simulation_queue g_simulation_queues[k_simulation_queue_count];

void c_simulation_world::gamestate_flush_immediate(void)
{
	if (!is_authority())
	{
		game_state_call_before_save_procs(0);
		game_state_call_after_save_procs(0);
	}
	return;
}

c_simulation_queue* c_simulation_world::queue_get(e_simulation_queue_type type) const
{
	return &g_simulation_queues[type];
}

void c_simulation_world::simulation_queue_allocate(e_event_queue_type type, int32 size, s_simulation_queue_element** out_allocated_elem)
{
	*out_allocated_elem = NULL;

	if (TEST_FLAG(FLAG(type), _simulation_queue_element_type_bookkeeping))
	{
		// player event, player update, gamestate clear
		queue_get(_simulation_queue_bookkeeping)->allocate(size, out_allocated_elem);
	}
	else
	{
		bool sim_queue_restrict_allocations = false;
		c_simulation_queue* simulation_queue = queue_get(_simulation_queue);

		if (!TEST_FLAG(FLAG(type), _simulation_queue_element_important_update))
		{
			real32 allocated_percentage;
			real32 allocated_in_bytes_percentage;
			simulation_queue->get_allocation_status(&allocated_percentage, &allocated_in_bytes_percentage);

			// if we allocated more than 90% of the buffer
			// skip some updates to aleviate some of the stress on the queue
			// especially if the game froze for multiple seconds
			// and allow the allocation for important updates only
			// entity deletion, entity promotion, and global game events
			if (allocated_percentage > 90.f / 100.f
				|| allocated_in_bytes_percentage > 90.f / 100.f)
			{
				sim_queue_restrict_allocations = true;
			}
		}

		// event, creation, update, entity_deletion, entity_promotion, game_global_event
		if (!sim_queue_restrict_allocations)
			simulation_queue->allocate(size, out_allocated_elem);
	}

	if (*out_allocated_elem)
	{
		(*out_allocated_elem)->type = type;
	}
}

void c_simulation_world::simulation_queue_free(s_simulation_queue_element* element)
{
	if (TEST_FLAG(FLAG(element->type), _simulation_queue_element_type_bookkeeping))
	{
		// player event, player update, gamestate clear
		queue_get(_simulation_queue_bookkeeping)->deallocate(element);
	}
	else
	{
		queue_get(_simulation_queue)->deallocate(element);
	}
}

void c_simulation_world::simulation_queue_enqueue(s_simulation_queue_element* element)
{
	if (TEST_FLAG(FLAG(element->type), _simulation_queue_element_type_bookkeeping))
	{
		// player event, player update, gamestate clear
		queue_get(_simulation_queue_bookkeeping)->enqueue(element);

		SIM_EVENT_QUEUE_DBG("queue 0x%08X allocated count: %d, size: %d",
			&g_simulation_queues[_simulation_queue_bookkeeping],
			g_simulation_queues[_simulation_queue_bookkeeping].allocated_count(),
			g_simulation_queues[_simulation_queue_bookkeeping].allocated_size_in_bytes());

		SIM_EVENT_QUEUE_DBG("queue 0x%08X queued count: %d, size: %d",
			&g_simulation_queues[_simulation_queue_bookkeeping],
			g_simulation_queues[_simulation_queue_bookkeeping].queued_count(),
			g_simulation_queues[_simulation_queue_bookkeeping].queued_size());
	}
	else
	{
		// event, creation, update, entity_deletion, entity_promotion, game_global_event

		queue_get(_simulation_queue)->enqueue(element);

		SIM_EVENT_QUEUE_DBG("queue 0x%08X allocated count: %d, size: %d",
			&g_simulation_queues[_simulation_queue],
			g_simulation_queues[_simulation_queue].allocated_count(),
			g_simulation_queues[_simulation_queue].allocated_size_in_bytes());

		SIM_EVENT_QUEUE_DBG("queue 0x%08X queued count: %d, size: %d",
			&g_simulation_queues[_simulation_queue],
			g_simulation_queues[_simulation_queue].queued_count(),
			g_simulation_queues[_simulation_queue].queued_size());
	}
}

void c_simulation_world::apply_simulation_queue(const c_simulation_queue* queue, simulation_update* update)
{
	if (queue->queued_count() > 0)
	{
		const s_simulation_queue_element* element = queue->get_first_element();

		while (element != NULL)
		{
			SIM_EVENT_QUEUE_DBG("appying element: %08X, type: %d to gamestate",
				element,
				element->type);

			switch (element->type)
			{
			case _simulation_queue_element_type_event:
				simulation_queue_event_apply(element);
				break;
			case _simulation_queue_element_type_entity_creation:
				simulation_queue_entity_creation_apply(element);
				break;
			case _simulation_queue_element_type_entity_update:
				simulation_queue_entity_update_apply(element);
				break;
			case _simulation_queue_element_type_entity_deletion:
				simulation_queue_entity_deletion_apply(element);
				break;
			case _simulation_queue_element_type_entity_promotion:
				simulation_queue_entity_promotion_apply(element);
				break;
			case _simulation_queue_element_type_game_global_event:
				simulation_queue_game_global_event_apply(element, update);
				break;
			case _simulation_queue_element_type_player_event:
				simulation_queue_player_event_apply(element);
				break;
			case _simulation_queue_element_type_player_update_event:
				if (!simulation_get_globals()->fatal_error)
					simulation_queue_player_update_apply(element);
				break;
			case _simulation_queue_element_type_gamestates_clear:
				break;
				//case _simulation_queue_element_type_sandbox_event:
					//break;
			default:
				// DEBUG unk event type
				break;
			}

			element = queue->get_next_element(element);
		}
	}
}

void c_simulation_world::attach_simulation_queues_to_update(
	bool simulation_in_progress,
	c_simulation_queue* out_bookkeepin_queue,
	c_simulation_queue* out_game_simulation_queue)
{
	out_bookkeepin_queue->transfer_elements(queue_get(_simulation_queue_bookkeeping));
	out_game_simulation_queue->transfer_elements(queue_get(_simulation_queue));
}

void c_simulation_world::queues_clear()
{
	for (int32 i = 0; i < k_simulation_queue_count; i++)
	{
		queue_get((e_simulation_queue_type)i)->clear();
	}
}

typedef void(__thiscall* t_c_simulation_world__initialize_world)(c_simulation_world*, int32, int32, int32);
t_c_simulation_world__initialize_world p_c_simulation_world__initialize_world;

void c_simulation_world::initialize_world(int32 a2, int32 a3, int32 a4)
{
	p_c_simulation_world__initialize_world(this, a2, a3, a4);
	if (!is_playback())
	{
		queues_initialize();

		if (m_world_type == _simulation_world_type_synchronous_authority)
		{
			//only used by synchronous-authority but we will initialize this here
			s_network_message_synchronous_update* authority_message = simulation_get_synchronous_message();
			csmemset(&authority_message->update, 0, sizeof(authority_message->update));
			authority_message->game_simulation_queue.initialize();
			authority_message->simulation_bookkeeping_queue.initialize();
		}
	}
}

void __declspec(naked) jmp_initialize_world() { __asm { jmp c_simulation_world::initialize_world } }


void c_simulation_world::delete_all_actors(void)
{
	for (uint32 i = 0; i < NUMBEROF(m_actors); i++)
	{
		c_simulation_actor* actor = &m_actors[i];
		if (actor->m_actor_index != NONE)
		{
			actor->destroy();
		}
	}
	return;
}

void c_simulation_world::update_queue_reset(void)
{
	typedef void(__thiscall* update_queue_reset_t)(c_simulation_world*);
	INVOKE_TYPE(0x1DCDC3, 0x1C4277, update_queue_reset_t, this);
	return;
}

void c_simulation_world::reset_world(void)
{
	m_time_immediate_update = false;
	m_out_of_sync = false;
	m_flush_gamestate = false;
	if (this->is_distributed())
	{
		m_distributed_world->m_entity_manager.reset();
		m_distributed_world->m_event_manager.reset();
		m_distributed_world->m_entity_database.reset();
		m_distributed_world->m_event_handler.reset();
		this->delete_all_actors();
	}

	if (!is_playback())
	{
		// during reset, discard just simulation updates
		// not bookkeeping updates
		queue_get(_simulation_queue)->clear();
	}

	if (!runs_simulation())
	{
		this->update_queue_reset();
	}

	return;
}

__declspec(naked) void jmp_reset_world() { __asm { jmp c_simulation_world::reset_world } }

typedef void(__thiscall* t_c_simulation_world__destroy_world)(c_simulation_world*);
t_c_simulation_world__destroy_world p_c_simulation_world__destroy_world;

void c_simulation_world::destroy_world(void)
{
	// call orig
	p_c_simulation_world__destroy_world(this);

	if (!is_playback())
	{
		queues_dispose();
	}
}

void __declspec(naked) jmp_destroy_world() { __asm { jmp c_simulation_world::destroy_world } }

void c_simulation_world::disconnect(void)
{
	return;
}

void c_simulation_world::send_player_acknowledgements_not_during_simulation_reset_in_progress(bool a1)
{
	if (!simulation_reset_in_progress())
	{
		send_player_acknowledgements(a1);
	}
}

void __declspec(naked) jmp_send_player_acknowledgements_not_during_simulation_reset_in_progress() {
	__asm { jmp c_simulation_world::send_player_acknowledgements_not_during_simulation_reset_in_progress }
}

void c_simulation_world::queues_initialize()
{
	for (int32 i = 0; i < k_simulation_queue_count; i++)
	{
		queue_get((e_simulation_queue_type)i)->initialize();
	}
}

void c_simulation_world::queues_dispose(void)
{
	for (int32 i = 0; i < k_simulation_queue_count; i++)
	{
		queue_get((e_simulation_queue_type)i)->dispose();
	}
}

void c_simulation_world::create_player(datum player_index)
{
	typedef void(__thiscall* create_player_t)(c_simulation_world*, datum);
	INVOKE_TYPE(0x1DC05C, 0x1C3511, create_player_t, this, player_index);
	return;
}

void c_simulation_world::delete_player(datum player_index)
{
	typedef void(__thiscall* delete_player_t)(c_simulation_world*, datum);
	INVOKE_TYPE(0x1DC124, 0x1C35D8, delete_player_t, this, player_index);
	return;
}

//typedef void(__thiscall* t_c_simulation_world__update_queue_retrieve_update)(c_simulation_world*, simulation_update*);
//t_c_simulation_world__update_queue_retrieve_update p_c_simulation_world__update_queue_retrieve_update;

simulation_update last_update;
void c_simulation_world::update_queue_retrieve_update(simulation_update* update)
{
	//INVOKE_TYPE(0x1DCE7C, 0x0, void(__thiscall*)(c_simulation_world*, simulation_update*), this, update);
	if (do_we_have_sufficient_updates())
	{
		s_simulation_update_node* update_node = this->m_synchronous_client_queue_head;
		ASSERT(update_nodee);

		csmemcpy(update, &update_node->update_message.update, sizeof(simulation_update));
		// transfer these to c_simulation_world so it can apply them in simulation_apply_before_game
		this->queue_get(_simulation_queue_bookkeeping)->duplicate(&update_node->update_message.simulation_bookkeeping_queue);
		this->queue_get(_simulation_queue)->duplicate(&update_node->update_message.game_simulation_queue);

		bool v4 = this->m_synchronous_client_queue_tail == update_node;
		this->m_synchronous_client_queue_head = update_node->next;
		if (v4)
			this->m_synchronous_client_queue_tail = nullptr;

		update_node->update_message.simulation_bookkeeping_queue.dispose();
		update_node->update_message.game_simulation_queue.dispose();
		network_heap_free_block((uint8*)update_node);


		--this->m_synchronous_client_queue_length;
		++this->m_synchronous_client_next_update_number_to_dequeue;

		csmemcpy(&last_update, update, sizeof(simulation_update));
	}
	else
	{
		LOG_CRITICAL_FUNC("using last update as backup");
		csmemcpy(update, &last_update, sizeof(simulation_update));
		update->simulation_in_progress = false;

	}

}
void __declspec(naked) jmp_update_queue_retrieve_update() {
	__asm { jmp c_simulation_world::update_queue_retrieve_update }
}

bool c_simulation_world::update_queue_handle_server_update(s_network_message_synchronous_update* host_update)
{
	//return INVOKE_TYPE(0x1DCE0F, 0x0, bool(__thiscall*)(c_simulation_world*, simulation_update*), this, host_update);
	s_simulation_update_node* update_node = (s_simulation_update_node*)network_heap_allocate_block(sizeof(s_simulation_update_node));
	if (update_node == nullptr)
		return false;

	s_simulation_update_node* tail = this->m_synchronous_client_queue_tail;
	if (tail)
		tail->next = update_node;
	else
		this->m_synchronous_client_queue_head = update_node;

	update_node->next = nullptr;

	++this->m_synchronous_client_queue_length;
	this->m_synchronous_client_queue_tail = update_node;

	csmemcpy(&update_node->update_message, host_update, sizeof(s_network_message_synchronous_update));
	//update_node->update_message.game_simulation_queue.duplicate(&host_update->game_simulation_queue);
	//update_node->update_message.simulation_bookkeeping_queue.duplicate(&host_update->simulation_bookkeeping_queue);
	
	this->m_synchronous_client_latest_update_number_received = host_update->update.simulation_time;

	return true;
}
void __declspec(naked) jmp_update_queue_handle_server_update() {
	__asm { jmp c_simulation_world::update_queue_handle_server_update }
}


void c_simulation_world::iterator_begin(s_simulation_world_view_iterator * iterator, uint32 view_type_mask)
{
	//INVOKE(0x1DBFF9, 0x0, c_simulation_world::iterator_begin, iterator, view_type_mask);

	ASSERT(view_type_mask == NONE || (view_type_mask != 0 && VALID_BITS(view_type_mask, k_simulation_view_type_count)));

	iterator->m_type_mask = view_type_mask;
	iterator->m_last_view_index = 0;
}

bool c_simulation_world::iterator_next(s_simulation_world_view_iterator* iterator, c_simulation_view** view)
{
	//return INVOKE_TYPE(0x1DC00D, 0x0, bool(__thiscall*)(c_simulation_world*, s_simulation_world_view_iterator*, c_simulation_view**), this, iterator, view);
	while (iterator->m_last_view_index >= 0)
	{
		datum index = iterator->m_last_view_index;
		if (index >= k_maximum_players)
			break;
		
		c_simulation_view* test_view = this->m_views[index];
		iterator->m_last_view_index = index + 1;

		//if (v6 && ((1 << LOBYTE(v6->m_view_type)) & iterator->m_type_mask) != 0)
		if (test_view && TEST_BIT(iterator->m_type_mask, test_view->view_type()))
		{
			*view = test_view;
			return true;
		}
	}
	return false;

}

c_simulation_view* c_simulation_world::get_client_view_by_machine_index(uint32 machine_index)
{
	//return INVOKE_TYPE(0x1DCEFC, 0x0, c_simulation_view * (__thiscall*)(c_simulation_world*, uint32), this, machine_index);
	
	s_simulation_world_view_iterator iterator;
	const uint32 client_mask = FLAG(_simulation_view_type_synchronous_to_remote_client) | FLAG(_simulation_view_type_distributed_to_remote_client);
	iterator_begin(&iterator, client_mask);

	c_simulation_view* test_view;
	if (!this->iterator_next(&iterator, &test_view))
		return nullptr;

	while (true)
	{
		if (test_view->get_machine_index() == machine_index)
			break;
		if (!this->iterator_next(&iterator, &test_view))
			return nullptr;
	}
	return test_view;
}

c_simulation_view* c_simulation_world::get_client_view_by_machine_identifier(s_machine_identifier* machine_identifier)
{
	//return INVOKE_TYPE(0x1DCF5F, 0x0, c_simulation_view * (__thiscall*)(c_simulation_world*, s_machine_identifier*), this, machine_identifier);

	s_simulation_world_view_iterator iterator;
	const uint32 client_mask = FLAG(_simulation_view_type_synchronous_to_remote_client) | FLAG(_simulation_view_type_distributed_to_remote_client);
	iterator_begin(&iterator, client_mask);

	c_simulation_view* test_view;
	if (this->iterator_next(&iterator, &test_view))
		return nullptr;

	while (true)
	{
		s_machine_identifier identifiers;
		test_view->get_machine_identifier(&identifiers);
		if (memcmp(identifiers.machine_identifier,machine_identifier->machine_identifier,sizeof(s_machine_identifier)))
			break;
		if (!this->iterator_next(&iterator, &test_view))
			return nullptr;
	}
	return test_view;

}

c_simulation_view* c_simulation_world::get_authority_view()
{
	//return INVOKE_TYPE(0x1DCED3, 0x0, c_simulation_view * (__thiscall*)(c_simulation_world*), this);

	s_simulation_world_view_iterator iterator;
	const uint32 authority_mask = FLAG(_simulation_view_type_synchronous_to_remote_authority) | FLAG(_simulation_view_type_distributed_to_remote_authority);
	iterator_begin(&iterator, authority_mask);

	c_simulation_view* test_view;
	this->iterator_next(&iterator, &test_view);
	return test_view;
}




void simulation_world_apply_patches()
{
	DETOUR_ATTACH(p_c_simulation_world__initialize_world, Memory::GetAddress<t_c_simulation_world__initialize_world>(0x1DDB4E, 0x1C500E), jmp_initialize_world);
	DETOUR_ATTACH(p_c_simulation_world__destroy_world, Memory::GetAddress<t_c_simulation_world__destroy_world>(0x1DE0A9, 0x1C5569), jmp_destroy_world);

	PatchCall(Memory::GetAddress(0x1AE82A, 0x1A8A84), jmp_reset_world);
	PatchCall(Memory::GetAddress(0x1DD9FB, 0x1C4EBB), jmp_send_player_acknowledgements_not_during_simulation_reset_in_progress);
	PatchCall(Memory::GetAddress(0x1DD273), jmp_update_queue_retrieve_update);
	PatchCall(Memory::GetAddress(0x1DD464), jmp_update_queue_handle_server_update);
	return;
}
