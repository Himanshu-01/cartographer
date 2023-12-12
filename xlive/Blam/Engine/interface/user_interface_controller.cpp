#include "stdafx.h"
#include "Util/Memory.h"
#include "user_interface_controller.h"

s_user_interface_controller_globals* get_user_interface_controller_globals(void)
{
	return Memory::GetAddress<s_user_interface_controller_globals*>(0x96C858);
}


bool __cdecl user_interface_controller_is_player_profile_valid(e_controller_index controller_index)
{

	return INVOKE(0x206B50, 0, user_interface_controller_is_player_profile_valid, controller_index);
}

uint32 __cdecl user_interface_controller_get_next_valid_index(e_controller_index controller_index)
{
	//return INVOKE(0x206B13, 0, user_interface_controller_get_next_valid_index, controller_index);
	switch (controller_index)
	{
	case NONE:
		return 0;
	case 0:
		return 1;
	case 1:
		return 2;
	case 2:
		return 3;
	}
	return NONE;
}

uint32 __cdecl user_interface_controller_get_user_index(e_controller_index controller_index)
{
	return INVOKE(0x20687F, 0, user_interface_controller_get_user_index, controller_index);
}

void __cdecl user_interface_controller_set_user_index(e_controller_index controller_index, uint32 user_index)
{
	INVOKE(0x207342, 0, user_interface_controller_set_user_index, controller_index, user_index);
}

e_controller_index __cdecl user_interface_controller_get_controller_for_user(uint32 user_index)
{
	return INVOKE(0x207365, 0, user_interface_controller_get_controller_for_user, user_index);
}

e_game_team __cdecl user_interface_controller_get_user_active_team(e_controller_index controller_index)
{
	return INVOKE(0x206907, 0, user_interface_controller_get_user_active_team, controller_index);
}

void __cdecl user_interface_controller_set_desired_team_index(e_controller_index controller_index, e_game_team desired_team_index)
{
	INVOKE(0x2068F2, 0, user_interface_controller_set_desired_team_index, controller_index, desired_team_index);
}

e_handicap __cdecl user_interface_controller_get_user_handicap_level(e_controller_index controller_index)
{
	return INVOKE(0x206938, 0, user_interface_controller_get_user_handicap_level, controller_index);
}

void __cdecl user_interface_controller_set_user_handicap_level(e_controller_index controller_index, e_handicap handicap)
{
	INVOKE(0x206923, 0, user_interface_controller_set_user_handicap_level, controller_index, handicap);
}

void __cdecl user_interface_controller_set_griefer(e_controller_index controller_index, bool griefing)
{
	INVOKE(0x206949, 0, user_interface_controller_set_griefer, controller_index, griefing);
}

bool __cdecl user_interface_controller_get_rumble_enabled(e_controller_index controller_index)
{
	return INVOKE(0x207600, 0, user_interface_controller_get_rumble_enabled, controller_index);
}
bool __cdecl user_interface_controller_get_autolevel_enabled(e_controller_index controller_index)
{
	return INVOKE(0x207627, 0, user_interface_controller_get_autolevel_enabled, controller_index);
}

wchar_t* __cdecl user_interface_controller_get_player_profile_name(e_controller_index controller_index)
{
	return INVOKE(0x206B67, 0, user_interface_controller_get_player_profile_name, controller_index);
}

uint32 __cdecl user_interface_controller_get_signed_in_controller_count()
{
	return INVOKE(0x2073AE, 0, user_interface_controller_get_signed_in_controller_count);
}

uint32 __cdecl user_interface_controller_get_signed_in_controllers_mask()
{
	return INVOKE(0x20758D, 0, user_interface_controller_get_signed_in_controllers_mask);
}

uint32 __cdecl user_interface_controller_get_not_signed_in_controller_count()
{
	return INVOKE(0x20740C, 0, user_interface_controller_get_not_signed_in_controller_count);
}

uint32 __cdecl user_interface_controller_get_last_level_played(e_controller_index controller_index)
{
	return INVOKE(0xFE106, 0, user_interface_controller_get_last_level_played, controller_index);
}

uint32 __cdecl user_interface_controller_get_highest_campaign_level_in_signed_in_controllers()
{
	return INVOKE(0x2076F7, 0, user_interface_controller_get_highest_campaign_level_in_signed_in_controllers);
}

void __cdecl user_interface_controller_update_network_properties(e_controller_index controller_index)
{
	INVOKE(0x206A97, 0, user_interface_controller_update_network_properties, controller_index);
}

void __cdecl user_interface_controller_sign_out(e_controller_index controller_index)
{
	INVOKE(0x208257, 0, user_interface_controller_sign_out, controller_index);
}

void __cdecl user_interface_controller_sign_out_all_controllers()
{
	INVOKE(0x208A28, 0, user_interface_controller_sign_out_all_controllers);
}

void __cdecl user_interface_controller_sign_in(e_controller_index controller_index, s_saved_game_file_player_profile* profile, uint32 profile_index, int caller_handles_failures)
{
	INVOKE(0x2087BF, 0, user_interface_controller_sign_in, controller_index, profile, profile_index, caller_handles_failures);
}
void __cdecl user_interface_controller_sign_in_with_identifier(e_controller_index controller_index, s_player_identifier* identifier)
{
	INVOKE(0x208986, 0, user_interface_controller_sign_in_with_identifier, controller_index, identifier);
}
void __cdecl user_interface_controller_sign_in_to_live(e_controller_index controller_index, bool online)
{
	INVOKE(0x208A01, 0, user_interface_controller_sign_in_to_live, controller_index, online);
}