#pragma once
#include "Blam\Cache\TagGroups\sound_cache_file_gestalt_definition.h"
#include "Blam\Cache\TagGroups\sound_definition.h"
#include"h2engine_sound_gestalt.h"

typedef s_sound_cache_file_gestalt_group_definition ugh;
/*
* A helper class to facilitate dynamic injection of sound chunks(ugh! tag to be specific)
*/
class sound_gestalt_manager
{
	static h2engine_sound_gestalt* pgestalt;

	static h2_memory_vector<ugh::s_playback_parameter_block> playback_parameter_block;
	static h2_memory_vector<ugh::s_scales_block> scales_block;
	static h2_memory_vector<ugh::s_import_names_block> import_names_block;
	static h2_memory_vector<ugh::s_pitch_range_parameters_block> pitch_range_param_block;
	static h2_memory_vector<ugh::s_pitch_ranges_block> pitch_ranges_block;
	static h2_memory_vector<ugh::s_permutations_block> permutations_block;
	static h2_memory_vector<ugh::s_custom_playbacks_block> custom_playbacks_block;
	static h2_memory_vector<ugh::s_runtime_permutation_flags_block> runtime_perm_flags_block;
	static h2_memory_vector<ugh::s_permuatation_chunks_block> permutation_chunks_block;
	static h2_memory_vector<ugh::s_promotions_block> promotions_block;
	static h2_memory_vector<ugh::s_extra_infos_block> extrainfo_block;

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
	static void Add_sound_chunk(s_sound_cache_file_gestalt_group_definition& arg0, s_sound_group_definition& out);
};
