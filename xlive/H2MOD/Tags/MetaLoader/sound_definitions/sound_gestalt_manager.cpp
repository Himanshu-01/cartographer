#include "stdafx.h"
#include "sound_gestalt_manager.h"
#include"..\H2MOD.h"
#include "..\Util\Hooks\Hook.h"
#include"..\h2_functions\h2_memory_funcs.h"

//define static variables
h2engine_sound_gestalt* sound_gestalt_manager::pgestalt;

h2_memory_vector<ugh::s_playback_parameter_block> sound_gestalt_manager::playback_parameter_block;
h2_memory_vector<ugh::s_scales_block> sound_gestalt_manager::scales_block;
h2_memory_vector<ugh::s_import_names_block> sound_gestalt_manager::import_names_block;
h2_memory_vector<ugh::s_pitch_range_parameters_block> sound_gestalt_manager::pitch_range_param_block;
h2_memory_vector<ugh::s_pitch_ranges_block> sound_gestalt_manager::pitch_ranges_block;
h2_memory_vector<ugh::s_permutations_block> sound_gestalt_manager::permutations_block;
h2_memory_vector<ugh::s_custom_playbacks_block> sound_gestalt_manager::custom_playbacks_block;
h2_memory_vector<ugh::s_runtime_permutation_flags_block> sound_gestalt_manager::runtime_perm_flags_block;
h2_memory_vector<ugh::s_permuatation_chunks_block> sound_gestalt_manager::permutation_chunks_block;
h2_memory_vector<ugh::s_promotions_block> sound_gestalt_manager::promotions_block;
h2_memory_vector<ugh::s_extra_infos_block> sound_gestalt_manager::extrainfo_block;

//* Patch call
void __cdecl patchcall_h2_DeallocUghTag()
{
	sound_gestalt_manager::Unload();
}

void sound_gestalt_manager::Init()
{
	pgestalt = (h2engine_sound_gestalt*)h2mod->GetAddress(0x482298);//TODO

	playback_parameter_block.Init(&pgestalt->playback_parameter_block);
	scales_block.Init(&pgestalt->scales_block);
	import_names_block.Init(&pgestalt->import_names_block);
	pitch_range_param_block.Init(&pgestalt->pitch_range_param_block);
	pitch_ranges_block.Init(&pgestalt->pitch_ranges_block);
	permutations_block.Init(&pgestalt->permutations_block);
	custom_playbacks_block.Init(&pgestalt->custom_playbacks_block);
	runtime_perm_flags_block.Init(&pgestalt->runtime_perm_flags_block);
	permutation_chunks_block.Init(&pgestalt->permutation_chunks_block);
	promotions_block.Init(&pgestalt->promotions_block);
	extrainfo_block.Init(&pgestalt->extra_info_block);

	//patch calls for auto unloading
	PatchCall(h2mod->GetAddress(0x30CEE), patchcall_h2_DeallocUghTag);//h2_sound_gestalt deallocator->TODO
}

void sound_gestalt_manager::Reload()
{
	if (pgestalt == nullptr)
		return;

	if (pgestalt->is_gestalt_initialized != 1)
		return;

	playback_parameter_block.Allocate_new();
	scales_block.Allocate_new();
	import_names_block.Allocate_new();
	pitch_range_param_block.Allocate_new();
	pitch_ranges_block.Allocate_new();
	permutations_block.Allocate_new();
	custom_playbacks_block.Allocate_new();
	runtime_perm_flags_block.Allocate_new();
	permutation_chunks_block.Allocate_new();
	promotions_block.Allocate_new();
	extrainfo_block.Allocate_new();

	if (pgestalt->palloc != nullptr)
	{
		halo2_memory_functions::_DeallocPtr((char*)pgestalt->palloc);
		pgestalt->palloc = nullptr;
	}
}
void sound_gestalt_manager::Unload()
{
	if (pgestalt == nullptr)
		return;

	if (pgestalt->is_gestalt_initialized != 1)
		return;

	pgestalt->is_gestalt_initialized = 0;

	playback_parameter_block.Deallocate_new();
	scales_block.Deallocate_new();
	import_names_block.Deallocate_new();
	pitch_range_param_block.Deallocate_new();
	pitch_ranges_block.Deallocate_new();
	permutations_block.Deallocate_new();
	custom_playbacks_block.Deallocate_new();
	runtime_perm_flags_block.Deallocate_new();
	permutation_chunks_block.Deallocate_new();
	promotions_block.Deallocate_new();
	extrainfo_block.Deallocate_new();

	if (pgestalt->palloc != nullptr)
	{
		halo2_memory_functions::_DeallocPtr((char*)pgestalt->palloc);
		pgestalt->palloc = nullptr;
	}
}

