#include"tag_load_funcs.h"
#include "tag_load_funcs.h"
#include "..\Util\filesys.h"

#include "Globals.h"
#include "H2MOD\Modules\OnScreenDebug\OnScreenDebug.h"
#include "H2MOD\Tags\global_tags_interface.h"
#include "..\Blam\Cache\Tags\tag_definitons.h"
#include"..\h2_tag_functions\h2_tag_funcs.h"

using Blam::Enums::Tags::TagGroupTypes;

//actual tag_loader namespace
namespace tag_loader
{
	//some function declarations
	void Fix_Tag_vftable_reference(datum datum_index);
	void Load_RAW_refs(datum datum_index);
	void Push_Back(int datum_index);

	std::string def_maps_dir;
	std::string cus_maps_dir;
	std::string cus_tag_dir;
	std::string plugins_dir;
	std::string mods_dir;

	//contains list of various plugin structures
	static std::map<std::string, std::shared_ptr<plugins_field>> plugins_list;
	//<datum_index,meta_junk>contains the list of tags that are currently in que to loaded in memory
	std::map<int, std::shared_ptr<meta>> que_meta_list;
	//map loc of the tags currently in que list
	std::string current_que_tags_map;
	//another var just to keep the keys along with the correct order
	std::vector<int> key_list;
	//contains various messages generated during various processes,shouldnt had named it error list
	std::vector<std::string> error_list;
	//contains a list of tag_indices along with their names(currently implemented only for module loading)
	std::vector<std::string> tag_list;

	//default meta size of a map
	unsigned int default_meta_size = 0;
	//injected meta size
	unsigned int injected_meta_size = 0;
	//starting index for injected tag
	unsigned int new_datum_base = -1;
	//pointer to newer allocated tables
	tags::tag_instance* new_Tables = nullptr;

