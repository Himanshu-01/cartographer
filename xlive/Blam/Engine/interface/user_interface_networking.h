#pragma once

enum e_session_protocol
{
	_protocol_splitscreen_coop = 0x0,
	_protocol_splitscreen_custom = 0x1,
	_protocol_system_link_coop = 0x2,
	_protocol_system_link_custom = 0x3,
	_protocol_live_coop = 0x4,
	_protocol_live_custom = 0x5,
	_protocol_live_optimatch = 0x6,
};

struct s_game_variant;

void user_interface_squad_clear_match_playlist();

char __cdecl user_interface_create_new_squad(bool a1, bool online);
void __cdecl user_interface_squad_clear_game_settings();
void __cdecl user_interface_squad_set_campaign_difficulty(int32 difficulty);
void __cdecl user_interface_set_desired_multiplayer_mode(int32 desired_mode);


e_session_protocol __cdecl user_interface_squad_get_active_protocol();
bool __cdecl user_interface_squad_local_peer_is_leader();
int16 __cdecl user_interface_squad_get_player_count();

bool __cdecl user_interface_session_get_map(uint32* campaign_id, uint32* map_id, uint32* custom_map_id);
int16 __cdecl user_interface_session_get_campaign_difficulty(void);
s_game_variant* __cdecl user_interface_session_get_game_variant(void);