#include "stdafx.h"
#include "simulation_watcher.h"
#include "simulation_players.h"
#include "simulation_world.h"



void c_simulation_watcher::generate_player_updates(int32* player_update_count, int32 maximum_player_update_count, simulation_player_update* player_updates)
{
	return INVOKE_TYPE(0x1D5D24, 0x1C2932, void(__thiscall*)(c_simulation_watcher*, int32*, int32, simulation_player_update*), this, player_update_count, maximum_player_update_count, player_updates);
}

bool c_simulation_watcher::need_to_generate_updates(void)
{
	bool result = INVOKE_TYPE(0x1D4B42, 0x1C188C, bool(__thiscall*)(c_simulation_watcher*), this);
	return (result || !m_sim_world->simulation_queues_empty()) && m_sim_world->is_distributed() && m_sim_world->is_authority();
}

bool c_simulation_watcher::maintain_connection(void)
{
	if (!m_sim_world)
	{
		// if no there is no world , we abort
		return false;
	}
	else if (m_sim_world->is_local() || m_sim_world->is_playback())
	{
		// if we are local or playback we dont want to abort
		return true;
	}

	// else call orignal 
	return INVOKE_TYPE(0x1D6531, 0x0, bool(__thiscall*)(c_simulation_watcher*), this);
}

__declspec(naked) void jmp_maintain_connection() { __asm { jmp c_simulation_watcher::maintain_connection } }

void simulation_watcher_apply_patches()
{
	PatchCall(Memory::GetAddress(0x1AE855, 0), jmp_maintain_connection); // in simulation_update
}
