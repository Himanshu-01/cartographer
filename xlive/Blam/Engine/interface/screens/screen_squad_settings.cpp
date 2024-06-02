#include "stdafx.h"
#include "screen_squad_settings.h"
#include "interface/user_interface_controller.h"
#include "interface/user_interface_networking.h"
#include "interface/user_interface_memory.h"
#include "interface/user_interface_bitmap_block.h"
#include "interface/user_interface_globals.h"
#include "bitmaps/bitmaps.h"
#include "game/game.h"
#include "main/levels.h"
#include "main/level_definitions.h"
#include "saved_games/game_variant.h"
#include "tag_files/global_string_ids.h"

enum e_squad_list_items : uint16
{
	_item_change_map = 0,
	_item_change_variant,
	_item_change_level,
	_item_change_difficulty,
	_item_quick_options,
	_item_change_playlist = 5, //unused in h2x
	_item_switch_to_coop,
	_item_switch_to_arranged,
	_item_switch_to_optimatch,
	_item_change_hopper,
	_item_party_management,
	k_total_no_of_squad_list_items = 0xA
};
//v4 = 0;
//v5 = string_id_change_map;
//v6 = 1;
//v7 = string_id_change_variant;
//v8 = 2;
//v9 = string_id_change_level;
//v10 = 3;
//v11 = string_id_change_difficulty;
//v12 = 4;
//v13 = string_id_quick_options;
//v14 = 6;
//v15 = string_id_switch_to_coop;
//v16 = 7;
//v17 = string_id_switch_to_arranged;
//v18 = 8;
//v19 = string_id_switch_to_optimatch;
//v20 = 9;
//v21 = string_id_change_hopper;

enum e_squad_settings_dialog_text_blocks
{
	_squad_settings_dialog_pane_0_text_change_map_help = 0,
	_squad_settings_dialog_pane_0_text_current_map,
	_squad_settings_dialog_pane_0_text_map,
	k_squad_settings_dialog_pane_0_text_count
};

enum e_squad_settings_dialog_bitmap_blocks
{
	_squad_settings_dialog_pane_0_bitmap_fs_dialog = 0,
	_squad_settings_dialog_pane_0_bitmap_1,
	_squad_settings_dialog_pane_0_bitmap_2,
	_squad_settings_dialog_pane_0_bitmap_3,
	_squad_settings_dialog_pane_0_bitmap_xbox_live,
	k_squad_settings_dialog_pane_0_bitmap_count
};

enum e_squad_settings_dialog_local_bitmap_blocks
{
	_squad_settings_dialog_local_bitmap_difficulty_options = 0,
	_squad_settings_dialog_local_bitmap_mp_games,
	k_squad_settings_dialog_local_bitmap_count
};

enum e_local_bitmap_difficulty_options
{
	_difficulty_option_easy = 0,
	_difficulty_option_normal,
	_difficulty_option_heroic,
	_difficulty_option_legendary
};

enum e_local_bitmap_mp_games
{
	_mp_game_type_slayer = 0,
	_mp_game_type_koth,
	_mp_game_type_2,
	_mp_game_type_oddball,
	_mp_game_type_juggernaut,
	_mp_game_type_5,
	_mp_game_type_ctf,
	_mp_game_type_assualt,
	_mp_game_type_territories,
};



