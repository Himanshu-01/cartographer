#include "stdafx.h"
#include "game_state.h"
#include "game_state_procs.h"
#include "debug/debug_gamestate.h"

void __cdecl game_state_initialize(void)
{
	INVOKE(0x30AA6, 0x53EAD, game_state_initialize);
	return;
}

void* __cdecl game_state_malloc(const char* name, const char* description, uint32 size)
{
	return INVOKE(0x2FF1D, 0x53327, game_state_malloc, name, description, size);
}

void* __cdecl game_state_get_buffer_address(uint32* buffer_size)
{	
	if(buffer_size!=nullptr)
	{
		return INVOKE(0x301B0, 0x0, game_state_get_buffer_address, buffer_size);
	}
	
	uint32 temp_buffer_size;
	return INVOKE(0x301B0, 0x0, game_state_get_buffer_address, &temp_buffer_size);
}

void __cdecl game_state_try_and_load_from_persistent_storage(uint32 profile_index)
{
	INVOKE(0x30729, 0x0, game_state_try_and_load_from_persistent_storage, profile_index);
}

void __cdecl game_state_call_after_load_procs_inside_game_state_try_and_load_from_persistent_storage(int32 a2)
{
	game_state_call_after_load_procs(a2);
	debug_gamestate_record_current_state();
}
void game_state_apply_patches()
{
	PatchCall(Memory::GetAddress(0x3082C), game_state_call_after_load_procs_inside_game_state_try_and_load_from_persistent_storage);
}
