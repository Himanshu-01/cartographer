#include "stdafx.h"
#include "sound_gestalt_manager.h"
#include"..\H2MOD.h"
#include "..\Util\Hooks\Hook.h"
#include"..\h2_functions\h2_memory_funcs.h"

//define static variables
h2engine_sound_gestalt* sound_gestalt_manager::pgestalt;

h2_memory_vector<PlaybackParameters> sound_gestalt_manager::playback_parameter_block;
h2_memory_vector<Scales> sound_gestalt_manager::scales_block;
h2_memory_vector<ImportNames> sound_gestalt_manager::import_names_block;
h2_memory_vector<PitchRangeParameters> sound_gestalt_manager::pitch_range_param_block;
h2_memory_vector<PitchRanges> sound_gestalt_manager::pitch_ranges_block;
h2_memory_vector<Permutations> sound_gestalt_manager::permutations_block;
h2_memory_vector<CustomPlaybacks> sound_gestalt_manager::custom_playbacks_block;
h2_memory_vector<RuntimePermutationFlags> sound_gestalt_manager::runtime_perm_flags_block;
h2_memory_vector<PermutationChunks> sound_gestalt_manager::permutation_chunks_block;
h2_memory_vector<Promotions> sound_gestalt_manager::promotions_block;
h2_memory_vector<ExtraInfo> sound_gestalt_manager::extrainfo_block;

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

void sound_gestalt_manager::Add_sound_chunk(ugh &ugh_tag, snd &out_snd_tag)
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

	playback_parameter_block.AddRange((PlaybackParameters*)ugh_tag.PlaybackParameters.GetTagBlockElements(), ugh_tag.PlaybackParameters.GetElementCount());
	scales_block.AddRange((Scales*)ugh_tag.Scales.GetTagBlockElements(), ugh_tag.Scales.GetElementCount());
	//Dont intend to add it
	//import_names_block.AddRange((ImportNames*)ugh_tag.ImportNames.GetTagBlockElements(), ugh_tag.ImportNames.GetElementCount());
	pitch_range_param_block.AddRange((PitchRangeParameters*)ugh_tag.PitchRangeParameters.GetTagBlockElements(), ugh_tag.PitchRangeParameters.GetElementCount());
	pitch_ranges_block.AddRange((PitchRanges*)ugh_tag.PitchRanges.GetTagBlockElements(), ugh_tag.PitchRanges.GetElementCount());
	for (int i = pitch_ranges_start; i < pitch_ranges_block.GetElementCount(); i++)
	{
		pitch_ranges_block[i]->ImportNameIndex = -1;
		if (pitch_ranges_block[i]->PitchRangeParameterIndex != -1)
			pitch_ranges_block[i]->PitchRangeParameterIndex += pitch_range_param_start;
		if (pitch_ranges_block[i]->EncodedRuntimePermutationFlagIndex != -1 && false)
			pitch_ranges_block[i]->EncodedRuntimePermutationFlagIndex += runtime_perm_flags_start;
		if (pitch_ranges_block[i]->FirstPermutation != -1)
			pitch_ranges_block[i]->FirstPermutation += permutations_start;
	}
	permutations_block.AddRange((Permutations*)ugh_tag.Permutations.GetTagBlockElements(), ugh_tag.Permutations.GetElementCount());
	for (int i = permutations_start; i < permutations_block.GetElementCount(); i++)
	{
		permutations_block[i]->ImportNameIndex = -1;
		if (permutations_block[i]->FirstChunk != -1)
			permutations_block[i]->FirstChunk += permutation_chunks_start;
	}
	custom_playbacks_block.AddRange((CustomPlaybacks*)ugh_tag.CustomPlaybacks.GetTagBlockElements(), ugh_tag.CustomPlaybacks.GetElementCount());
	runtime_perm_flags_block.AddRange((RuntimePermutationFlags*)ugh_tag.RuntimePermutationFlags.GetTagBlockElements(), ugh_tag.CustomPlaybacks.GetElementCount());
	promotions_block.AddRange((Promotions*)ugh_tag.Promotions.GetTagBlockElements(), ugh_tag.Promotions.GetElementCount());
	for (int i = promotions_start; i < promotions_block.GetElementCount(); i++)
	{
		for (int j = 0; j < promotions_block[i]->Rules.GetElementCount(); j++)
		{
			promotions_block[i]->Rules[j]->PitchRangeIndex += pitch_ranges_start;
		}
	}
	permutation_chunks_block.AddRange((PermutationChunks*)ugh_tag.PermutationChunks.GetTagBlockElements(), ugh_tag.PermutationChunks.GetElementCount());
	extrainfo_block.AddRange((ExtraInfo*)ugh_tag.ExtraInfo.GetTagBlockElements(), ugh_tag.ExtraInfo.GetElementCount());

	out_snd_tag.PlaybackParameterIndex = playback_param_start;
	out_snd_tag.PitchRangeIndex = pitch_ranges_start;
	out_snd_tag.ScaleIndex = scales_start;
	out_snd_tag.PromotionIndex = promotions_start;
	out_snd_tag.CustomPlaybackIndex = custom_playback_start;
	out_snd_tag.ExtraInfoIndex = extrainfo_start;
}
