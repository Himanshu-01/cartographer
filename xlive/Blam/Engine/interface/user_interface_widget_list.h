#pragma once
#include "user_interface_widget.h"
#include "user_interface_widget_list_item.h"
#include "signal_slot.h"
#include "memory/data.h"

class c_list_widget : public c_user_interface_widget
{
protected:
	s_data_array* m_list_data;
	uint16 m_up_arrow_transition_time;
	uint16 m_down_arrow_transition_time;
	int32 m_intro_delay_milliseconds;
	uint16 m_tabbing_count;
	uint8 gap_7E[2];
	real32 m_up_arrows_position_x0;
	real32 m_up_arrows_position_x1;
	real32 m_up_arrows_position_y0;
	real32 m_up_arrows_position_y1;
	real32 m_down_arrows_position_x0;
	real32 m_down_arrows_position_x1;
	real32 m_down_arrows_position_y0;
	real32 m_down_arrows_position_y1;
	bool m_up_arrow_drawn;
	bool m_down_arrow_drawn;
	bool field_A2;
	bool m_list_wraps;
	bool m_list_interactive;
	bool m_list_has_hidden_items;
	bool field_A6;
	bool field_A7;
	_slot* signal1;
	_slot* signal2;

public:
	c_list_widget(int16 user_flags);

	virtual ~c_list_widget();
	virtual int setup_children() override;
	virtual void on_screen_leave() override;
	virtual void update() override;
	virtual void render_widget(rectangle2d* viewport_bounds) override;
	virtual int get_intro_delay() override;
	virtual char handle_event(s_event_record* event) override;
	virtual void construct_animation_on_region_enter(int a1) override;
	virtual c_user_interface_text* get_interface() override;

	// c_list_widget includes

	virtual int link_item_widgets();
	virtual c_list_item_widget* get_list_items() = 0;
	virtual int get_list_items_count() = 0;
	virtual void update_list_items(c_list_item_widget* item, int skin_index) = 0;
	virtual bool verify_item_in_focus(c_list_item_widget* item);
};
CHECK_STRUCT_SIZE(c_list_widget, 0xB0);