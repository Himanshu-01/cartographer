#include "stdafx.h"
#include "simulation_world.h"

#include "simulation_queue_events.h"
#include "simulation_queue_entities.h"
#include "simulation_queue_global_events.h"

#include "simulation.h"
#include "simulation_update.h"
#include "simulation_encoding.h"
#include "simulation_watcher.h"

#include "game/game_time.h"
#include "math/random_math.h"
#include "memory/bitstream.h"
#include "networking/network_event.h"
#include "networking/network_memory.h"
#include "networking/network_time.h"
#include "networking/network_configuration.h"
#include "networking/Session/network_session.h"
#include "saved_games/game_state_procs.h"

#include "H2MOD/GUI/imgui_integration/Console/ImGui_ConsoleImpl.h"

enum
{
	k_simulation_world_maximum_synchronous_updates = 128
};


// TODO verify if these buffers get saturated quickly
// if that's the case, increse the buffer size
c_simulation_queue g_simulation_queues[k_simulation_queue_count];

static const char* get_state_string(
	int32 world_state)
{
	const char* result;

	switch (world_state)
	{
	case _simulation_world_state_none:
		result = "none";
		break;
	case _simulation_world_state_dead:
		result = "dead";
		break;
	case _simulation_world_state_disconnected:
		result = "disconnected";
		break;
	case _simulation_world_state_joining:
		result = "joining";
		break;
	case _simulation_world_state_active:
		result = "active";
		break;
	case _simulation_world_state_handoff:
		result = "handoff";
		break;
	case _simulation_world_state_leaving:
		result = "leaving";
		break;
	default:
		result = "<unknown>";
		break;
	}
	return result;
}

c_simulation_queue* c_simulation_world::queue_get(e_simulation_queue_type type) const
{
	return &g_simulation_queues[type];
}

void c_simulation_world::simulation_queue_allocate(e_event_queue_type type, int32 data_size, s_simulation_queue_element** out_allocated_elem)
{
	ASSERT(type != _simulation_queue_element_type_none);
	ASSERT(data_size > 0);
	ASSERT(out_allocated_elem != NULL);

	if (!is_playback())
	{

		int32 queued_count = 0;
		int32 queued_size_in_bytes = 0;
		int32 queued_encoded_size_in_bytes = 0;

		*out_allocated_elem = NULL;
		if (TEST_BIT(_simulation_queue_element_type_bookkeeping, type))
		{
			// player event, player update, gamestate clear
			c_simulation_queue* bookkeeping_queue = queue_get(_simulation_queue_bookkeeping);
			bookkeeping_queue->allocate(data_size, out_allocated_elem);

			queued_count = bookkeeping_queue->queued_count();
			queued_size_in_bytes = bookkeeping_queue->queued_size_in_bytes();
			queued_encoded_size_in_bytes = bookkeeping_queue->queued_encoded_size_in_bytes();
		}
		else
		{
			bool sim_queue_restrict_allocations = false;
			c_simulation_queue* simulation_queue = queue_get(_simulation_queue);

			if (!TEST_BIT(_simulation_queue_element_important_update, type))
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
					event(_event_fatal, "networking:simulation:world: game simulation queue in danger, restricting allocations [%f/%f]",
						allocated_percentage,
						allocated_in_bytes_percentage);

					sim_queue_restrict_allocations = true;
				}
			}

			// event, creation, update, entity_deletion, entity_promotion, game_global_event
			if (!sim_queue_restrict_allocations)
				simulation_queue->allocate(data_size, out_allocated_elem);

			queued_count = simulation_queue->queued_count();
			queued_size_in_bytes = simulation_queue->queued_size_in_bytes();
			queued_encoded_size_in_bytes = simulation_queue->queued_encoded_size_in_bytes();
		}

		if (*out_allocated_elem)
		{
			(*out_allocated_elem)->type = type;
		}
		else
		{
			event(_event_fatal, "networking:simulation:world: allocations are failing for simulation event queue [req. size %d] [count %d size %d enc. size %d]",
				data_size,
				queued_count,
				queued_size_in_bytes,
				queued_encoded_size_in_bytes);
		}
	}
}

void c_simulation_world::simulation_queue_free(s_simulation_queue_element* element)
{
	if (!game_is_playback())
	{
		if (TEST_BIT(_simulation_queue_element_type_bookkeeping, element->type))
		{
			// player event, player update, gamestate clear
			queue_get(_simulation_queue_bookkeeping)->deallocate(element);
		}
		else
		{
			queue_get(_simulation_queue)->deallocate(element);
		}
	}
}

void c_simulation_world::simulation_queue_enqueue(s_simulation_queue_element* element)
{
	ASSERT(element);
	ASSERT(element->type != _simulation_queue_element_type_none);
	ASSERT(element->data_size > 0);

	if (!is_playback())
	{
		if (TEST_BIT(_simulation_queue_element_type_bookkeeping, element->type))
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
				g_simulation_queues[_simulation_queue_bookkeeping].queued_size_in_bytes());
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
				g_simulation_queues[_simulation_queue].queued_size_in_bytes());
		}
	}
}

void c_simulation_world::apply_simulation_queue(
	const c_simulation_queue* simulation_queue)
{
	ASSERT(simulation_queue != NULL);

	if (simulation_queue->queued_count() > 0)
	{
		const s_simulation_queue_element* element = simulation_queue->get_first_element();
		int32 update_count = 0;
		int32 total_size = 0;

		// Added logic so we don't apply player updates if one of them fails
		bool apply_player_updates = true;

		while (element != NULL)
		{
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
				simulation_queue_game_global_event_apply(element);
				break;
			case _simulation_queue_element_type_player_event:
				simulation_queue_player_event_apply(element);
				break;
			case _simulation_queue_element_type_player_update_event:
				if (apply_player_updates && !simulation_queue_player_update_apply(element))
				{
					simulation_fatal_error();
					apply_player_updates = false;
				}
				break;
			case _simulation_queue_element_type_gamestates_clear:
				break;
			case _simulation_queue_element_type_sandbox_event:
				ASSERT(false);
				break;
			default:
				event(
					_event_error,
					"networking:simulation:world: apply_simulation_queue() unknown/invalid element type %d",
					element->type
				);
				break;
			}

			++update_count;
			total_size += simulation_queue->get_element_size_in_bytes(element);
			element = simulation_queue->get_next_element(element);
		}

		if (update_count != simulation_queue->queued_count())
		{
			event(
				_event_error,
				"networking:simulation:world: simulation queue from simulation update count mismatch [%d != %d]",
				update_count,
				simulation_queue->queued_count()
			);
		}

		if (total_size != simulation_queue->queued_size_in_bytes())
		{
			event(
				_event_error,
				"networking:simulation:world: simulation queue from simulation update size mismatch [%d != %d]",
				total_size,
				simulation_queue->queued_size_in_bytes()
			);
		}
	}
	return;
}

void c_simulation_world::attach_simulation_queues_to_update(
	struct simulation_update* update)
{
	ASSERT(update);

	c_simulation_queue* bookkeeping_simulation_queue = queue_get(_simulation_queue_bookkeeping);
	c_simulation_queue* game_simulation_queue = queue_get(_simulation_queue);

