#pragma once
#include "user_interface_widget.h"

class c_group_widget : public c_user_interface_widget
{
public:
	c_group_widget(e_user_interface_widget_type widget_type, int16 user_flags);

	virtual ~c_group_widget();
	/*virtual void render_widget(rectangle2d* viewport_bounds) override;
	virtual int get_intro_delay() override;*/
	virtual c_user_interface_text* get_interface() override;

};
CHECK_STRUCT_SIZE(c_group_widget, 0x70);