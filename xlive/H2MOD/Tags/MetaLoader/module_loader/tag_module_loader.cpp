#include "stdafx.h"
#include "tag_module_loader.h"
#include "..\Util\filesys.h"
#include"..\h2_functions\h2_tag_funcs.h"
#include"..\h2_functions\h2_memory_funcs.h"
#include"..\misc\string_functions.h"
#include"..\meta_cache_loader\cache_loader.h"
#include "..\Util\Hooks\Hook.h"
#include"..\..\TagInterface.h"
#include"..\..\global_tags_interface.h"
#include"..\tag_loader_constants.h"

namespace tag_loader
{
	using Blam::Enums::Tags::TagGroupTypes;

	//some function declarations for call patching purpose
	unsigned int __cdecl patchcall_h2_AllocateMemory(int default_meta_size, char arg_4);
	DWORD __cdecl patchcall_h2_LoadResource1(int datum_index, unsigned int map_off, int size, void *mem_addr, __int8 *a5, int a6, int a7);
	DWORD __cdecl patchcall_h2_LoadResource2(int owner_tag_index, int MapOff, int sizeM, unsigned int destination_ptr);

	//initialize static variables
	std::vector<tag_module_loader::tag_module_listing_internal> tag_module_loader::loaded_module_list;
	std::vector<tag_loader_tag_instance> tag_module_loader::loaded_tag_list;
	std::vector<std::string> tag_module_loader::message_list;
	unsigned int tag_module_loader::default_meta_size = 0;
	unsigned int tag_module_loader::injected_meta_size = 0;
	unsigned int tag_module_loader::new_datum_base = INJECTED_TAG_DATUM_BASE;
	tags::tag_instance* tag_module_loader::new_global_tag_tables = nullptr;