	if (bookkeeping_simulation_queue->queued_count() > 0)
	{
		ASSERT(bookkeeping_simulation_queue->queued_size_in_bytes() > 0);
		update->bookkeeping_simulation_queue.transfer_elements(bookkeeping_simulation_queue);
	}
	else
	{
		ASSERT(bookkeeping_simulation_queue->queued_size_in_bytes() == 0);
	}


	if (game_simulation_queue->queued_count() > 0)
	{
		update->game_simulation_queue_requires_application = true;
	}

	if (update->simulation_in_progress || update->game_simulation_queue_requires_application)
	{
		if (game_simulation_queue->queued_count() > 0)
		{
			ASSERT(game_simulation_queue->queued_size_in_bytes() > 0);
			update->game_simulation_queue.transfer_elements(game_simulation_queue);
		}
		else
		{
			ASSERT(!update->game_simulation_queue_requires_application);
			ASSERT(game_simulation_queue->queued_size_in_bytes() == 0);
		}
	}
	return;
}

void c_simulation_world::queues_clear(void)
{
	for (int32 i = 0; i < k_simulation_queue_count; i++)
	{
		queue_get((e_simulation_queue_type)i)->clear();
	}
}

typedef void(__thiscall* t_c_simulation_world__initialize_world)(c_simulation_world*, c_simulation_type_collection*, c_simulation_watcher*, c_simulation_distributed_world*);
t_c_simulation_world__initialize_world p_c_simulation_world__initialize_world;

CLASS_HOOK_DECLARE_LABEL(c_simulation_world__initialize_world, c_simulation_world::initialize_world);
void c_simulation_world::initialize_world(c_simulation_type_collection* type_collection, c_simulation_watcher* watcher, c_simulation_distributed_world* distributed_world)
{
	ASSERT(type_collection);
	ASSERT(watcher);
	ASSERT(m_world_type == _simulation_world_type_none);

	m_watcher = watcher;

	switch (game_simulation_get())
	{
	case _game_simulation_local:
		m_world_type = _simulation_world_type_local;
		break;
	case _game_simulation_synchronous_client:
		m_world_type = _simulation_world_type_synchronous_client;
		break;
	case _game_simulation_synchronous_server:
		m_world_type = _simulation_world_type_synchronous_authority;
		break;
	case _game_simulation_distributed_client:
		m_world_type = _simulation_world_type_distributed_client;
		break;
	case _game_simulation_distributed_server:
		m_world_type = _simulation_world_type_distributed_authority;
		break;
	default:
		unreachable();
		break;
	}

	ASSERT(m_world_type > _simulation_world_type_none && m_world_type < k_simulation_world_type_count);

	m_world_state = _simulation_world_state_none;
	m_time_running = false;
	m_time_immediate_update = false;
	m_attached_to_map = false;
	m_out_of_sync = false;
	m_next_update_number = 0;
	m_gamestate_flushed = false;
	m_unsuccessful_join_attempts = 0;
	m_last_active_timestamp = network_time_get();
	m_next_view_establishment_identifier = 0;
	m_joining_total_wait_msec = 0;

	if (is_distributed())
	{
		ASSERT(distributed_world);
		m_distributed_world = distributed_world;
		m_distributed_world->m_entity_manager.initialize();
		m_distributed_world->m_event_manager.initialize(&m_distributed_world->m_entity_manager);
		m_distributed_world->m_entity_database.initialize(this, &m_distributed_world->m_entity_manager, type_collection);
		m_distributed_world->m_event_handler.initialize(this, &m_distributed_world->m_event_manager, type_collection, &m_distributed_world->m_entity_database);
	}
	else
	{
		m_synchronous_gamestate_read_in_progress = false;
		m_synchronous_gamestate_write_progress = NONE;
		m_synchronous_gamestate_write_buffer = NULL;
		m_synchronous_catchup_initiation_failure_timestamp = (uint32)NONE;
	}

	if (!is_playback())
	{
		queues_initialize();
	}

	if (!runs_simulation())
	{
		m_update_queue_length = NULL;
		m_update_queue_head = NULL;
		m_update_queue_tail = NULL;
		update_queue_reset();
	}

	m_view_count = 0;
	csmemset(m_views, 0, sizeof(m_views));
	m_local_machine_identifier_valid = false;
	m_local_machine_index = NONE;

	change_state_disconnected();

	return;
}

bool c_simulation_world::claim_authority_gameworld(
	void)
{
	return INVOKE_TYPE(0x1DE3D0, 0x1C5890, bool(__thiscall*)(c_simulation_world*), this);
}

void __declspec(naked) jmp_initialize_world(void)
{
	CLASS_HOOK_JMP(c_simulation_world__initialize_world, c_simulation_world::initialize_world);
}

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


void c_simulation_world::update_queue_start(
	int32 next_update_number)
{
	ASSERT(exists());
	ASSERT(!m_time_running);
	ASSERT(!runs_simulation());

	update_queue_reset();

	ASSERT(m_update_queue_length == 0);

	m_update_queue_latest_update_number_received = next_update_number - 1;
	m_update_queue_next_update_number_to_dequeue = next_update_number;

	event(
		_event_message,
		"simulation:world: update queue started at #%d (expected: #%d)",
		m_update_queue_latest_update_number_received,
		m_update_queue_next_update_number_to_dequeue
	);

	return;

}

void c_simulation_world::update_queue_stop(
	void)
{
	ASSERT(exists());
	ASSERT(m_time_running);
	ASSERT(!runs_simulation());

	update_queue_reset();
	return;
}

void c_simulation_world::update_queue_reset(
	void)
{
	//INVOKE_TYPE(0x1DCDC3, 0x1C4277, void(__thiscall*)(c_simulation_world*), this);
	//need to reimplement this because sizeof(s_simulation_queued_update) changed

	ASSERT(exists());
	ASSERT(!m_time_running);
	ASSERT(!runs_simulation());

	while (m_update_queue_head)
	{
		//network_heap_verify_block(m_update_queue_head);
		s_simulation_queued_update* next = m_update_queue_head->next_node;
		simulation_destroy_update(&m_update_queue_head->update);
		network_heap_free_block(m_update_queue_head);

		m_update_queue_head = next;
	}

	m_update_queue_head = nullptr;
	m_update_queue_tail = nullptr;
	m_update_queue_length = 0;
	m_update_queue_next_update_number_to_dequeue = 0;
	m_update_queue_latest_update_number_received = NONE;

	return;
}

void c_simulation_world::reset_world(void)
{
	ASSERT(!is_authority());

	m_time_immediate_update = false;
	m_out_of_sync = false;
	m_gamestate_flushed = false;

	if (is_distributed())
	{
		ASSERT(m_distributed_world);

		m_distributed_world->m_entity_manager.reset();
		m_distributed_world->m_event_manager.reset();
		m_distributed_world->m_entity_database.reset();
		m_distributed_world->m_event_handler.reset();
		//simulation_gamestate_entities_notify_simulation_world_reset();
		delete_all_actors();
	}

	if (!is_playback())
	{
		// during reset, discard just simulation updates
		// not bookkeeping updates
		queue_get(_simulation_queue)->clear();
	}

	if (!runs_simulation())
	{
		update_queue_reset();
	}

	return;
}


