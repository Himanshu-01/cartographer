#pragma once
#include "user_interface_group_widget.h"

class c_player_widget : protected c_group_widget
{
protected:
	int32 m_screen_player_index;
	void* m_tag_block;

public:
	virtual ~c_player_widget();
	virtual int setup_children() override;
};
CHECK_STRUCT_SIZE(c_player_widget, 0x78);