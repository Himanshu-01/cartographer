#include "stdafx.h"

#include "user_interface_player_widget.h"


c_player_widget::~c_player_widget()
{
	//return INVOKE_TYPE(0x220475, 0x0, c_user_interface_widget*(__thiscall*)(c_player_widget*, char), lpMem,a2);
}
int c_player_widget::setup_children()
{
	//return INVOKE_TYPE(0x220441, 0x0, int(__thiscall*)(c_player_widget*), this);
	int result = c_user_interface_widget::setup_children();
	this->m_visible = false;
	return result;
}
