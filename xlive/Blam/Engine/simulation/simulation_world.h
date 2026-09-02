#pragma once
#include "simulation_actors.h"
#include "simulation_entity_database.h"
#include "simulation_event_handler.h"
#include "simulation_players.h"
#include "simulation_queue.h"
#include "simulation_view.h"

#include "networking/replication/replication_event_manager.h"

/* constants */

enum
{
	k_simulation_world_maximum_views = k_maximum_players,
	k_network_maximum_actors_per_simulation = 16,
};

/* enums */

enum e_simulation_queue_type
{
	_simulation_queue_bookkeeping,
	_simulation_queue,

	k_simulation_queue_count
};

enum e_simulation_world_type
{
	_simulation_world_type_none = 0x0,
	_simulation_world_type_local = 0x1,
	_simulation_world_type_synchronous_authority = 0x2,
	_simulation_world_type_synchronous_client = 0x3,
	_simulation_world_type_distributed_authority = 0x4,
	_simulation_world_type_distributed_client = 0x5,
	k_simulation_world_type_count = 0x6,
};

enum e_simulation_world_state
{
	_simulation_world_state_none = 0x0,
	_simulation_world_state_dead = 0x1,
	_simulation_world_state_disconnected = 0x2,
	_simulation_world_state_joining = 0x3,
	_simulation_world_state_active = 0x4,
	_simulation_world_state_handoff = 0x5,
	_simulation_world_state_leaving = 0x6,
	k_simulation_world_state_count = 0x7,
};

/* structure */

struct s_world_state_disconnected
{
	uint32 disconnected_timestamp;
};

struct s_world_state_data_joining
{
	uint32 join_start_timestamp;
	uint32 join_client_machine_mask;
};

struct s_world_state_data_active
{
	uint32 active_client_machine_mask;
};

union s_world_state_data
{
	s_world_state_disconnected disconnected;
	s_world_state_data_joining joining;
	s_world_state_data_active active;
};
ASSERT_STRUCT_SIZE(s_world_state_data, 0x8);

struct s_simulation_world_view_iterator
{
	uint32 view_type_mask;
	datum next_world_view_index;
};
ASSERT_STRUCT_SIZE(s_simulation_world_view_iterator, 8);


/* classes */


class c_simulation_distributed_world
{
public:
	c_replication_entity_manager m_entity_manager;
	c_replication_event_manager m_event_manager;
	c_simulation_entity_database m_entity_database;
	c_simulation_event_handler m_event_handler;	
};
ASSERT_STRUCT_SIZE(c_simulation_distributed_world, 45260);

class c_simulation_world
{
	enum e_join_progress
	{
		_join_progress_waiting = 0,
		_join_progress_ready,
		_join_progress_complete,
		_join_progress_failed,
		k_join_progress_count,
	};

	class c_simulation_watcher* m_watcher;
	c_simulation_distributed_world* m_distributed_world;
	e_simulation_world_type m_world_type;
	bool m_local_machine_identifier_valid;
	s_machine_identifier m_local_machine_identifier;
	int32 m_local_machine_index;
	e_simulation_world_state m_world_state;
	s_world_state_data m_world_state_data;
	bool m_time_running;
	bool m_time_immediate_update;
	int m_next_update_number;
	bool m_out_of_sync;
	bool m_out_of_sync_determinism_failure;
	bool m_gamestate_flushed;
	bool m_attached_to_map;
	int32 m_unsuccessful_join_attempts;
	uint32 m_last_active_timestamp;
	int32 m_next_view_establishment_identifier;
	int32 m_joining_total_wait_msec;
	int32 m_view_count;
	c_simulation_view* m_views[k_simulation_world_maximum_views];
	int32 m_player_count; // guessed name for potential use, field is completely unused
	c_simulation_player m_players[k_maximum_players];
	c_simulation_actor m_actors[k_network_maximum_actors_per_simulation];
	bool m_synchronous_gamestate_read_in_progress;
	int32 m_synchronous_gamestate_write_progress;
	void* m_synchronous_gamestate_write_buffer;
	uint32 m_synchronous_catchup_initiation_failure_timestamp;
	int32 m_update_queue_next_update_number_to_dequeue;
	int32 m_update_queue_latest_update_number_received;
	int32 m_update_queue_length;
	struct s_simulation_queued_update* m_update_queue_head;
	struct s_simulation_queued_update* m_update_queue_tail;
	int32 _pad_12AC;

public:

