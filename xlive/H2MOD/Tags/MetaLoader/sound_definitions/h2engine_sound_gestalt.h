#pragma once
#include"..\Blam\Cache\Tags\tag_definitons.h"
#include"tagblock.h"

using namespace Blam::Cache::Tags;
/*
* Structure employed by halo2 to store and manage sound gestalt data 
* In case of now secondary_sound_gestalt,no allocation takes place and the structure contains pointers to different tag_blocks
* Consolidated_sound_gestalt is formed in presence of secondary gestalt along with heap allocation for it
*/
struct h2engine_sound_gestalt
{
	unsigned int is_gestalt_initialized;
	h2_memory_array<PlaybackParameters> playback_parameter_block;
	h2_memory_array<Scales> scales_block;
	h2_memory_array<ImportNames> import_names_block;
	h2_memory_array<PitchRangeParameters> pitch_range_param_block;
	h2_memory_array<PitchRanges> pitch_ranges_block;
	h2_memory_array<Permutations> permutations_block;
	h2_memory_array<CustomPlaybacks> custom_playbacks_block;
	h2_memory_array<RuntimePermutationFlags> runtime_perm_flags_block;
	h2_memory_array<PermutationChunks> permutation_chunks_block;
	h2_memory_array<Promotions> promotions_block;
	h2_memory_array<ExtraInfo> extra_info_block;
	void* palloc;
};
TAG_BLOCK_SIZE_ASSERT(h2engine_sound_gestalt, 0x60);