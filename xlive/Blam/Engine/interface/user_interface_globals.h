#pragma once
#include "user_interface.h"


int32 __cdecl user_interface_globals_get_game_difficulty();
void __cdecl user_interface_globals_set_game_difficulty_real(int32 difficulty);
void __cdecl user_interface_globals_campaign_unk(bool a1);


void* __cdecl c_screen_bungie_news_load(s_screen_parameters* params);
void* __cdecl c_screen_network_squad_browser_load(s_screen_parameters* params);