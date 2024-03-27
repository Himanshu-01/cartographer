#include "stdafx.h"

#include "screen_press_start_introduction.h"
#include "Blam/Engine/interface/user_interface_memory.h"


c_screen_press_start_introduction::c_screen_press_start_introduction(e_user_interface_channel_type channel_type, e_user_interface_render_window window_index, int16 user_flags) :
	c_screen_widget(_screen_press_start_intro, channel_type, window_index, user_flags),
	m_start_button(0, user_flags),
	m_slot(this, &c_screen_press_start_introduction::handle_item_pressed_event)
{
	this->m_has_input_saved = false;
	LOG_INFO_FUNC(" init");
}

c_screen_press_start_introduction::~c_screen_press_start_introduction()
{
}

void c_screen_press_start_introduction::update()
{
	//LOG_DEBUG_FUNC("bum");
	INVOKE_TYPE(0x23F2FD, 0x0, void(__thiscall*)(c_screen_press_start_introduction*), this);
}

char c_screen_press_start_introduction::handle_event(s_event_record* event)
{
	LOG_INFO_FUNC("hey did u tick me");

	return INVOKE_TYPE(0x23F0BA, 0x0, char(__thiscall*)(c_screen_press_start_introduction*, s_event_record*), this, event);
}

void c_screen_press_start_introduction::pre_initialize(s_screen_parameters* parameters)
{
	LOG_INFO_FUNC(" ok");
	INVOKE_TYPE(0x23F180, 0x0, void(__thiscall*)(c_screen_press_start_introduction*, s_screen_parameters*), this, parameters);
}

void c_screen_press_start_introduction::sub_60EBC2(int a1)
{
	//LOG_DEBUG_FUNC(" init");
	INVOKE_TYPE(0x23F011, 0x0, void(__thiscall*)(c_screen_press_start_introduction*, int), this, a1);
}

void* c_screen_press_start_introduction::load_proc()
{
	return &c_screen_press_start_introduction::load;
}

void* c_screen_press_start_introduction::load(s_screen_parameters* parameters)
{
	c_screen_press_start_introduction* screen;

	parameters->m_flags |= 4u;
	void* pool = ui_pool_allocate_space(sizeof(c_screen_press_start_introduction), 0);
	if (pool)
	{
		screen = new (pool) c_screen_press_start_introduction(
			parameters->m_channel_type,
			parameters->m_window_index,
			parameters->user_flags);

		screen->m_allocated = true;
		user_interface_register_screen_to_channel(screen, parameters);
	}
	else
	{
		screen = 0;
	}

	return screen;
}

char c_screen_press_start_introduction::handle_item_pressed_event(s_event_record** pevent, short* pitem_index)
{
	s_event_record* arg = *pevent;
	
	this->m_saved_input.type = arg->type;
	this->m_saved_input.controller = arg->controller;
	this->m_saved_input.component = arg->component;
	this->m_saved_input.value = arg->value;

	this->m_has_input_saved = true;

	LOG_DEBUG_FUNC("saving ur kid");
	return arg->value;
}

