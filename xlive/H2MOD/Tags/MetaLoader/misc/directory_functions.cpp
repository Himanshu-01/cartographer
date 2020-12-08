#include"directory_functions.h"

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