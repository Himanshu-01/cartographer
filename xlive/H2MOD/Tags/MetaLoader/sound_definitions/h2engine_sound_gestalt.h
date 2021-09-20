#pragma once
#include "Blam\Cache\TagGroups\sound_cache_file_gestalt_definition.h"
#include "memory_array.h"

typedef s_sound_cache_file_gestalt_group_definition ugh;

/*
* Structure employed by halo2 to store and manage sound gestalt data 
* In case of now secondary_sound_gestalt,no allocation takes place and the structure contains pointers to different tag_blocks
* Consolidated_sound_gestalt is formed in presence of secondary gestalt along with heap allocation for it
*/
struct h2engine_sound_gestalt
{
	unsigned int is_gestalt_initialized;
	h2_memory_array<ugh::s_playback_parameter_block> playback_parameter_block;
	h2_memory_array<ugh::s_scales_block> scales_block;
	h2_memory_array<ugh::s_import_names_block> import_names_block;
	h2_memory_array<ugh::s_pitch_range_parameters_block> pitch_range_param_block;
	h2_memory_array<ugh::s_pitch_ranges_block> pitch_ranges_block;
	h2_memory_array<ugh::s_permutations_block> permutations_block;
	h2_memory_array<ugh::s_custom_playbacks_block> custom_playbacks_block;
	h2_memory_array<ugh::s_runtime_permutation_flags_block> runtime_perm_flags_block;
	h2_memory_array<ugh::s_permuatation_chunks_block> permutation_chunks_block;
	h2_memory_array<ugh::s_promotions_block> promotions_block;
	h2_memory_array<ugh::s_extra_infos_block> extra_info_block;
	void* palloc;
};
TAG_BLOCK_SIZE_ASSERT(h2engine_sound_gestalt, 0x60);