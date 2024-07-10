#pragma once

#include "interface/user_interface_widget_window.h"

class c_screen_loading_progress : protected c_screen_widget
{
protected:
	uint32 fieldA5C;

	void update_hook();
public:
	static void apply_patches();
	static void apply_tag_patches();
};
CHECK_STRUCT_SIZE(c_screen_loading_progress, 0xA60);