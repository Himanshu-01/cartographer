#include "stdafx.h"
#include "screen_squad_settings.h"
#include "interface/user_interface_controller.h"
#include "interface/user_interface_networking.h"
#include "interface/user_interface_memory.h"

enum e_squad_list_items : uint16
{
	_item_change_map = 0,
	_item_change_variant,
	_item_quick_options,
	_item_party_management,
	_item_type_4,
	_item_switch_to_arranged,
	_item_switch_to_optimatch,
	_item_change_hopper,
	k_total_no_of_squad_list_items
};
//v4 = 0;
//v6 = 1;
//v8 = 2;
//v10 = 3;
//v12 = 5;
//v14 = 6;
//v16 = 7;
//v5 = HS_CHANGE_MAP;
//v7 = HS_CHANGE_VARIANT;
//v9 = HS_QUICK_OPTIONS;
//v11 = HS_PARTY_MANAGEMENT;
//v13 = HS_SWITCH_TO_ARRANGED;
//v15 = HS_SWITCH_TO_OPTIMATCH;
//v17 = HS_CHANGE_HOPPER;

struct s_squad_list_item
{
	uint16 unk;
	e_squad_list_items item_type;
};
CHECK_STRUCT_SIZE(struct s_squad_list_item, 4);


c_squad_settings_list::c_squad_settings_list(int16 user_flags) :
	c_list_widget(user_flags),
	m_slot(this, &c_squad_settings_list::handle_item_pressed_event)
{
	this->m_list_data = ui_list_data_new("squad setting list", k_total_no_of_squad_list_items, sizeof(s_squad_list_item));
	s_data_array::data_make_valid(this->m_list_data);
	this->field_464 = 1;



		// yes this sucks
#define SQUAD_ITEM_GET_NEW() \
		static_cast<s_squad_list_item*>(datum_get(this->m_list_data, s_data_array::datum_new_in_range(this->m_list_data)))

	const e_session_protocol active_protocol = user_interface_squad_get_active_protocol();
	switch (active_protocol)
	{
	case _protocol_splitscreen_custom:
	case _protocol_system_link_custom:
		(SQUAD_ITEM_GET_NEW())->item_type = _item_change_map;
		(SQUAD_ITEM_GET_NEW())->item_type = _item_change_variant;
		(SQUAD_ITEM_GET_NEW())->item_type = _item_quick_options;
		break;

	case _protocol_live_custom:
		(SQUAD_ITEM_GET_NEW())->item_type = _item_change_map;
		(SQUAD_ITEM_GET_NEW())->item_type = _item_change_variant;
		(SQUAD_ITEM_GET_NEW())->item_type = _item_quick_options;
		if (user_interface_squad_local_peer_is_leader() && user_interface_squad_get_player_count() > 1)
		{
			(SQUAD_ITEM_GET_NEW())->item_type = _item_party_management;
			this->field_464 = 0;
		}
		else
		{
			this->field_464 = 1;
		}
		break;

	case _protocol_live_optimatch:
		(SQUAD_ITEM_GET_NEW())->item_type = _item_change_hopper;
		(SQUAD_ITEM_GET_NEW())->item_type = _item_switch_to_arranged;
		break;
	default:
		break;

	}

	// create all list items at once
			//	(SQUAD_ITEM_GET_NEW())->item_type = _item_change_map;
			//	(SQUAD_ITEM_GET_NEW())->item_type = _item_change_variant;
			//	(SQUAD_ITEM_GET_NEW())->item_type = _item_quick_options;
			//	(SQUAD_ITEM_GET_NEW())->item_type = _item_party_management;
			//	//(SQUAD_ITEM_GET_NEW())->item_type = _item_type_4;
			//	(SQUAD_ITEM_GET_NEW())->item_type = _item_switch_to_arranged;
			//	(SQUAD_ITEM_GET_NEW())->item_type = _item_switch_to_optimatch ;
			//	(SQUAD_ITEM_GET_NEW())->item_type = _item_change_hopper;



	// PLS FIX THIS SHIT thank you
	//this->signal2->link_list_signal(&this->m_slot);
	////this->m_slot.link_list_signal((_slot*) &this->signal2, &this->m_slot);
	//static_cast<_slot2<>*>(&this->signal2)
	this->signal2->link_signal_to_slot((_slot*)&this->signal2, &this->m_slot);

#undef SQUAD_ITEM_GET_NEW

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
	INVOKE_TYPE(0x24EEEF, 0x0, void(__thiscall*)(c_squad_settings_list*, c_list_item_widget*, int), this, item, skin_index);
}

char c_squad_settings_list::handle_item_pressed_event(s_event_record** pevent, long* pitem_index)
{
	LOG_INFO_FUNC("dont tickle me : controller {}  datum {} ", (*pevent)->controller, *pitem_index);
	return INVOKE_TYPE(0x24FA19, 0x0, char(__thiscall*)(c_squad_settings_list*, s_event_record**, long*), this, pevent, pitem_index);
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
	INVOKE_TYPE(0x24F0EB, 0x0, void(__thiscall*)(c_screen_squad_settings*), this);
}

char c_screen_squad_settings::handle_event(s_event_record* event)
{
	LOG_INFO_FUNC("do i have a role here? : controller {}  ", event->controller);
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
