#include "stdafx.h"
#include "simulation_world.h"

#include "simulation_queue_events.h"
#include "simulation_queue_entities.h"
#include "simulation_queue_global_events.h"

#include "simulation.h"
#include "simulation_update.h"
#include "simulation_encoding.h"

#include "game/game_time.h"
#include "math/random_math.h"
#include "memory/bitstream.h"
#include "networking/network_event.h"
#include "networking/network_memory.h"
#include "saved_games/game_state_procs.h"

#include "H2MOD/GUI/imgui_integration/Console/ImGui_ConsoleImpl.h"

// TODO verify if these buffers get saturated quickly
// if that's the case, increse the buffer size
c_simulation_queue g_simulation_queues[k_simulation_queue_count];


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

	p_c_simulation_world__initialize_world(this, type_collection, watcher, distributed_world);
	if (!is_playback())
	{
		queues_initialize();
	}
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


CLASS_HOOK_DECLARE_LABEL(c_simulation_world__update_queue_reset, c_simulation_world::update_queue_reset);
__declspec(naked) void jmp_update_queue_reset(void)
{
	CLASS_HOOK_JMP(c_simulation_world__update_queue_reset, c_simulation_world::update_queue_reset);
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

CLASS_HOOK_DECLARE_LABEL(c_simulation_world__reset_world, c_simulation_world::reset_world);
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

__declspec(naked) void jmp_reset_world(void)
{
	CLASS_HOOK_JMP(c_simulation_world__reset_world, c_simulation_world::reset_world);
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

void c_simulation_world::disconnect(void)
{
	return;
}

CLASS_HOOK_DECLARE_LABEL(c_simulation_world__send_player_acknowledgements_not_during_simulation_reset_in_progress, c_simulation_world::send_player_acknowledgements_not_during_simulation_reset_in_progress);
void c_simulation_world::send_player_acknowledgements_not_during_simulation_reset_in_progress(bool a1)
{
	if (!simulation_reset_in_progress())
	{
		send_player_acknowledgements(a1);
	}
}

void __declspec(naked) jmp_send_player_acknowledgements_not_during_simulation_reset_in_progress()
{
	CLASS_HOOK_JMP(c_simulation_world__send_player_acknowledgements_not_during_simulation_reset_in_progress, c_simulation_world::send_player_acknowledgements_not_during_simulation_reset_in_progress);
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
	return INVOKE_TYPE(0x1DC421, 0x1C38D5, int32(__thiscall*)(c_simulation_world*), this);
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

	WriteJmpTo(Memory::GetAddress(0x1DCDC3, 0x1C4277), jmp_update_queue_reset);
	PatchCall(Memory::GetAddress(0x1AE82A, 0x1A8A84), jmp_reset_world);
	PatchCall(Memory::GetAddress(0x1DD9FB, 0x1C4EBB), jmp_send_player_acknowledgements_not_during_simulation_reset_in_progress);
	return;
}
