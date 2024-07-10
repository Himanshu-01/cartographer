#include "stdafx.h"
#include "screen_loading_progress.h"
#include "interface/user_interface_bitmap_block.h"
#include "interface/user_interface_screen_widget_definition.h"

#include "H2MOD/Tags/TagInterface.h"

#define retn_instruction 0xC3


enum e_loading_screen_bitmap_blocks
{
	_loading_screen_pane_0_bitmap_loading_screen = 0,
	_loading_screen_pane_0_bitmap_loading_screen_sweep,
	k_loading_screen_pane_0_bitmap_count
};

real32 loading_screen_get_progress(void)
{
	return *Memory::GetAddress<real32*>(0x978E0C);
}

void c_screen_loading_progress::update_hook()
{
	//call the main_screen update
	INVOKE_TYPE(0x248104, 0x0, void(__thiscall*)(c_screen_loading_progress*), this);

	c_bitmap_widget* global_loading_screen_bitmap = try_find_bitmap_widget(_loading_screen_pane_0_bitmap_loading_screen);
	if(global_loading_screen_bitmap)
	{
		//global_loading_screen_bitmap->set_bounds(&this->m_bounds);
		global_loading_screen_bitmap->set_visible(false);
	}

	if (!this->fieldA5C)
	{
		//update progress for : ui\screens\misc\loading_screen\sweep.bitmap 
		c_bitmap_widget* sweep_bitmap = try_find_bitmap_widget(_loading_screen_pane_0_bitmap_loading_screen_sweep);
		int16 screen_width = rectangle2d_width(&this->m_bounds);
		if(sweep_bitmap)
		{
			rectangle2d bitmap_bounds;
			//real_vector2d render_scale = { loading_screen_get_progress(),loading_screen_get_progress() };
			//sweep_bitmap->set_render_scale(&render_scale);
			//sweep_bitmap->sub_611703(&bitmap_bounds);
			//
			sweep_bitmap->get_animating_bounds(&bitmap_bounds);

			int16 bitmap_width = rectangle2d_width(&bitmap_bounds);
			//v11 = v15.right - v15.left;
			bitmap_bounds.left = (int)(float)((float)screen_width * loading_screen_get_progress()) - screen_width;
			//v15.left = (int)(float)((float)screen_width * flt_509334) - screen_width / 2;
			bitmap_bounds.right = bitmap_width + bitmap_bounds.left;
			//v15.right = v11 + v15.left;
			sweep_bitmap->set_bounds(&bitmap_bounds);
		}
	}

}
void __declspec(naked) jmp_loading_progress_update() { __asm { jmp c_screen_loading_progress::update_hook } }

void c_screen_loading_progress::apply_patches()
{
	//inside c_screen_loading_progress::pre_initialize
	NopFill(Memory::GetAddress(0x247E71), 14);

	//disable loading.bin calling
	WriteValue<uint32>(Memory::GetAddress(0x26B85E), retn_instruction);
	NopFill(Memory::GetAddress(0x26B85E + 1), 5);

	//hook into c_screen_loading_progress::update
	WritePointer(Memory::GetAddress(0x3CFEF0), jmp_loading_progress_update);
}

void c_screen_loading_progress::apply_tag_patches()
{
	LOG_INFO_FUNC("applying tag fixes");

	const char* main_widget_tag_path = "ui\\screens\\misc\\loading_screen\\loading_screen";
	const int16 scale_factor = 2;

	datum main_widget_datum_index = tags::find_tag(blam_tag::tag_group_type::userinterfacescreenwidgetdefinition, main_widget_tag_path);

	if (DATUM_IS_NONE(main_widget_datum_index))
		return;

	s_user_interface_screen_widget_definition* main_widget_tag = tags::get_tag_fast<s_user_interface_screen_widget_definition>(main_widget_datum_index);
	s_window_pane_reference* base_pane = main_widget_tag->panes[0];


	const rectangle2d text_bounds[] = {
		{ 20    ,  -290    ,  -20     ,  290  },
	};

	if (base_pane->text_blocks.size > 0)
	{
		for (uint8 itr = 0; itr < base_pane->text_blocks.size; itr++)
		{
			rectangle2d og = text_bounds[itr];
			rectangle2d_scale(&og, scale_factor);
			base_pane->text_blocks[itr]->text_bounds = og;
		}
	}



	const point2d bitmap_positions[] = {
		{ -320   ,   240   },
		{ -320   ,   240   },
	};

	if (base_pane->bitmap_blocks.size > 0)
	{
		for (uint8 itr = 0; itr < base_pane->bitmap_blocks.size; itr++)
		{
			point2d bitmap_pos = bitmap_positions[itr];
			point2d_scale(&bitmap_pos, scale_factor);
			base_pane->bitmap_blocks[itr]->topleft = bitmap_pos;
		}
	}

	
}
