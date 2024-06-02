#include "stdafx.h"
#include "levels.h"
#include "level_definitions.h"



s_multiplayer_ui_level_definition* __cdecl levels_get_multiplayer_ui_level(int32 map_id)
{
	return INVOKE(0x5997C, 0x0, levels_get_multiplayer_ui_level, map_id);
}

s_campaign_ui_level_definition*__cdecl levels_get_campaign_ui_level(int campaign_id, int map_id)
{
	return INVOKE(0x5948C, 0x0, levels_get_campaign_ui_level, campaign_id, map_id);
}

bool __cdecl levels_get_custom_map_ui_level_definition(int32 custom_map_id, void* out_level_definition)
{
	return INVOKE(0x59369, 0x0, levels_get_custom_map_ui_level_definition, custom_map_id, out_level_definition);
}

s_campaign_runtime_level_definition* __cdecl levels_get_runtime_campaign_map(int32 campaign_id, int32 map_id)
{
	return INVOKE(0x59526, 0x0, levels_get_runtime_campaign_map, campaign_id, map_id);
}
