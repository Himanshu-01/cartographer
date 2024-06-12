#pragma once
#include "simulation_encoding.h"
#include "simulation_type_collection.h"
#include "simulation_watcher.h"
#include "simulation_world.h"

#include "game/player_control.h"

#define k_maximum_simulation_player_updates 64

struct alignas(8) simulation_update
{
	int32 simulation_time;
	bool simulation_in_progress;
	uint32 player_action_mask;
	int32 field_C;
	player_action player_actions[k_maximum_players];
	uint32 unit_control_mask;
	datum control_unit_index[k_maximum_players];
	unit_control_data unit_control[k_maximum_players];
	bool machine_update_valid;
	simulation_machine_update machine_update;
	int32 player_update_count;
	simulation_player_update player_updates[k_maximum_simulation_player_updates];
	bool flush_gamestate;
	int32 game_time_ticks;
	uint32 random_seed;
};
ASSERT_STRUCT_SIZE(simulation_update, 0x3BD8);

#define k_orginal_sizeof_s_network_message_synchronous_update 15320
#pragma pack(push,1)
struct s_network_message_synchronous_update
{
	simulation_update update;
	c_simulation_queue simulation_bookkeeping_queue;
	c_simulation_queue game_simulation_queue;
};
ASSERT_STRUCT_SIZE(s_network_message_synchronous_update, k_orginal_sizeof_s_network_message_synchronous_update + sizeof(c_simulation_queue) * 2);
#pragma pack(pop)

#define k_orginal_sizeof_s_simulation_update_node (k_orginal_sizeof_s_network_message_synchronous_update + 8)
struct s_simulation_update_node
{
	s_network_message_synchronous_update update_message;
	s_simulation_update_node* next;
	char gap[4];
};
ASSERT_STRUCT_SIZE(s_simulation_update_node, k_orginal_sizeof_s_simulation_update_node + sizeof(c_simulation_queue) * 2);

struct s_simulation_globals
{
	bool initialized;
	bool fatal_error;
	bool aborted;
	int32 field_4;
	bool simulation_invalidate;
	bool simulation_reset_pending;
	bool simulation_reset_in_progress;
	bool field_B;
	c_simulation_world* world;
	c_simulation_watcher* simulation_watcher;
	c_simulation_type_collection* simulation_type_collection;
};
ASSERT_STRUCT_SIZE(s_simulation_globals, 24);

c_simulation_world* simulation_get_world();
s_simulation_globals* simulation_get_globals();

void simulation_reset();
void simulation_fatal_error(void);
bool simulation_reset_in_progress();
bool simulation_starting_up(void);
void simulation_notify_reset_complete();

bool simulation_aborted();
bool simulation_in_progress();
void simulation_destroy_update(void);
bool simulation_query_object_is_predicted(datum object_datum);
c_simulation_type_collection* simulation_get_type_collection();

void __cdecl simulation_process_input(uint32 player_action_mask, const player_action* player_actions);

bool __cdecl simulation_get_machine_active_in_game(s_machine_identifier* machine_identifier);
c_simulation_view* __cdecl simulation_get_remote_view_by_channel(uint32 channel_index);
s_network_message_synchronous_update* simulation_get_synchronous_message();

void simulation_apply_patches(void);
