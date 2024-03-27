#include "stdafx.h"

#include "user_interface_bitmap_block.h"

void c_bitmap_widget::verify_and_update_bitmap_index(int16 index)
{
	INVOKE_TYPE(0x21C300, 0x0, void(__thiscall*)(c_bitmap_widget*, int16), this, index);
}

c_bitmap_widget::~c_bitmap_widget()
{
}

void c_bitmap_widget::update()
{
	return INVOKE_TYPE(0x21C9AC, 0x0, void(__thiscall*)(c_bitmap_widget*), this);
}

void c_bitmap_widget::render_widget(rectangle2d* viewport_bounds)
{
	return INVOKE_TYPE(0x21D046, 0x0, void(__thiscall*)(void*,rectangle2d*), this, viewport_bounds);
}

int c_bitmap_widget::get_intro_delay()
{
	return INVOKE_TYPE(0x21C2EF, 0x0, int(__thiscall*)(c_bitmap_widget*), this);
}

void* c_bitmap_widget::sub_611703(rectangle2d* unprojected_bounds)
{
	return INVOKE_TYPE(0x21C364, 0x0, void*(__thiscall*)(c_bitmap_widget*, rectangle2d*), this, unprojected_bounds);
}

c_user_interface_text* c_bitmap_widget::get_interface()
{
	//return INVOKE_TYPE(0x21C5DC, 0x0, c_user_interface_text*(__thiscall*)(c_bitmap_widget*), this);
	return nullptr;
}