typedef void(__thiscall* t_c_simulation_world__destroy_world)(c_simulation_world*);
t_c_simulation_world__destroy_world p_c_simulation_world__destroy_world;

CLASS_HOOK_DECLARE_LABEL(c_simulation_world__destroy_world, c_simulation_world::destroy_world);
void c_simulation_world::destroy_world(void)
{
	// call orig
	p_c_simulation_world__destroy_world(this);

	if (!is_playback())
	{
		queues_dispose();
	}
}

void __declspec(naked) jmp_destroy_world(void)
{
	CLASS_HOOK_JMP(c_simulation_world__destroy_world, c_simulation_world::destroy_world);
}

void c_simulation_world::remove_all_views()
{
	INVOKE_TYPE(0x1DD3B4, 0x0, void(__thiscall*)(c_simulation_world*), this);
	return;
}

void c_simulation_world::disconnect(void)
{

	if (is_connected() || is_joining() || m_view_count > 0)
	{
		event(
			_event_message,
			"simulation:world: disconnected (state %s, %d views)",
			get_state_string(m_world_state),
			m_view_count
		);
	}

	remove_all_views();
	if (is_connected() || is_joining())
	{
		change_state_disconnected();
	}

	return;
}

CLASS_HOOK_DECLARE_LABEL(c_simulation_world__update, c_simulation_world::update);
void c_simulation_world::update(
	void)
{
	ASSERT(exists());

	if (is_authority())
	{
		if (!is_connected()
			&& !is_joining()
			&& !is_dead()
			&& !is_out_of_sync())
		{
			update_authority_join_initiate();
		}
		if (is_joining())
		{
			update_authority_join_progress();
		}
		if (is_active())
		{
			update_authority_active();
		}
		if (m_world_state == _simulation_world_state_handoff)
		{
			update_authority_handoff();
		}

		update_player_activation();
	}
	else
	{
		if (!is_connected() 
			&& !is_joining() 
			&& !is_dead()
			&& !is_out_of_sync())
		{
			update_client_join_initiate();
		}
		if (is_joining())
		{
			update_client_join_progress();
		}
		if (!is_active() && !is_dead())
		{
			update_client_failure();
		}
		if (is_connected() || is_joining())
		{
			update_client_disconnection();
		}
		
		if (!simulation_reset_in_progress()) //h3 check
		{
			send_player_acknowledgements(false);
		}
	}

	if (m_world_type == _simulation_world_type_synchronous_client && m_time_immediate_update)
	{
		ASSERT(m_world_state == _simulation_world_state_joining);
	}
	else
	{
		ASSERT(time_running() == is_active());
	}

	s_simulation_world_view_iterator iterator;
	iterator_begin(&iterator, (uint32)NONE);

	c_simulation_view* view;
	while (iterator_next(&iterator, &view))
	{
		ASSERT(view);
		view->update();
	}

	return;
}
static void __declspec(naked) jmp_update(void)
{
	CLASS_HOOK_JMP(c_simulation_world__update, c_simulation_world::update);
}

void c_simulation_world::queues_initialize(void)
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
	event(_event_verbose, "simulation:players: create player 0x%08X", player_index);
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

void c_simulation_world::time_start(
	int32 next_update_number)
{
	ASSERT(exists());
	ASSERT(!m_time_running);

	ASSERT(m_local_machine_index != NONE);
	ASSERT(next_update_number >= 0);

	m_next_update_number = next_update_number;

	if (!runs_simulation())
	{
		update_queue_start(next_update_number);
	}
	m_time_running = true;

	return;
}

void c_simulation_world::time_stop(
	void)
{
	ASSERT(exists());
	ASSERT(m_time_running);

	m_time_running = false;

	if (!runs_simulation())
	{
		update_queue_stop();
	}

	return;
}

void c_simulation_world::time_set_immediate_update(
	bool time_immediate_update)
{
	ASSERT(exists());
	m_time_immediate_update = time_immediate_update;

	if (m_time_immediate_update)
	{
		ASSERT(m_world_type == _simulation_world_type_synchronous_client);

		bool match_remote_time;
		while (time_get_available(&match_remote_time))
		{
			game_tick();
		}
	}

	return;
}

int32 c_simulation_world::time_get_available(
	bool* match_remote_time)
{
	int32 available_time = 0;

	ASSERT(exists());
	ASSERT(match_remote_time);

	*match_remote_time = false;

	if (m_time_running)
	{
		available_time = INT32_MAX;
		switch (m_world_type)
		{
		case _simulation_world_type_local:
		case _simulation_world_type_distributed_authority:
		case _simulation_world_type_distributed_client:
			break;
		case _simulation_world_type_synchronous_authority:
			available_time = synchronous_authority_get_maximum_updates();
			break;
		case _simulation_world_type_synchronous_client:
			available_time = update_queue_get_available_updates();
			*match_remote_time = true;
			break;
		default:
			unreachable();
		}
	}

	return available_time;
}

void c_simulation_world::iterator_begin(
	s_simulation_world_view_iterator* iterator,
	uint32 view_type_mask)
{
	ASSERT(iterator);
	ASSERT(view_type_mask == NONE || (view_type_mask != 0 && VALID_BITS(view_type_mask, k_simulation_view_type_count)));

	iterator->view_type_mask = view_type_mask;
	iterator->next_world_view_index = 0;
	return;
}

bool c_simulation_world::iterator_next(
	s_simulation_world_view_iterator* iterator,
	c_simulation_view** view) const
{
	bool result = false;

	ASSERT(iterator);
	ASSERT(view);

	while (iterator->next_world_view_index < NUMBEROF(m_views))
	{
		c_simulation_view* current_view = m_views[iterator->next_world_view_index++];
		if (current_view && TEST_BIT(iterator->view_type_mask, current_view->view_type()))
		{
			*view = current_view;
			result = true;
			break;
		}
	}

	return result;
}

c_simulation_view* c_simulation_world::get_authority_view(
	void)
{
	//return INVOKE_TYPE(0x1DCED3, 0x0, c_simulation_view * (__thiscall*)(c_simulation_world*), this);

	ASSERT(exists());
	c_simulation_view* view = NULL;
	s_simulation_world_view_iterator iterator;
	iterator_begin(&iterator, FLAG(_simulation_view_type_synchronous_to_remote_authority) | FLAG(_simulation_view_type_distributed_to_remote_authority));

	if (iterator_next(&iterator, &view))
	{
		ASSERT(view != NULL);
	}

	return view;
}

