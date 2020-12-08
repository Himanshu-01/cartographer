#pragma once
#include"tag_base.h"

namespace Blam
{
	namespace Cache
	{
		namespace Tags
		{
			struct PlaybackParameters :tag_base
			{
				float MinimumDistance;//0x0
				float MaximumDistance;//0x4
				float SkipFraction;//0x8
				float MaximumBendPerSecond;//0xC
				float GainBase;//0x10
				float GainVariance;//0x14
				__int16 RandomPitchBoundsMin;//0x18
				__int16 RandomPitchBoundsMax;//0x1A
				float InnerConeAngle;//0x1C
				float OuterConeAngle;//0x20
				float OuterConeGain;//0x24
				struct Flags
				{
					unsigned char OverrideAzimuth : 1;
					unsigned char Override3DGain : 1;
					unsigned char OverrideSpeakerGain : 1;
					unsigned char bit3 : 1;
					unsigned char bit4 : 1;
					unsigned char bit5 : 1;
					unsigned char bit6 : 1;
					unsigned char bit7 : 1;
					unsigned char bit8 : 1;
					unsigned char bit9 : 1;
					unsigned char bit10 : 1;
					unsigned char bit11 : 1;
					unsigned char bit12 : 1;
					unsigned char bit13 : 1;
					unsigned char bit14 : 1;
					unsigned char bit15 : 1;
					unsigned char bit16 : 1;
					unsigned char bit17 : 1;
					unsigned char bit18 : 1;
					unsigned char bit19 : 1;
					unsigned char bit20 : 1;
					unsigned char bit21 : 1;
					unsigned char bit22 : 1;
					unsigned char bit23 : 1;
					unsigned char bit24 : 1;
					unsigned char bit25 : 1;
					unsigned char bit26 : 1;
					unsigned char bit27 : 1;
					unsigned char bit28 : 1;
					unsigned char bit29 : 1;
					unsigned char bit30 : 1;
					unsigned char bit31 : 1;
				}Flags;
				TAG_BLOCK_SIZE_ASSERT(Flags, 0x4);
				//0x28
				float Azimuth;//0x2C
				float PositionalGain;//0x30
				float FirstPersonGain;//0x34
			};
			TAG_BLOCK_SIZE_ASSERT(PlaybackParameters, 0x38);

			struct Scales :tag_base
			{
				float GainModifierMin;//0x0
				float GainModifierMax;//0x4
				__int16 PitchModifierMin;//0x8
				__int16 PitchModifierMax;//0xA
				float SkipFractionModifierMin;//0xC
				float SkipFractionModifierMax;//0x10
			};
			TAG_BLOCK_SIZE_ASSERT(Scales, 0x14);

			struct ImportNames :tag_base
			{
				int StringID_Name;//0x0
			};
			TAG_BLOCK_SIZE_ASSERT(ImportNames, 0x4);

			struct PitchRangeParameters :tag_base
			{
				__int16 NaturalPitch;//0x0
				__int16 BendBoundsMin;//0x2
				__int16 BendBoundsMax;//0x4
				__int16 MaxGainPitchBoundsMin;//0x6
				__int16 MaxGainPitchBoundsMax;//0x8
			};
			TAG_BLOCK_SIZE_ASSERT(PitchRangeParameters, 0xA);

			struct PitchRanges :tag_base
			{
				__int16 ImportNameIndex;//0x0
				__int16 PitchRangeParameterIndex;//0x2
				__int16 EncodedPermutationDataIndex;//0x4
				__int16 EncodedRuntimePermutationFlagIndex;//0x6
				__int16 FirstPermutation;//0x8
				__int16 PermutationCount;//0xA
			};
			TAG_BLOCK_SIZE_ASSERT(PitchRanges, 0xC);

			struct Permutations :tag_base
			{
				__int16 ImportNameIndex;//0x0
				__int16 EncodedSkipFraction;//0x2
				__int8 Gain;//0x4
				__int8 PermutationInfoIndex;//0x5
				__int16 LanguageNeutralTime;//0x6
				unsigned __int32 SampleSize;//0x8
				__int16 FirstChunk;//0xC
				__int16 ChunkCount;//0xE
			};
			TAG_BLOCK_SIZE_ASSERT(Permutations, 0x10);