	void simulation_queue_allocate(e_event_queue_type type, int32 encoded_size, s_simulation_queue_element** out_allocated_elem);
	void simulation_queue_free(s_simulation_queue_element* element);
	void simulation_queue_enqueue(s_simulation_queue_element* element);

	void queues_initialize(void);
	void apply_simulation_queue(c_simulation_queue const* queue);

	void attach_simulation_queues_to_update(struct simulation_update* update);

	c_simulation_queue* queue_get(e_simulation_queue_type type) const;

	c_simulation_distributed_world* get_distributed_world(void) const { return m_distributed_world; }

	void initialize_world(c_simulation_type_collection* type_collection, class c_simulation_watcher* watcher, c_simulation_distributed_world* distributed_world);
	
	bool claim_authority_gameworld(void);

	void delete_all_actors(void);

	void update_queue_start(int32 next_update_number);
	void update_queue_stop(void);
	void update_queue_reset(void);

	// discard resources
	void reset_world(void);

	void destroy_world(void);
	void disconnect(void);
	void update(void);

	void queues_dispose(void);

	void create_player(datum player_index);
	void delete_player(datum player_index);


	void time_start(int32 next_update_number);
	void time_stop(void);
	void time_set_immediate_update(bool time_immediate_update);
	int32 time_get_available(bool* match_remote_time);

	void iterator_begin(struct s_simulation_world_view_iterator* iterator, uint32 view_type_mask);
	bool iterator_next(struct s_simulation_world_view_iterator* iterator, class c_simulation_view** view) const;
	class c_simulation_view* get_authority_view(void);
	class c_simulation_view* get_client_view_by_machine_index(int32 remote_machine_index);
	void handle_view_establishment(const class c_simulation_view* view, bool established);
	void handle_view_activation(const class c_simulation_view* view, bool active);
	void remove_all_views();

	bool authority_join_timeout_expired(void);
	void update_authority_join_initiate(void);
	void update_authority_join_progress(void);
	void update_authority_active(void);
	void update_authority_handoff(void);

	void update_client_join_initiate(void);
	void update_client_join_progress(void);
	void update_client_failure(void);
	void update_client_disconnection(void);

	void drop_simulation_from_active_to_joining(void);
	e_join_progress update_joining_view(class c_simulation_view* view);//#TODO
	void update_establishing_view(class c_simulation_view* view);
	void update_player_activation(void);
	void verify_player_activation(void);


	void change_state_internal(e_simulation_world_state new_state);
	void change_state_joining(uint32 joining_client_machine_mask);
	void change_state_active(void);
	void change_state_disconnected(void);
	void change_state_dead(void);
	void change_state_handoff(void);
	void change_state_leaving(void);

	void build_player_actions(struct simulation_update* update);

	void build_update(struct simulation_update* update);
	void distribute_update(const struct simulation_update* update);
	void advance_update(const struct simulation_update* update);
	void destroy_update(struct simulation_update* update);
	void synchronous_authority_dispatch_update(struct simulation_update const* update);
	int32 synchronous_authority_get_maximum_updates(void);
	bool handle_synchronous_update(const struct simulation_update* update);
	bool update_queue_handle_server_update(const struct simulation_update* update);
	void update_queue_retrieve_update(struct simulation_update* update);
	int32 update_queue_get_available_updates(void) const;


	void distributed_authority_dispatch_player_actions(uint32 player_valid_mask, const struct player_action* player_actions);
	void distributed_authority_dispatch_actor_control(uint32 actor_valid_mask, const struct unit_control_data* actor_control);

	void gamestate_flush(void);
	void go_out_of_sync(void);
	int32 get_time(void) const;

	bool can_generate_updates(void);

	void queues_update_statistics(void)
	{
		for (int32 i = 0; i < k_simulation_queue_count; i++)
		{
			queue_get((e_simulation_queue_type)i)->build_statistics();
		}
	}