c_simulation_view* c_simulation_world::get_client_view_by_machine_index(
	int32 remote_machine_index)
{
	c_simulation_view* view = NULL;

	ASSERT(exists());

	s_simulation_world_view_iterator iterator;
	iterator_begin(&iterator, FLAG(_simulation_view_type_synchronous_to_remote_client) | FLAG(_simulation_view_type_distributed_to_remote_client));

	c_simulation_view* test_view;
	while (iterator_next(&iterator, &test_view))
	{
		view = test_view;
		ASSERT(test_view != NULL);

		if (view->get_machine_index() == remote_machine_index)
		{
			break;
		}
	}

	return view;
}
void c_simulation_world::handle_view_establishment(
	const c_simulation_view* view,
	bool established)
{
	//INVOKE_TYPE(0x1DD9B9, 0x0, void(__thiscall*)(c_simulation_world*, const c_simulation_view*, bool), this, view, established);
	if (established && view == get_authority_view())
	{
		ASSERT(!is_authority());

		if (!is_synchronous())
		{
			simulation_reset();
		}
		
		if (!simulation_reset_in_progress()) //h3 check
		{
			send_player_acknowledgements(true);
		}
	}
	return;
}

void c_simulation_world::handle_view_activation(
	const c_simulation_view* view,
	bool active)
{
	//INVOKE_TYPE(0x1DE53D, 0x0, void(__thiscall*)(c_simulation_world*, const c_simulation_view*, bool), this, view, active);
	if (active)
	{
		if (view == get_authority_view() 
			&& is_joining())
		{
			update_client_join_progress();
		}
	}
#ifdef USE_H3_DROP_SIMULATION_TO_JOINING
	//h3 addon
	else
	{
		if (view == get_authority_view() 
			&& is_active()
			&& view->get_view_establishment_mode() == _simulation_view_establishment_mode_joining)
		{
			event(_event_message, "networking:simulation:world: view of authority has become inactive, moving world back to joining");
			change_state_joining(NULL);
		}
	}	
#endif
	return;
}

bool c_simulation_world::authority_join_timeout_expired(
	void)
{
	ASSERT(exists());
	ASSERT(is_authority());

	int32 wait_time = m_joining_total_wait_msec;
	if (is_joining())
		wait_time += network_time_since(m_world_state_data.joining.join_start_timestamp);

	return wait_time >= global_network_configuration_get()->simulation.world.join_total_wait_timeout;


}

void c_simulation_world::update_authority_join_initiate(
	void)
{
	ASSERT(exists());
	ASSERT(is_authority());
	ASSERT(!is_connected() && !is_joining() && !is_dead());

	if (is_local())
	{
		change_state_joining(NULL);
		change_state_active();
	}
	else
	{
		change_state_joining(m_watcher->get_machine_valid_mask());
	}
	return;
}

void c_simulation_world::update_authority_join_progress(
	void)
{
	int32 machine_index;
	uint32 join_blocking_machine_mask;

	uint32 machine_valid_mask = m_watcher->get_machine_valid_mask();
	uint32 join_client_machine_mask = 0;
	uint32 join_connected_machine_mask = 0;
	uint32 join_established_machine_mask = 0;
	uint32 join_waiting_machine_mask = 0;
	uint32 join_in_progress_machine_mask = 0;
	uint32 join_complete_machine_mask = 0;
	int32 join_wait_time_msec = network_time_since(m_world_state_data.disconnected.disconnected_timestamp);

	ASSERT(exists());
	ASSERT(is_authority());
	ASSERT(!is_local());
	ASSERT(m_world_state == _simulation_world_state_joining);

	for (machine_index = 0; machine_index < NUMBEROF(m_views); ++machine_index)
	{
		if (TEST_BIT(machine_valid_mask, machine_index) && machine_index != get_machine_index())
		{
			c_simulation_view* view = get_client_view_by_machine_index(machine_index);
			SET_BIT(join_client_machine_mask, machine_index, true);

			if (view)
			{
				if (!view->is_dead(NULL))
				{
					SET_BIT(join_connected_machine_mask, machine_index, true);

					ASSERT(!view->active());

					if (view->established())
					{
						SET_BIT(join_established_machine_mask, machine_index, true);

						if (TEST_BIT(m_world_state_data.joining.join_client_machine_mask, machine_index))
						{
							if (update_joining_view(view))
							{
								SET_BIT(join_complete_machine_mask, machine_index, true);
							}
							else
							{
								SET_BIT(join_in_progress_machine_mask, machine_index, true);
							}
						}
						else
						{
							SET_BIT(join_waiting_machine_mask, machine_index, true);
						}
					}
					else
					{
						update_establishing_view(view);
					}
				}
			}
		}
	}

	join_blocking_machine_mask =
		join_client_machine_mask &
		m_world_state_data.joining.join_client_machine_mask &
		~join_complete_machine_mask;


	if (!join_blocking_machine_mask || join_wait_time_msec >= global_network_configuration_get()->simulation.world.join_timeout)
	{
		event(
			_event_message,
			"simulation:world: update_authority_join: JOIN-%s, clients total/conn/est/wait/join/complete"
			" 0x%04X/0x%04X/0x%04X/0x%04X/0x%04X/0x%04X join-client/block 0x%04X/0x%04X after %dms",
			join_waiting_machine_mask ? "TIMEOUT" : "COMPLETE",
			join_client_machine_mask,
			join_connected_machine_mask,
			join_established_machine_mask,
			join_waiting_machine_mask,
			join_in_progress_machine_mask,
			join_complete_machine_mask,
			m_world_state_data.joining.join_client_machine_mask,
			join_blocking_machine_mask,
			join_wait_time_msec
		);

		change_state_active();
	}

	return;
}

void c_simulation_world::update_authority_active(
	void)
{
	//INVOKE_TYPE(0x1DDDCE, 0x0, void(__thiscall*)(c_simulation_world*), this);

	ASSERT(exists());
	ASSERT(is_authority());
	ASSERT(m_world_state == _simulation_world_state_active);

	uint32 active_client_views_mask = NULL;

	if (!is_local())
	{
		ASSERT(VALID_INDEX(m_local_machine_index, k_network_maximum_machines_per_session));

		s_simulation_world_view_iterator iterator;
		iterator_begin(&iterator, FLAG(_simulation_view_type_synchronous_to_remote_client) | FLAG(_simulation_view_type_distributed_to_remote_client));

		c_simulation_view* view;
		while (iterator_next(&iterator, &view))
		{
			ASSERT(view);
			if (!view->is_dead(NULL))
			{
				if (view->active())
				{
					SET_BIT(active_client_views_mask, view->get_machine_index(), true);
				}
				else if (view->get_view_establishment_mode() != _simulation_view_establishment_mode_active)
				{
					if (view->established())
						update_joining_view(view);
					else
						update_establishing_view(view);
				}
			}
		}

		if (m_watcher->in_online_networked_session())
		{
			uint32 machine_valid_mask = m_watcher->get_machine_valid_mask();
			uint32 active_client_machine_mask = m_world_state_data.active.active_client_machine_mask;

			uint32 active_blocking_mask = machine_valid_mask & active_client_machine_mask & ~active_client_views_mask;
			if (active_blocking_mask && machine_valid_mask)
			{
				int32 session_machine_count = 0;
				int32 clients_pending_to_go_active_count = 0;

				for (int8 i = 0; i < k_network_maximum_machines_per_session; ++i)
				{
					session_machine_count += TEST_BIT(machine_valid_mask, i);
					clients_pending_to_go_active_count += TEST_BIT(active_blocking_mask, i);
				}

				ASSERT(session_machine_count > 0);


				if ((((real32)clients_pending_to_go_active_count / (real32)session_machine_count)
					- global_network_configuration_get()->simulation.world.pause_game_required_machines_fraction) > -k_real_epsilon)
				{

					if (m_watcher->get_session()->get_session_parameters()->dedicated_server) //h2mcc
					{
						//if dedicated server
						event(
							_event_message,
							"networking:simulation:world: IGNORING pausing active simulation to wait for clients because we're a dedi (currently 0x%04X, tracking 0x%04X, session 0x%04X, blocking 0x%04X)",
							active_client_views_mask,
							active_client_machine_mask,
							machine_valid_mask,
							active_blocking_mask
						);
					}
					else
					{
						if (authority_join_timeout_expired())
						{
							event(
								_event_message,
								"simulation:world: join timeout expired, booting blocking clients (currently 0x%04X, tracking 0x%04X, session 0x%04X, blocking 0x%04X)",
								active_client_views_mask,
								active_client_machine_mask,
								machine_valid_mask,
								active_blocking_mask
							);
							m_watcher->boot_machines(active_blocking_mask, false);
						}
						else
						{
							event(
								_event_message,
								"simulation:world: pausing active simulation to wait for clients (currently 0x%04X, tracking 0x%04X, session 0x%04X, blocking 0x%04X)",
								active_client_views_mask,
								active_client_machine_mask,
								machine_valid_mask,
								active_blocking_mask
							);

							drop_simulation_from_active_to_joining();
						}
					}
				}
			}
			else
			{
				ASSERT(m_world_state == _simulation_world_state_active);
				m_world_state_data.active.active_client_machine_mask = machine_valid_mask & (active_client_views_mask | active_client_machine_mask);
			}
		}
	}

	return;
}

