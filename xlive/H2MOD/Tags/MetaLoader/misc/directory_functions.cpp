#include"directory_functions.h"
#include "util\filesys.h"
#include "H2MOD\Tags\MetaLoader\tag_loader_constants.h""
std::string Get_file(std::string file_loc)
{
	return file_loc.substr(file_loc.find_last_of('\\') + 1, file_loc.find_last_of('.') - file_loc.find_last_of('\\') - 1);
}
std::string Get_file_directory(std::string file_loc)
{
	return file_loc.substr(0x0, file_loc.find_last_of('\\'));
}
std::string Get_file_type(std::string file)
{
	return file.substr(file.find_last_of('.') + 1, file.length() - file.find_last_of('.') - 1);
}
bool Map_exists(std::string map)
{
	std::string def_maps_dir = GetExeDirectoryNarrow() + "\\maps";
	std::string mods_dir = GetExeDirectoryNarrow() + TAGS_RELATIVE_PATH;
	
	std::string map_loc;
	if (Get_file_type(map) == "map")
		map_loc = mods_dir + "\\maps\\" + map;
	else
		map_loc = mods_dir + "\\maps\\" + map + ".map";

	if (PathFileExistsA(map_loc.c_str()))
		return true;
	
	if (Get_file_type(map) == "map")
		map_loc = def_maps_dir + '\\' + map;
	else
		map_loc = def_maps_dir + '\\' + map + ".map";

	if (PathFileExistsA(map_loc.c_str()))	
		return true;


	return false;
}