#pragma once

#include "interface/user_interface_widget_window.h"
#include "interface/user_interface_widget_list.h"

#define k_no_of_visible_items_for_squad_settings 7

class c_squad_settings_list : protected c_list_widget
{
protected:
	c_list_item_widget m_list_items[k_no_of_visible_items_for_squad_settings];
	c_slot2<c_squad_settings_list,s_event_record*,long> m_slot;
	bool field_464;
	uint8 gap_465[3];

	char handle_item_pressed_event(s_event_record** pevent, long* pitem_index);

public:
	c_squad_settings_list(int16 user_flags);

	virtual ~c_squad_settings_list();
	char handle_event(s_event_record* event) override;
	virtual c_list_item_widget* get_list_items() override;
	virtual int get_list_items_count() override;
	virtual void update_list_items(c_list_item_widget* item, int skin_index) override;

};
CHECK_STRUCT_SIZE(c_squad_settings_list, 0x468);


class c_screen_squad_settings : protected c_screen_widget
{
protected:
	c_squad_settings_list m_squad_settings_list;
public:
	static void apply_patches();
	static void* load(s_screen_parameters* parameters);
	c_screen_squad_settings(e_user_interface_channel_type channel_type, e_user_interface_render_window window_index, int16 user_flags);

public:
	virtual ~c_screen_squad_settings();
	virtual void update() override;
	virtual char handle_event(s_event_record* event) override;
	virtual void pre_initialize(s_screen_parameters* parameters) override;
	virtual void* load_proc() override;
};
CHECK_STRUCT_SIZE(c_screen_squad_settings, 0xEC4);
