#include"string_functions.h"

std::string to_hex_string(int a)
{
	char temp[15];

	sprintf(temp, "%X", a);

	std::string ret = temp;
	return ret;
}