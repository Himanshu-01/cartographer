#pragma once

#include "game/game_options.h"

struct s_game_state_header
{
	uint32 alloc_checksum;
	void* base_address;
	static_string256 scenario_name;
	static_string32 game_build;
	uint32 map_checksum;
	int32 field_12C;
	s_game_options options;
	int16 structure_bsp_index;
	int16 pad;
	int32 field_12BC;
	int32 field_12C0;
	int32 field_12C4;
	int32 field_12C8;
	int32 field_12CC;
	int32 field_12D0;
	int32 field_12D4;
	int32 field_12D8;
	int32 field_12DC;
	int32 field_12E0;
	int32 field_12E4;
	int32 field_12E8;
	int32 field_12EC;
	int32 field_12F0;
	int32 field_12F4;
};
ASSERT_STRUCT_SIZE(s_game_state_header, 0x12F8);

void __cdecl game_state_initialize(void);

void* __cdecl game_state_malloc(const char* name, const char* description, uint32 size);

// returns pointer to gamestate_allocation_base_address
void* __cdecl game_state_get_buffer_address(uint32* buffer_size);

void game_state_apply_patches();