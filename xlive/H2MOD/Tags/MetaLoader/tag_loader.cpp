#include"tag_loader.h"
#include"Globals.h"
#include"H2MOD\Modules\OnScreenDebug\OnScreenDebug.h"
#include"query_parser\query_parser.h"
#include"module_loader\tag_module_loader.h"
#include"..\Util\filesys.h"
#include"..\TagInterface.h"
#include"..\global_tags_interface.h"
#include"tag_loader_constants.h"

using Blam::Enums::Tags::TagGroupTypes;

void _test_sub01()
{
	
	///
	//global_tag_interface testing code
	///

	if (tags::get_cache_header()->type != tags::cache_header::scnr_type::MainMenu)
	{
		datum temp(0xE1940018);
		auto test = (Blam::Cache::Tags::itmc*)TagInterface::global_tags_interface::GetTagInterface(temp, (int)TagGroupTypes::itemcollection);

		//resolve ambiguity
		struct Blam::Cache::Tags::itmc::ItemPermutations itemperm;

		itemperm.Weight = 100.0f;

		itemperm.Item.TagGroup = TagGroupTypes::vehicle;
		itemperm.Item.TagIndex = 0xE7A42D3F;
		test->ItemPermutations.PushBack(&itemperm);

		itemperm.Item.TagGroup = TagGroupTypes::biped;
		itemperm.Item.TagIndex = 0xE35028EB;
		test->ItemPermutations.PushBack(&itemperm);


		//test->ItemPermutations.RemoveAt(0);
		test->ItemPermutations[0]->Item.TagGroup = TagGroupTypes::weapon;
		test->ItemPermutations[0]->Item.TagIndex = 0x3BA4;

	}
	
}

//patch call
bool _cdecl patchcall_h2_LoadTagsandMapBases(int a)
{
	// basic load_Tag call
	typedef bool(_cdecl *LoadTagsandSetMapBases)(int a);
	LoadTagsandSetMapBases pLoadTagsandSetMapBases;
	pLoadTagsandSetMapBases = h2mod->GetAddress<LoadTagsandSetMapBases>(0x31348);
	bool result = pLoadTagsandSetMapBases(a);

	//reset tag loader and gestatlt manager upon map load
	tag_loader::tag_module_loader::Reload();
	sound_gestalt_manager::Reload();

	//tag_injector testing
	///temporary untill squirel based scripting is implemented
	///tag_loader::tag_module_loader::tag_module_load("tags.cache");

	std::string game_root_directory(GetExeDirectoryNarrow());
	std::string tag_module_full_path = game_root_directory + TAGS_RELATIVE_PATH + "load_tags.txt";
	Parse_query_file(tag_module_full_path);

	//TODO :: Make Use of TraceFunctions to Log each step
	std::string t;
	tag_loader::tag_module_loader::pop_messages(t);
	addDebugText(t.c_str());

	if (tags::get_cache_header()->type == tags::cache_header::scnr_type::SinglePlayer)
	{
		DWORD has_appended_tables = *(DWORD*)((char*)tags::get_cache_header() + 0x2FC);
		if (has_appended_tables == 1)
		{
			DWORD original_parent_info_count = *(DWORD*)((char*)tags::get_cache_header() + 0x300);
			auto pptag_offset_header = tags::get_tags_header(); //h2mod->GetAddress<tags::tag_offset_header**>(0x47D568);
			pptag_offset_header->tag_parent_info_count = original_parent_info_count;
		}
	}
	return result;
}

void Initialize_tag_loader()
{
	//TODO dedicated server
	PatchCall(h2mod->GetAddress(0x3166B), patchcall_h2_LoadTagsandMapBases);//default maps meta loading
	PatchCall(h2mod->GetAddress(0x315ED), patchcall_h2_LoadTagsandMapBases);//custom maps meta loading,i know i am taking risks	

	tag_loader::tag_module_loader::Init();
	sound_gestalt_manager::Init();
}