void c_simulation_world::update_authority_handoff(
	void)
{
	ASSERT(exists());
	ASSERT(is_authority());
	ASSERT(m_world_state == _simulation_world_state_handoff);

	s_simulation_world_view_iterator iterator;
	iterator_begin(&iterator, (uint32)NONE);

	c_simulation_view* view;
	while (iterator_next(&iterator, &view))
	{
		ASSERT(view);

		if (!view->is_dead(NULL) && view->get_view_establishment_mode() > _simulation_view_establishment_mode_established)
		{
			event(
				_event_message,
				"simulation:world: update_authority_handoff: view %s being paused for handoff (mode %d -> %d)",
				view->get_view_description(),
				view->get_view_establishment_mode(),
				_simulation_view_establishment_mode_established
			);

			view->set_view_establishment(
				_simulation_view_establishment_mode_established,
				view->get_view_establishment_identifier());
		}
	}

	return;
}

void c_simulation_world::update_client_join_initiate(
	void)
{
	c_simulation_view* view = get_authority_view();

	ASSERT(exists());
	ASSERT(!is_authority());
	ASSERT(!is_connected() && !is_joining() && !is_dead());

	if (view && view->get_channel_index() != NONE && !view->is_dead(NULL))
	{
		event(
			_event_message,
			"simulation:world: client join initiated over remote authority view %s (mode %d -> %d)",
			view->get_view_description(),
			view->get_view_establishment_mode(),
			_simulation_view_establishment_mode_connected
		);

		view->set_view_establishment(_simulation_view_establishment_mode_connected, NONE);
		change_state_joining(NULL);
	}

	return;
}

void c_simulation_world::update_client_join_progress(
	void)
{
	c_simulation_view* authority_view = get_authority_view();
	bool should_disconnect = false;

	ASSERT(exists());
	ASSERT(!is_authority());
	ASSERT(is_joining());

	if (!authority_view)
	{
		event(_event_message, "simulation:world: client join aborted, remote authority view has been deleted");
		should_disconnect = true;
	}
	else
	{
		if (authority_view->active())
		{
			event(_event_message, "simulation:world: client join complete, going active");
			change_state_active();
		}
		else
		{
			s_network_configuration* g_network_configuration = global_network_configuration_get();

			const int32 time_since_disconnect = network_time_since(m_world_state_data.disconnected.disconnected_timestamp);
			if (time_since_disconnect > g_network_configuration->simulation.world.join_timeout)
			{
				event(
					_event_warning,
					"simulation:world: client join timeout, aborting after %d>%dmsec",
					time_since_disconnect,
					g_network_configuration->simulation.world.join_timeout
				);
				should_disconnect = true;
			}
		}
	}

	if (should_disconnect)
	{
		disconnect();
	}

	return;
}

void c_simulation_world::update_client_failure(
	void)
{
	ASSERT(exists());
	ASSERT(!is_authority());
	ASSERT(!is_active());

	bool simulation_failed = false;
	int32 time_since_last_active = network_time_since(m_last_active_timestamp);
	s_network_configuration* g_network_configuration = global_network_configuration_get();
	if (m_unsuccessful_join_attempts < g_network_configuration->simulation.world.client_join_failure_count)
	{
		if (time_since_last_active < g_network_configuration->simulation.world.client_activation_failure_timeout)
		{
		}
		else
		{
			event(
				_event_warning,
				"simulation:world: client not yet active after %d > %d msec (%d join failures), simulation has failed, world is dying",
				time_since_last_active,
				g_network_configuration->simulation.world.join_timeout,
				m_unsuccessful_join_attempts
			);
			simulation_failed = true;
		}
	}
	else
	{
		event(
			_event_warning,
			"simulation:world: client activation failed %d times after %d msec, simulation has failed, world is dying",
			m_unsuccessful_join_attempts,
			time_since_last_active
		);
		simulation_failed = true;
	}


	if (simulation_failed)
	{
		change_state_dead();
	}

	return;
}

void c_simulation_world::update_client_disconnection(
	void)
{

	ASSERT(exists());
	ASSERT(is_connected() || is_joining());

	bool disconnected = false;

	c_simulation_view const* view = get_authority_view();
	if (view)
	{
		if (!view->established() && !is_joining())
		{
			disconnected = true;
		}
	}
	else
	{
		disconnected = true;
	}

	if (disconnected)
	{
		disconnect();
	}

	return;
}

void c_simulation_world::drop_simulation_from_active_to_joining(void)
{
	ASSERT(is_authority());

	if (is_active())
	{
		event(_event_message, "networking:simulation:world: moving from active to joining");

		s_simulation_world_view_iterator iterator;
		iterator_begin(&iterator, FLAG(_simulation_view_type_synchronous_to_remote_client) | FLAG(_simulation_view_type_distributed_to_remote_client));

		c_simulation_view* view;
		while (iterator_next(&iterator, &view))
		{
			ASSERT(view);

			if (!view->is_dead(NULL))
			{
#ifndef USE_H3_DROP_SIMULATION_TO_JOINING
				//h2 code				 
				event(
					_event_message,
					"simulation:world: pausing active client view %s by killing it, it didn't actually fail to join, this is just the simplest way",
					view->get_view_description()
				);

				view->failed_to_join();
				verify_player_activation();

#else 
				//h3 code
				if (view->get_view_establishment_mode() == _simulation_view_establishment_mode_active)
				{
					event(
						_event_message,
						"networking:simulation:world: pausing active client view %s (mode %d -> %d)",
						view->get_view_description(),
						view->get_view_establishment_mode(),
						_simulation_view_establishment_mode_joining
					);

					view->set_view_establishment(_simulation_view_establishment_mode_joining, view->get_view_establishment_identifier());
				}
				verify_player_activation();
#endif
			}
		}

		//change authoriy_world to joining
		change_state_joining(m_world_state_data.active.active_client_machine_mask);

	}
	else
	{
		event(
			_event_warning,
			"networking:simulation:world: world state is not active, can't drop simulation to joining [%d]",
			m_world_state
		);
	}

}

