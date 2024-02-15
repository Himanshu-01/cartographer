#pragma once
#include "Blam/Engine/cseries/cseries.h"+
#include "Blam/Engine/math/color_math.h"
#include "user_interface.h"
//#include "user_interface_text.h"


#pragma pack(push,1)
struct s_screen_animation_block
{
	int32 field_0;
	int32 keyframes_tag_blocks;
	int16 number_of_keyframes;
	int16 last_keyframe_index;
	int16 direction;
	uint16 m_flags;
	int32 intro_delay_ms;
	int32 looping_stlye;
	int32 field_18;
	int32 last_frame_time;
	int32 period_milliseconds;
	real32 field_24;
	real32 field_28;
	real32 field_2C;
	real32 current_alpha;
};
#pragma pack(pop)
CHECK_STRUCT_SIZE(s_screen_animation_block, 0x34);

struct s_event_record;
class c_user_interface_text;

#pragma pack(push , 1)
class c_user_interface_widget
{
protected:
	//void* __vtable;
	e_user_interface_widget_type m_widget_type;
	int16 m_user_flags;
	int16 m_block_index;
	int32 m_hierarchy_order;
	c_user_interface_widget* parent_widget;
	c_user_interface_widget* child_widget;
	c_user_interface_widget* next_widget;
	c_user_interface_widget* previous_widget;
	rectangle2d m_bounds;
	real_rgb_color m_widget_color;
	s_screen_animation_block m_current_animation;
	int16 m_animation_index;
	int16 m_render_dept_bias;
	bool m_allocated;
	bool field_6D;
	bool m_visible;
	bool m_can_handle_events;


	void destroy_recursive();
	void initialize_animation(s_screen_animation_block* animation);
	void set_bounds(rectangle2d*  bounds);

public:
	c_user_interface_widget(e_user_interface_widget_type widget_type, int16 user_flags);

	e_user_interface_widget_type get_type();
	c_user_interface_widget* get_next();
	c_user_interface_widget* get_previous();
	c_user_interface_widget* get_parent();
	c_user_interface_widget* try_find_child(e_user_interface_widget_type type, uint32 idx, bool recursive_search);
	c_user_interface_widget* try_find_text_widget(uint32 idx);
	c_user_interface_widget* try_find_hud_widget(uint32 idx);
	c_user_interface_widget* try_find_bitmap_widget(uint32 idx);
	c_user_interface_widget* try_find_player_widget(uint32 idx);
	c_user_interface_widget* try_find_model_widget(uint32 idx);

	// Virtual functions

	virtual ~c_user_interface_widget();
	virtual int setup_children();
	virtual void on_screen_leave();
	virtual void update();
	virtual void render_widget(rectangle2d* viewport_bounds);
	virtual void* get_mouse_region(rectangle2d* mouse_region_out);
	virtual int initialize_child_animations(s_screen_animation_block* a2);
	virtual int get_intro_delay();
	virtual void* sub_611703(rectangle2d* unprojected_bounds);
	virtual void sub_612A7C(c_user_interface_widget* a2);
	virtual c_user_interface_widget* sub_612ABC();
	virtual c_user_interface_widget* sub_612BCA();
	virtual char handle_event(s_event_record* event);
	virtual e_user_interface_channel_type get_parent_channel();
	virtual e_user_interface_render_window get_parent_render_window();
	virtual void construct_animation_on_region_enter(int a1);
	virtual void construct_animation_on_region_leave(int a1);
	virtual c_user_interface_widget* sub_6121F6(rectangle2d* point);
	virtual bool can_interact();
	virtual c_user_interface_text* get_interface() = 0;
	virtual bool sub_6114B9();
};
#pragma pack(pop)
CHECK_STRUCT_SIZE(c_user_interface_widget, 0x70);

