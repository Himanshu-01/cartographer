#pragma once
#include "game/player_control.h"
#include "simulation/simulation.h"
#include "simulation/simulation_queue.h"

struct s_network_message_synchronous_actions
{
	int32 action_number;
	int32 current_update_number;
	bool out_of_sync;
	uint8 gap_9[3];
	uint32 user_flags;
	player_action user_actions[4];
};
ASSERT_STRUCT_SIZE(s_network_message_synchronous_actions, 0x190);

struct s_network_message_synchronous_join
{
	int32 next_update_number;
};
ASSERT_STRUCT_SIZE(s_network_message_synchronous_join, 4);

struct s_network_message_synchronous_gamestate
{
	int32 gamestate_offset;
	int32 gamestate_size;
};
ASSERT_STRUCT_SIZE(s_network_message_synchronous_gamestate, 8);

#define k_orginal_sizeof_s_network_message_synchronous_update 15320
#pragma pack(push,1)
struct s_network_message_synchronous_update
{
	struct simulation_update update;
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
