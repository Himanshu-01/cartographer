#include "stdafx.h"
#include "game_engine_util.h"

#include "game/game.h"
#include "game/game_engine.h"
#include "networking/network_game_definitions.h"

/* public code */

bool game_engine_in_round()
{
	return current_game_engine() != NULL
		&& game_engine_globals_get()->field_6C == 1
		&& (game_is_predicted() || game_engine_globals_get()->field_C44 == 1);
}

void game_engine_check_for_round_winner()
{
	typedef void(__cdecl game_engine_check_for_round_winner_t)();
	auto p_game_engine_check_for_round_winner = Memory::GetAddress<game_engine_check_for_round_winner_t*>(0x70F49, 0x6FA4A);
	p_game_engine_check_for_round_winner();
}

void game_engine_end_round_with_winner(int player_datum_or_team_index, bool go_to_next_round)
{
	typedef void(__cdecl game_engine_end_round_with_winner_t)(int player_datum_or_team_index, bool go_to_next_round);
	auto p_game_engine_end_round_with_winner = Memory::GetAddress<game_engine_end_round_with_winner_t*>(0x70A6F, 0x6F570);
	p_game_engine_end_round_with_winner(player_datum_or_team_index, go_to_next_round);
}

bool game_engine_has_teams()
{
	if (current_game_engine())
	{
		return TEST_BIT(current_game_variant()->game_engine_flags, _game_engine_teams_bit);
	}

	return false;
}

e_network_game_simulation_protocol game_engine_get_simulation_protocol(struct s_game_variant* variant)
{
	e_network_game_simulation_protocol result = _network_game_simulation_protocol_synchronous;
	switch (variant->variant_game_engine_index)
	{
	case _game_engine_type_ctf:
	case _game_engine_type_slayer:
	case _game_engine_type_oddball:
	case _game_engine_type_koth:
	case _game_engine_type_juggernaut:
	case _game_engine_type_territories:
	case _game_engine_type_assault:
		result = _network_game_simulation_protocol_distributed;
		break;
	}
	return result;
}

bool __cdecl sub_4701B6(datum player_index)
{
	return INVOKE(0x701B6, 0x0, sub_4701B6, player_index);
}
