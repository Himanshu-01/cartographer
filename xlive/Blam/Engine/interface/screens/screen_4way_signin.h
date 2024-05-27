#pragma once

#include "Blam/Engine/interface/user_interface_widget_window.h"

enum e_4_way_signin_types
{
	_4_way_signin_type_campaign = 0x0,
	_4_way_signin_type_splitscreen = 0x1,
	_4_way_signin_type_system_link = 0x2,
	_4_way_signin_type_xbox_live = 0x3,
	_4_way_signin_type_crossgame_invite = 0x4,
};


#pragma pack(push , 1)
class c_screen_4way_signin : protected c_screen_widget
{
protected:
	e_4_way_signin_types m_call_context;

	char handle_main_events(s_event_record* event);
	char handle_default_events(s_event_record* event);
public:

	static void* load(s_screen_parameters* parameters);
	static void* load_type4(s_screen_parameters* parameters);
	static void* load_type3(s_screen_parameters* parameters);
	static void* load_type2(s_screen_parameters* parameters);
	static void* load_type1(s_screen_parameters* parameters);
	static void* load_type0(s_screen_parameters* parameters);
	static void apply_patches();
	c_screen_4way_signin(e_user_interface_channel_type channel_type, e_user_interface_render_window window_index, int16 user_flags);

	virtual ~c_screen_4way_signin();
	virtual void update() override;
	virtual char handle_event(s_event_record* event) override;
	virtual void pre_initialize(s_screen_parameters* parameters) override;
	virtual void* load_proc() override;
};
#pragma pack(pop)
CHECK_STRUCT_SIZE(c_screen_4way_signin, 0xA60);