c_squad_settings_list::c_squad_settings_list(int16 user_flags) :
	c_list_widget(user_flags),
	m_slot(this, &c_squad_settings_list::handle_item_pressed_event)
{
	this->m_list_data = ui_list_data_new("squad setting list", k_total_no_of_squad_list_items, sizeof(s_dynamic_list_item));
	s_data_array::data_make_valid(this->m_list_data);
	this->m_party_mgmt_item_deleted = 1;



	// yes this sucks
#define SQUAD_ITEM_GET_NEW() \
		static_cast<s_dynamic_list_item*>(datum_get(this->m_list_data, s_data_array::datum_new_in_range(this->m_list_data)))

	const e_session_protocol active_protocol = user_interface_squad_get_active_protocol();
	switch (active_protocol)
	{
	case _protocol_splitscreen_custom:
	case _protocol_system_link_custom:
		(SQUAD_ITEM_GET_NEW())->item_id = _item_change_map;
		(SQUAD_ITEM_GET_NEW())->item_id = _item_change_variant;
		(SQUAD_ITEM_GET_NEW())->item_id = _item_quick_options;
		(SQUAD_ITEM_GET_NEW())->item_id = _item_switch_to_coop;
		break;

	case _protocol_live_custom:
		(SQUAD_ITEM_GET_NEW())->item_id = _item_change_map;
		(SQUAD_ITEM_GET_NEW())->item_id = _item_change_variant;
		(SQUAD_ITEM_GET_NEW())->item_id = _item_quick_options;
		(SQUAD_ITEM_GET_NEW())->item_id = _item_switch_to_coop;
		(SQUAD_ITEM_GET_NEW())->item_id = _item_switch_to_optimatch;
		if (user_interface_squad_local_peer_is_leader() && user_interface_squad_get_player_count() > 1)
		{
			(SQUAD_ITEM_GET_NEW())->item_id = _item_party_management;
			this->m_party_mgmt_item_deleted = 0;
		}
		else
		{
			this->m_party_mgmt_item_deleted = 1;
		}
		break;


	case _protocol_splitscreen_coop:
	case _protocol_system_link_coop:
		(SQUAD_ITEM_GET_NEW())->item_id = _item_change_level;
		(SQUAD_ITEM_GET_NEW())->item_id = _item_change_difficulty;
		(SQUAD_ITEM_GET_NEW())->item_id = _item_switch_to_arranged;
		break;
	case _protocol_live_coop:
		(SQUAD_ITEM_GET_NEW())->item_id = _item_change_level;
		(SQUAD_ITEM_GET_NEW())->item_id = _item_change_difficulty;
		(SQUAD_ITEM_GET_NEW())->item_id = _item_switch_to_arranged;
		(SQUAD_ITEM_GET_NEW())->item_id = _item_switch_to_optimatch;
		break;
	case _protocol_live_optimatch:
		(SQUAD_ITEM_GET_NEW())->item_id = _item_change_hopper;
		(SQUAD_ITEM_GET_NEW())->item_id = _item_switch_to_coop;
		(SQUAD_ITEM_GET_NEW())->item_id = _item_switch_to_arranged;
		break;
	default:
		break;

	}

	// create all list items at once
	//(SQUAD_ITEM_GET_NEW())->item_id = _item_change_map;
	//(SQUAD_ITEM_GET_NEW())->item_id = _item_change_variant;
	//(SQUAD_ITEM_GET_NEW())->item_id = _item_change_level;
	//(SQUAD_ITEM_GET_NEW())->item_id = _item_change_difficulty;
	//(SQUAD_ITEM_GET_NEW())->item_id = _item_quick_options;
	//(SQUAD_ITEM_GET_NEW())->item_id = _item_switch_to_coop;
	//(SQUAD_ITEM_GET_NEW())->item_id = _item_switch_to_arranged;
	//(SQUAD_ITEM_GET_NEW())->item_id = _item_switch_to_optimatch;
	//(SQUAD_ITEM_GET_NEW())->item_id = _item_change_hopper;
	//(SQUAD_ITEM_GET_NEW())->item_id = _item_party_management;




	// PLS FIX THIS SHIT thank you
	//this->signal2->link_list_signal(&this->m_slot);
	////this->m_slot.link_list_signal((_slot*) &this->signal2, &this->m_slot);
	//static_cast<_slot2<>*>(&this->signal2)
	this->signal2->link_signal_to_slot((_slot*)&this->signal2, &this->m_slot);

#undef SQUAD_ITEM_GET_NEW

}

uint16 c_squad_settings_list::get_last_item_type()
{
	datum item_idx = this->get_old_data_index();
	s_dynamic_list_item* item_datum = (s_dynamic_list_item*)datum_try_and_get(this->m_list_data, item_idx);
	if (item_datum)
		return item_datum->item_id;
	return NONE;
}

bool c_squad_settings_list::party_management_exists()
{
	return !this->m_party_mgmt_item_deleted;
}

void c_squad_settings_list::party_management_delete_item()
{
	c_list_item_widget* item = this->try_find_item_widget(_item_party_management);
	this->remove_focused_item_datum_from_data_array();
	datum item_index_first = 0;
	this->set_focused_item_index(item_index_first);
	this->remove_item_from_list(item);
	this->m_party_mgmt_item_deleted = 1;
}

c_squad_settings_list::~c_squad_settings_list()
{
	//return INVOKE_TYPE(0x24FD05, 0x0, c_squad_settings_list(*__thiscall*)(c_squad_settings_list*, char), lpMem,a2);
}

