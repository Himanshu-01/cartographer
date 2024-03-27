#pragma once

#include "Blam/Engine/interface/user_interface_widget_window.h"
#include "Blam/Engine/interface/user_interface_widget_button.h"
#include "Blam/Engine/interface/user_interface_controller.h"

#pragma pack(push , 1)
class c_screen_press_start_introduction : protected c_screen_widget
{
protected:
	c_button_widget m_start_button;
	int32 m_creation_time;
	c_slot2<c_screen_press_start_introduction> m_slot;
	bool m_has_input_saved;
	char gap[3];
	s_event_record m_saved_input;

	
	char handle_item_pressed_event(s_event_record** pevent, short* pitem_index);

public:
	static void* load(s_screen_parameters* parameters);
	c_screen_press_start_introduction(e_user_interface_channel_type channel_type, e_user_interface_render_window window_index, int16 user_flags);

	virtual ~c_screen_press_start_introduction();
	virtual void update() override;
	virtual char handle_event(s_event_record* event) override;
	virtual void pre_initialize(s_screen_parameters* parameters) override;
	virtual void sub_60EBC2(int a1) override;
	virtual void* load_proc() override;
};
#pragma pack(pop)
CHECK_STRUCT_SIZE(c_screen_press_start_introduction, 0xB8C);