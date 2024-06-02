#pragma once

#include "interface/user_interface_widget_window.h"
#include "interface/user_interface_widget_list.h"

#define k_no_of_visible_items_for_squad_settings 7

class c_squad_settings_list : public c_list_widget
{
protected:
	c_list_item_widget m_list_items[k_no_of_visible_items_for_squad_settings];
	c_slot2<c_squad_settings_list,s_event_record*,long> m_slot;
	bool m_party_mgmt_item_deleted;
	uint8 gap_465[3];

	char handle_item_pressed_event(s_event_record** pevent, long* pitem_index);
	char handle_item_change_map(s_event_record** pevent);
	char handle_item_change_variant(s_event_record** pevent);
	char handle_item_change_level(s_event_record** pevent);
	char handle_item_change_difficulty(s_event_record** pevent);
	char handle_item_quick_options(s_event_record** pevent);
	char handle_item_switch_to_coop(s_event_record** pevent);
	char handle_item_switch_to_arranged(s_event_record** pevent);
	char handle_item_switch_to_optimatch(s_event_record** pevent);
	char handle_item_change_hopper(s_event_record** pevent);
	char handle_item_party_management(s_event_record** pevent);


public:
	c_squad_settings_list(int16 user_flags);
	uint16 get_last_item_type();
	bool party_management_exists();
	void party_management_delete_item();

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