	bool queue_describe(e_simulation_queue_type type, const s_simulation_queue_stats** out_stats) const
	{
		return queue_get(type)->get_statistics(out_stats);
	}

	void queues_clear(void);

	bool is_playback(void) const
	{
		// todo: re-add once destroy_world function is re-written
		//ASSERT(exists());
		return false;
	}

	bool is_distributed(void) const
	{
		return m_world_type == _simulation_world_type_distributed_authority
			|| m_world_type == _simulation_world_type_distributed_client;
	}

	bool is_synchronous(void) const
	{
		return m_world_type == _simulation_world_type_synchronous_authority
			|| m_world_type == _simulation_world_type_synchronous_client;
	}

	bool exists(void) const
	{
		return m_world_type != _simulation_world_type_none;
	}

	bool runs_simulation(void) const
	{
		ASSERT(exists());
		return m_world_type != _simulation_world_type_synchronous_client && !is_playback();
	}

	bool is_authority(void) const
	{
		ASSERT(exists());
		return m_world_type != _simulation_world_type_distributed_client && m_world_type != _simulation_world_type_synchronous_client;
	}

	bool is_active(void) const
	{
		ASSERT(exists());
		return m_world_state == _simulation_world_state_active;
	}

	bool is_dead(void) const
	{
		ASSERT(exists());
		return m_world_state == _simulation_world_state_dead;
	}

	bool is_connected(
		void) const
	{
		ASSERT(exists());

		return IN_RANGE(m_world_state, _simulation_world_state_active, _simulation_world_state_leaving);
	}

	bool is_joining(
		void) const
	{
		ASSERT(exists());

		return m_world_state == _simulation_world_state_joining;
	}

	bool is_local(
		void) const
	{
		ASSERT(exists());

		bool is_local = m_world_type == _simulation_world_type_local;

		ASSERT(!is_local || m_view_count == 0);

		return is_local;
	}

	bool simulation_queues_empty(void) const
	{
		return queue_get(_simulation_queue_bookkeeping)->queued_count() == 0 && queue_get(_simulation_queue)->queued_count() == 0;
	}

	void send_player_acknowledgements(bool force_acknowledgement)
	{
		INVOKE_TYPE(0x1DD777, 0x1C4C37, void(__thiscall*)(c_simulation_world*, bool), this, force_acknowledgement);
		return;
	}

	uint32 c_simulation_world::get_acknowledged_player_mask(
		void) const
	{
		return INVOKE_TYPE(0x1DCA76, 0x0, uint32(__thiscall*)(c_simulation_world const*), this);
	}

	bool time_running(void) const
	{
		ASSERT(exists());
		return m_time_running;
	}

	e_simulation_world_state get_state(
		void) const
	{
		ASSERT(exists());
		return m_world_state;
	}

	int32 get_next_update_number(
		void) const
	{
		ASSERT(exists());
		return m_next_update_number;
	}

	int32 update_queue_get_next_expected_update_number(
		void) const
	{
		return m_update_queue_latest_update_number_received + 1;
	}

	bool is_out_of_sync(
		void) const
	{
		ASSERT(exists());
		return (!is_authority() || is_playback()) && m_out_of_sync;
	}

	int32 get_view_count(
		void) const
	{
		return m_view_count;
	}

	bool synchronous_gamestate_write_in_progress(
		void) const
	{
		return exists() 
			&& !is_authority() 
			&& !is_distributed() 
			&& m_synchronous_gamestate_write_progress != NONE;
	}

	void synchronous_gamestate_write_stop(
		void)
	{
		INVOKE_TYPE(0x1DCD9B, 0x0, void(__thiscall*)(c_simulation_world*), this);
		return;
	}

	bool synchronous_catchup_in_progress(
		void) const
	{
		return m_world_type
			&& !is_authority()
			&& !is_distributed()
			&& m_synchronous_gamestate_write_buffer != NULL;
	}

	int32 c_simulation_world::get_machine_index(
		void) const
	{
		return m_local_machine_index;
	}

};
ASSERT_STRUCT_SIZE(c_simulation_world, 0x12B0);

void simulation_world_apply_patches(void);