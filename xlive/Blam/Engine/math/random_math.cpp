#include "stdafx.h"
#include "random_math.h"

#include "random_direction_table.h"
#include "game/game_time.h"
#include "game/game.h"
#include "debug/debug_determinism.h"


// More info:
// https://en.wikipedia.org/wiki/Linear_congruential_generator
// https://www.cec.uchile.cl/cinetica/pcordero/MC_libros/NumericalRecipesinC.pdf
// Page 284 to 285

#define RANDOM_A 1664525
#define RANDOM_C 1013904223
#define RANDOM_NEW_SEED(prev_seed) ((RANDOM_A * (prev_seed) + RANDOM_C))

#define RANDOM_FLONE 1.0f
#define RANDOM_MASK 0xffff // 0xffff

// DIV_BY_MAX_MASK Compiles to 0.000015259022
#define DIV_BY_MAX_MASK_REAL (RANDOM_FLONE / (real32)RANDOM_MASK)

// get the HIGH bits of the seed (between 65535 ... 0)
#define SEED_HIWORD(seed) ((seed) >> 16)

// integer random
#define INTEGER_RANDOM(seed) SEED_HIWORD(seed)
#define INTEGER_RANDOM_RANGE(seed, lower_bound, delta) ((lower_bound) + (((delta) * INTEGER_RANDOM(seed)) >> 16))

// floating point random
// generates a value between 1 ... 0 <===> (65535 ... 0) / 65535 
// [65535 = (2^16) - 1]
#define REAL_RANDOM(seed) (DIV_BY_MAX_MASK_REAL * SEED_HIWORD(seed))
#define REAL_RANDOM_RANGE(seed, lower_bound, delta) ((lower_bound) + (delta) * REAL_RANDOM(seed))

#pragma intrinsic(_ReturnAddress)

int32 g_deterministic_seed_allowed_usages = 0;

s_random_math* random_math_get_globals()
{
	return *Memory::GetAddress<s_random_math**>(0x4A8280, 0x4D2500);
}

uint32 get_random_seed()
{
	return random_math_get_globals()->global_seed;
}

uint32 get_local_random_seed()
{
	return random_math_get_globals()->local_seed;
}

uint32* get_random_seed_address()
{
	return &random_math_get_globals()->global_seed;
}

uint32* get_local_random_seed_address()
{
	return &random_math_get_globals()->local_seed;
}

void set_random_seed(uint32 seed)
{
	random_math_get_globals()->global_seed = seed;
}

void set_local_random_seed(uint32 seed)
{
	random_math_get_globals()->local_seed = seed;
}

void seed_iterate_until_point(uint32 seed, uint32 seed_point, int32* out_seed_increment_count)
{
	*out_seed_increment_count = 0;
	while (seed != seed_point)
	{
		seed = RANDOM_NEW_SEED(seed);
		(*out_seed_increment_count)++;
	}
}

real32 _real_random_range(uint32* seed, real32 lower_bound, real32 upper_bound)
{
	*seed = RANDOM_NEW_SEED(*seed);
	return REAL_RANDOM_RANGE(*seed, lower_bound, upper_bound - lower_bound);
}

int16 _random_range(uint32* seed, int16 lower_bound, int16 upper_bound)
{
	*seed = RANDOM_NEW_SEED(*seed);
	return INTEGER_RANDOM_RANGE(*seed, lower_bound, upper_bound - lower_bound);
}

int16 _random_integer(uint32* seed)
{
	*seed = RANDOM_NEW_SEED(*seed);
	return INTEGER_RANDOM(*seed);
}

real_vector3d* _random_direction3d(uint32* seed, const char* type, char* file, int32 line, real_vector3d* direction)
{
	int32 index = _random_range(seed, 0, k_random_direction_table_size);
	*direction = g_random_direction_table[index];
	return direction;
}

const char* random_seed_disallow_calls[] =
{
	"effects_update",
	"fp_weapons_update",
	"game_initialize_for_new_map",
	"game_initialize_for_new_structure_bsp",
	"game_start",
	"game_create_objects",
	"game_create_missing_objects",
	"game_create_ai",
	"game_engine_prepare_change_team",
	"player_effects_impulsive_melee",
	"simulation_update_pregame",
};

static void __cdecl random_math_log_bad_access(DWORD ret_address)
{
	ret_address -= Memory::GetAddress();
	if (g_deterministic_seed_allowed_usages <= 0)
	{
		LOG_CRITICAL_NETWORK("someone is using the global random number generator when they shouldn't be at 0x{:X}  time [{}]", (void*)ret_address, time_globals::get_game_time());
	}

	if(!game_is_ui_shell())
	{
		debug_random_record_call_entry(ret_address);
	}

}

__declspec(naked) uint32* get_random_seed_address_hook()
{
	__asm
	{
		// grab caller_address
		mov     eax, [esp]
		push	eax
		call    random_math_log_bad_access
		add esp, 4
		//retn

		// original code
		call	random_math_get_globals
		retn
	}
}

void random_seed_allow_use()
{
	++g_deterministic_seed_allowed_usages;
}

void random_seed_disallow_use(e_random_seed_calls caller)
{
	if (g_deterministic_seed_allowed_usages <= 0)
	{
		//"unmatched call to random_seed_disallow() somewhere",
		LOG_CRITICAL_NETWORK("unmatched call to random_seed_disallow() at {} , value : {} time [{}]", random_seed_disallow_calls[caller],g_deterministic_seed_allowed_usages, time_globals::get_game_time());
	}
	else
	{
		--g_deterministic_seed_allowed_usages;
	}
}


void random_math_apply_patches()
{
	// hooking into deterministic random no generator
	WriteJmpTo(Memory::GetAddress(0x5908F), get_random_seed_address_hook);

	// patch game_sound_deterministic_pick_permutation to use global seed
	// these use deterministic-seed in h3
	PatchCall(Memory::GetAddress(0xA57AB), get_random_seed_address_hook);
	PatchCall(Memory::GetAddress(0xA5892), get_random_seed_address_hook);

	// patch animation function to force use local_seed_address only
	//  breaks game dont use
	// PatchCall(Memory::GetAddress(0xF6E00), get_local_random_seed_address);
	

	// path effect_tag_get_random_seed to always use local_seed
	// breaks game eventually
	// PatchCall(Memory::GetAddress(0xA750B), get_local_random_seed_address);	
	
	// path effect_object_accelerate to use local_seed
	// breaks game very fast dont use
	//PatchCall(Memory::GetAddress(0xA8784), get_local_random_seed_address);


}