void sound_gestalt_manager::Add_sound_chunk(s_sound_cache_file_gestalt_group_definition& ugh_tag, s_sound_group_definition& out_snd_tag)
{
	int playback_param_start = playback_parameter_block.GetElementCount();
	int scales_start = scales_block.GetElementCount();
	//int importnames_start = import_names_block.GetElementCount();
	int pitch_range_param_start = pitch_range_param_block.GetElementCount();
	int pitch_ranges_start = pitch_ranges_block.GetElementCount();
	int permutations_start = permutations_block.GetElementCount();
	int custom_playback_start = custom_playbacks_block.GetElementCount();
	int runtime_perm_flags_start = runtime_perm_flags_block.GetElementCount();
	int permutation_chunks_start = permutation_chunks_block.GetElementCount();
	int promotions_start = promotions_block.GetElementCount();
	int extrainfo_start = extrainfo_block.GetElementCount();

	playback_parameter_block.AddRange(ugh_tag.playback_parameters.begin(), ugh_tag.playback_parameters.size);
	scales_block.AddRange(ugh_tag.scales.begin(), ugh_tag.scales.size);
	//Dont intend to add it
	//import_names_block.AddRange((ImportNames*)ugh_tag.ImportNames.GetTagBlockElements(), ugh_tag.ImportNames.GetElementCount());
	pitch_range_param_block.AddRange(ugh_tag.pitch_range_parameters.begin(), ugh_tag.pitch_range_parameters.size);
	pitch_ranges_block.AddRange(ugh_tag.pitch_ranges.begin(), ugh_tag.pitch_ranges.size);
	for (int i = pitch_ranges_start; i < pitch_ranges_block.GetElementCount(); i++)
	{
		pitch_ranges_block[i]->name_index = -1;
		if (pitch_ranges_block[i]->parameter_index != -1)
			pitch_ranges_block[i]->parameter_index += pitch_range_param_start;
		if (pitch_ranges_block[i]->encoded_runtime_permutation_flag_index != -1 && false)//no need so disable it for now
			pitch_ranges_block[i]->encoded_runtime_permutation_flag_index += runtime_perm_flags_start;
		if (pitch_ranges_block[i]->first_permutation != -1)
			pitch_ranges_block[i]->first_permutation += permutations_start;
	}
	permutations_block.AddRange(ugh_tag.permutations.begin(), ugh_tag.permutations.size);
	for (int i = permutations_start; i < permutations_block.GetElementCount(); i++)
	{
		permutations_block[i]->name_index = -1;
		if (permutations_block[i]->first_chunk != -1)
			permutations_block[i]->first_chunk += permutation_chunks_start;
	}
	custom_playbacks_block.AddRange(ugh_tag.custom_playbacks.begin(), ugh_tag.custom_playbacks.size);
	runtime_perm_flags_block.AddRange(ugh_tag.runtime_permutation_flags.begin(), ugh_tag.runtime_permutation_flags.size);
	promotions_block.AddRange(ugh_tag.promotions.begin(), ugh_tag.promotions.size);
	for (int i = promotions_start; i < promotions_block.GetElementCount(); i++)
	{
		for (int j = 0; j < promotions_block[i]->promotion_rules.size; j++)
		{
			promotions_block[i]->promotion_rules[j]->pitch_range_index += pitch_ranges_start;
		}
	}
	permutation_chunks_block.AddRange(ugh_tag.permutation_chunks.begin(), ugh_tag.permutation_chunks.size);
	extrainfo_block.AddRange(ugh_tag.extra_infos.begin(), ugh_tag.extra_infos.size);

	out_snd_tag.playback_index = playback_param_start;
	out_snd_tag.first_pitch_range_index = pitch_ranges_start;
	out_snd_tag.scale_index = scales_start;
	out_snd_tag.promotion_index = promotions_start;
	out_snd_tag.custom_playback_index = custom_playback_start;
	out_snd_tag.extra_info_index = extrainfo_start;
}