char c_squad_settings_list::handle_event(s_event_record* event)
{
	return INVOKE_TYPE(0x24F979, 0x0, char(__thiscall*)(c_squad_settings_list*, s_event_record*), this, event);
}

c_list_item_widget* c_squad_settings_list::get_list_items()
{
	//return INVOKE_TYPE(0x24FCF8, 0x0, c_list_item_widget*(__thiscall*)(c_squad_settings_list*), this);
	return this->m_list_items;
}

int c_squad_settings_list::get_list_items_count()
{
	//return INVOKE_TYPE(0x24FCFF, 0x0, char(__thiscall*)(c_squad_settings_list*), this);
	return k_no_of_visible_items_for_squad_settings;
}

void c_squad_settings_list::update_list_items(c_list_item_widget* item, int skin_index)
{
	//INVOKE_TYPE(0x24EEEF, 0x0, void(__thiscall*)(c_squad_settings_list*, c_list_item_widget*, int), this, item, skin_index);

	s_item_text_mapping items_map[k_total_no_of_squad_list_items] =
	{
		{_item_change_map			, HS_CHANGE_MAP				},
		{_item_change_variant		, HS_CHANGE_VARIANT			},
		{_item_change_level			, HS_CHANGE_LEVEL			},
		{_item_change_difficulty	, HS_CHANGE_DIFFICULTY		},
		{_item_quick_options		, HS_QUICK_OPTIONS			},
		{_item_switch_to_coop		, HS_SWITCH_TO_COOP			},
		{_item_switch_to_arranged	, HS_SWITCH_TO_ARRANGED		},
		{_item_switch_to_optimatch 	, HS_SWITCH_TO_OPTIMATCH	},
		{_item_change_hopper		, HS_CHANGE_HOPPER			},
		{_item_party_management		, HS_PARTY_MANAGEMENT		}
	};

	this->update_list_items_from_mapping(item, skin_index, 0, items_map, k_total_no_of_squad_list_items);

}

char c_squad_settings_list::handle_item_pressed_event(s_event_record** pevent, long* pitem_index)
{
	//LOG_INFO_FUNC("dont tickle me : controller {}  datum {} ", (*pevent)->controller, *pitem_index);
	//return INVOKE_TYPE(0x24FA19, 0x0, char(__thiscall*)(c_squad_settings_list*, s_event_record**, long*), this, pevent, pitem_index);

	if (!DATUM_IS_NONE(*pitem_index))
	{
		s_dynamic_list_item* item = (s_dynamic_list_item*)datum_try_and_get(this->m_list_data, *pitem_index);
		e_squad_list_items item_type = (e_squad_list_items)item->item_id;

		switch (item_type)
		{
		case _item_change_map:
			this->handle_item_change_map(pevent);
			break;
		case _item_change_variant:
			this->handle_item_change_variant(pevent);
			break;
		case _item_change_level:
			this->handle_item_change_level(pevent);
			break;
		case _item_change_difficulty:
			this->handle_item_change_difficulty(pevent);
			break;
		case _item_quick_options:
			this->handle_item_quick_options(pevent);
			break;
		case _item_switch_to_coop:
			this->handle_item_switch_to_coop(pevent);
			break;
		case _item_switch_to_arranged:
			this->handle_item_switch_to_arranged(pevent);
			break;
		case _item_switch_to_optimatch:
			this->handle_item_switch_to_optimatch(pevent);
			break;
		case _item_change_hopper:
			this->handle_item_change_hopper(pevent);
			break;
		case _item_party_management:
			this->handle_item_party_management(pevent);
			break;

		}
	}
	return true;
}

