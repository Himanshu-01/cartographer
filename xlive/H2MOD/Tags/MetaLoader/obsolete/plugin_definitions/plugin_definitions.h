#pragma once

namespace meta_struct
{
	/// <summary>
	/// A class representing the structure halo2 plugins
	/// </summary>
	class plugins_field
	{
		std::string name;
		int offset;
		int entry_size;

		std::map<int, std::string> Tag_refs;//<offset,name>
		std::map<int, std::string> Data_refs;//<offset,name>
		std::map<int, std::string> stringID;//<offset,name>
		std::map<int, std::string> WCTag_refs;//<offset,name>(withClass tagRefs,somewhat different from the normal tags)

		std::list<std::shared_ptr<plugins_field>> reflexive;

	public:
		//constructor
		plugins_field(std::string name, int off, int entry_size);

		int Get_offset();
		int Get_entry_size();
		std::string Get_name();

		void Add_tag_ref(int off, std::string name);
		void Add_data_ref(int off, std::string name);
		void Add_BLOCK(std::shared_ptr<plugins_field> field);
		void Add_stringid_ref(int off, std::string name);
		void Add_WCtag_ref(int off, std::string name);
		std::list<int> Get_tag_ref_list();
		std::list<int> Get_data_ref_list();
		std::list<int> Get_stringID_ref_list();
		std::list<int> Get_WCtag_ref_list();
		std::list<std::shared_ptr<plugins_field>> Get_reflexive_list();
	};
}