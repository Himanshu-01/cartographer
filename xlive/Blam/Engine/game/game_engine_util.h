#pragma once

/* public code */
enum e_network_game_simulation_protocol : int32;

/* public code */

bool game_engine_in_round();

void game_engine_check_for_round_winner();
void game_engine_end_round_with_winner(int player_datum_or_team_index, bool go_to_next_round);
bool game_engine_has_teams();

e_network_game_simulation_protocol game_engine_get_simulation_protocol(struct s_game_variant* variant);

// ### TODO: function name
bool __cdecl sub_4701B6(datum player_index);
