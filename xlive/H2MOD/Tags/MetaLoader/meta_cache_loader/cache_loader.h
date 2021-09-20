#pragma once
#include"..\stdafx.h"
#include<fstream>
#include<string.h>
#include<list>

namespace meta_cache_loader
{
	/*
	* Dont confuse it with blam_cache
	* It loads the cache file which is generate via MapHandler
	* -Refactoring and removing unecessary codes
	* -added version control and removed string based block searching
	*/
	enum class block_type
	{
		_undefined,
		Tag_raw_block,
		Tag_table_block,
		Tag_data_block,
		Tag_rebase_block,
		Tag_index_rebase_block,
		Tag_names_block,
		Module_Internal_name,
	};
	struct cache_headerv1
	{
		int magic;
		int version;
		int raw_table_offset;
		int raw_table_size;
		int tag_table_offset;
		int tag_table_size;
		int tag_data_offset;
		int tag_data_size;
		int rebase_table_offset;
		int rebase_table_size;
		int index_rebase_table_offset;
		int index_rebase_table_size;
		int tag_names_offset;
		int tag_names_size;
		int internal_name_offset;
		int internal_name_size;
		int sound_gestalt_index;
	};
	struct cache_BLOCK
	{
		block_type block_type;
		int size;
		char* data;//actual data if generated at runtime
		//ctor
		cache_BLOCK()
		{
			size = 0;
			data = nullptr;
		}
		//dtor
		~cache_BLOCK()
		{
			if (size > 0)
			{
				if (data != nullptr)
					delete data;
				size = 0;
			}
			block_type = block_type::_undefined;
		}
	};
	class cache_loader
	{
		cache_headerv1 _header;
		//version
		int version_iterator = 0x1003;
		//cache file loc
		std::string cache_file_loc;
		//stream readers
		std::ifstream cache_in;
		//status variable
		bool is_file_opened;
		//block list
		std::vector<cache_BLOCK*> block_list_ptr;
		//message counter
		std::vector<std::string> _messages;
		//actual block reading function based on version
		cache_BLOCK* read_BLOCK(block_type);
	public:
		//returns a pointer as per the supplied block type
		const cache_BLOCK* const get_BLOCK(block_type);
		//function to return messages,error messages
		void pop_messages(std::string &out);
		//is the supported and opened for reading
		bool isopened();

		cache_loader(std::string file_loc);//open a file for IO
		~cache_loader();
	};
}
