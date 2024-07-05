#include "stdafx.h"
#include "game_options.h"

bool __cdecl game_options_verify(s_game_options* options)
{
	return INVOKE(0x48DD2, 0x0, game_options_verify, options);
}

void game_options_new(s_game_options* game_options)
{
	typedef void(__cdecl* game_options_new_t)(s_game_options*);
	auto p_game_options_new = Memory::GetAddress<game_options_new_t>(0x48D6F, 0x42010);
	p_game_options_new(game_options);
	return;
}
