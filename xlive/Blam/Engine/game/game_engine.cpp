#include "stdafx.h"
#include "game_engine.h"
#include "game_engine_util.h"
#include "game.h"
#include "game_time.h"
#include "players.h"

s_game_engine_globals* game_engine_globals_get(void)
{
	return *Memory::GetAddress<s_game_engine_globals**>(0x4BF8F8, 0x4EA028);
}

bool __cdecl game_engine_get_change_colors(s_player_profile* player_profile, e_game_team team_index, real_rgb_color* change_colors)
{
	return INVOKE(0x6E5C3, 0x6D1BF, game_engine_get_change_colors, player_profile, team_index, change_colors);
}

bool __cdecl game_engine_variant_cleanup(uint16* flags)
{
	return INVOKE(0x5B720, 0x3D380, game_engine_variant_cleanup, flags);
}

void __cdecl game_engine_player_activated(datum player_index)
{
	INVOKE(0x6A29E, 0x69CB6, game_engine_player_activated, player_index);
	return;
}

void __cdecl game_engine_game_starting(void)
{	
	INVOKE(0x6FE78, 0x0, game_engine_game_starting);
}

void __cdecl game_engine_player_prepare_to_change_team(datum player_index, e_game_team new_team)
{
	// INVOKE(0x6FD29, 0x0, game_engine_player_prepare_to_change_team, a1, a2);

    if (current_game_engine() && game_engine_has_teams() && !game_is_predicted())
    {
        s_player* player = s_player::get(player_index);
        if (player->properties->team_index != new_team && !DATUM_IS_NONE(player->unit_index))
        {
            random_seed_allow_use();
            object_kill_instantly(player->unit_index, false, true, true, nullptr);
            random_seed_disallow_use(_random_seed_in_game_engine_prepare_change_team);
            
            if(!DATUM_IS_NONE(player->unit_index))
            {
                //"killed player 0x%08X but failed to detach from unit 0x%08X %s"
                player_set_unit_index(player_index, NONE);
                player->field_148 = time_globals::get_tickrate() * 2;
            }

        }
    }

}

void game_engine_apply_patches(void)
{
    if (!Memory::IsDedicatedServer())
    {
        PatchCall(Memory::GetAddress(0x550B1, 0), game_engine_player_prepare_to_change_team);
    }
}