c_simulation_world::e_join_progress c_simulation_world::update_joining_view(
	c_simulation_view* view)
{
	return INVOKE_TYPE(
		0x1DD4BB,
		0x0,
		c_simulation_world::e_join_progress(__thiscall*)(c_simulation_world*, c_simulation_view * view),
		this,
		view
	);
}

void c_simulation_world::update_establishing_view(
	c_simulation_view* view)
{
	ASSERT(exists());
	ASSERT(is_authority());
	ASSERT(view);
	ASSERT(!view->established());

	if (view->get_view_establishment_mode() != _simulation_view_establishment_mode_established)
	{
		if (view->ready_to_establish())
		{
			int32 new_establishment_identifier = m_next_view_establishment_identifier;
			m_next_view_establishment_identifier++;

			event(
				_event_message,
				"simulation:world: simulation connected, go established - advancing remote client view %s (mode %d -> %d, new identifier %d)",
				view->get_view_description(),
				view->get_view_establishment_mode(),
				_simulation_view_establishment_mode_established,
				m_next_view_establishment_identifier
			);

			view->set_view_establishment(_simulation_view_establishment_mode_established, new_establishment_identifier);
		}
		else if (view->get_view_establishment_mode() != _simulation_view_establishment_mode_connected)
		{
			event(
				_event_message,
				"simulation:world: view ready to connect, advancing remote client view %s (mode %d -> %d)",
				view->get_view_description(),
				view->get_view_establishment_mode(),
				_simulation_view_establishment_mode_connected
			);

			view->set_view_establishment(_simulation_view_establishment_mode_connected, NONE);
		}
	}

	return;
}

void c_simulation_world::update_player_activation(void)
{
	//INVOKE_TYPE(0x1DD6E3, 0x0, void(__thiscall*)(c_simulation_world*), this);

	uint32 player_acknowledged_mask = get_acknowledged_player_mask();
	for (int32 player_index = 0; player_index < NUMBEROF(m_players); ++player_index)
	{
		c_simulation_player* player = &m_players[player_index];
		if (player->exists() && !player->active())
		{
			s_player_identifier player_identifier;
			player->get_identifier(&player_identifier);

			if (m_watcher->get_player_is_in_game(player_index, &player_identifier))
			{
				if (TEST_BIT(player_acknowledged_mask, player_index) && !player->pending_deletion())
				{
					event(_event_message, "simulation:world: player %d going active", player_index);
					player->set_active(true);
				}
			}
		}
	}
	verify_player_activation();

	return;
}

void c_simulation_world::verify_player_activation(void)
{
	if (is_authority()
		&& is_distributed())
	{
		uint32 player_acknowledged_mask = get_acknowledged_player_mask();
		for (int32 player_index = 0; player_index < NUMBEROF(m_players); ++player_index)
		{
			c_simulation_player const* player = &m_players[player_index];

			if (player->exists() && player->active())
			{
				s_player_identifier player_identifier;

				ASSERT(TEST_BIT(player_acknowledged_mask, player_index));

				player->get_identifier(&player_identifier);

				ASSERT(m_watcher->get_player_is_in_game(player_index, &player_identifier));
			}
		}
	}
	

	return;
}

void c_simulation_world::change_state_internal(
	e_simulation_world_state new_state)
{
	ASSERT(exists());
	ASSERT(new_state > _simulation_world_type_none && new_state < k_simulation_world_state_count);
	ASSERT(new_state != m_world_state);

	if (!is_local())
	{
		event(
			_event_message,
			"simulation:world: state %s -> %s",
			get_state_string(m_world_state),
			get_state_string(new_state)
		);
	}

	if (new_state == _simulation_world_state_active)
	{
		ASSERT(time_running());
	}
	else
	{
		time_set_immediate_update(false);
		if (time_running())
		{
			time_stop();
		}
	}

	if (m_world_state == _simulation_world_state_joining)
	{
		if (is_authority())
		{
			m_joining_total_wait_msec += network_time_since(m_world_state_data.joining.join_start_timestamp);
		}

		if (new_state != _simulation_world_state_active)
		{
			if (synchronous_catchup_in_progress())
			{
				synchronous_gamestate_write_stop();
			}

			++m_unsuccessful_join_attempts;
			event(
				_event_message,
				"simulation:world: recording an unsuccessful join (now %d failures)",
				m_unsuccessful_join_attempts
			);
		}
	}
	else if (m_world_state == _simulation_world_state_active)
	{
		m_last_active_timestamp = network_time_get();
	}

	ASSERT(!synchronous_gamestate_write_in_progress());

	m_world_state = new_state;

	return;
}

void c_simulation_world::change_state_joining(
	uint32 joining_client_machine_mask)
{
	ASSERT(exists());
	ASSERT((!is_connected() && !is_dead()) || is_active());

	change_state_internal(_simulation_world_state_joining);
	m_world_state_data.joining.join_start_timestamp= network_time_get();
	m_world_state_data.joining.join_client_machine_mask = joining_client_machine_mask;

	return;
}

void c_simulation_world::change_state_active(
	void)
{
	ASSERT(exists());
	ASSERT(is_joining());

	if (time_running())
	{
		ASSERT(m_world_type == _simulation_world_type_synchronous_client);
		ASSERT(m_time_immediate_update);

		time_set_immediate_update(false);
	}
	else
	{
		time_start(m_next_update_number);
	}

	change_state_internal(_simulation_world_state_active);
	m_world_state_data.active.active_client_machine_mask= NULL;

	return;
}

void c_simulation_world::change_state_disconnected(
	void)
{
	ASSERT(exists());
	ASSERT(m_world_state != _simulation_world_state_dead);

	if (m_world_state != _simulation_world_state_disconnected)
	{
		change_state_internal(_simulation_world_state_disconnected);
		m_world_state_data.disconnected.disconnected_timestamp = network_time_get();
	}

	return;
}

void c_simulation_world::change_state_dead(
	void)
{
	ASSERT(exists());

	if (!is_dead())
	{
		disconnect();
		change_state_internal(_simulation_world_state_dead);
	}

	return;
}

void c_simulation_world::change_state_handoff(
	void)
{
	ASSERT(exists());
	ASSERT(is_authority());

	if (is_active())
	{
		change_state_internal(_simulation_world_state_handoff);
	}
	else
	{
		change_state_leaving();
	}

	return;
}

