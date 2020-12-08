#pragma once
#include"..\Blam\Cache\Tags\tag_definitons.h"
#include"h2engine_sound_gestalt.h"


using namespace Blam::Cache::Tags;

/*
* A helper class to facilitate dynamic injection of sound chunks(ugh! tag to be specific)
*/

class sound_gestalt_manager
{
	static h2engine_sound_gestalt* pgestalt;

	static h2_memory_vector<PlaybackParameters> playback_parameter_block;
	static h2_memory_vector<Scales> scales_block;
	static h2_memory_vector<ImportNames> import_names_block;
	static h2_memory_vector<PitchRangeParameters> pitch_range_param_block;
	static h2_memory_vector<PitchRanges> pitch_ranges_block;
	static h2_memory_vector<Permutations> permutations_block;
	static h2_memory_vector<CustomPlaybacks> custom_playbacks_block;
	static h2_memory_vector<RuntimePermutationFlags> runtime_perm_flags_block;
	static h2_memory_vector<PermutationChunks> permutation_chunks_block;
	static h2_memory_vector<Promotions> promotions_block;
	static h2_memory_vector<ExtraInfo> extrainfo_block;

public:
	//function to setup pointers and patchcalls for loading and unloading purpose
	static void Init();
	//function to be called when a newer map gets loaded
	static void Reload();
	//function to be called when an older map gets unloaded to clean stuff
	///mind u,call this before the pointers are erased by the game functions, else LEAKS ^_^
	static void Unload();
	//function to add sound chunks from an ugh! tag
	//returns injected indices of various chunk blocks in the variable 'out'
	static void Add_sound_chunk(ugh &arg0, snd &out);
};
