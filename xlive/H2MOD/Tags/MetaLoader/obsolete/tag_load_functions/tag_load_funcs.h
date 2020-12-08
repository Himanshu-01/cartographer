#pragma once
#include "..\meta_definitions\meta_struct.h"
#include "..\meta_cache_loader\cache_loader.h"
#include "..\H2MOD\Tags\TagInterface.h"
#include "..\tag_loader_constants.h"

using meta_struct::meta;
using meta_struct::plugins_field;
using meta_struct::injectRefs;

//The TAG LOADER
namespace tag_loader
{
	//Loads a tag from specified map in accordance with the datum index supplied
	void Load_tag(int datum_index, bool recursive, std::string map, bool custom = false);
	//Return the size of the meta that is currently in the que
	unsigned int Que_meta_size();
	//sets various directories required for working of tag stuff
	void Set_directories(std::string default_maps, std::string custom_maps, std::string custom_tags, std::string plugin_loc);
	//Updates datum_indexes and rebases tags before inserting into memory
	//pushes the tag_data in que to the tag_tables and tag_memory in the custom_tags allocated space,safer method
	int Push_Back();
	//Dumps meta data in que in the specified tag folder(integrity check)
	void Dump_Que_meta();
	//return and clears all the error messages incurred
	void Pop_messages(std::string&);
	//return a tag_name list
	void Pop_tag_list(std::vector<std::string>&);
	//return the target_index of the first tag in the memory
	int Load_tag_module(std::string loc);
	//verifies and adds the valid tags to GlobalScenario->SimulationDefinitionTable Block
	void Add_to_sync_list(int type, DWORD index);
	//returns new_Datum_base
	int Get_new_datum_base();
	//returns mods directory
	void Get_mods_dir(std::string &out);
	//return custom tags directory
	void Get_custom_tags_dir(std::string &out);
}
//function to initialize tag_loader variables
void _tag_loader_init();
//function to be called when a map is loaded to correctly refer stuff
void _tag_loader_onMapLoad();