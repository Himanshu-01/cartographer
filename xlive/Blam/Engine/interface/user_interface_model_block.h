#pragma once
#include "user_interface_widget.h"
#include "game/players.h"

class c_model_widget : public c_user_interface_widget
{
protected:
	void* m_tag_block;

public:
	void apply_appearance_and_character(s_player_profile_traits* appearance, e_character_type character);
	void set_model_animation_mode(string_id mode);

	virtual ~c_model_widget();
	virtual int setup_children() override;
	virtual void render_widget(rectangle2d* viewport_bounds) override;
	virtual int get_intro_delay() override;
	virtual c_user_interface_text* get_interface() override;
};
CHECK_STRUCT_SIZE(c_model_widget, 0x74);