char c_squad_settings_list::handle_item_change_map(s_event_record** pevent)
{
	return INVOKE_TYPE(0x24F9A1, 0x0, char(__thiscall*)(c_squad_settings_list*, s_event_record**), this, pevent);
}
char c_squad_settings_list::handle_item_change_variant(s_event_record** pevent)
{
	return INVOKE_TYPE(0x24F9DD, 0x0, char(__thiscall*)(c_squad_settings_list*, s_event_record**), this, pevent);
}
char c_squad_settings_list::handle_item_change_level(s_event_record** pevent)
{
	s_screen_parameters params;
	params.m_flags = 0;
	params.m_window_index = _window_4;
	params.field_C = 0;
	params.user_flags = FLAG((*pevent)->controller);
	params.m_channel_type = _user_interface_channel_type_interface;
	params.m_screen_state.field_0 = 0xFFFFFFFF;
	params.m_screen_state.field_4 = 0xFFFFFFFF;
	params.m_screen_state.field_8 = 0xFFFFFFFF;
	params.m_load_function = c_screen_single_player_level_select_load_lobby;
	c_screen_single_player_level_select_load_lobby(&params);

	return 1;
}
char c_squad_settings_list::handle_item_change_difficulty(s_event_record** pevent)
{
	s_screen_parameters params;
	params.m_flags = 0;
	params.m_window_index = _window_4;
	params.field_C = 0;
	params.user_flags = FLAG((*pevent)->controller);
	params.m_channel_type = _user_interface_channel_type_interface;
	params.m_screen_state.field_0 = 0xFFFFFFFF;
	params.m_screen_state.field_4 = 0xFFFFFFFF;
	params.m_screen_state.field_8 = 0xFFFFFFFF;
	params.m_load_function = c_screen_single_player_difficulty_select_load_lobby;
	c_screen_single_player_difficulty_select_load_lobby(&params);

	return 1;
}
char c_squad_settings_list::handle_item_quick_options(s_event_record** pevent)
{
	return INVOKE_TYPE(0x24EF79, 0x0, char(__thiscall*)(c_squad_settings_list*, s_event_record**), this, pevent);
}
char c_squad_settings_list::handle_item_switch_to_coop(s_event_record** pevent)
{
	if (game_is_beta_build())
	{
		screen_error_ok_dialog_show(_user_interface_channel_type_game_error, _ui_error_beta_feature_disabled, _window_4, FLAG((*pevent)->controller), 0, 0);
		return 1;
	}

	user_interface_squad_clear_game_settings();
	user_interface_set_desired_multiplayer_mode(0);
	int32 difficulty = user_interface_globals_get_game_difficulty();
	user_interface_globals_set_game_difficulty_real(difficulty);

	user_interface_globals_campaign_unk(false);
	user_interface_squad_set_campaign_difficulty(difficulty);

	this->get_parent_screen()->animate_widget(3);

	return 1;
}
char c_squad_settings_list::handle_item_switch_to_arranged(s_event_record** pevent)
{
	return INVOKE_TYPE(0x24F015, 0x0, char(__thiscall*)(c_squad_settings_list*, s_event_record**), this, pevent);
}
char c_squad_settings_list::handle_item_switch_to_optimatch(s_event_record** pevent)
{
	// maybe someday
	//return INVOKE_TYPE(0x211BA1, 0x0, char(__thiscall*)(c_squad_settings_list*, s_event_record**), this, pevent);
	return 1;
}
char c_squad_settings_list::handle_item_change_hopper(s_event_record** pevent)
{
	//return INVOKE_TYPE(0x24F68A, 0x0, char(__thiscall*)(c_squad_settings_list*, s_event_record**), this, pevent);
	return 1;
}
char c_squad_settings_list::handle_item_party_management(s_event_record** pevent)
{
	// TODO : figure out why is this broken or invoke a custom menu to handle this
	return INVOKE_TYPE(0x24F5FD, 0x0, char(__thiscall*)(c_squad_settings_list*, s_event_record**), this, pevent);
}


//
//
//
// c_screen_squad_settings class functions begin here
// 
// 
//


c_screen_squad_settings::c_screen_squad_settings(e_user_interface_channel_type channel_type, e_user_interface_render_window window_index, int16 user_flags) :
	c_screen_widget(_screen_squad_settings, channel_type, window_index, user_flags),
	m_squad_settings_list(user_flags)
{
	LOG_INFO_FUNC("starting ctor");
}

c_screen_squad_settings::~c_screen_squad_settings()
{
}

