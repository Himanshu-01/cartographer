#pragma once
///
//<-----------------The query parser or more of a tag_load script parser------------------------------->
///
class query_parser
{
	std::unordered_map<std::string, int> DWORD_list;//hash table containing the variables allocated in the script   
	//function to parse string to integer
	int try_parse_int(std::string);
	//bunch of log text
	std::vector<std::string> logs;
	//remove comments ,simplify assignment and others
	std::vector<std::string> clean_string(std::string);
public:
	//reference to vector of queries
	query_parser(std::vector<std::string>&);
	//complete file location
	query_parser(std::string file_loc);
	//return all logs
	std::string _getlogs();
private:
	//_mov parser
	void _mov_parser(std::string dest, std::string src);
	//void replace_tag
	void replace_tag(std::string dest, std::string src);
	//void replace tag
	void replace_tag(int dest, int src);
	//check for keywords and standard functions and returns appropriately
	int keyword_check(std::string);
};
//loades and execute instructions in the query file
void Parse_query_file(std::string loc);