			struct CustomPlaybacks :tag_base
			{
				PAD(0x8);//0x0
				struct Flags
				{
					unsigned char Use3DRadioHack : 1;
					unsigned char bit1 : 1;
					unsigned char bit2 : 1;
					unsigned char bit3 : 1;
					unsigned char bit4 : 1;
					unsigned char bit5 : 1;
					unsigned char bit6 : 1;
					unsigned char bit7 : 1;
					unsigned char bit8 : 1;
					unsigned char bit9 : 1;
					unsigned char bit10 : 1;
					unsigned char bit11 : 1;
					unsigned char bit12 : 1;
					unsigned char bit13 : 1;
					unsigned char bit14 : 1;
					unsigned char bit15 : 1;
					unsigned char bit16 : 1;
					unsigned char bit17 : 1;
					unsigned char bit18 : 1;
					unsigned char bit19 : 1;
					unsigned char bit20 : 1;
					unsigned char bit21 : 1;
					unsigned char bit22 : 1;
					unsigned char bit23 : 1;
					unsigned char bit24 : 1;
					unsigned char bit25 : 1;
					unsigned char bit26 : 1;
					unsigned char bit27 : 1;
					unsigned char bit28 : 1;
					unsigned char bit29 : 1;
					unsigned char bit30 : 1;
					unsigned char bit31 : 1;
				}Flags;
				TAG_BLOCK_SIZE_ASSERT(Flags, 0x4);
				//0x8
				PAD(0x8);//0xC
				struct Filter :tag_base
				{
					__int32 FilterType;//0x0
					__int32 FilterWidth;//0x4
					struct {
						float ScaleBoundsMin;//0x8
						float ScaleBoundsMax;//0xC
						float RandomBase;//0x10
						float RandomVariance;//0x14
					}LeftFilterFrequency;
					struct {
						float ScaleBoundsMin;//0x8
						float ScaleBoundsMax;//0xC
						float RandomBase;//0x10
						float RandomVariance;//0x14
					}LeftFilterGain;
					struct {
						float ScaleBoundsMin;//0x8
						float ScaleBoundsMax;//0xC
						float RandomBase;//0x10
						float RandomVariance;//0x14
					}RightFilterFrequency;
					struct {
						float ScaleBoundsMin;//0x8
						float ScaleBoundsMax;//0xC
						float RandomBase;//0x10
						float RandomVariance;//0x14
					}RightFilterGain;
				};
				TAG_BLOCK_SIZE_ASSERT(Filter, 0x48);
				Blam::Cache::DataTypes::Reflexive<Filter> Filter;//0x14
				PAD(0x18);//0x1C
			};
			TAG_BLOCK_SIZE_ASSERT(CustomPlaybacks, 0x34);

			struct RuntimePermutationFlags :tag_base
			{
				__int8 Unknown;//0x0
			};
			TAG_BLOCK_SIZE_ASSERT(RuntimePermutationFlags, 0x1);

			struct PermutationChunks :tag_base
			{
				unsigned __int32 FileOffset;//0x0
				unsigned __int16 Flags;//0x4
				unsigned __int16 Size;//0x6
				__int32 RuntimeIndex;//0x8
			};
			TAG_BLOCK_SIZE_ASSERT(PermutationChunks, 0xC);

			struct Promotions :tag_base
			{
				struct Rules :tag_base
				{
					__int16 PitchRangeIndex;//0x0
					__int16 MaximumPlayingCount;//0x2
					float SuppressionTime;//0x4
					__int32 Unknown8;//0x8
					__int32 UnknownC;//0xC
				};
				TAG_BLOCK_SIZE_ASSERT(Rules, 0x10);
				Blam::Cache::DataTypes::Reflexive<Rules> Rules;//0x0
				struct RuntimeTimers :tag_base
				{
					__int32 Unknown;//0x0
				};
				TAG_BLOCK_SIZE_ASSERT(RuntimeTimers, 0x4);
				Blam::Cache::DataTypes::Reflexive<RuntimeTimers> RuntimeTimers;//0x8
				PAD(0xC);//0x10
			};
			TAG_BLOCK_SIZE_ASSERT(Promotions, 0x1C);

			struct ExtraInfo :tag_base
			{
				PAD(0x8);//0x0
				unsigned __int32 BlockOffset;//0x8
				unsigned __int32 BlockSize;//0xC
				unsigned __int32 SectionDataSize;//0x10
				unsigned __int32 ResourceDataSize;//0x14
				struct Resources :tag_base
				{
					__int8 Type;//0x0
					__int8 Unknown1;//0x1
					__int16 Unknown2;//0x2
					__int16 PrimaryLocator;//0x4
					__int16 SecondaryLocator;//0x6
					unsigned __int32 ResourceDataSize;//0x8
					unsigned __int32 ResourceDataOffset;//0xC
				};
				TAG_BLOCK_SIZE_ASSERT(Resources, 0x10);
				Blam::Cache::DataTypes::Reflexive<Resources> Resources;//0x18
				unsigned __int32 owner_tag_index;//0x20
				PAD(0x8);//0x24
			};
			TAG_BLOCK_SIZE_ASSERT(ExtraInfo, 0x2C);
			/*********************************************************************
			* name:
			* group_tag : ugh
			* header size : 88
			* *********************************************************************/
			struct ugh :tag_base
			{
				Blam::Cache::DataTypes::Reflexive<PlaybackParameters> PlaybackParameters;//0x0
				Blam::Cache::DataTypes::Reflexive<Scales> Scales;//0x8
				Blam::Cache::DataTypes::Reflexive<ImportNames> ImportNames;//0x10
				Blam::Cache::DataTypes::Reflexive<PitchRangeParameters> PitchRangeParameters;//0x18
				Blam::Cache::DataTypes::Reflexive<PitchRanges> PitchRanges;//0x20
				Blam::Cache::DataTypes::Reflexive<Permutations> Permutations;//0x28
				Blam::Cache::DataTypes::Reflexive<CustomPlaybacks> CustomPlaybacks;//0x30
				Blam::Cache::DataTypes::Reflexive<RuntimePermutationFlags> RuntimePermutationFlags;//0x38
				Blam::Cache::DataTypes::Reflexive<PermutationChunks> PermutationChunks;//0x40
				Blam::Cache::DataTypes::Reflexive<Promotions> Promotions;//0x48
				Blam::Cache::DataTypes::Reflexive<ExtraInfo> ExtraInfo;//0x50
			};
			TAG_BLOCK_SIZE_ASSERT(ugh, 0x58);
		}
	}
}
