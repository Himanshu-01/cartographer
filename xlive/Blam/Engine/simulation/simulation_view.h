#pragma once

#include "machine_id.h"
#include "Networking/Transport/network_observer.h"
#include "memory/ring_buffer.h"

enum e_simulation_view_type : int16
{
	_simulation_view_type_none = 0x0,
	_simulation_view_type_synchronous_to_remote_authority = 0x1,
	_simulation_view_type_synchronous_to_remote_client = 0x2,
	_simulation_view_type_distributed_to_remote_authority = 0x3,
	_simulation_view_type_distributed_to_remote_client = 0x4,
	k_simulation_view_type_count = 0x5,
};

enum e_simulation_view_reason
{
	_simulation_view_reason_none = 0x0,
	_simulation_view_reason_disconnected = 0x1,
	_simulation_view_reason_out_of_sync = 0x2,
	_simulation_view_reason_failed_to_join = 0x3,
	_simulation_view_reason_blocking = 0x4,
	_simulation_view_reason_catchup_fail = 0x5,
	_simulation_view_reason_ended = 0x6,
	_simulation_view_reason_mode_error = 0x7,
	_simulation_view_reason_player_error = 0x8,
	_simulation_view_reason_replication_entity = 0x9,
	_simulation_view_reason_replication_event = 0xA,
	_simulation_view_reason_replication_game_results = 0xB,
	k_simulation_view_reason_count = 0xC,
};

enum e_simulation_view_establishment_mode
{
	_simulation_view_establishment_mode_none = 0x0,
	_simulation_view_establishment_mode_ready_to_connect = 0x1,
	_simulation_view_establishment_mode_established = 0x2,
	_simulation_view_establishment_mode_ready_to_join = 0x3,
	_simulation_view_establishment_mode_joining = 0x4,
	_simulation_view_establishment_mode_active = 0x5,
	k_simulation_view_establishment_mode_count = 0x6,
};

enum e_synchronous_catchup_block_header
{
	_synchronous_catchup_join_initiate_block,
	_synchronous_catchup_gamestate_block,
	_synchronous_catchup_update_block,
};

struct player_action;
struct simulation_update;
struct s_network_message_synchronous_update;
class c_simulation_distributed_view;
class c_simulation_world;

struct s_synchronous_block_header
{
	e_synchronous_catchup_block_header block_type;
	uint32 block_size;
};
ASSERT_STRUCT_SIZE(s_synchronous_block_header, 8);

#pragma pack(push,1)
class c_simulation_view
{
	uint8 field_0[2];
	e_simulation_view_type m_view_type;
	datum m_view_datum_index;
	c_simulation_distributed_view* m_distributed_view;
	c_simulation_world* m_world;
	uint32 m_view_index;
	s_machine_identifier m_machine_identifier;
	uint8 gap_1A[2];
	uint32 m_remote_machine_index;
	c_network_observer* m_observer;
	uint32 m_observer_channel_index;
	e_simulation_view_reason m_view_death_reason;
	e_simulation_view_establishment_mode m_view_establishment_mode;
	uint32 m_view_establishment_identifier;
	e_simulation_view_establishment_mode m_remote_establishment_mode;
	uint32 m_remote_establishment_identifier;
	uint32 m_channel_index;
	uint32 m_channel_connection_identifier;
	void* m_simulation_interface;
	int32 field_48;
	int32 field_4C;
	int32 field_50;
	int32 field_54;
	int32 field_58;
	int32 field_5C;
	int32 field_60;
	int32 field_64;
	int32 field_68;
	int32 field_6C;
	int32 field_70;
	uint8 gap_74;
	uint8 field_74;
	uint8 field_76[2];
	bool m_view_active;
	uint8 field_78[3];
	uint32 m_acknowledged_player_mask;
	int32 m_synchronous_client_action_no;
	int32 m_synchronous_client_update_no;
	uint8 field_88[8];
	int32 m_synchronous_catchup_attempts;
	void* m_synchronous_catchup_buffer;
	uint32 m_synchronous_catchup_buffer_size;
	c_ring_stream m_synchronous_catchup_ring_stream;
	int32 m_synchronous_catchup_buffers_count;
	int32 m_next_action_no;

	bool synchronous_catchup_submit_update(s_network_message_synchronous_update* update);
	void synchronous_catchup_send_data();
	const char* get_view_description(void);
public:
	bool exists() const
	{
		m_view_type != _simulation_view_type_none;
	}
	e_simulation_view_type view_type(void) const
	{
		return m_view_type;
	}
	uint32 get_machine_index(void) const
	{
		return m_remote_machine_index;
	}
	void get_machine_identifier(s_machine_identifier* out)
	{
		csmemcpy(out->machine_identifier, m_machine_identifier.machine_identifier, sizeof(s_machine_identifier));
	}
	e_simulation_view_establishment_mode get_view_establishment_mode(void) const
	{
		return m_view_establishment_mode;
	}
	uint32 get_view_establishment_identifier(void) const
	{
		return m_view_establishment_identifier;
	}
	bool synchronous_catchup_in_progress(void) const
	{
		return m_synchronous_catchup_buffer !=nullptr;
	}
	bool is_dead() const
	{
		m_view_death_reason != _simulation_view_reason_none;
	}
	bool is_dead(e_simulation_view_reason* out_reason) const
	{
		if (out_reason)
			*out_reason = m_view_death_reason;

		return is_dead();
	}

	void kill_view(e_simulation_view_reason reason);
	void dispatch_synchronous_update(simulation_update* host_update);
	void send_message(int32 message_type, uint32 message_size , void* data, bool out_of_band);
	void go_out_of_sync(void);
	
	bool handle_synchronous_update(const s_network_message_synchronous_update* update);
	bool handle_synchronous_actions(int32 action_no, int32 update_no, bool is_out_of_sync, uint32 user_flags, player_action const* actions);
	bool handle_synchronous_join(int32 next_update_number);
	bool handle_synchronous_gamestate(int32 gamestate_offset, int32 message_gamestate_size, void* gamestate_data);
	
	
	static void apply_patches();
};
#pragma pack(pop)
ASSERT_STRUCT_SIZE(c_simulation_view, 0xB4);

class c_simulation_distributed_view
{
	int32 m_field_0;
	void* m_replication_scheduler_vftable;
	int8 m_replication_scheduler[40];
	void* m_replication_entity_manager_view_vftable;
	int8 m_replication_entity_manager_view[20540];
	void* m_replication_event_manager_view_vftable;
	int8 m_replication_event_manager_view[36];
	void* m_replication_control_view_vftable;
	int8 m_replication_control_view[3868];
	void* m_simulation_view_telemetry_provider_vftable;
	int8 m_simulation_view_telemetry_provider[19616];
};
ASSERT_STRUCT_SIZE(c_simulation_distributed_view, 44124);


struct s_simulation_world_view_iterator
{
	uint32 m_type_mask;
	datum m_last_view_index;
};
ASSERT_STRUCT_SIZE(s_simulation_world_view_iterator, 8);