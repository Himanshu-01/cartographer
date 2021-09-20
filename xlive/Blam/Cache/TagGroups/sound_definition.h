#pragma once
#include"Blam\Cache\DataTypes.h"
#include "Blam\Cache\TagGroups.hpp"
#include"Blam\Common\Common.h"

/*********************************************************************
* name: sound
* group_tag : snd!
* header size : 20
* *********************************************************************/

#pragma pack(push,1)
struct s_sound_group_definition :TagGroup<'snd!'>
{
	enum class e_flags : __int16
	{
		fit_to_adpcm_blocksize = FLAG(0),
		split_long_sound_into_permutations = FLAG(1),
		always_spatialize = FLAG(2),
		never_obstruct = FLAG(3),
		internal_dont_touch = FLAG(4),
		use_huge_sound_transmission = FLAG(5),
		link_count_to_owner_unit = FLAG(6),
		pitch_range_is_language = FLAG(7),
		dont_use_sound_class_speaker_flag = FLAG(8),
		dont_use_lipsync_data = FLAG(9),
	};
	e_flags flags;//0x0
	enum class e_sound_class : __int8
	{
		projectile_impact = 0,
		projectile_detonation = 1,
		projectile_flyby = 2,
		projectile_detonation_lod = 3,
		weapon_fire = 4,
		weapon_ready = 5,
		weapon_reload = 6,
		weapon_empty = 7,
		weapon_charge = 8,
		weapon_overheat = 9,
		weapon_idle = 10,
		weapon_melee = 11,
		weapon_animation = 12,
		object_impacts = 13,
		particle_impacts = 14,
		weapon_fire_lod = 15,
		unused1_impacts = 16,
		unused2_impacts = 17,
		unit_footsteps = 18,
		unit_dialog = 19,
		unit_animation = 20,
		unit_unused = 21,
		vehicle_collision = 22,
		vehicle_engine = 23,
		vehicle_animation = 24,
		vehicle_engine_lod = 25,
		device_door = 26,
		device_unused0 = 27,
		device_machinery = 28,
		device_stationary = 29,
		device_unused1 = 30,
		device_unused2 = 31,
		music = 32,
		ambient_nature = 33,
		ambient_machinery = 34,
		ambient_stationary = 35,
		huge_ass = 36,
		object_looping = 37,
		cinematic_music = 38,
		unknown_unused0 = 39,
		unknown_unused1 = 40,
		unknown_unused2 = 41,
		unknown_unused3 = 42,
		unknown_unused4 = 43,
		mission_unused0 = 44,
		cortana_mission = 45,
		cortana_cinematic = 46,
		mission_dialog = 47,
		cinematic_dialog = 48,
		scripted_cinematic_foley = 49,
		game_event = 50,
		ui = 51,
		test = 52,
		multilingual_test = 53,
	};
	e_sound_class sound_class;//0x2
	enum class e_sample_rate : __int8
	{
		_22khz = 0,
		_44khz = 1,
		_32khz = 2,
	};
	e_sample_rate sample_rate;//0x3
	enum class e_encoding : __int8
	{
		mono = 0,
		stereo = 1,
		codec = 2,
	};
	e_encoding encoding;//0x4
	enum class e_compression : __int8
	{
		none_big_endian = 0,
		xbox_adpcm = 1,
		ima_adpcm = 2,
		none_little_endian = 3,
		wma = 4,
	};
	e_compression compression;//0x5
	__int16 playback_index;//0x6
	__int16 first_pitch_range_index;//0x8
	__int8 pitch_range_count;//0xA
	__int8 scale_index;//0xB
	__int8 promotion_index;//0xC
	__int8 custom_playback_index;//0xD
	__int16 extra_info_index;//0xE
	__int32 maximum_play_time;//0x10
};
TAG_GROUP_SIZE_ASSERT(s_sound_group_definition, 0x14);
#pragma pack(pop)