#include "stdafx.h"
#include "user_interface_networking.h"

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
