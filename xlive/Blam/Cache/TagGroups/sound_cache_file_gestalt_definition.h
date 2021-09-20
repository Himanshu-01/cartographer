#pragma once
#include"Blam\Cache\DataTypes.h"
#include"Blam\Maths\real_math.h"
#include"Blam\Maths\integer_math.h""
#include"Blam\Common\Common.h"

/*********************************************************************
* name: sound_cache_file_gestalt
* group_tag : ugh!
* header size : 88
* *********************************************************************/

#pragma pack(push,1)
struct s_sound_cache_file_gestalt_group_definition :TagGroup<'ugh!'>
{
	struct s_playback_parameter_block
	{
		float minimum_distance;//0x0
		float maximum_distance;//0x4
		float skip_fraction;//0x8
		float maximum_bend_per_second;//0xC
		float gain_base;//0x10
		float gain_variance;//0x14
		short_bounds random_pitch_bounds;//0x18
		angle inner_cone_angle;//0x1C
		angle outer_cone_angle;//0x20
		float outer_cone_gain;//0x24
		enum class e_flags : __int32
		{
			override_azimuth = FLAG(0),
			override_3d_gain = FLAG(1),
			override_speaker_gain = FLAG(2),
		};
		e_flags flags;//0x28
		angle azimuth;//0x2C
		float positional_gain;//0x30
		float first_person_gain;//0x34
	};
	TAG_BLOCK_SIZE_ASSERT(s_playback_parameter_block, 0x38);
	tag_block<s_playback_parameter_block> playback_parameters;//0x0
	struct s_scales_block
	{
		real_bounds gain_modifier;//0x0
		short_bounds pitch_modifier;//0x8		
		real_bounds skip_fraction_modifier;//0xC
	};
	TAG_BLOCK_SIZE_ASSERT(s_scales_block, 0x14);
	tag_block<s_scales_block> scales;//0x8
	struct s_import_names_block
	{
		string_id import_name;//0x0
	};
	TAG_BLOCK_SIZE_ASSERT(s_import_names_block, 0x4);
	tag_block<s_import_names_block> import_names;//0x10
	struct s_pitch_range_parameters_block
	{
		__int16 natural_pitch;//0x0
		short_bounds bend_bounds;//0x2
		short_bounds max_gain_pitch_bounds;//0x6
	};
	TAG_BLOCK_SIZE_ASSERT(s_pitch_range_parameters_block, 0xA);
	tag_block<s_pitch_range_parameters_block> pitch_range_parameters;//0x18
	struct s_pitch_ranges_block
	{
		__int16 name_index;//0x0
		__int16 parameter_index;//0x2
		__int16 encoded_permutation_data_index;//0x4
		__int16 encoded_runtime_permutation_flag_index;//0x6
		__int16 first_permutation;//0x8
		__int16 permutation_count;//0xA
	};
	TAG_BLOCK_SIZE_ASSERT(s_pitch_ranges_block, 0xC);
	tag_block<s_pitch_ranges_block> pitch_ranges;//0x20
	struct s_permutations_block
	{
		__int16 name_index;//0x0
		__int16 encoded_skip_fraction;//0x2
		__int8 encoded_gain;//0x4
		__int8 permutation_info_index;//0x5
		__int16 language_neutral_time;//0x6
		__int32 sample_size;//0x8
		__int16 first_chunk;//0xC
		__int16 chunk_count;//0xE
	};
	TAG_BLOCK_SIZE_ASSERT(s_permutations_block, 0x10);
	tag_block<s_permutations_block> permutations;//0x28
	struct s_custom_playbacks_block
	{
		PAD(0x8);//0x0
		enum class e_flags : __int32
		{
			use_3d_radio_hack = FLAG(0),
		};
		e_flags flags;//0x8
		PAD(0x8);//0xC
		struct s_filter_block
		{
			enum class e_filter_type : __int32
			{
				parametric_eq = 0,
				dls2 = 1,
				both_only_valid_for_mono = 2,
			};
			e_filter_type filter_type;//0x0
			__int32 filter_width;//0x4

			struct
			{
				real_bounds scale_bounds_lower;//0x8
				real_bounds random_base_and_variance_lower;//0x10
			}left_filter_frequency;

			struct
			{
				real_bounds scale_bounds_lower;//0x18
				real_bounds random_base_and_variance_lower;//0x20
			}left_filter_gain;

			struct
			{
				real_bounds scale_bounds_lower;//0x28
				real_bounds random_base_and_variance_lower;//0x30
			}right_filter_frequency;

			struct
			{
				real_bounds scale_bounds_lower;//0x38
				real_bounds random_base_and_variance_lower;//0x40
			}right_filter_gain;
		};
		TAG_BLOCK_SIZE_ASSERT(s_filter_block, 0x48);
		tag_block<s_filter_block> filter;//0x14
		struct s_pitch_lfo_block
		{
			struct
			{
				real_bounds scale_bounds_lower;//0x0
				real_bounds random_base_and_variance_lower;//0x8
			}delay;

			struct
			{
				real_bounds scale_bounds_lower;//0x10
				real_bounds random_base_and_variance_lower;//0x18
			}frequency;

