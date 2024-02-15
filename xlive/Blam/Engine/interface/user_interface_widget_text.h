#pragma once
#include "user_interface_widget.h"
#include "user_interface_text_block.h"

#pragma pack(push , 1)
class c_text_widget : protected c_user_interface_widget
{
protected:
	int32 intro_animation_delay_ms;
	bool field_74;
	uint8 gap[3];

public:
	c_text_widget(int16 user_flags);
	c_text_widget(datum user_index);

	virtual ~c_text_widget();
	virtual void render_widget(rectangle2d* viewport_bounds) override;
	virtual int get_intro_delay() override;

};
CHECK_STRUCT_SIZE(c_text_widget, 0x78);

class c_small_text_widget : protected c_text_widget
{
protected:
	c_small_user_interface_text m_interface;

public:
	c_small_text_widget(int16 user_flags);
	c_small_text_widget(datum user_index);

	virtual ~c_small_text_widget();
	virtual c_user_interface_text* get_interface() override;

};
CHECK_STRUCT_SIZE(c_small_text_widget, 0xFC);

class c_normal_text_widget : protected c_text_widget
{
protected:
	c_normal_user_interface_text m_interface;

public:
	c_normal_text_widget(int16 user_flags);
	c_normal_text_widget(datum user_index);

	virtual ~c_normal_text_widget();
	virtual c_user_interface_text* get_interface() override;

};
CHECK_STRUCT_SIZE(c_normal_text_widget, 0x4BC);

class c_long_text_widget : protected c_text_widget
{
protected:
	c_long_user_interface_text m_interface;

public:
	c_long_text_widget(int16 user_flags);
	c_long_text_widget(datum user_index);

	virtual ~c_long_text_widget();
	virtual c_user_interface_text* get_interface() override;

};
#pragma pack(pop)
CHECK_STRUCT_SIZE(c_long_text_widget, 0x8BC);