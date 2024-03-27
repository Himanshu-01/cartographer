#pragma once

#include "Blam/Engine/interface/user_interface_widget_window.h"

#pragma pack(push , 1)
class c_screen_esrb_warning : protected c_screen_widget
{
protected:
	int32 m_creation_time;
	int32 m_elapsed_time;
public:
	static void apply_patches();
	static void* load(s_screen_parameters* parameters);
	c_screen_esrb_warning(e_user_interface_channel_type channel_type, e_user_interface_render_window window_index, int16 user_flags);

public:
	virtual ~c_screen_esrb_warning();
	virtual void update() override;
	virtual char handle_event(s_event_record* event) override;
	virtual void pre_initialize(s_screen_parameters* parameters) override;
	virtual void* load_proc() override;
};
#pragma pack(pop)
CHECK_STRUCT_SIZE(c_screen_esrb_warning, 0xA64);