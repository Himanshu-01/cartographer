#pragma once

/* constants */

enum
{
	k_multiplayer_team_count = 8,
};

/* prototypes */

bool __cdecl network_session_interface_initialize(class c_network_session_manager* session_manager);
bool __cdecl network_squad_session_set_game_variant(struct s_game_variant* variant);

void network_session_interface_patches();

/* globals */

extern bool debug_net_distributed_always;
extern bool debug_net_distributed_never;

