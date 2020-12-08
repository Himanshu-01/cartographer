#pragma once
#include"..\Blam\\Cache\Tags\tag_definitons.h"
#include"..\..\TagInterface.h"
#include"..\sound_definitions\sound_gestalt_manager.h"

/*
* -> The second iteration of the tag loader
* -> Loading of tags directly from maps is removed
* -> This version only supports tag loading from tag modules
* -> Current version employs rebase tables methodology to rebase the tags to newer offsets and indices
* -> Working on : sound injection functionality
* -> TODO : update offsets for dedicated server
*/
namespace tag_loader
{
	struct tag_loader_tag_instance
	{
		tags::tag_instance tag_instance;
		std::string debug_tag_name;
		std::string	tag_module_name;

		tag_loader_tag_instance() {};
		tag_loader_tag_instance(const tag_loader_tag_instance& other)
		{
			tag_instance = other.tag_instance;
			debug_tag_name = other.debug_tag_name;
			tag_module_name = other.tag_module_name;
		}
		tag_loader_tag_instance& operator=(const tag_loader_tag_instance& other)
		{
			tag_instance = other.tag_instance;
			debug_tag_name = other.debug_tag_name;
			tag_module_name = other.tag_module_name;
			return *this;
		}
	};
	class tag_module_loader
	{
		/*
		* Internal structure to store information on loaded tag modules and operate accordingly
		*/
		class tag_module_listing_internal
		{

		public:

			std::string module_relative_path;
			HANDLE module_HANDLE;
			int module_target_index;
			int module_end_index;

			///default ctor
			tag_module_listing_internal(std::string module_rel_path, HANDLE module_HANDLE, int module_target_index, int module_end_index)
			{
				this->module_relative_path = module_rel_path;
				this->module_HANDLE = module_HANDLE;
				this->module_target_index = module_target_index;
				this->module_end_index = module_end_index;
			}
			///copy constructor
			tag_module_listing_internal(const tag_module_listing_internal& other)
			{
				module_relative_path = other.module_relative_path;
				module_HANDLE = other.module_HANDLE;
				module_target_index = other.module_target_index;
				module_end_index = other.module_end_index;
			}
			///assignment operator
			tag_module_listing_internal& operator=(const tag_module_listing_internal& other)
			{
				module_relative_path = other.module_relative_path;
				module_HANDLE = other.module_HANDLE;
				module_target_index = other.module_target_index;
				module_end_index = other.module_end_index;
				return *this;
			}
		};

		//array to store info on tag modules
		static std::vector<tag_module_listing_internal> loaded_module_list;
		//array to store info on tags loaded via modules
		static std::vector<tag_loader_tag_instance> loaded_tag_list;
		//contains various messages generated during various processes
		static std::vector<std::string> message_list;
		//default meta size of a map
		static unsigned int default_meta_size;
		//injected meta size
		static unsigned int injected_meta_size;
		//starting index for injected tag
		static unsigned int new_datum_base;
		//pointer to newer allocated tables
		static tags::tag_instance* new_global_tag_tables;
	public:
		//function to initialize tag_loader variables and setup patch calls
		static void Init();
		//function to be called when a newer map gets loaded
		static void Reload();
		//function to be called when a newer allocation for tags is made to setup base for injecting tags
		static void SetDefaultMetaSize(int);
		//loads tag module of given name from the TAGS_RELATIVE_PATH directory,no absoulte path allowed
		static int tag_module_load(std::string tag_module_rel_path, int target_index = -1);
		static void tag_module_get_injected_list(std::vector<std::string> &out);
		static void tag_get_injected_list(std::vector<tag_loader_tag_instance> &out);
		static HANDLE tag_get_module_handle(datum datum_index);
		static void pop_messages(std::string &out);
	private:
		static void tag_load_vftable_reference(datum datum_index);
		static void tag_load_resource_data(datum datum_index);
		static void tag_add_to_sync_list(int type, datum index);
	};
}
