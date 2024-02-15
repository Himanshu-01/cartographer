#pragma once
#include "user_interface_widget.h"
#include "game/players.h"

class c_hud_widget : protected c_user_interface_widget
{
protected:
	void* m_tag_block;
	s_player_profile_traits m_appearance;
	int16 m_team_index;
	int16 m_rank_index;

public:
	virtual ~c_hud_widget();
	virtual void update() override;
	virtual void render_widget(rectangle2d* viewport_bounds) override;
	virtual int get_intro_delay() override;
	virtual c_user_interface_text* get_interface() override;
};
CHECK_STRUCT_SIZE(c_hud_widget, 0x88);