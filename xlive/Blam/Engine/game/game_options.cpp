#include "stdafx.h"
#include "game_options.h"

void __cdecl game_options_new(s_game_options* game_options)
{
	INVOKE(0x48D6F, 0x42010, game_options_new, game_options);
	return;
}

bool __cdecl game_options_verify(s_game_options* options)
{
	return INVOKE(0x48DD2, 0x0, game_options_verify, options);
}