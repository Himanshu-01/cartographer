#include "stdafx.h"

#include "user_interface_group_widget.h"

c_group_widget::~c_group_widget()
{
	//return INVOKE_TYPE(0x242768, 0x0, c_user_interface_widget*(__thiscall*)(c_user_interface_widget*, char), lpMem,a2);
}
c_user_interface_text* c_group_widget::get_interface()
{
	//return INVOKE_TYPE(0x220050, 0x0, c_user_interface_text*(__thiscall*)(c_small_text_widget*), this);
	return nullptr;
}
