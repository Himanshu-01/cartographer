#include "stdafx.h"
#include "user_interface_networking.h"
#include "saved_games/game_variant.h"

void user_interface_squad_clear_match_playlist()
{
	return INVOKE_TYPE(0x209DF4, 0x0, void(*__cdecl)(void));
}

char __cdecl user_interface_create_new_squad(bool a1, bool online)
{
	return INVOKE(0x216345, 0x0, user_interface_create_new_squad, a1, online);
}

void __cdecl user_interface_squad_clear_game_settings()
{
	INVOKE(0x2171A0, 0x0, user_interface_squad_clear_game_settings);
}

void __cdecl user_interface_squad_set_campaign_difficulty(int32 difficulty)
{
	INVOKE(0x215624, 0x0, user_interface_squad_set_campaign_difficulty, difficulty);
}

void __cdecl user_interface_set_desired_multiplayer_mode(int32 desired_mode)
{
	INVOKE(0x217138, 0x0, user_interface_set_desired_multiplayer_mode, desired_mode);
}

e_session_protocol __cdecl user_interface_squad_get_active_protocol()
{
	return INVOKE(0x215EA9, 0x0, user_interface_squad_get_active_protocol);
}

bool __cdecl user_interface_squad_local_peer_is_leader()
{
	return INVOKE(0x2152B0, 0x0, user_interface_squad_local_peer_is_leader);
}

int16 __cdecl user_interface_squad_get_player_count()
{
	return INVOKE(0x21525A, 0x0, user_interface_squad_get_player_count);
}

bool __cdecl user_interface_session_get_map(uint32* campaign_id, uint32* map_id, uint32* custom_map_id)
{
	return INVOKE(0x21564E, 0x0, user_interface_session_get_map, campaign_id, map_id, custom_map_id);
}

int16 __cdecl user_interface_session_get_campaign_difficulty(void)
{
	return INVOKE(0x215697, 0x0, user_interface_session_get_campaign_difficulty);
}

s_game_variant* __cdecl user_interface_session_get_game_variant(void)
{
	return INVOKE(0x215692, 0x0, user_interface_session_get_game_variant);
}