	void tag_module_loader::Init()
	{
		if (new_global_tag_tables == nullptr)
			new_global_tag_tables = new tags::tag_instance[MAX_TAG_TABLE_SIZE];

		///allocating more space for meta loading
		PatchCall(h2mod->GetAddress(0x313B2), patchcall_h2_AllocateMemory);//for injecting meta data->TODO
		///patching resource loading calls
		PatchCall(h2mod->GetAddress(0x3C59E), patchcall_h2_LoadResource1);//for sounds raw->TODO
		PatchCall(h2mod->GetAddress(0x265208), patchcall_h2_LoadResource1);//for load resource stuff->TODO
		PatchCall(h2mod->GetAddress(0x265A4E), patchcall_h2_LoadResource1);//for bitmaps->TODO
		PatchCall(h2mod->GetAddress(0x265A81), patchcall_h2_LoadResource2);//for bitmpas->TODO

		//client side desync fix
		///(noping out jump instructions)	
		NopFill(h2mod->GetAddress(0x316CE), 2);//TODO
		NopFill(h2mod->GetAddress(0x316DC), 2);//TODO
	}
	void tag_module_loader::Reload()
	{
		for (int i = 0; i < loaded_module_list.size(); i++)
			CloseHandle(loaded_module_list[i].module_HANDLE);
		loaded_module_list.clear();		

		// reset starting_datum index
		injected_meta_size = 0;
		new_datum_base = (tags::get_tag_count() > INJECTED_TAG_DATUM_BASE) ? tags::get_tag_count() : INJECTED_TAG_DATUM_BASE;

		// extending tag_tables
		DWORD *ppglobal_tag_table = h2mod->GetAddress<DWORD*>(0x47CD50, 0x4A29B8);
		memcpy((BYTE*)new_global_tag_tables, (BYTE*)*ppglobal_tag_table, new_datum_base * 0x10);
		*ppglobal_tag_table = (DWORD)new_global_tag_tables;
	}
	void tag_module_loader::SetDefaultMetaSize(int def_meta_size)
	{
		default_meta_size = def_meta_size;
	}
	int tag_module_loader::tag_module_load(std::string tag_module_rel_path, int target_index)
	{
		int ret = -1;

		if (!new_global_tag_tables)
		{
			std::string _error = "tag_module_loader : new_global_tag_tables uninitialized,couldn't load module";
			message_list.push_back(_error);
			return -1;
		}
		if (!default_meta_size || new_datum_base == -1)
		{
			std::string _error = "tag_module_loader :  tag loader variables uninitialized,couldn't load module";
			message_list.push_back(_error);
			return -1;
		}

		std::string game_root_directory(GetExeDirectoryNarrow());
		std::string tag_module_full_path = game_root_directory + TAGS_RELATIVE_PATH + tag_module_rel_path;

		auto my_loader = new meta_cache_loader::cache_loader(tag_module_full_path);

		if (my_loader->isopened())
		{
			auto module_tag_table = my_loader->get_BLOCK(meta_cache_loader::block_type::Tag_table_block);
			auto module_tag_data = my_loader->get_BLOCK(meta_cache_loader::block_type::Tag_data_block);
			auto module_tag_rebase_table = my_loader->get_BLOCK(meta_cache_loader::block_type::Tag_rebase_block);
			auto module_index_rebase_table = my_loader->get_BLOCK(meta_cache_loader::block_type::Tag_index_rebase_block);
			auto module_tag_names = my_loader->get_BLOCK(meta_cache_loader::block_type::Tag_names_block);
			auto module_internal_name = my_loader->get_BLOCK(meta_cache_loader::block_type::Module_Internal_name);

			//create a handle_listing and add it as early as possible
			HANDLE file_handle = CreateFileA(tag_module_full_path.c_str(), GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
			
			if (file_handle == INVALID_HANDLE_VALUE)
			{
				std::string _error = "tag_module_loader : Unable to open file handle to module : " + tag_module_full_path;
				message_list.push_back(_error);

				delete my_loader;
				return -1;
			}
			
			tag_module_listing_internal t_module_handle_listing = tag_module_listing_internal(tag_module_rel_path, file_handle, new_datum_base, new_datum_base + (module_tag_table->size / 0x10));

			loaded_module_list.push_back(t_module_handle_listing);

			if (injected_meta_size + module_tag_data->size > MAX_ADDITIONAL_TAG_SIZE)
			{
				std::string error = "tag_module_loader : Couldnt load module, MAX_ADDITIONAL_TAG_SIZE_reached";
				message_list.push_back(error);
				delete my_loader;

				return ret;
			}
			ret = new_datum_base;
			std::string loading_message = "tag_module_loader : Loading module: " + tag_module_full_path + " to datum_index: " + to_hex_string(new_datum_base);
			message_list.push_back(loading_message);

			//target memory address where module has to be copied
			int new_mem_off_base = default_meta_size + injected_meta_size;

			//copying tables and tag_data
			tags::tag_instance* tagtables_new_element = &new_global_tag_tables[new_datum_base];
			char* new_tag_data_base_ptr = tags::get_tag_data() + new_mem_off_base;
			char* tag_names_buffer_ptr = module_tag_names->data;

			memcpy(tagtables_new_element, module_tag_table->data, module_tag_table->size);
			memcpy(new_tag_data_base_ptr, module_tag_data->data, module_tag_data->size);

			//stuff to update memory_offsets and datum indices utilizing rebase tables
			int memory_offset_difference = new_mem_off_base - tagtables_new_element->data_offset;
			int datum_index_difference = new_datum_base - tagtables_new_element->datum_index.ToAbsoluteIndex();

			//fix meta data
			if (memory_offset_difference != 0)
			{
				int* rebase_table_element = (int*)module_tag_rebase_table->data;
				for (int i = 0; i < (module_tag_rebase_table->size / 4); i++)
					*(int*)(new_tag_data_base_ptr + rebase_table_element[i]) = *(int*)(new_tag_data_base_ptr + rebase_table_element[i]) + memory_offset_difference;

			}
			if (datum_index_difference != 0)
			{
				int* index_table_element = (int*)module_index_rebase_table->data;
				for (int i = 0; i < (module_index_rebase_table->size / 4); i++)
					*(int*)(new_tag_data_base_ptr + index_table_element[i]) = *(int*)(new_tag_data_base_ptr + index_table_element[i]) + datum_index_difference;

			}
			//fix tag tables and before calling raw loading proceedure
			for (int i = 0; (i < module_tag_table->size / 0x10); i++)
			{
				tagtables_new_element[i].datum_index.Index = tagtables_new_element[i].datum_index.Index + datum_index_difference;
				tagtables_new_element[i].data_offset = tagtables_new_element[i].data_offset + memory_offset_difference;
			}
			//load the sound gestalt
			snd out_snd_tag;
			ugh *tertiery_gestalt = (ugh*)(tags::get_tag_data() + tagtables_new_element[(module_tag_table->size / 0x10) - 1].data_offset);
			sound_gestalt_manager::Add_sound_chunk(*tertiery_gestalt, out_snd_tag);

			//vftable references and resource loading
			for (int i = 0; (i < module_tag_table->size / 0x10); i++)
			{
				tag_loader_tag_instance tag_instance;

				tag_instance.tag_instance = tagtables_new_element[i];
				tag_instance.tag_module_name = tag_module_rel_path;
				tag_instance.debug_tag_name = tag_names_buffer_ptr;

				loaded_tag_list.push_back(tag_instance);

				tag_add_to_sync_list(tagtables_new_element[i].type.as_int(), new_datum_base + i);
				tag_load_vftable_reference(new_datum_base + i);
				tag_load_resource_data(new_datum_base + i);

				//snd! tag index fixes
				if (tagtables_new_element[i].type == 'snd!')
				{
					snd *t_snd_tag = (snd*)(tags::get_tag_data() + tagtables_new_element[i].data_offset);

					if (t_snd_tag->PlaybackParameterIndex != (short)-1)
						t_snd_tag->PlaybackParameterIndex += out_snd_tag.PlaybackParameterIndex;
					if (t_snd_tag->PitchRangeIndex != (short)-1)
						t_snd_tag->PitchRangeIndex += out_snd_tag.PitchRangeIndex;
					if (t_snd_tag->ScaleIndex != (unsigned __int8)-1)
						t_snd_tag->ScaleIndex += out_snd_tag.ScaleIndex;
					if (t_snd_tag->PromotionIndex != (unsigned __int8)-1)
						t_snd_tag->PromotionIndex += out_snd_tag.PromotionIndex;
					if (t_snd_tag->CustomPlaybackIndex != (unsigned __int8)-1)
						t_snd_tag->CustomPlaybackIndex += out_snd_tag.CustomPlaybackIndex;
					if (t_snd_tag->ExtraInfoIndex != (short)-1)
						t_snd_tag->ExtraInfoIndex += out_snd_tag.ExtraInfoIndex;
				}

				tag_names_buffer_ptr += tag_instance.debug_tag_name.size() + 1;
			}
			new_datum_base += (module_tag_table->size / 0x10);
			injected_meta_size += module_tag_data->size;
		}
		else
		{
			std::string _error;
			my_loader->pop_messages(_error);
			message_list.push_back(_error);
		}
		delete my_loader;

		return ret;
	}
	void tag_module_loader::tag_module_get_injected_list(std::vector<std::string> &out)
	{

	}
	void tag_module_loader::tag_get_injected_list(std::vector<tag_loader_tag_instance> &out)
	{

	}
	void tag_module_loader::pop_messages(std::string &out)
	{
		out = "";
		for each (auto i in message_list)
			out += i + "\n";
		message_list.clear();
	}
	void tag_module_loader::tag_load_vftable_reference(datum datum_index)
	{
		blam_tag type = new_global_tag_tables[datum_index.ToAbsoluteIndex()].type;
		datum Tdatum_index = new_global_tag_tables[datum_index.ToAbsoluteIndex()].datum_index;

		if (Tdatum_index != datum_index)
		{
			std::string error = "tag_module_loader : Tag: " + datum_index.ToInt();
			error += " not loaded into tag tables and tag memory";
			throw new std::exception(error.c_str());
		}

		switch (type.as_int())
		{
		case 'crea':
			halo2_tag_vftable_reference_fix::crea_fix(datum_index.ToAbsoluteIndex());
			break;
		case 'bipd':
			halo2_tag_vftable_reference_fix::bipd_fix(datum_index.ToAbsoluteIndex());
			break;
		case 'coll':
			halo2_tag_vftable_reference_fix::coll_fix(datum_index.ToAbsoluteIndex());
			break;
		case 'phmo':
			halo2_tag_vftable_reference_fix::phmo_fix(datum_index.ToAbsoluteIndex(), false);
			break;
		case 'vehi':
			halo2_tag_vftable_reference_fix::vehi_fix(datum_index.ToAbsoluteIndex());
			break;
		default:
			break;
		}
	}
	void tag_module_loader::tag_load_resource_data(datum datum_index)
	{
		tags::tag_instance* tag_table_element = &new_global_tag_tables[datum_index.ToAbsoluteIndex()];
		char* tag_data = tags::get_tag_data() + new_global_tag_tables[datum_index.ToAbsoluteIndex()].data_offset;

		//fail safe
		if (tag_table_element->datum_index.ToAbsoluteIndex() != datum_index.ToAbsoluteIndex())
		{
			std::string error = "tag_module_loader : Tag: " + datum_index.ToInt();
			error += " not loaded into tag tables and tag memory";
			throw new std::exception(error.c_str());
		}

		switch (tag_table_element->type.as_int())
		{
		case 'mode':

			if (*(int*)(tag_data + 0x24) > 0)
			{
				int v15 = 0;

				int off = 0;
				do
				{
					int sections_off = *(int*)(tag_data + 0x28);
					int sections_base = 0;
					if (sections_off != -1)
						sections_base = (DWORD)tags::get_tag_data() + sections_off;
					((void(__cdecl *)(int, unsigned int))h2mod->GetAddress(0x2652BC))(sections_base + off + 0x38, 3u);//TODO
					++v15;
					off += 0x5C;
				} while (v15 < *(int*)(tag_data + 0x24));
			}
			break;

		case 'bitm':
		{

			int old_list_field = *h2mod->GetAddress<DWORD*>(0xA49270 + 0x1FC);//TODO

			for (int i = 0; i < *(int*)(tag_data + 0x44); i++)
			{

				int bitmaps_field_off = *(int*)(tag_data + 0x48);

				int bitmaps_field_base = 0;
				if (bitmaps_field_off != -1)
					bitmaps_field_base = (DWORD)tags::get_tag_data() + bitmaps_field_off;

				int bitmaps_field = bitmaps_field_base + 0x74 * i;

				*h2mod->GetAddress<DWORD*>(0xA49270 + 0x1FC) = bitmaps_field;//TODO

				int temp = 0;
				((int(__cdecl *)(int, char, int, void*))h2mod->GetAddress(0x265986))(bitmaps_field, 2, 0, &temp);//TODO

			}
			*h2mod->GetAddress<DWORD*>(0xA49270 + 0x1FC) = old_list_field;//TODO
			break;
		}
		default:
			break;
		}
	}
	HANDLE tag_module_loader::tag_get_module_handle(datum datum_index)
	{
		for (int i = 0; i < loaded_module_list.size(); i++)
		{
			if (datum_index.Index >= loaded_module_list[i].module_target_index)
			{
				if (datum_index < loaded_module_list[i].module_end_index)
					return loaded_module_list[i].module_HANDLE;
			}
		}
		return INVALID_HANDLE_VALUE;
	}
	void tag_module_loader::tag_add_to_sync_list(int type, datum index)
	{
		switch ((TagGroupTypes)type)
		{
		case TagGroupTypes::biped:
		case TagGroupTypes::vehicle:
		case TagGroupTypes::weapon:
		case TagGroupTypes::garbage:
		case TagGroupTypes::projectile:
		case TagGroupTypes::crate:
		case TagGroupTypes::damageeffect:
		case TagGroupTypes::device:
		case TagGroupTypes::scenery:
		case TagGroupTypes::devicelightfixture:
		case TagGroupTypes::soundscenery:
		case TagGroupTypes::creature:
		case TagGroupTypes::devicemachine:
		case TagGroupTypes::equipment:

			datum scnr_index = tags::get_tags_header()->scenario_datum;
			auto GlobalSCNR = (Blam::Cache::Tags::scnr*)TagInterface::global_tags_interface::GetTagInterface(scnr_index, (int)TagGroupTypes::scenario);

			struct Blam::Cache::Tags::scnr::SimulationDefinitionTable block;
			block.Tag = index;

			GlobalSCNR->SimulationDefinitionTable.PushBack(&block);

			break;
		}
	}
#pragma region patch_calls
	//Function to allocate some more space for tagtables and tags
	unsigned int __cdecl patchcall_h2_AllocateMemory(int default_meta_size, char alignment)
	{
		//i need to allocate more space
		int new_meta_size = default_meta_size + MAX_ADDITIONAL_TAG_SIZE;
		tag_module_loader::SetDefaultMetaSize(default_meta_size + 0x20);//spacing<-----------this here acts as a protective measure to inidicate that further allocation has taken place

		return  halo2_memory_functions::_AllocatePtr(new_meta_size, alignment);
	}
	//Function to allow loading of Resource data besides from default maps
	DWORD __cdecl patchcall_h2_LoadResource1(int datum_index, unsigned int map_off, int size, void *mem_addr, __int8 *a5, int a6, int a7)
	{
		HANDLE tag_handle = tag_module_loader::tag_get_module_handle(datum_index);

		if (tag_handle == INVALID_HANDLE_VALUE)
		{
			///call default
			typedef DWORD(__cdecl *h2_Load_Resource)(int datum_index, unsigned int map_off, int size, void *mem_addr, __int8 *a5, int a6, int a7);
			h2_Load_Resource ph2_Load_Resource;
			ph2_Load_Resource = h2mod->GetAddress<h2_Load_Resource>(0x64E6F);//TODO

			return ph2_Load_Resource(datum_index, map_off, size, mem_addr, a5, a6, a7);
		}
		else
		{
			///force load our stuff
			typedef DWORD(__cdecl *h2_Load_Resource_from_map)(HANDLE *file_index, int datum_index, unsigned int map_off, int size, void* mem_addr, __int8 * a5, int a6, int a7);
			h2_Load_Resource_from_map ph2_Load_Resource_from_map;
			ph2_Load_Resource_from_map = h2mod->GetAddress<h2_Load_Resource_from_map>(0x64E07);//TODO

			return ph2_Load_Resource_from_map(&tag_handle, datum_index, map_off, size, mem_addr, a5, a6, a7);
		}
	}
	//Function to allow loading of Resource data besides from default maps
	DWORD __cdecl patchcall_h2_LoadResource2(int owner_tag_index, int MapOff, int sizeM, unsigned int destination_ptr)
	{
		HANDLE tag_handle = tag_module_loader::tag_get_module_handle(owner_tag_index);

		if (tag_handle == INVALID_HANDLE_VALUE)
		{
			///call default
			typedef DWORD(__cdecl *sub_A94D01)(int owner_tag_index, int MapOff, int sizeM, unsigned int destination_ptr);
			sub_A94D01 ph2_Load_Resource;
			ph2_Load_Resource = h2mod->GetAddress<sub_A94D01>(0x64D01);//TODO

			return ph2_Load_Resource(owner_tag_index, MapOff, sizeM, destination_ptr);
		}
		else
		{
			///force load our stuff
			typedef DWORD(__cdecl *h2_Load_Resource_from_map)(HANDLE *file_index, int datum_index, unsigned int map_off, int size, int mem_addr, __int8 a5);
			h2_Load_Resource_from_map ph2_Load_Resource_from_disk;
			ph2_Load_Resource_from_disk = h2mod->GetAddress<h2_Load_Resource_from_map>(0x64C72);//TODO

			return ph2_Load_Resource_from_disk(&tag_handle, owner_tag_index, MapOff, sizeM, destination_ptr, 0);
		}
	}
#pragma endregion
}