void c_simulation_world::change_state_leaving(
	void)
{
	ASSERT(exists());

	if ((is_connected() || is_joining()) &&
		m_world_state != _simulation_world_state_leaving)
	{
		change_state_internal(_simulation_world_state_leaving);
	}

	return;
}

void c_simulation_world::gamestate_flush(
	void)
{
	ASSERT(exists());
	ASSERT(m_world_type == _simulation_world_type_synchronous_client);

	game_state_call_before_save_procs(0);
	game_state_call_after_save_procs(0);

	return;
}

void c_simulation_world::go_out_of_sync(
	void)
{
	ASSERT(exists());
	ASSERT(!runs_simulation());
	ASSERT(m_time_running);

	// TODO: add debug hs global check here 
	//if (g_hs_net_allow_out_of_sync)
	{
		m_out_of_sync = true;
	}

	return;
}

int32 c_simulation_world::get_time(
	void) const
{
	ASSERT(exists());
	return (int32)game_time_get();
}

bool c_simulation_world::can_generate_updates(void)
{
	s_simulation_world_view_iterator iterator;
	c_simulation_view* view = NULL;
	bool result = true;

	if (is_synchronous()
		&& is_authority())
	{
		if (is_active())
		{
			iterator_begin(&iterator, FLAG(_simulation_view_type_synchronous_to_remote_client));
			while (iterator_next(&iterator, &view))
			{
				if (view->observer_channel_backlogged(_network_message_type_synchronous_update))
				{
					//tell game to not generate more updates
					view->observer_channel_set_waiting_on_backlog(_network_message_type_synchronous_update);
					result = false;
					event(_event_message,
						"simulation:world:pregame: backlog detected, stopping new updates..."
					);
					break;
				}
			}
		}
		else
		{
			result = false;
		}
	}

	return result;
}

void c_simulation_world::build_player_actions(struct simulation_update* update)
{
	INVOKE_TYPE(0x1DBE3F, 0x1C32F4, void(__thiscall*)(c_simulation_world*, struct simulation_update*), this, update);
}

void c_simulation_world::build_update(
	struct simulation_update* update)
{
	if (runs_simulation())
	{
		update->update_number = get_next_update_number();
		update->verify_game_time = get_time();

		random_seed_allow_use();
		update->verify_random_seed = get_random_seed();
		random_seed_disallow_use();

		simulation_build_machine_update(&update->machine_update_valid, &update->machine_update);
		simulation_build_player_updates(&update->player_update_count, NUMBEROF(update->player_updates), update->player_updates);
		update->simulation_in_progress = simulation_in_progress();

		if (update->simulation_in_progress)
		{
			build_player_actions(update);

			if (is_distributed())
			{
				// Do nothing?
			}

			if (is_authority() && m_gamestate_flushed)
			{
				update->flush_gamestate = true;
				m_gamestate_flushed = false;
			}
		}

		update->bookkeeping_simulation_queue.initialize();
		update->game_simulation_queue.initialize();
		attach_simulation_queues_to_update(update);


		//sim-encode-decode-check-begin
		uint8 temp_buffer[0xFFFF];
		c_bitstream temporary_stream(temp_buffer, sizeof(temp_buffer));

		temporary_stream.begin_writing(k_bitstream_default_alignment);
		simulation_update_encode(&temporary_stream, update);
		temporary_stream.finish_writing(NULL);

		destroy_update(update);

		temporary_stream.begin_reading();
		bool decode_success = simulation_update_decode(&temporary_stream, update);

		ASSERT(!temporary_stream.error_occurred());
		temporary_stream.finish_reading();
		//sim-encode-decode-check-end

		ASSERT(decode_success);
	}
	else
	{
		update_queue_retrieve_update(update);
	}
	return;
}


void c_simulation_world::distribute_update(
	const struct simulation_update* update)
{
	ASSERT(update);
	ASSERT(exists());
	ASSERT(is_authority());

	if (m_world_type == _simulation_world_type_synchronous_authority)
	{
		synchronous_authority_dispatch_update(update);
	}
	else if (m_world_type == _simulation_world_type_distributed_authority)
	{
		distributed_authority_dispatch_player_actions(update->player_action_mask, update->player_actions);
		distributed_authority_dispatch_actor_control(update->unit_control_mask, update->unit_control);
	}

	return;
}

void c_simulation_world::advance_update(
	const struct simulation_update* update)
{
	ASSERT(update);
	m_next_update_number = update->update_number + 1;

	return;
}

void c_simulation_world::destroy_update(struct simulation_update* update)
{
	ASSERT(update);
	update->bookkeeping_simulation_queue.dispose();
	update->game_simulation_queue.dispose();
	return;
}


void c_simulation_world::synchronous_authority_dispatch_update(
	struct simulation_update const* update)
{
	ASSERT(exists());
	ASSERT(is_authority());

	s_simulation_world_view_iterator iterator;
	iterator_begin(&iterator, FLAG(_simulation_view_type_synchronous_to_remote_client));

	c_simulation_view* view;
	while (iterator_next(&iterator, &view))
	{
		ASSERT(view);
		view->dispatch_synchronous_update(update);
	}

	return;
}

int32 c_simulation_world::synchronous_authority_get_maximum_updates(
	void)
{
	//return INVOKE_TYPE(0x1DC421, 0x1C38D5, int32(__thiscall*)(c_simulation_world*), this);

	c_simulation_view* lowest_view = NULL;
	c_simulation_view* bad_client_view = NULL;
	int32 lowest_update_num = INT32_MAX;
	int32 result = INT32_MAX;

	s_simulation_world_view_iterator iterator;
	iterator_begin(&iterator, FLAG(_simulation_view_type_synchronous_to_remote_client));

	// Iterate through every view and find the view that's fallen the most behind in updates
	c_simulation_view* view;
	while (iterator_next(&iterator, &view))
	{
		ASSERT(view);
		if (view->active())
		{
			const int32 update_num = view->synchronous_client_get_acknowledged_update_number();
			if (update_num < lowest_update_num)
			{
				lowest_view = view;
				lowest_update_num = update_num;
			}
		}
	}

	if (lowest_view)
	{
		const int32 updates_to_send = get_next_update_number() - 1 - lowest_update_num;
		const int32 maximum_queue_size = k_simulation_world_maximum_synchronous_updates - updates_to_send;
		if (updates_to_send < 0)
		{
			DISPLAY_ASSERT("simulation world believes its clients are all in the future!");
		}

		if (maximum_queue_size > 0)
		{
			result = maximum_queue_size;
		}
		else
		{
			bad_client_view = lowest_view;
		}
	}

	//h3 - force to wait until backlogs are cleared
	if (!bad_client_view)
	{
		iterator_begin(&iterator, FLAG(_simulation_view_type_synchronous_to_remote_client));
		while (iterator_next(&iterator, &view))
		{
			if (view->observer_channel_backlogged(_network_message_type_synchronous_update))
			{
				bad_client_view = view;
				bad_client_view->observer_channel_set_waiting_on_backlog(_network_message_type_synchronous_update);
				break;
			}
		}
	}	

	result = bad_client_view != NULL ? 0 : result;

	iterator_begin(&iterator, FLAG(_simulation_view_type_synchronous_to_remote_client));
	while (iterator_next(&iterator, &view))
	{
		const bool views_match = bad_client_view == view;
		ASSERT(view);
		view->synchronous_client_block(views_match);
	}

	return result;
}

