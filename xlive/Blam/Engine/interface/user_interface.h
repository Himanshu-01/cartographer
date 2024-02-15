#pragma once

enum e_user_interface_widget_type : int32
{
	_widget_type_screen = 0x0,
	_widget_type_list = 0x1,
	_widget_type_list_item = 0x2,
	_widget_type_button = 0x3,
	_widget_type_4 = 0x4,
	_widget_type_table_view = 0x5,
	_widget_type_text = 0x6,
	_widget_type_model = 0x7,
	_widget_type_bitmap = 0x8,
	_widget_type_hud = 0x9,
	_widget_type_player = 0xA
};

enum e_user_interface_channel_type
{
	_user_interface_channel_type_hardware_error = 0x0,
	_user_interface_channel_type_game_error = 0x1,
	_user_interface_channel_type_keyboard = 0x2,
	_user_interface_channel_type_interface = 0x3,
	_user_interface_channel_type_dialog = 0x4,
	_user_interface_channel_type_gameshell = 0x5,
	_user_interface_channel_type_gameshell_background = 0x6,
	k_number_of_user_interface_channels = 0x7
};


enum e_user_interface_render_window
{
	_render_window_0 = 0x0,
	_render_window_1 = 0x1,
	_render_window_2 = 0x2,
	_render_window_3 = 0x3,
	_render_window_4 = 0x4,
	k_number_of_render_windows = 0x5
};


struct s_screen_state
{
	int32 field_0;
	int32 field_4;
	int32 field_8;
};

struct s_screen_parameters
{
	int16 m_flags;
	uint16 user_flags;
	e_user_interface_channel_type m_channel_type;
	e_user_interface_render_window m_window_index;
	int32 field_C;
	s_screen_state m_screen_state;
	void*(__cdecl* m_load_function)(s_screen_parameters*);
};
CHECK_STRUCT_SIZE(s_screen_parameters, 0x20);