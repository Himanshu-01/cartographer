#pragma once
#include "real_math.h"

enum e_random_seed_calls
{
	_random_seed_in_effects_update,
	_random_seed_in_fp_weapons_update,
	_random_seed_in_game_initialize_for_new_map,
	_random_seed_in_game_initialize_for_new_structure_bsp,
	_random_seed_in_game_start,
	_random_seed_in_game_create_objects,
	_random_seed_in_game_create_missing_objects,
	_random_seed_in_game_create_ai,
	_random_seed_in_game_engine_prepare_change_team,
	_random_seed_in_player_effects_impulsive_melee,
	_random_seed_in_simulation_update_pregame,

};

struct s_random_math
{
	uint32 global_seed;
	uint32 local_seed;
};

s_random_math* random_math_get_globals();

uint32 get_random_seed();

uint32 get_local_random_seed();

uint32* get_local_random_seed_address();

void set_random_seed(uint32 seed);

void set_local_random_seed(uint32 random_number);

real32 _real_random_range(uint32* seed, real32 lower_bound, real32 upper_bound);

void seed_iterate_until_point(uint32 seed, uint32 seed_point, int32* out_seed_increment_count);

int16 _random_integer(uint32* seed);

int16 _random_range(uint32* seed, int16 lower_bound, int16 upper_bound);

real_vector3d* _random_direction3d(uint32* seed, const char* type, char* file, int32 line, real_vector3d* direction);

void random_seed_allow_use();
void random_seed_disallow_use(e_random_seed_calls);
void random_math_dump_call_stack();

void random_math_apply_patches();