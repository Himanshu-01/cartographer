#include"..\stdafx.h"
#include "cache_loader.h"

namespace meta_cache_loader
{
	cache_loader::cache_loader(std::string file_loc) :cache_file_loc(file_loc)
	{
		is_file_opened = false;
		//read stuff
		cache_in.open(file_loc.c_str(), std::ios::binary | std::ios::in | std::ios::beg);
		if (cache_in.is_open())
		{
			//read header
			cache_in.seekg(0x0);
			cache_in.read((char*)&_header, sizeof(_header));

			if (_header.magic == 'modu')
			{
				if (_header.version == version_iterator)
				{
					//version i seek
					is_file_opened = true;
				}
				else
				{
					//unmatched versions
					std::string _error = "meta_cache_loader : Unsupported module version : ";
					_error += file_loc;
					_messages.push_back(_error);

					cache_in.close();
				}
			}
			else
			{
				//invalid file
				std::string _error = "meta_cache_loader : Invalid file : ";
				_error += file_loc;
				_messages.push_back(_error);

				cache_in.close();
			}
		}
		else
		{
			std::string _error = "meta_cache_loader : Unable to locate module : ";
			_error += file_loc;
			_messages.push_back(_error);
		}
	}
	cache_loader::~cache_loader()
	{
		for each (auto i in block_list_ptr)
			delete i;		
		
		//close the input stream 
		cache_in.close();
		block_list_ptr.clear();
	}
	//returns pointer to a BLOCK from the cache file based on the name supplied
	//no need to call delete,its a pointer managed by cache_loader and cleaned up when the loader object gets destroyed
	const cache_BLOCK* const cache_loader::get_BLOCK(block_type type)
	{
		if (!is_file_opened)
			return nullptr;

		for each (auto i in block_list_ptr)
		{
			if (i->block_type == type)
				return i;
		}
		return read_BLOCK(type);
	}
	cache_BLOCK* cache_loader::read_BLOCK(block_type type)
	{
		if (!is_file_opened)
			return nullptr;

		if (type == block_type::_undefined)
		{
			std::string _error = "meta_cache_loader : _undefined block_type,wtf are u passing dude";
			_messages.push_back(_error);

			return nullptr;
		}

		cache_BLOCK* ret = new cache_BLOCK();
		ret->block_type = type;

		int t_offset = -1;
		switch (type)
		{
		case meta_cache_loader::block_type::Tag_raw_block:
			t_offset = _header.raw_table_offset;
			ret->size = _header.raw_table_size;
			break;
		case meta_cache_loader::block_type::Tag_table_block:
			t_offset = _header.tag_table_offset;
			ret->size = _header.tag_table_size;
			break;
		case meta_cache_loader::block_type::Tag_data_block:
			t_offset = _header.tag_data_offset;
			ret->size = _header.tag_data_size;
			break;
		case meta_cache_loader::block_type::Tag_rebase_block:
			t_offset = _header.rebase_table_offset;
			ret->size = _header.rebase_table_size;
			break;
		case meta_cache_loader::block_type::Tag_index_rebase_block:
			t_offset = _header.index_rebase_table_offset;
			ret->size = _header.index_rebase_table_size;
			break;
		case meta_cache_loader::block_type::Tag_names_block:
			t_offset = _header.tag_names_offset;
			ret->size = _header.tag_names_size;
			break;
		case meta_cache_loader::block_type::Module_Internal_name:
			t_offset = _header.internal_name_offset;
			ret->size = _header.internal_name_size;
			break;
		default:
			std::string _error = "meta_cache_loader : Invalid Block_type request : ";
			_messages.push_back(_error);

			return nullptr;
			break;
		}
		ret->data = new char[ret->size];
		cache_in.seekg(t_offset);
		cache_in.read(ret->data, ret->size);

		block_list_ptr.push_back(ret);

		return ret;
	}
	void cache_loader::pop_messages(std::string &out)
	{
		out = "";
		for each (auto i in _messages)
			out += i + "\n";
		_messages.clear();
	}
	bool cache_loader::isopened()
	{
		return is_file_opened;
	}
}