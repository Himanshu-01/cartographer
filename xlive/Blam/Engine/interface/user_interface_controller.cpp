#include "stdafx.h"
#include "Util/Memory.h"
#include "input/input_windows.h"
#include "user_interface_controller.h"
#include "Networking/online/online_account_xbox.h"

s_user_interface_controller_globals* get_user_interface_controller_globals(void)
{
	return Memory::GetAddress<s_user_interface_controller_globals*>(0x96C858,0x999038);
}


bool __cdecl user_interface_controller_is_player_profile_valid(e_controller_index controller_index)
{

	//return INVOKE(0x206B50, 0x1F3F78, user_interface_controller_is_player_profile_valid, controller_index);
	if(IN_RANGE_INCLUSIVE(controller_index,_controller_index_0,_controller_index_3))
		return get_user_interface_controller_globals()->controllers[controller_index].m_flags.test(_controller_state_has_signed_in_bit);
	return false;
}

uint32 __cdecl user_interface_controller_get_next_valid_index(e_controller_index controller_index)
{
	//return INVOKE(0x206B13, 0x1F3F3A, user_interface_controller_get_next_valid_index, controller_index);
	switch (controller_index)
	{
	case NONE:
		return _controller_index_0;
	case _controller_index_0:
		return _controller_index_1;
	case _controller_index_1:
		return _controller_index_2;
	case _controller_index_2:
		return _controller_index_3;
	}
	// should probably replace this with _controller_index_invalid 
	return NONE;
}

uint32 __cdecl user_interface_controller_get_user_index(e_controller_index controller_index)
{
	return INVOKE(0x20687F, 0x1F3CE8, user_interface_controller_get_user_index, controller_index);
}

void __cdecl user_interface_controller_set_user_index(e_controller_index controller_index, uint32 user_index)
{
	INVOKE(0x207342, 0x1F43F2, user_interface_controller_set_user_index, controller_index, user_index);
}

e_controller_index __cdecl user_interface_controller_get_controller_for_user(uint32 user_index)
{
	return INVOKE(0x207365, 0x1F4415, user_interface_controller_get_controller_for_user, user_index);
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
	INVOKE(0x208257, 0x1F491B, user_interface_controller_sign_out, controller_index);
}

void __cdecl user_interface_controller_sign_out_all_controllers()
{
	INVOKE(0x208A28, 0x1F4E9F, user_interface_controller_sign_out_all_controllers);
}

void __cdecl user_interface_controller_sign_in(e_controller_index controller_index, s_saved_game_file_player_profile* profile, uint32 profile_index, int caller_handles_failures)
{
	INVOKE(0x2087BF, 0x1F4CD8, user_interface_controller_sign_in, controller_index, profile, profile_index, caller_handles_failures);
}
void __cdecl user_interface_controller_sign_in_with_identifier(e_controller_index controller_index, s_player_identifier* identifier)
{
	INVOKE(0x208986, 0, user_interface_controller_sign_in_with_identifier, controller_index, identifier);
}
void __cdecl user_interface_controller_sign_in_to_live(e_controller_index controller_index, bool online)
{
	INVOKE(0x208A01, 0, user_interface_controller_sign_in_to_live, controller_index, online);
}

void user_interface_controller_get_profile_data(e_controller_index controller_index, s_saved_game_file_player_profile* profile, uint32* profile_index)
{
	INVOKE_TYPE(0x206890, 0x0, void(__cdecl*)(e_controller_index, s_saved_game_file_player_profile*, uint32*), controller_index, profile, profile_index);
}

bool user_interface_controller_has_gamepad(e_controller_index controller_index)
{
	if (VALID_INDEX(controller_index, k_number_of_controllers))
	{
		return input_has_gamepad(controller_index, nullptr);
		//return input_has_gamepad_plugged(controller_index);
	}
	return false;
}

void  user_interface_controller_pick_profile_dialog(e_controller_index controller_index, int live)
{
	return INVOKE_TYPE(0x209236, 0x0, void(__cdecl*)(e_controller_index, int), controller_index, live);
}

bool __cdecl user_interface_controller_is_guest(e_controller_index controller_index)
{
	//	
	//	v1 = 0xC70 * controller_index;
	//	invalid_xuid = (&g_controller_user_identifiers + v1) == 0;
	//	v3 = &g_controller_user_identifiers + v1;
	//	v4 = 0;
	//	if (!invalid_xuid)
	//		v4 = v3;
	//	return (*(v4 + 8) & 3) != 0;
	//	
	s_user_interface_controller_globals* g_user_interface_controller_globals = get_user_interface_controller_globals();
	XUID* identifier = (XUID*)& g_user_interface_controller_globals->controllers[controller_index].controller_user_identifier;
	if (!ONLINE_USER_VALID(*identifier))
		return false;

	return online_xuid_is_guest_account(*identifier);
}

uint32 __cdecl user_interface_controller_get_guest_controllers_count_for_master(e_controller_index master_controller_index)
{
	if (user_interface_controller_is_guest(master_controller_index))
		return 0;


	s_user_interface_controller_globals* g_user_interface_controller_globals = get_user_interface_controller_globals();
	XUID master_identifier = *(XUID*)&g_user_interface_controller_globals->controllers[master_controller_index].controller_user_identifier;
	if (!ONLINE_USER_VALID(master_identifier))
		return 0;

	uint32 count = 0;
	for (e_controller_index controller_idx = first_controller();
		controller_idx != k_no_controller;
		controller_idx = next_controller(controller_idx))
	{
		if (controller_idx == master_controller_index)
			continue;

		//	if(g_user_interface_controller_globals->controllers[controller_idx].m_flags.test(_controller_state_has_xbox_live_bit))
		//	we dont use xlive sign in in cartographer
		//	skipping this check
		
		if (g_user_interface_controller_globals->controllers[controller_idx].m_flags.test(_controller_state_has_xbox_live_bit))
		{
			s_player_identifier player_id = g_user_interface_controller_globals->controllers[controller_idx].controller_user_identifier;
			XUID compare_id = *(XUID*)&g_user_interface_controller_globals->controllers[controller_idx].controller_user_identifier;
			if (!ONLINE_USER_VALID(compare_id))
				continue;

			if ((compare_id & ~0x3ULL) == (master_identifier & ~0x3ULL))
				count++;
		}

	}

	return count;	

}
