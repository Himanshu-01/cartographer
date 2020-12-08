#pragma once
#include "..\Blam\Cache\Tags\tag_definitons.h"

namespace halo2_map_handle
{
	//contains some game functions that returns HANDLE
	HANDLE __cdecl get_map_Handle_from_scnr(const char *pScenario);
}
namespace halo2_tag_vftable_reference_fix
{
	//certain functions which relate tags to their global declarations based on type(such as Havok objects)(vftables perhaps)
	void __cdecl bipd_fix(datum datum_index);
	void __cdecl crea_fix(datum datum_index);
	void __cdecl vehi_fix(datum datum_index);
	void __cdecl coll_fix(datum datum_index);
	void __cdecl phmo_fix(datum datum_index, bool unk);
}