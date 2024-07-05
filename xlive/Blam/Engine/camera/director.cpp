#include "stdafx.h"
#include "director.h"
#include "game/game.h"

int32 __cdecl director_get_perspective(int32 user_index)
{
	return INVOKE(0x59E2B, 0x47E58, director_get_perspective, user_index);
}

bool director_in_scripted_camera()
{
	return *Memory::GetAddress<bool*>(0x4A8490, 0);
}

void __cdecl director_initialize_for_saved_game()
{
	//INVOKE(0x5A854, 0x0, director_initialize_for_saved_game);
	if (!game_is_playback())
	{
		director_initialize_for_new_map();
		LOG_DEBUG_FUNC("director: initialize_for_new_map");
	}
	director_script_camera(director_in_scripted_camera());
	LOG_DEBUG_FUNC("director: after load success");
}

void __cdecl director_initialize_for_new_map()
{
	INVOKE(0x5A53B, 0x0, director_initialize_for_new_map);
}

void __cdecl director_script_camera(bool script_camera)
{
	INVOKE(0x5A181, 0x0, director_script_camera, script_camera);
}

void director_apply_patches()
{
	constexpr int director_after_load_proc_index = 13;
	WritePointer(Memory::GetAddress(0x413810 + 4* director_after_load_proc_index, 0), director_initialize_for_saved_game); // in after_load_procs
}
