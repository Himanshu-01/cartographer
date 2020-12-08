#include "query_parser.h"
#include "H2MOD\Modules\OnScreenDebug\OnScreenDebug.h"
#include "..\module_loader\tag_module_loader.h"
#include "..\..\TagInterface.h"

query_parser::query_parser(std::vector<std::string>& vec_query)
{
	DWORD_list.try_emplace("eax", 0);///lol
	int eax = 0;
	for (size_t i = 0; i < vec_query.size(); i++)
	{
		///only add small code sections here to prevent cluttering
		std::string t = vec_query[i];
		if (t.find("DWORD") != std::string::npos)
		{
			//well we got some variable definitons
			std::string var_name = t.substr(t.find("DWORD") + 6);
			//add, add to list code here
			DWORD_list.try_emplace(var_name, eax);
		}
		else if (t.find("module_load") != std::string::npos)
		{
			//gotta load some tag
			std::string s2 = t.substr(t.find('"') + 1, t.rfind('"') - t.find('"') - 1);
			std::string cus_tag_dir;
			eax = tag_loader::tag_module_loader::tag_module_load(s2);
			DWORD_list["eax"] = eax;
		}
		else if (t.find("_mov") != std::string::npos)
		{
			//primitive mov function _mov(dest,src)
			std::string dest = t.substr(t.find('(') + 1, t.find(',') - t.find('(') - 1);
			std::string src = t.substr(t.find(',') + 1, t.find(')') - t.find(',') - 1);
			//add call to move function here
			_mov_parser(dest, src);
		}
		else if (t.find("replace_tag") != std::string::npos)
		{
			std::string dest = t.substr(t.find('(') + 1, t.find(',') - t.find('(') - 1);
			std::string src = t.substr(t.find(',') + 1, t.find(')') - t.find(',') - 1);

			replace_tag(dest, src);
		}
	}

}
query_parser::query_parser(std::string file_loc)
{
	DWORD_list.try_emplace("eax", 0);

	std::vector<std::string> vec_query;

	std::ifstream fin;
	fin.open(file_loc.c_str(), std::ios::in);

	if (!fin.is_open())
	{
		logs.push_back("Couldnt load query file: " + file_loc);
		return;
	}

	char query[64];
	while (!fin.eof())
	{
		fin.getline(query, 64);
		std::vector<std::string> temp = clean_string(query);

		if (temp.size())
			vec_query.insert(vec_query.end(), temp.begin(), temp.end());
	}
	fin.close();
	query_parser::query_parser(vec_query);
}
std::vector<std::string> query_parser::clean_string(std::string txt)
{
	std::vector<std::string> ret;
	std::string temp = "";
	//remove comment sections
	for (size_t i = 0; i < txt.length(); i++)
	{
		if (txt[i] == '/'&&txt[i + 1] == '/')
			break;
		temp += txt[i];
	}
	//check for 0 length
	if (!temp.length())
		return ret;

	//codes for handling basic assignment(=)
	///seperate instructions
	int end_pos = temp.length();
	for (int i = temp.length() - 1; i >= 0; i--)
	{
		if (txt[i] == '=')
		{
			ret.push_back(temp.substr(i + 1, end_pos - (i + 1)));
			end_pos = i;
		}
	}
	ret.push_back(temp.substr(0, end_pos));
	//-----Put some trimming operations here

	///apply some fixups for assignment operations
	for (size_t i = 0; i < ret.size(); i++)
	{
		if (!keyword_check(ret[i]))
			if (i == 0)
				ret[i] = "_mov(eax," + ret[i] + ")";
			else ret[i] = "_mov(" + ret[i] + ",eax)";
	}
	return ret;
}
int query_parser::try_parse_int(std::string arg)
{
	//well one could pass funky values at times,i dont want the game crash over here
	try
	{
		//check for hex value
		if (arg.find("0x") == 0)
			return  std::stoul(arg.c_str(), nullptr, 16);
		else //decimal
			return  std::stoul(arg.c_str(), nullptr, 10);
	}
	catch (const std::invalid_argument& ia)
	{
		logs.push_back("Invalid integer : " + arg + ",Plz recheck UR PATHETIC SCRIPT");
	}

	return 0x0;
}
std::string query_parser::_getlogs()
{
	std::string ret = "";
	for (size_t i = 0; i < logs.size(); i++)
		ret += '\n' + logs[i];
	logs.clear();
	return ret;
}
void query_parser::_mov_parser(std::string dest, std::string src)
{
	int val = 0x0;

	if (DWORD_list.find(src) != DWORD_list.end())
		val = DWORD_list[src];
	else val = try_parse_int(src);

	if (DWORD_list.find(dest) != DWORD_list.end())
		DWORD_list[dest] = val;
	else logs.push_back("Undeclared variable : " + dest);
}
void query_parser::replace_tag(std::string dest, std::string src)
{
	int a = 0;
	int b = 0;

	if (DWORD_list.find(dest) != DWORD_list.end())
		a = DWORD_list[dest];
	else a = try_parse_int(dest);

	if (DWORD_list.find(src) != DWORD_list.end())
		b = DWORD_list[src];
	else b = try_parse_int(src);

	auto tag_instance = tags::get_tag_instances();

	//Only replace tags if they do exist
	//Game uses similar method to check if the tag actually exists in the table 
	if (tag_instance[a & 0xFFFF].datum_index.Index == (a & 0xFFFF))
	{
		tag_instance[a & 0xFFFF].data_offset = tag_instance[b & 0xFFFF].data_offset;
		tag_instance[a & 0xFFFF].type = tag_instance[b & 0xFFFF].type;
	}


	//Only replace tags if they do exist
	//Game uses similar method to check if the tag actually exists in the table 
	//if (Runtime::Globals::GlobalTagInstances[a & 0xFFFF]->tag_index.Index == (a & 0xFFFF))
	//Runtime::Globals::GlobalTagInstances[a & 0xFFFF]->offset = Runtime::Globals::GlobalTagInstances[b & 0xFFFF]->offset;
}
void query_parser::replace_tag(int dest, int src)
{
	int a = dest;
	int b = src;

	auto tag_instance = tags::get_tag_instances();

	//Only replace tags if they do exist
	//Game uses similar method to check if the tag actually exists in the table 
	if (tag_instance[a & 0xFFFF].datum_index.Index == (a & 0xFFFF))
	{
		tag_instance[a & 0xFFFF].data_offset = tag_instance[b & 0xFFFF].data_offset;
		tag_instance[a & 0xFFFF].type = tag_instance[b & 0xFFFF].type;
	}
}
int query_parser::keyword_check(std::string t)
{
	if (t.find("DWORD") != std::string::npos)
		return 1;
	else if (t.find("module_load") != std::string::npos)
		return 1;
	else if (t.find("_mov") != std::string::npos)
		return 1;
	else if (t.find("replace_tag") != std::string::npos)
		return 1;
	else if (t.find("tag_loadEx") != std::string::npos)
		return 1;
	else if (t.find("tag_load") != std::string::npos)
		return 1;
	return 0;
}
//Simiplify the injection process
void Parse_query_file(std::string loc)
{
	query_parser* my_parser = new query_parser(loc);
	addDebugText(my_parser->_getlogs().c_str());
	delete my_parser;
}