#include "stdafx.h"

#include "screen_esrb_warning.h"
#include "Blam/Engine/interface/user_interface_memory.h"
#include "Blam/Engine/interface/user_interface_controller.h"
#include "Blam/Engine/rasterizer/rasterizer_globals.h"
#include "Blam/Engine/tag_files/global_string_ids.h"


void c_screen_esrb_warning::apply_patches()
{
	//Disables the ESRB warning (only occurs for English Language).
	WriteValue<bool>(Memory::GetAddress(0x411030), 0);
	//disables the one after the intro video, by removing 0x40 flag from 0x7C6 bitmask
	WriteValue(Memory::GetAddress(0x39948 + 2), 0x7C6 & ~FLAG(6));
}


c_screen_esrb_warning::c_screen_esrb_warning(e_user_interface_channel_type channel_type, e_user_interface_render_window window_index, int16 user_flags) :
	c_screen_widget(_screen_esrb_warning, channel_type, window_index, user_flags)

{
	m_elapsed_time = 0;
	//addDebugText("new" __FUNCTION__);
}

c_screen_esrb_warning::~c_screen_esrb_warning()
{
	//addDebugText("destructing" __FUNCTION__);
}

void c_screen_esrb_warning::update()
{
	//INVOKE_TYPE(0x23EDEB, 0x0, void(__thiscall*)(c_screen_esrb_warning*), this);
	c_screen_widget::update();
}

char c_screen_esrb_warning::handle_event(s_event_record* event)
{
	//return INVOKE_TYPE(0x23EE2D, 0x0, char(__thiscall*)(c_screen_esrb_warning*, s_event_record*), this, event);
	//return 0;

	//return c_screen_widget::handle_event(event);

	if (event->type == _user_interface_event_type_keyboard_button_pressed)
	{
		LOG_DEBUG_FUNC(" keeeebd ");
		c_text_widget* text = (c_text_widget*)try_find_text_widget(1);

		if (event->component == _user_interface_keyboard_component_button_tab)
		{
			LOG_DEBUG_FUNC(" noop noop ");
			text->get_interface()->set_pulsating(true);
			text->get_interface()->set_color(global_real_rgb_green);
			text->set_visible(true);

		}
		if (event->component == _user_interface_keyboard_component_buton_space)
		{
			LOG_DEBUG_FUNC(" bow ");
			text->get_interface()->set_pulsating(false);
			text->get_interface()->set_color(global_real_rgb_red);
			text->set_string(L"HEY KID");
			text->set_visible(true);

		}
		if (event->component == _user_interface_keyboard_component_button_backspace)
		{

			rectangle2d bounds;
			text->sub_611703(&bounds);

			/*bounds.left = 6000;
			bounds.right = 100;
			bounds.top = 65000;
			bounds.bottom = 65416;*/
			//rasterizer_get_frame_bounds(&bounds);
			//bounds.top += 40;
			//bounds.left += 40;
			bounds.bottom -= 40;

			void* pool = ui_pool_allocate_space(sizeof(c_normal_text_widget), 0);
			c_normal_text_widget* test_child = new(pool)c_normal_text_widget(m_user_flags);
			test_child->set_allocated(true);
			test_child->set_visible(true);

			test_child->set_text_properties(1,
				2, 
				const_cast <real_argb_color*>(global_real_argb_red), 
				text->get_interface()->m_custom_font_type, 
				&bounds);
			test_child->get_interface()->set_pulsating(true);
			test_child->get_interface()->set_color(global_real_rgb_red);

			this->add_new_child(test_child);
			//this->setup_children();

			text->set_visible(false);
			test_child->set_string(L"TEST TEST");
			//test_child->set_screen_string(0x10001b5f);
			
		}
	}

	return true;
}

void c_screen_esrb_warning::pre_initialize(s_screen_parameters* parameters)
{
	INVOKE_TYPE(0x23ED7F, 0x0, void(__thiscall*)(c_screen_esrb_warning*, s_screen_parameters*), this, parameters);
}

void* c_screen_esrb_warning::load_proc()
{
	return &c_screen_esrb_warning::load;
}

void* c_screen_esrb_warning::load(s_screen_parameters* parameters)
{
	c_screen_esrb_warning* screen;

	parameters->m_flags |= 4u;
	void* pool = ui_pool_allocate_space(sizeof(c_screen_esrb_warning), 0);
	if (pool)
	{
		screen = new (pool) c_screen_esrb_warning(
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
