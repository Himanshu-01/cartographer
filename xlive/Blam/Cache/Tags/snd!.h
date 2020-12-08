#pragma once
#include"tag_base.h"


namespace Blam
{
	namespace Cache
	{
		namespace Tags
		{
			/*********************************************************************
			* name:
			* group_tag : snd
			* header size : 20
			* *********************************************************************/
			struct snd :tag_base
			{
				struct Flags
				{
					unsigned char FitToADPCMBlocksize : 1;
					unsigned char SplitLongSoundIntoPermutations : 1;
					unsigned char AlwaysSpatialize : 1;
					unsigned char NeverObstruct : 1;
					unsigned char InternalDontTouch : 1;
					unsigned char UseHugeSoundTransmission : 1;
					unsigned char LinkCountToOwnerUnit : 1;
					unsigned char PitchRangeIsLanguage : 1;
					unsigned char DontUseSoundClassSpeakerFlag : 1;
					unsigned char DontUseLipsyncData : 1;
					unsigned char Bit10 : 1;
					unsigned char Bit11 : 1;
					unsigned char Bit12 : 1;
					unsigned char Bit13 : 1;
					unsigned char Bit14 : 1;
					unsigned char Bit15 : 1;
				}Flags;
				TAG_BLOCK_SIZE_ASSERT(Flags, 0x2);
				//0x0
				enum class SoundClass : __int8
				{
					Projectile_impact = 0,
					Projectile_detonation = 1,
					Projectile_flyby = 2,
					Projectile_detonation_lod = 3,
					Weapon_fire = 4,
					Weapon_ready = 5,
					Weapon_reload = 6,
					Weapon_empty = 7,
					Weapon_charge = 8,
					Weapon_overheat = 9,
					Weapon_idle = 10,
					Weapon_melee = 11,
					Weapon_animation = 12,
					Object_impacts = 13,
					Particle_impacts = 14,
					Weapon_fire_lod = 15,
					Unused1_impacts = 16,
					Unused2_impacts = 17,
					Unit_footsteps = 18,
					Unit_dialog = 19,
					Unit_animation = 20,
					Unit_unused = 21,
					Vehicle_collision = 22,
					Vehicle_engine = 23,
					Vehicle_animation = 24,
					Vehicle_engine_lod = 25,
					Device_door = 26,
					Device_unused0 = 27,
					Device_machinery = 28,
					Device_stationary = 29,
					Device_unused1 = 30,
					Device_unused2 = 31,
					Music = 32,
					Ambient_nature = 33,
					Ambient_machinery = 34,
					Ambient_stationary = 35,
					Huge_ass = 36,
					Object_looping = 37,
					Cinematic_music = 38,
					Unknown_unused0 = 39,
					Unknown_unused1 = 40,
					Unknown_unused2 = 41,
					Unknown_unused3 = 42,
					Unknown_unused4 = 43,
					Mission_unused0 = 44,
					Cortana_mission = 45,
					Cortana_cinematic = 46,
					Mission_dialog = 47,
					Cinematic_dialog = 48,
					Scripted_cinematic_foley = 49,
					Game_event = 50,
					Ui = 51,
					Test = 52,
					Multilingual_test = 53,
				};
				SoundClass SoundClass;//0x2
				enum class SampleRate : __int8
				{
					NUM_22kHz = 0,
					NUM_44kHz = 1,
					NUM_32kHz = 2,
				};
				SampleRate SampleRate;//0x3
				enum class Encoding : __int8
				{
					Mono = 0,
					Stereo = 1,
					Codec = 2,
				};
				Encoding Encoding;//0x4
				enum class Compression : __int8
				{
					NoneBigEndian = 0,
					XboxADPCM = 1,
					IMAADPCM = 2,
					NoneLittleEndian = 3,
					WMA = 4,
				};
				Compression Compression;//0x5
				__int16 PlaybackParameterIndex;//0x6
				__int16 PitchRangeIndex;//0x8
				unsigned __int8 Unknown;//0xA
				unsigned __int8 ScaleIndex;//0xB
				unsigned __int8 PromotionIndex;//0xC
				unsigned __int8 CustomPlaybackIndex;//0xD
				__int16 ExtraInfoIndex;//0xE
				__int32 MaximumPlayTime;//0x10
			};
			TAG_BLOCK_SIZE_ASSERT(snd, 0x14);
		}
	}
}
