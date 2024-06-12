#pragma once
#include "game/player_control.h"

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