	//handle listing for various tags and modules
	struct s_handle_listing
	{
		///handle to the module file or map
		HANDLE file_handle;
		///first index in the tag_table
		int start_datum_index;
		///excluding the last index
		int end_datum_index;
		///default ctor
		s_handle_listing()
		{
			file_handle = INVALID_HANDLE_VALUE;
			start_datum_index = -1;
			end_datum_index = -1;
		}
		///copy constructor
		s_handle_listing(const s_handle_listing& other)
		{
			memcpy(this, &other, sizeof(s_handle_listing));
		}
		///assignment operator
		s_handle_listing& operator=(const s_handle_listing& other)
		{
			memcpy(this, &other, sizeof(s_handle_listing));
			return *this;
		}
	};
	std::vector<s_handle_listing> tags_handle_list;
	//returns the file_handle concerned with respective injected tag
	//as per contained in tags_handle_list
	HANDLE Get_file_handle_from_tag_index(int datum_index)
	{
		for each (auto iter in tags_handle_list)
			if (datum_index >= iter.start_datum_index && datum_index < iter.end_datum_index)
				return iter.file_handle;

		return INVALID_HANDLE_VALUE;
	}
	//returns reference to plugin of specified type
	std::shared_ptr<plugins_field> Get_Tag_structure_from_type(std::string type)
	{
		//we first look into the already loaded plugin list
		for (auto& i : plugins_list)
		{
			if (i.first == type)
				return i.second;
		}
		//it doesnt contain it therfore we need to load the plugin
		std::string plugin_loc = plugins_dir + '\\' + type + ".xml";
		std::shared_ptr<plugins_field> temp_plugin = meta_struct::Get_Tag_stucture_from_plugin(plugin_loc);

		if (temp_plugin)
			plugins_list.emplace(type, temp_plugin);
		else
		{
			std::string error = "Couldnt load plugin " + type + ".xml";
			error_list.push_back(error);
		}
		return temp_plugin;
	}
	//returns whether the map is a shared map or not
	bool Check_shared(std::ifstream* fin)
	{
		char* map_header = new char[0x800];
		fin->seekg(0x0);
		fin->read(map_header, 0x800);

		if (tags::get_cache_header()->type == tags::cache_header::scnr_type::MultiplayerShared || tags::get_cache_header()->type == tags::cache_header::scnr_type::SinglePlayerShared)
		{
			delete[] map_header;
			return true;
		}
		delete[] map_header;
		return false;
	}
	//Loads a tag from specified map in accordance with the datum index supplied
	///custom flag is no more needed
	void Load_tag(int datum_index, bool recursive, std::string map, bool custom)
	{
		std::ifstream* fin;
		std::string map_loc;

		//logic to check the existence of the map at subsequent directories
		///mods->default->custom
		if (Get_file_type(map) == "map")
			map_loc = mods_dir + "\\maps\\" + map;
		else
			map_loc = mods_dir + "\\maps\\" + map + ".map";

		if (PathFileExistsA(map_loc.c_str()));
		else
		{
			if (Get_file_type(map) == "map")
				map_loc = def_maps_dir + '\\' + map;
			else
				map_loc = def_maps_dir + '\\' + map + ".map";

			if (PathFileExistsA(map_loc.c_str()));
			else
			{
				if (Get_file_type(map) == "map")
					map_loc = cus_maps_dir + '\\' + map;
				else
					map_loc = cus_maps_dir + '\\' + map + ".map";
			}
		}

		fin = new std::ifstream(map_loc.c_str(), std::ios::binary | std::ios::in);

		if (fin->is_open())
		{
			std::string temp_string = "Loading tag : " + meta_struct::to_hex_string(datum_index) + " from " + map;
			error_list.push_back(temp_string);

			//some meta reading prologue
			int table_off, table_size = 0;

			fin->seekg(0x10);
			fin->read((char*)&table_off, 4);
			fin->read((char*)&table_size, 4);


			fin->seekg(table_off + 4);
			int temp;
			fin->read((char*)&temp, 4);

			int table_start = table_off + 0xC * temp + 0x20;
			int scnr_off = table_off + table_size;

			int scnr_memaddr;
			fin->seekg(table_start + 0x8);
			fin->read((char*)&scnr_memaddr, 4);

			tags::tag_instance tag_info;

			//create a tag_list to load
			std::list<int> load_tag_list;
			load_tag_list.push_back(datum_index);

			//----------------LOOPING STUFF
			while (load_tag_list.size())
			{
				if (*load_tag_list.cbegin() != -1 && *load_tag_list.cbegin() != 0)
				{
					//method to read tag type
					fin->seekg(table_start + (0xFFFF & *(load_tag_list.cbegin())) * sizeof(tags::tag_instance));

					fin->read((char*)&tag_info, sizeof(tags::tag_instance));

					if (*(load_tag_list.cbegin()) == tag_info.datum_index.ToInt())
					{
						std::shared_ptr<plugins_field> temp_plugin = Get_Tag_structure_from_type(tag_info.type.as_string());

						//precaution for plugin load errors
						if (!temp_plugin)
							break;

						//we first check the integrity of the datum_index
						if (tag_info.datum_index.ToInt() == *(load_tag_list.cbegin()) && tag_info.data_offset && tag_info.size > 0 && (que_meta_list.find(tag_info.datum_index.ToInt()) == que_meta_list.cend()))
						{
							//read the meta data from the map            
							char* data = new char[tag_info.size];

							int map_off;
							if (!Check_shared(fin))
								map_off = scnr_off + (tag_info.data_offset - scnr_memaddr);
							else
								map_off = scnr_off + (tag_info.data_offset - 0x3C000);
							fin->seekg(map_off);
							//0x3c000 is a hardcoded value in blam engine

							fin->read(data, tag_info.size);

							//create a meta object
							std::shared_ptr<meta> temp_meta = std::make_shared<meta>(data, tag_info.size, tag_info.data_offset, temp_plugin, fin, map_off, 1, *(load_tag_list.cbegin()), map_loc, tag_info.type.as_string());
							//add it to que listing
							que_meta_list.emplace(*(load_tag_list.cbegin()), temp_meta);
							key_list.push_back(*(load_tag_list.cbegin()));

							if (recursive)
							{
								std::list<int> temp_tag_ref = temp_meta->Get_all_tag_refs();

								//to remove redundancies
								std::list<int>::iterator ref_iter = temp_tag_ref.begin();
								while (ref_iter != temp_tag_ref.end())
								{
									if (que_meta_list.find(*ref_iter) != que_meta_list.end())
										ref_iter = temp_tag_ref.erase(ref_iter);
									else
										ref_iter++;
								}

								load_tag_list.insert(load_tag_list.end(), temp_tag_ref.begin(), temp_tag_ref.end());
							}
						}
						else
						{
							//most of time this is caused due to shared stuff
							std::string temp_error = "Invalid Datum index :0x" + meta_struct::to_hex_string(*(load_tag_list.cbegin()));
							error_list.push_back(temp_error);
						}
					}
				}
				//list cleaning stuff
				load_tag_list.pop_front();
			}
			fin->close();
			current_que_tags_map = map_loc;
		}
		else
		{
			std::string temp_error;
			if (!custom)
				temp_error = "Couldnt open default map " + map;
			else
				temp_error = "Couldnt open custom map " + map;

			error_list.push_back(temp_error);
		}
		delete fin;//uh forgot that
	}
	//Returns the size of the meta that is currently in the que to be injected
	unsigned int Que_meta_size()
	{
		unsigned int ret = 0;

		for (auto& i : que_meta_list)
		{
			ret += i.second->Get_Total_size();
		}

		return ret;
	}
	//sets various directories required for working of tag stuff
	void Set_directories(std::string default_maps, std::string custom_maps, std::string custom_tags, std::string plugin_loc)
	{
		def_maps_dir = default_maps;
		cus_maps_dir = custom_maps;
		cus_tag_dir = custom_tags;
		plugins_dir = plugin_loc;
	}
	//Updates datum_indexes and rebases tags before inserting into memory
	//pushes the tag_data in que list to the tag_tables and tag_memory in the custom_tags allocated space
	int Push_Back()
	{
		int ret = new_datum_base;

		if ((injected_meta_size + Que_meta_size() < MAX_ADDITIONAL_TAG_SIZE))
		{//does some maths to help u out
			Push_Back(new_datum_base);
			return ret;
		}
		else
		{
			std::string error = "Coudn't inject, Max meta size reached";
			error_list.push_back(error);
			return -1;
		}
	}
	//pushes the tag_data in que list to the tag_tables and tag_memory at specified tag_table index
	//usually i call this to overwrite tag_table of some preloaded tag(for replacing purpose)
	void Push_Back(int datum_index)
	{
		//create a handle_listing and add it as early as possible
		s_handle_listing t_map_handle_listing;
		t_map_handle_listing.start_datum_index = datum_index;
		t_map_handle_listing.file_handle = CreateFileA(current_que_tags_map.c_str(), GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
		t_map_handle_listing.end_datum_index = new_datum_base + key_list.size();
		tags_handle_list.push_back(t_map_handle_listing);

		int mem_off = default_meta_size + injected_meta_size;
		//build up inject refs
		std::list<injectRefs> my_inject_refs;

		bool replaced = false;
		for (size_t i = 0; i < key_list.size(); i++)
		{
			injectRefs temp;
			temp.old_datum = key_list[i];

			if (!replaced)
			{
				if ((datum_index & 0xFFFF) < new_datum_base)
					temp.new_datum = datum_index;
				else
					temp.new_datum = new_datum_base++;
				replaced = true;
			}
			else
			{
				temp.new_datum = new_datum_base++;
			}

			my_inject_refs.push_back(temp);

		}
		std::string temp = "Pushing back tag : " + meta_struct::to_hex_string(my_inject_refs.begin()->old_datum) + " to : " + meta_struct::to_hex_string(my_inject_refs.begin()->new_datum);
		error_list.push_back(temp);

		//update the que list tags
		for (auto& i : key_list)
			que_meta_list[i]->Update_datum_indexes(my_inject_refs);

		//Add them to the tables
		for (auto& my_inject_refs_iter : my_inject_refs)
		{

			if (default_meta_size)
			{
				int meta_size = que_meta_list[my_inject_refs_iter.old_datum]->Get_Total_size();
				tags::tag_instance tables_data;

				que_meta_list[my_inject_refs_iter.old_datum]->Rebase_meta(mem_off);
				char* meta_data = que_meta_list[my_inject_refs_iter.old_datum]->Generate_meta_file();

				blam_tag type = std::stoi(que_meta_list[my_inject_refs_iter.old_datum]->Get_type());

				tables_data.type = type;
				tables_data.data_offset = mem_off;
				tables_data.size = meta_size;
				tags::tag_instance *temp_write_off = &tag_loader::new_Tables[my_inject_refs_iter.new_datum & 0xFFFF];
				memcpy(temp_write_off, &tables_data, sizeof(tags::tag_instance));//copy to the tables
				memcpy(tags::get_tag_data() + mem_off, meta_data, meta_size);//copy to the tag memory
				//Load RAW
				Load_RAW_refs(my_inject_refs_iter.new_datum);
				//fix the global_refs
				Fix_Tag_vftable_reference(my_inject_refs_iter.new_datum);

				delete[] meta_data;
				mem_off += meta_size;
				injected_meta_size += meta_size;
			}
			else break;
			//meta_list.emplace(my_inject_refs_iter->new_datum, que_iter->second);//add it to the meta _list				
		}
		my_inject_refs.clear();
		que_meta_list.clear();
		current_que_tags_map = "";
	}
	//Rebases to 0x0 and dumps meta data in que in the specified tag folder
	void Dump_Que_meta()
	{
		std::ofstream fout;

		for (auto& i : que_meta_list)
		{
			std::string file_loc = cus_tag_dir + "\\que\\" + meta_struct::to_hex_string(i.first);

			i.second->Rebase_meta(0x0);
			int size = i.second->Get_Total_size();
			char* data = i.second->Generate_meta_file();
			std::string type = i.second->Get_type();

			file_loc += '.' + type;
			fout.open(file_loc, std::ios::out | std::ios::binary);

			fout.write(data, size);

			delete[] data;
			fout.close();
		}

	}
	//return and clears all the error messages incurred
	void Pop_messages(std::string& ret)
	{
		ret = "";
		while (!error_list.empty())
		{
			ret = error_list[error_list.size() - 1] + '\n' + ret;
			error_list.pop_back();
		}
	}
	//returns the injected tag list 
	void Pop_tag_list(std::vector<std::string>& out)
	{
		out = tag_list;
	}
	//function to try and return a handle to the map (map_name or scenario_name(same as the actual map_name) supported)
	//Checks inside mods//maps folder first then maps folder and finally inside custom maps folder
	HANDLE try_find_map(std::string map)
	{
		std::string map_loc = map;
		//checking for full path length
		if (!PathFileExistsA(map_loc.c_str()))
		{
			if (map.find('\\') == std::string::npos)
			{
				//could be both map_name with or without extension
				if (Get_file_type(map) == "map")
					map_loc = mods_dir + "\\maps\\" + map;
				else
					map_loc = mods_dir + "\\maps\\" + map + ".map";

				if (PathFileExistsA(map_loc.c_str()));
				else
				{
					if (Get_file_type(map) == "map")
						map_loc = def_maps_dir + '\\' + map;
					else
						map_loc = def_maps_dir + '\\' + map + ".map";

					if (PathFileExistsA(map_loc.c_str()));
					else
					{
						if (Get_file_type(map) == "map")
							map_loc = cus_maps_dir + '\\' + map;
						else
							map_loc = cus_maps_dir + '\\' + map + ".map";
					}
				}
				return CreateFileA(map_loc.c_str(), GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
			}
			else
			{
				//scenario name
				//try retrieving map_name from scenario
				std::string map_name = map_loc.substr(map_loc.rfind('\\') + 1);
				map_loc = mods_dir + "\\maps\\" + map_name + ".map";
				//only tries to load from <mods_dir>\\maps cause game can auto load from default locations
				///i suggest naming map_names same as scenario names--saves the trouble
				return CreateFileA(map_loc.c_str(), GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
			}
		}
		else
		{
			//full path length
			return CreateFileA(map_loc.c_str(), GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
		}
	}
	//Function to load RAW Resource of a tag
	//The function calls concerned game function as per the tag supplied
	void Load_RAW_refs(datum datum_index)
	{
		tags::tag_instance* tag_info = &new_Tables[datum_index.ToAbsoluteIndex()];
		char* tag_data = tags::get_tag_data() + new_Tables[datum_index.ToAbsoluteIndex()].data_offset;
		DWORD ETCOFFSET = *(DWORD*)(h2mod->GetBase() + 0x482290);

		//fail safe
		if (tag_info->datum_index.ToAbsoluteIndex() != datum_index.ToAbsoluteIndex())
		{
			std::string error = "Tag: " + datum_index.ToInt();
			error += " not loaded into tag tables and tag memory";
			throw new std::exception(error.c_str());
		}

		switch (tag_info->type.as_int())
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
						sections_base = ETCOFFSET + sections_off;
					((void(__cdecl *)(int, unsigned int))h2mod->GetAddress(0x2652BC))(sections_base + off + 0x38, 3u);
					++v15;
					off += 0x5C;
				} while (v15 < *(int*)(tag_data + 0x24));
			}
			break;

		case 'bitm':
		{

			int old_list_field = *h2mod->GetAddress<DWORD*>(0xA49270 + 0x1FC);

			for (int i = 0; i < *(int*)(tag_data + 0x44); i++)
			{

				int bitmaps_field_off = *(int*)(tag_data + 0x48);

				int bitmaps_field_base = 0;
				if (bitmaps_field_off != -1)
					bitmaps_field_base = bitmaps_field_off + ETCOFFSET;

				int bitmaps_field = bitmaps_field_base + 0x74 * i;

				*h2mod->GetAddress<DWORD*>(0xA49270 + 0x1FC) = bitmaps_field;

				int temp = 0;
				((int(__cdecl *)(int, char, int, void*))h2mod->GetAddress(0x265986))(bitmaps_field, 2, 0, &temp);

				((int(__cdecl *)(int, char, int, void*))h2mod->GetAddress(0x265986))(bitmaps_field, 0, 0, &temp);

			}
			*h2mod->GetAddress<DWORD*>(0xA49270 + 0x1FC) = old_list_field;
			break;
		}
		default:
			break;
		}
	}
	//Fixes the reference of the tags to their global objects(vftables)
	void Fix_Tag_vftable_reference(datum datum_index)
	{
		blam_tag type = new_Tables[datum_index.ToAbsoluteIndex()].type;
		datum Tdatum_index = new_Tables[datum_index.ToAbsoluteIndex()].datum_index;

		if (Tdatum_index != datum_index)
		{
			std::string error = "Tag: " + datum_index.ToInt();
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
	//Basically created to easily transfer working tags from system to system
	int Load_tag_module(std::string loc)
	{
		int ret = -1;

		if (!new_Tables)
			return -1;

		auto my_loader = new meta_cache_loader::cache_loader(loc);

		if (my_loader->isopened())
		{
			auto module_tag_table = my_loader->get_BLOCK(meta_cache_loader::block_type::Tag_table_block);
			auto module_tag_data = my_loader->get_BLOCK(meta_cache_loader::block_type::Tag_data_block);
			auto module_tag_rebase_table = my_loader->get_BLOCK(meta_cache_loader::block_type::Tag_rebase_block);
			auto module_index_rebase_table = my_loader->get_BLOCK(meta_cache_loader::block_type::Tag_index_rebase_block);
			auto module_tag_names = my_loader->get_BLOCK(meta_cache_loader::block_type::Tag_names_block);
			auto module_tag_maps = my_loader->get_BLOCK(meta_cache_loader::block_type::Tag_map_block);

			//create a handle_listing and add it as early as possible
			s_handle_listing t_module_handle_listing;
			t_module_handle_listing.start_datum_index = new_datum_base;
			t_module_handle_listing.file_handle = CreateFileA(loc.c_str(), GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
			t_module_handle_listing.end_datum_index = new_datum_base + module_tag_table->size / 0x10;
			tags_handle_list.push_back(t_module_handle_listing);

			if (injected_meta_size + module_tag_data->size > MAX_ADDITIONAL_TAG_SIZE)
			{
				std::string error = "Couldnt load module, MAX_ADDITIONAL_TAG_SIZE_ reached";
				error_list.push_back(error);
				delete my_loader;

				return ret;
			}
			ret = new_datum_base;
			std::string error = "Loading module: " + loc + " to datum_index: " + meta_struct::to_hex_string(new_datum_base);
			error_list.push_back(error);

			//target memory address where module has to be copied
			int new_mem_off_base = default_meta_size + injected_meta_size;

			//copying tables and tag_data
			tags::tag_instance* new_tag_table_ptr = &new_Tables[new_datum_base];
			char* new_tag_data_base_ptr = tags::get_tag_data() + new_mem_off_base;
			char* n_ptr = module_tag_names->data;
			//char* m_ptr = module_tag_maps->data;
			memcpy(new_tag_table_ptr, module_tag_table->data, module_tag_table->size);
			memcpy(new_tag_data_base_ptr, module_tag_data->data, module_tag_data->size);

			//stuff to update memory_offsets and datum indices
			int memory_offset_difference = new_mem_off_base - new_tag_table_ptr->data_offset;
			int datum_index_difference = new_datum_base - new_tag_table_ptr->datum_index.ToAbsoluteIndex();

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
			//fix tag tables and call raw loading proceedure
			for (int i = 0; (i < module_tag_table->size / 0x10); i++)
			{
				new_tag_table_ptr[i].datum_index.Index = new_tag_table_ptr[i].datum_index.Index + datum_index_difference;
				new_tag_table_ptr[i].data_offset = new_tag_table_ptr[i].data_offset + memory_offset_difference;
			}
			///if any of the game function try to internaly acceses newer tags,it may cause crash
			///therefore for precaution
			for (int i = 0; (i < module_tag_table->size / 0x10); i++)
			{
				std::string t_name = n_ptr;

				tag_list.push_back(t_name.substr(t_name.rfind('\\') + 1) + ",0x" + meta_struct::to_hex_string(new_datum_base + i));
				tag_loader::Add_to_sync_list(new_tag_table_ptr[i].type.as_int(), new_datum_base + i);

				Load_RAW_refs(new_datum_base + i);
				Fix_Tag_vftable_reference(new_datum_base + i);

				n_ptr += t_name.size() + 1;
			}
			new_datum_base += (module_tag_table->size / 0x10);
			injected_meta_size += module_tag_data->size;
		}
		else
		{
			std::string _error;
			my_loader->pop_messages(_error);
			error_list.push_back(_error);
		}
		delete my_loader;

		return ret;
	}
	//updating SimulationBlock
	void Add_to_sync_list(int type, DWORD index)
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
	int Get_new_datum_base()
	{
		return new_datum_base;
	}
	void Get_mods_dir(std::string &out)
	{
		out = mods_dir;
	}
	void Get_custom_tags_dir(std::string &out)
	{
		out = cus_tag_dir;
	}
}
//Patch call function
//Function to allocate some more space for tagtables and tags
unsigned int __cdecl patch_callAllocateMemory(int old_size, char arg_4)
{
	typedef unsigned int(_cdecl *h2_Allocate_memory)(int size, char arg_4);
	h2_Allocate_memory pAllocate_memory;
	pAllocate_memory = h2mod->GetAddress<h2_Allocate_memory>(0x37E69);

	//i need to allocate more space
	int new_size = old_size + MAX_ADDITIONAL_TAG_SIZE;
	tag_loader::default_meta_size = old_size + 0x20;//spacing

	return pAllocate_memory(new_size, arg_4);
}
//Patch call function
//Function to allow loading of Resource data besides from default maps
wchar_t *__cdecl patch_callLoad_Resource(int datum_index, unsigned int map_off, int size, void *mem_addr, __int8 *a5, int a6, int a7)
{
	HANDLE t_handle = tag_loader::Get_file_handle_from_tag_index(datum_index);

	if (t_handle == INVALID_HANDLE_VALUE)
	{
		///call default
		//Todo :: Update Offsets for Dedi
		typedef wchar_t *(__cdecl *h2_Load_Resource)(int datum_index, unsigned int map_off, int size, void *mem_addr, __int8 *a5, int a6, int a7);
		h2_Load_Resource ph2_Load_Resource;
		ph2_Load_Resource = h2mod->GetAddress<h2_Load_Resource>(0x64E6F);

		return ph2_Load_Resource(datum_index, map_off, size, mem_addr, a5, a6, a7);
	}
	else
	{
		///force load our stuff
		//Todo :: Update Offsets for Dedi
		typedef wchar_t *(__cdecl *h2_Load_Resource_from_map)(HANDLE *file_index, int datum_index, unsigned int map_off, int size, void* mem_addr, __int8 * a5, int a6, int a7);
		h2_Load_Resource_from_map ph2_Load_Resource_from_map;
		ph2_Load_Resource_from_map = h2mod->GetAddress<h2_Load_Resource_from_map>(0x64E07);

		return ph2_Load_Resource_from_map(&t_handle, datum_index, map_off, size, mem_addr, a5, a6, a7);
	}
}
//function to initialize tag_loader variables
void _tag_loader_init()
{
	std::string game_dir(GetExeDirectoryNarrow());
	std::string def_maps_loc = game_dir + "\\maps";

	tag_loader::mods_dir = game_dir + "\\mods";
	tag_loader::Set_directories(def_maps_loc, "", tag_loader::mods_dir + "\\tags", tag_loader::mods_dir + "\\plugins");//no custom support yet

	if (tag_loader::new_Tables == nullptr)
		tag_loader::new_Tables = new tags::tag_instance[MAX_TAG_TABLE_SIZE];

	//Todo :: Update Offsets for Dedi
	///allocating more space for meta loading
	PatchCall(h2mod->GetAddress(0x313B2), patch_callAllocateMemory);
	///patching resource loading calls
	//PatchCall(h2mod->GetAddress(0x3C59E), patch_callLoad_Resource);//for sounds->TODO sound gestalt
	PatchCall(h2mod->GetAddress(0x265208), patch_callLoad_Resource);
	PatchCall(h2mod->GetAddress(0x265A4E), patch_callLoad_Resource);
															   
	//client side desync fix
	///(noping out jump instructions)	
	NopFill(h2mod->GetAddress(0x316CE), 2);
	NopFill(h2mod->GetAddress(0x316DC), 2);
}
//function to be called when a map is loaded to clear previously loaded stuff
void _tag_loader_onMapLoad()
{
	//closing opened handles and clearing handle listing 
	for each (auto i in tag_loader::tags_handle_list)
		CloseHandle(i.file_handle);
	tag_loader::tags_handle_list.clear();

	// reset starting_datum index
	tag_loader::injected_meta_size = 0;
	tag_loader::new_datum_base = -1;

	// extending tag_tables and loading tag for all mutiplayer maps and mainmenu map

	DWORD *TagTableStart = h2mod->GetAddress<DWORD*>(0x47CD50);
	///---------------TABLE EXTENSION  STUFF
	memcpy((BYTE*)tag_loader::new_Tables, (BYTE*)*TagTableStart, 0x3BA40);
	*TagTableStart = (DWORD)tag_loader::new_Tables;
}