bool c_simulation_world::handle_synchronous_update(
	const struct simulation_update* update)
{
	bool result = false;

	ASSERT(exists());
	ASSERT(m_world_type == _simulation_world_type_synchronous_client);
	ASSERT(m_time_running);
	ASSERT(update);

	const int32 next_expected_update_number = update_queue_get_next_expected_update_number();
	if (synchronous_gamestate_write_in_progress())
	{
		event(_event_error, "simulation:world: OUT OF SYNC: server update arrived while gamestate transfer was incomplete");
		go_out_of_sync();
	}
	else if (update->update_number < next_expected_update_number)
	{
		event(
			_event_warning,
			"simulation:world: synchronous-update discarded (expected #%ld, got old #%ld)",
			next_expected_update_number,
			update->update_number
		);
	}
	else if (update->update_number != next_expected_update_number)
	{
		event(
			_event_error,
			"simulation:world: OUT OF SYNC: missed a server update (expected #%ld, got #%ld)",
			next_expected_update_number,
			update->update_number
		);
		go_out_of_sync();
	}
	else if (is_active() && m_time_immediate_update)
	{
		event(
			_event_error,
			"simulation:world: OUT OF SYNC: server update arrived while world was unable to process it (state %d)",
			get_state()
		);
		go_out_of_sync();
	}
	else if (!update_queue_handle_server_update(update))
	{
		event(
			_event_error,
			"simulation:world: synchronous-update #%ld couldn't be inserted into update queue",
			update->update_number
		);
		simulation_fatal_error();
	}
	else
	{
		result = true;
		if (m_time_immediate_update)
		{
			bool time_available;
			event(
				_event_message,
				"simulation:world: processing immediate updates (%d at update #%d time #%d)",
				time_get_available(&time_available),
				get_next_update_number(),
				game_time_get()
			);

			while (game_in_progress() 
				&& !simulation_aborted() 
				&& m_out_of_sync 
				&& time_get_available(&time_available) <= 0)
			{
				game_tick();
			}
		}
	}

	return result;
}

bool c_simulation_world::update_queue_handle_server_update(
	const struct simulation_update* update)
{
	//return INVOKE_TYPE(0x1DCE0F, 0x1C42C3, bool(__thiscall*)(c_simulation_world*, const simulation_update*), this, update);

	bool success = false;

	ASSERT(update);

	ASSERT(exists());
	ASSERT(m_time_running);
	ASSERT(!runs_simulation());
	ASSERT(update->update_number == update_queue_get_next_expected_update_number());
	ASSERT(m_update_queue_tail == NULL || (update->update_number == m_update_queue_tail->update.update_number + 1));

	struct s_simulation_queued_update* update_storage = (struct s_simulation_queued_update*)network_heap_allocate_block(sizeof(*update_storage));
	if (update_storage)
	{
		if (m_update_queue_tail)
		{
			//network_heap_verify_block(m_update_queue_tail);
			m_update_queue_tail->next_node = update_storage;
		}
		else
		{
			m_update_queue_head = update_storage;
		}

		update_storage->next_node = NULL;
		++m_update_queue_length;
		m_update_queue_tail = update_storage;

		event(
			_event_verbose,
			"simulation:world: update queue received #%d (previously received: #%d)",
			update->update_number,
			m_update_queue_next_update_number_to_dequeue
		);

		update_storage->update = *update;
		m_update_queue_latest_update_number_received = update->update_number;
		success = true;
	}
	else
	{
#ifdef EVENTS_ENABLED
		char heapbuf[1024];
		event(
			_event_error,
			"simulation:world: OUT OF MEMORY allocating stored update [#%d] (queue [#%d]/[#%d] length [%d]) [%s]",
			update->update_number,
			m_update_queue_next_update_number_to_dequeue,
			m_update_queue_latest_update_number_received,
			m_update_queue_length,
			network_heap_describe(heapbuf, sizeof(heapbuf))
		);
#endif
	}

	return success;
}


void c_simulation_world::update_queue_retrieve_update(struct simulation_update* update)
{
	//INVOKE_TYPE(0x1DCE7C, 0x1C4330, void(__thiscall*)(c_simulation_world *, struct simulation_update*), this, update);

	ASSERT(update);
	ASSERT(exists());
	ASSERT(m_time_running);
	ASSERT(!runs_simulation());


	ASSERT(m_update_queue_next_update_number_to_dequeue <= m_update_queue_latest_update_number_received);
	ASSERT(m_update_queue_length > 0);
	ASSERT(m_update_queue_head != NULL);

	s_simulation_queued_update* update_node = m_update_queue_head;

	//network_heap_verify_block(update_node);
	ASSERT(update_node->update.update_number == m_update_queue_next_update_number_to_dequeue);

	csmemcpy(update, update_node, sizeof(*update));

	const bool last_node = m_update_queue_tail == update_node;
	m_update_queue_head = update_node->next_node;
	if (last_node)
	{
		m_update_queue_tail = NULL;
	}

	network_heap_free_block(update_node);

	const int32 new_length = --m_update_queue_length;
	++m_update_queue_next_update_number_to_dequeue;
	if (new_length > 0)
	{
		ASSERT(m_update_queue_head != NULL);

		ASSERT(m_update_queue_head->update.update_number == m_update_queue_next_update_number_to_dequeue);
	}
	return;
}

int32 c_simulation_world::update_queue_get_available_updates(
	void) const
{
	ASSERT(exists());
	ASSERT(m_time_running);
	ASSERT(!runs_simulation());

	const int32 available_updates = m_update_queue_latest_update_number_received - m_update_queue_next_update_number_to_dequeue + 1;
	ASSERT(m_update_queue_length == available_updates);

	return available_updates;
}


void c_simulation_world::distributed_authority_dispatch_player_actions(
	uint32 player_valid_mask,
	const player_action* player_actions)
{
	INVOKE_TYPE(0x1DC5B3, 0x1C3A67, void(__thiscall*)(c_simulation_world*, uint32, const player_action*), this, player_valid_mask, player_actions);
	return;
}

void c_simulation_world::distributed_authority_dispatch_actor_control(
	uint32 actor_valid_mask,
	const unit_control_data* actor_control)
{
	INVOKE_TYPE(0x1DC761, 0x1C3C15, void(__thiscall*)(c_simulation_world*, uint32, const unit_control_data*), this, actor_valid_mask, actor_control);
	return;
}

void simulation_world_apply_patches(void)
{
	DETOUR_ATTACH(p_c_simulation_world__initialize_world, Memory::GetAddress<t_c_simulation_world__initialize_world>(0x1DDB4E, 0x1C500E), jmp_initialize_world);
	DETOUR_ATTACH(p_c_simulation_world__destroy_world, Memory::GetAddress<t_c_simulation_world__destroy_world>(0x1DE0A9, 0x1C5569), jmp_destroy_world);

	PatchCall(Memory::GetAddress(0x1AE872, 0x0), jmp_update);
	return;
}
