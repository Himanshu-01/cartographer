#include "stdafx.h"
#include "user_interface.h"

float* get_ui_scale_factor()
{
	return Memory::GetAddress<float*>(0xA3E424);
}

void __cdecl screen_error_ok_dialog_show(int32 a1, e_ui_error_types ui_error_index, int32 a3, int16 a4, void* a5, void* a6)
{
	INVOKE(0x20E1D8, 0x0, screen_error_ok_dialog_show, a1, ui_error_index, a3, a4, a5, a6);
	return;
}

bool __cdecl user_interface_channel_is_busy(e_user_interface_channel_type channel_type)
{
	return INVOKE(0x20A540, 0x0, user_interface_channel_is_busy, channel_type);
}

void __cdecl user_interface_error_display_ok_cancle_dialog_with_ok_callback(e_user_interface_channel_type channel_type, e_user_interface_render_window window_index, uint16 user_flags, void* ok_callback_handle, e_ui_error_types error_type)
{
	INVOKE(0x20E3BB, 0x0, user_interface_error_display_ok_cancle_dialog_with_ok_callback, channel_type, window_index, user_flags, ok_callback_handle, error_type);
}

bool __cdecl user_interface_back_out_from_channel_by_id(e_user_interface_channel_type channel_type, e_user_interface_render_window window_index, e_user_interface_screen_id id)
{
	return INVOKE(0x209850, 0x0, user_interface_back_out_from_channel_by_id, channel_type, window_index, id);
}

void __cdecl user_interface_enter_game_shell(int32 context)
{
	INVOKE(0x20CE70, 0x0, user_interface_enter_game_shell, context);
}

