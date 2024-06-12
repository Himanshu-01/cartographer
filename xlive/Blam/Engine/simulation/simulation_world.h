#pragma once
#include "simulation_actors.h"
#include "simulation_entity_database.h"
#include "simulation_event_handler.h"
#include "simulation_players.h"
#include "simulation_queue.h"
#include "simulation_view.h"

#include "Networking/replication/replication_event_manager.h"
#define k_simulation_world_maximum_views (k_maximum_players + 1)

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

struct s_simulation_update_node;
struct s_network_message_synchronous_update;

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
	void* m_watcher;
	c_simulation_distributed_world* m_distributed_world;
	e_simulation_world_type m_world_type;
	bool m_local_machine_identifier_valid;
	s_machine_identifier m_local_machine_identifier;
	uint8 gap_13;
	int32 m_local_machine_index;
	e_simulation_world_state m_world_state;
	int32 m_join_time_start;
	int32 m_valid_machines_mask;
	bool m_time_running;
	bool m_time_immediate_update;
	uint8 gap_26[2];
	int32 m_next_update_number;
	bool m_out_of_sync;
	uint8 gap_2D;
	bool m_flush_gamestate;
	bool m_attached_to_map;
	int32 m_join_attempt_count;
	int32 m_join_failure_start;
	int field_38;
	int32 m_join_timeout;
	int32 m_view_count;
	c_simulation_view* m_views[k_simulation_world_maximum_views];
	c_simulation_player m_players[k_maximum_players];
	c_simulation_actor m_actors[k_maximum_players];
	char field_1288;
	uint8 gap_1289[3];
	int32 m_synchronous_gamestate_write_progress;
	void* m_synchronous_gamestate_write_buffer;
	uint32 field_1294;
	int32 m_synchronous_client_next_update_number_to_dequeue;
	int32 m_synchronous_client_latest_update_number_received;
	int32 m_synchronous_client_queue_length;
	s_simulation_update_node* m_synchronous_client_queue_head;
	s_simulation_update_node* m_synchronous_client_queue_tail;
	char gap_12A8[4];

public:
	void gamestate_flush_immediate(void);

	void simulation_queue_allocate(e_event_queue_type type, int32 encoded_size, s_simulation_queue_element** out_allocated_elem);
	void simulation_queue_free(s_simulation_queue_element* element);
	void simulation_queue_enqueue(s_simulation_queue_element* element);

	void queues_initialize();
	void apply_simulation_queue(const c_simulation_queue* queue, struct simulation_update* update);

	void attach_simulation_queues_to_update(bool simulation_in_progress, c_simulation_queue* out_bookkeepin_queue, c_simulation_queue* out_game_simulation_queue);

	c_simulation_queue* queue_get(e_simulation_queue_type type) const;

	c_simulation_distributed_world* get_distributed_world(void) const { return m_distributed_world; }

	void initialize_world(int32 a2, int32 a3, int32 a4);

	void delete_all_actors(void);

	void update_queue_reset(void);

	// discard resources
	void reset_world(void);

	void destroy_world();
	void disconnect(void);

	void queues_dispose(void);

	void create_player(datum player_index);
	void delete_player(datum player_index);

	int32 get_time(void) const;

	void build_update(simulation_update* update);
	void update_queue_retrieve_update(simulation_update* update);
	bool update_queue_handle_server_update(const s_network_message_synchronous_update* host_update);
	int32 time_get_available(bool* match_remote_time);
	
	bool handle_synchronous_update(const s_network_message_synchronous_update* update);
	void handle_synchronous_client_actions(s_machine_identifier const* remote_machine_identifier, uint32 user_flags, player_action const* action);
	bool handle_synchronous_join(s_network_message_synchronous_update* update);
	bool handle_synchronous_gamestate(s_network_message_synchronous_update* update);

	void time_start(int32 next_update_number);
	void time_set_immediate_update(bool update_immediately);
	bool synchronous_gamestate_write_start(void);
	bool synchronous_gamestate_decompress_and_load(int32 gamestate_offset);
	bool synchronous_gamestate_write_chunk(int32 gamestate_offset, int32 gamestate_size, void* gamestate_data);

	static void iterator_begin(s_simulation_world_view_iterator* iterator, uint32 view_type_mask);
	bool iterator_next(s_simulation_world_view_iterator* iterator, c_simulation_view** view);
	c_simulation_view* get_client_view_by_machine_index(uint32 machine_index);
	c_simulation_view* get_client_view_by_machine_identifier(s_machine_identifier* machine_identifier);
	c_simulation_view* get_authority_view();

	void queues_update_statistsics()
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

	void queues_clear();

	bool is_playback() const
	{
		// todo: re-add once destroy_world function is re-written
		ASSERT(exists());
		return false;
	}

	bool is_distributed(void) const
	{
		return m_world_type == _simulation_world_type_distributed_authority
			|| m_world_type == _simulation_world_type_distributed_client;
	}

	bool exists() const
	{
		return m_world_type != _simulation_world_type_none;
	}

	bool runs_simulation() const
	{
		ASSERT(exists());
		return m_world_type != _simulation_world_type_synchronous_client;
	}

	bool is_authority() const
	{
		ASSERT(exists());
		return m_world_type != _simulation_world_type_distributed_client && m_world_type != _simulation_world_type_synchronous_client;
	}

	bool is_active() const
	{
		ASSERT(exists());
		return m_world_state == _simulation_world_state_active;
	}

	bool is_out_of_sync(void) const
	{
		ASSERT(exists());
		if (!is_authority() || is_playback())
			return m_out_of_sync;
	}

	e_simulation_world_type get_world_type(void) const
	{
		ASSERT(exists());
		return m_world_type;
	}

	e_simulation_world_state get_state(void) const
	{
		ASSERT(exists());
		return m_world_state;
	}

	int32 get_next_update_number(void) const
	{
		ASSERT(exists());
		return m_next_update_number;
	}

	bool simulation_queues_empty()
	{
		return queue_get(_simulation_queue_bookkeeping)->queued_count() == 0 && queue_get(_simulation_queue)->queued_count() == 0;
	}

	void send_player_acknowledgements_not_during_simulation_reset_in_progress(bool a1);

	void send_player_acknowledgements(bool a1)
	{
		INVOKE_TYPE(0x1DD777, 0x1C4C37, void(__thiscall*)(c_simulation_world*, bool), this, a1);
	}

	bool time_running(void)
	{
		ASSERT(exists());
		return m_time_running;
	}

	bool synchronous_gamestate_write_in_progress() const
	{
		if (exists() && !is_authority() && !is_distributed())
		{
			return m_synchronous_gamestate_write_progress != NONE;
		}
		return false;
	}

	int32 update_queue_get_next_expected_update_number() const
	{
		return m_synchronous_client_latest_update_number_received + 1;
	}

	bool do_we_have_sufficient_updates(void)
	{
		return m_synchronous_client_queue_length > 0;
	}

	void force_immediate_update(void)
	{
		m_time_immediate_update = true;
	}

	void go_out_of_sync(void)
	{
		m_out_of_sync = true;
	}
};
ASSERT_STRUCT_SIZE(c_simulation_world, 0x12B0);

void simulation_world_apply_patches();