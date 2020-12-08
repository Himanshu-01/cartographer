#include"plugin_definitions.h"

namespace meta_struct
{
	///
	//<--------------------------------plugins_field members definition------------------------------------------------>
	///
	plugins_field::plugins_field(std::string name, int off, int entry_size)
	{
		//initialse some stuff
		this->name = name;
		this->offset = off;
		this->entry_size = entry_size;
	}
	int plugins_field::Get_offset()
	{
		return this->offset;
	}
	int plugins_field::Get_entry_size()
	{
		return this->entry_size;
	}
	std::string plugins_field::Get_name()
	{
		return this->name;
	}
	void plugins_field::Add_tag_ref(int off, std::string name)
	{
		Tag_refs.try_emplace(off, name);
	}
	void plugins_field::Add_data_ref(int off, std::string name)
	{
		Data_refs.try_emplace(off, name);
	}
	void plugins_field::Add_BLOCK(std::shared_ptr<plugins_field> field)
	{
		reflexive.push_back(field);
	}
	void plugins_field::Add_stringid_ref(int off, std::string name)
	{
		stringID.try_emplace(off, name);
	}
	void plugins_field::Add_WCtag_ref(int off, std::string name)
	{
		WCTag_refs.try_emplace(off, name);
	}
	std::list<int> plugins_field::Get_tag_ref_list()
	{
		std::list<int> ret;
		for (auto& i : Tag_refs)
		{
			ret.push_back(i.first);
		}
		return ret;
	}
	std::list<int> plugins_field::Get_data_ref_list()
	{
		std::list<int> ret;

		for (auto& i : Data_refs)
		{
			ret.push_back(i.first);
		}
		return ret;
	}
	std::list<int> plugins_field::Get_stringID_ref_list()
	{
		std::list<int> ret;

		for (auto& i : stringID)
		{
			ret.push_back(i.first);
		}
		return ret;
	}
	std::list<int> plugins_field::Get_WCtag_ref_list()
	{
		std::list<int> ret;

		for (auto& i : WCTag_refs)
		{
			ret.push_back(i.first);
		}
		return ret;
	}
	std::list<std::shared_ptr<plugins_field>> plugins_field::Get_reflexive_list()
	{
		return reflexive;
	}
}