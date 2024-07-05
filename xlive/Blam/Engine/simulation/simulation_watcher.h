#pragma once

struct simulation_player_update;

class c_simulation_world;
class c_simulation_watcher
{
public:
	uint8 gap_0[4];
	c_simulation_world* m_sim_world;

	void generate_player_updates(int32* player_update_count, int32 maximum_player_update_count, simulation_player_update* player_updates);

	bool need_to_generate_updates(void);

	bool maintain_connection(void);

};


void simulation_watcher_apply_patches();