void c_screen_squad_settings::update()
{
	//INVOKE_TYPE(0x24F0EB, 0x0, void(__thiscall*)(c_screen_squad_settings*), this);

	e_squad_list_items item_type = (e_squad_list_items)this->m_squad_settings_list.get_last_item_type();
	c_text_widget* option_help_text_block = (c_text_widget*)this->try_find_text_widget(TEXT_BLOCK_INDEX_TO_WIDGET_INDEX(_squad_settings_dialog_pane_0_text_change_map_help));
	c_text_widget* option_header_text_block = (c_text_widget*)this->try_find_text_widget(TEXT_BLOCK_INDEX_TO_WIDGET_INDEX(_squad_settings_dialog_pane_0_text_current_map));
	c_text_widget* option_value_text_block = (c_text_widget*)this->try_find_text_widget(TEXT_BLOCK_INDEX_TO_WIDGET_INDEX(_squad_settings_dialog_pane_0_text_map));
	c_bitmap_widget* option_bitmap = (c_bitmap_widget*)this->try_find_bitmap_widget(_squad_settings_dialog_pane_0_bitmap_xbox_live);

	if (option_bitmap)
		option_bitmap->assign_new_bitmap_block(nullptr);

	uint32 campaign_id, map_id, custom_map_id;
	bool map_is_set = user_interface_session_get_map(&campaign_id, &map_id, &custom_map_id);

	string_id help_string = HS_EMPTY_STRING;
	string_id header_string = HS_EMPTY_STRING;
	string_id value_string = HS_EMPTY_STRING;
	uint32 bitm_index = 0;

	s_game_variant* game_variant = user_interface_session_get_game_variant();

	switch (item_type)
	{
	case _item_change_map:
		help_string = HS_CHANGE_MAP_HELP;
		header_string = HS_CURRENT_MAP;
		value_string = HS_MAP;

		if (map_is_set && option_bitmap)
		{
			if (custom_map_id)
			{
				// TODO 
				//	if (levels_get_custom_map_ui_level_definition(custom_map_id, (int)&block))
				//		option_bitmap->assign_new_bitmap_block(block);
			}
			else
			{
				s_multiplayer_ui_level_definition* level_definition = levels_get_multiplayer_ui_level(map_id);
				if (level_definition)
				{
					bitmap_data* bitmap_block = bitmap_tag_get_bitmaps_block(level_definition->bitmap.TagIndex, 0);
					option_bitmap->assign_new_bitmap_block(bitmap_block);
				}
			}
		}
		break;
	case _item_change_variant:
		help_string = HS_CHANGE_VARIANT_HELP;
		header_string = HS_CURRENT_VARIANT;
		value_string = HS_VARIANT;


		if (option_bitmap && game_variant && game_variant->variant_game_engine_index)
		{
			switch (game_variant->variant_game_engine_index)
			{
			case _game_engine_ctf:
				option_bitmap->set_local_bitmap(_squad_settings_dialog_local_bitmap_mp_games, _mp_game_type_ctf);
				break;
			case _game_engine_slayer:
				option_bitmap->set_local_bitmap(_squad_settings_dialog_local_bitmap_mp_games, _mp_game_type_slayer);
				break;
			case _game_engine_oddball:
				option_bitmap->set_local_bitmap(_squad_settings_dialog_local_bitmap_mp_games, _mp_game_type_oddball);
				break;
			case _game_engine_koth:
				option_bitmap->set_local_bitmap(_squad_settings_dialog_local_bitmap_mp_games, _mp_game_type_koth);
				break;
			case _game_engine_juggernaut:
				option_bitmap->set_local_bitmap(_squad_settings_dialog_local_bitmap_mp_games, _mp_game_type_juggernaut);
				break;
			case _game_engine_territories:
				option_bitmap->set_local_bitmap(_squad_settings_dialog_local_bitmap_mp_games, _mp_game_type_territories);
				break;
			case _game_engine_assault:
				option_bitmap->set_local_bitmap(_squad_settings_dialog_local_bitmap_mp_games, _mp_game_type_assualt);
				break;
			}
		}

		break;
	case _item_change_level:
		help_string = HS_CHANGE_LEVEL_HELP;
		header_string = HS_CURRENT_LEVEL;
		value_string = HS_LEVEL;

		if (map_is_set && option_bitmap)
		{
			s_campaign_ui_level_definition* level_definition = levels_get_campaign_ui_level(campaign_id, map_id);
			if (level_definition)
			{
				bitmap_data* bitmap_block = bitmap_tag_get_bitmaps_block(level_definition->bitmap.TagIndex, 0);
				option_bitmap->assign_new_bitmap_block(bitmap_block);
			}
		}

		break;
	case _item_change_difficulty:
		help_string = HS_CHANGE_DIFFICULTY_HELP;
		header_string = HS_CURRENT_DIFFICULTY;
		value_string = HS_DIFFICULTY;

		if (option_bitmap)
		{
			switch (user_interface_session_get_campaign_difficulty())
			{
			case _difficulty_option_easy:
				option_bitmap->set_local_bitmap(_squad_settings_dialog_local_bitmap_difficulty_options, _difficulty_option_easy);
				break;
			case _difficulty_option_normal:
				option_bitmap->set_local_bitmap(_squad_settings_dialog_local_bitmap_difficulty_options, _difficulty_option_normal);
				break;
			case _difficulty_option_heroic:
				option_bitmap->set_local_bitmap(_squad_settings_dialog_local_bitmap_difficulty_options, _difficulty_option_heroic);
				break;
			case _difficulty_option_legendary:
				option_bitmap->set_local_bitmap(_squad_settings_dialog_local_bitmap_difficulty_options, _difficulty_option_legendary);
				break;
			}
		}


		break;
	case _item_quick_options:
		help_string = HS_QUICK_OPTIONS_HELP;
		header_string = HS_CURRENT_VARIANT;
		value_string = HS_VARIANT;
		bitm_index = 2;
		break;
	case _item_switch_to_coop:
		help_string = HS_SWITCH_TO_COOP_HELP;
		header_string = HS_SWITCH_TO_COOP;
		value_string = HS_EMPTY_STRING;
		bitm_index = 7;
		break;
	case _item_switch_to_arranged:
		help_string = HS_SWITCH_TO_CUSTOM_GAME_HELP;
		header_string = HS_SWITCH_TO_ARRANGED;
		value_string = HS_EMPTY_STRING;
		bitm_index = 6;
		break;
	case _item_switch_to_optimatch:
		help_string = HS_SWITCH_TO_OPTIMATCH_HELP;
		header_string = HS_SWITCH_TO_OPTIMATCH;
		value_string = HS_EMPTY_STRING;
		bitm_index = 0;
		break;
	case _item_change_hopper:
		help_string = HS_CHANGE_HOPPER_HELP;
		header_string = HS_CURRENT_HOPPER;
		value_string = HS_HOPPER;
		bitm_index = 4;
		break;
	case _item_party_management:
		help_string = HS_PARTY_MANAGEMENT_HELP;
		header_string = HS_PARTY_MANAGEMENT_HEADER;
		value_string = HS_PLAYER_OPTIONS;
		bitm_index = 1;
		break;
	default:
		help_string = HS_EMPTY_STRING;
		header_string = HS_EMPTY_STRING;
		value_string = HS_EMPTY_STRING;
		bitm_index = 0;

	}

	if (option_help_text_block)
		option_help_text_block->set_screen_string(help_string);
	if (option_header_text_block)
		option_header_text_block->set_screen_string(header_string);
	if (option_value_text_block)
		option_value_text_block->set_screen_string(value_string);
	if (option_bitmap)
		option_bitmap->verify_and_update_bitmap_index(bitm_index);

	c_list_item_widget* item = this->m_squad_settings_list.try_find_item_widget(_item_party_management);
	if (item && !this->m_squad_settings_list.party_management_exists() && user_interface_squad_get_player_count() < 2)
	{
		this->m_squad_settings_list.party_management_delete_item();
	}
	c_user_interface_widget::update();

}

