#include"h2_tag_funcs.h"

//TODO for dedicated server
namespace halo2_map_handle
{
	HANDLE __cdecl get_map_Handle_from_scnr(const char *pScenario)
	{
		return	((HANDLE(__cdecl *)(const char*))h2mod->GetAddress(0x38607))(pScenario);//TODO
	}
}
namespace halo2_tag_vftable_reference_fix
{
	void __cdecl bipd_fix(datum datum_index)
	{
		void(_cdecl*sub_EC23F)(datum);
		sub_EC23F = (void(_cdecl*)(datum))h2mod->GetAddress(0x1389B0);//TODO
		sub_EC23F(datum_index);
	}

	void __cdecl crea_fix(datum datum_index)
	{
		int(_cdecl*sub_EC23F)(datum);
		sub_EC23F = (int(_cdecl*)(datum))h2mod->GetAddress(0x138985);//TODO
		sub_EC23F(datum_index);
	}
	void __cdecl vehi_fix(datum datum_index)
	{
		int(_cdecl*sub_EC23F)(datum);
		sub_EC23F = (int(_cdecl*)(datum))h2mod->GetAddress(0x13895A);//TODO
		sub_EC23F(datum_index);
	}
	void __cdecl coll_fix(datum datum_index)
	{
		int(_cdecl*sub_EC23F)(datum);
		sub_EC23F = (int(_cdecl*)(datum))h2mod->GetAddress(0x7BE5C);//TODO
		sub_EC23F(datum_index);
	}
	void __cdecl phmo_fix(datum datum_index, bool unk)
	{
		int(_cdecl*sub_EC23F)(datum, bool);
		sub_EC23F = (int(_cdecl*)(datum, bool))h2mod->GetAddress(0x7B844);//TODO
		sub_EC23F(datum_index, unk);
	}
}