			struct
			{
				real_bounds scale_bounds_lower;//0x20
				real_bounds random_base_and_variance_lower;//0x28
			}pitch_modulation;
		};
		TAG_BLOCK_SIZE_ASSERT(s_pitch_lfo_block, 0x30);
		tag_block<s_pitch_lfo_block> pitch_lfo;//0x1C
		struct s_filter_lfo_block
		{
			struct
			{
				real_bounds scale_bounds_lower;//0x0
				real_bounds random_base_and_variance_lower;//0x8
			}delay;

			struct
			{
				real_bounds scale_bounds_lower;//0x10
				real_bounds random_base_and_variance_lower;//0x18
			}frequency;

			struct
			{
				real_bounds scale_bounds_lower;//0x20
				real_bounds random_base_and_variance_lower;//0x28
			}cutoff_modulation;

			struct
			{
				real_bounds scale_bounds_lower;//0x30
				real_bounds random_base_and_variance_lower;//0x38
			}gain_modulation;
		};
		TAG_BLOCK_SIZE_ASSERT(s_filter_lfo_block, 0x40);
		tag_block<s_filter_lfo_block> filter_lfo;//0x24
		struct s_sound_effect_block
		{
			tag_reference template_;//0x0
			struct s_components_block
			{
				tag_reference sound;//0x0
				float gain;//0x8
				enum class e_flags : __int32
				{
					dont_play_at_start = FLAG(0),
					play_on_stop = FLAG(1),
					play_alternate = FLAG(3),
					sync_with_origin_looping_sound = FLAG(5),
				};
				e_flags flags;//0xC
			};
			TAG_BLOCK_SIZE_ASSERT(s_components_block, 0x10);
			tag_block<s_components_block> components;//0x8
			PAD(0x18);
		};
		TAG_BLOCK_SIZE_ASSERT(s_sound_effect_block, 0x28);
		tag_block<s_sound_effect_block> sound_effect;//0x2C
	};
	TAG_BLOCK_SIZE_ASSERT(s_custom_playbacks_block, 0x34);
	tag_block<s_custom_playbacks_block> custom_playbacks;//0x30
	struct s_runtime_permutation_flags_block
	{
		__int8 flags;//0x0
	};
	TAG_BLOCK_SIZE_ASSERT(s_runtime_permutation_flags_block, 0x1);
	tag_block<s_runtime_permutation_flags_block> runtime_permutation_flags;//0x38
	struct s_permuatation_chunks_block
	{
		__int32 file_offset;//0x0
		short flags;//0x4
		short size;//0x6
		__int32 runtime_index;//0x8
	};
	TAG_BLOCK_SIZE_ASSERT(s_permuatation_chunks_block, 0xC);
	tag_block<s_permuatation_chunks_block> permutation_chunks;//0x40
	struct s_promotions_block
	{
		struct s_promotion_rules_block
		{
			__int16 pitch_range_index;//0x0
			__int16 maximum_playing_count;//0x2
			float suppression_time;//0x4
			PAD(0x8);//0x8
		};
		TAG_BLOCK_SIZE_ASSERT(s_promotion_rules_block, 0x10);
		tag_block<s_promotion_rules_block> promotion_rules;//0x0
		struct s_runtime_timers
		{
			PAD(4);//0x0
		};
		TAG_BLOCK_SIZE_ASSERT(s_runtime_timers, 0x4);
		tag_block<s_runtime_timers> runtime_timers;//0x8
		PAD(0xC);//0x10
	};
	TAG_BLOCK_SIZE_ASSERT(s_promotions_block, 0x1C);
	tag_block<s_promotions_block> promotions;//0x48
	struct s_extra_infos_block
	{
		struct s_encoded_permutation_section_block
		{
			data_block encoded_data;//0x0
			struct s_sound_dialogue_info_block
			{
				__int32 mouth_data_offset;//0x0
				__int32 mouth_data_length;//0x4
				__int32 lipsync_data_offset;//0x8
				__int32 lipsync_data_length;//0xC
			};
			TAG_BLOCK_SIZE_ASSERT(s_sound_dialogue_info_block, 0x10);
			tag_block<s_sound_dialogue_info_block> sound_dialogue_info;//0x8
		};
		TAG_BLOCK_SIZE_ASSERT(s_encoded_permutation_section_block, 0x10);
		tag_block<s_encoded_permutation_section_block> encoded_permutation_section;//0x0
		__int32 block_offset;//0x8
		__int32 block_size;//0xC
		__int32 section_data_size;//0x10
		__int32 resource_data_size;//0x14
		struct s_resources_block
		{
			enum class e_type : __int8
			{
				tag_block = 0,
				tag_data = 1,
				vertex_buffer = 2,
			};
			e_type type;//0x0
			PAD(0x3);//0x1
			__int16 primary_locator;//0x4
			__int16 secondary_locator;//0x6
			__int32 resource_data_size;//0x8
			__int32 resource_data_offset;//0xC
		};
		TAG_BLOCK_SIZE_ASSERT(s_resources_block, 0x10);
		tag_block<s_resources_block> resources;//0x18
		datum owner_tag_index;//0x20
		__int16 owner_tag_section_offset;//0x24
		PAD(0x6);//0x26
	};
	TAG_BLOCK_SIZE_ASSERT(s_extra_infos_block, 0x2C);
	tag_block<s_extra_infos_block> extra_infos;//0x50
};
TAG_GROUP_SIZE_ASSERT(s_sound_cache_file_gestalt_group_definition, 0x58);
#pragma pack(pop)