char c_screen_squad_settings::handle_event(s_event_record* event)
{
	//LOG_INFO_FUNC("do i have a role here? : controller {}  ", event->controller);
	return INVOKE_TYPE(0x24FE49, 0x0, char(__thiscall*)(c_screen_widget*, s_event_record*), this, event);
}

void c_screen_squad_settings::pre_initialize(s_screen_parameters* parameters)
{
	INVOKE_TYPE(0x24F043, 0x0, void(__thiscall*)(c_screen_squad_settings*, s_screen_parameters*), this, parameters);
}

void* c_screen_squad_settings::load(s_screen_parameters* parameters)
{
	//return INVOKE(0x24FE89, 0x0, c_screen_squad_settings::load, parameters);

	c_screen_squad_settings* screen;

	//parameters->m_flags |= 4u;
	void* pool = ui_pool_allocate_space(sizeof(c_screen_squad_settings), 0);
	if (pool)
	{
		screen = new (pool) c_screen_squad_settings(
			parameters->m_channel_type,
			parameters->m_window_index,
			parameters->user_flags);

		screen->m_allocated = true;
		user_interface_register_screen_to_channel(screen, parameters);
	}
	else
	{
		screen = 0;
	}

	return screen;
}


void* c_screen_squad_settings::load_proc()
{
	return &c_screen_squad_settings::load;
}

void c_screen_squad_settings::apply_patches()
{
	//Replace orignal call with custom one
	WriteValue(Memory::GetAddress(0x246356) + 1, c_screen_squad_settings::load);
}
