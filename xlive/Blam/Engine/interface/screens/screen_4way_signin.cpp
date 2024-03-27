#include "stdafx.h"

#include "screen_4way_signin.h"
#include "interface/user_interface_memory.h"
#include "interface/user_interface_networking.h"
#include "interface/user_interface_player_widget.h"
#include "interface/user_interface_model_block.h"
#include "interface/user_interface_bitmap_block.h"
#include "interface/user_interface_controller.h"
#include "tag_files/global_string_ids.h"

enum e_4way_signin_main_text_blocks
{


	//_4way_signin_main_pane_0_text_player0_profile_name = 0,
	// Note : all text blocks are shifted by 2 (internally) wrt tag block index
	// probably to account for menu title text and button key text
	_4way_signin_main_pane_0_text_player0_profile_name = 2,
	_4way_signin_main_pane_0_text_player0_insert_controller,
	_4way_signin_main_pane_0_text_player1_profile_name,
	_4way_signin_main_pane_0_text_player1_insert_controller,
	_4way_signin_main_pane_0_text_player2_profile_name,
	_4way_signin_main_pane_0_text_player2_insert_controller,
	_4way_signin_main_pane_0_text_player3_profile_name,
	_4way_signin_main_pane_0_text_player3_insert_controller,

	_4way_signin_main_pane_0_text_player0_profile_name_heading,
	_4way_signin_main_pane_0_text_player0_gamertag_heading,
	_4way_signin_main_pane_0_text_player0_gamertag,
	_4way_signin_main_pane_0_text_player1_profile_name_heading,
	_4way_signin_main_pane_0_text_player1_gamertag_heading,
	_4way_signin_main_pane_0_text_player1_gamertag,
	_4way_signin_main_pane_0_text_player2_profile_name_heading,
	_4way_signin_main_pane_0_text_player2_gamertag_heading,
	_4way_signin_main_pane_0_text_player2_gamertag,
	_4way_signin_main_pane_0_text_player3_profile_name_heading,
	_4way_signin_main_pane_0_text_player3_gamertag_heading,
	_4way_signin_main_pane_0_text_player3_gamertag,

	_4way_signin_main_pane_0_text_player0_press_a_to_join,
	_4way_signin_main_pane_0_text_player1_press_a_to_join,
	_4way_signin_main_pane_0_text_player2_press_a_to_join,
	_4way_signin_main_pane_0_text_player3_press_a_to_join,

};

enum e_4way_signin_main_bitmap_blocks
{
	_4way_signin_main_pane_0_bitmap_mp_signin = 0,
	_4way_signin_main_pane_0_bitmap_global_ul_09,
	_4way_signin_main_pane_0_bitmap_global_br_09,
	_4way_signin_main_pane_0_bitmap_3,
	_4way_signin_main_pane_0_bitmap_4,
	_4way_signin_main_pane_0_bitmap_5,
	_4way_signin_main_pane_0_bitmap_6,
	_4way_signin_main_pane_0_bitmap_player_0_controller_signin,
	_4way_signin_main_pane_0_bitmap_player_1_controller_signin,
	_4way_signin_main_pane_0_bitmap_player_2_controller_signin,
	_4way_signin_main_pane_0_bitmap_player_3_controller_signin,
};

enum e_4way_signin_main_model_blocks
{
	_4way_signin_main_pane_0_model_ui_player1 = 0,
	_4way_signin_main_pane_0_model_ui_player2,
	_4way_signin_main_pane_0_model_ui_player3,
	_4way_signin_main_pane_0_model_ui_player4,
	_4way_signin_main_pane_0_model_ui_player1b,
	_4way_signin_main_pane_0_model_ui_player2b,
	_4way_signin_main_pane_0_model_ui_player3b,
	_4way_signin_main_pane_0_model_ui_player4b,
};

c_screen_4way_signin::c_screen_4way_signin(e_user_interface_channel_type channel_type, e_user_interface_render_window window_index, int16 user_flags) :
	c_screen_widget(_screen_4way_join_screen, channel_type, window_index, user_flags)
{
	m_call_context = _4_way_signin_type_splitscreen;
}

c_screen_4way_signin::~c_screen_4way_signin()
{
}

void c_screen_4way_signin::update()
{

	//c_user_interface_widget::update();
	//return;

	const uint32 all_users_mask = NONE;
	const bool has_live_privileges = true;

	this->update_users_mask(all_users_mask);
	c_player_widget_representation representations[k_number_of_controllers];
	for (e_controller_index controller = _controller_index_0;
		controller != NONE;
		controller = (e_controller_index)user_interface_controller_get_next_valid_index(controller))
	{
		bool gamepad_connected = user_interface_controller_has_gamepad(controller);
		bool controller_has_joined = has_live_privileges && user_interface_controller_is_player_profile_valid(controller);
		bool show_gamertag_text = false;

		s_saved_game_file_player_profile profile;
		if (controller_has_joined)
		{
			uint32 profile_index;
			user_interface_controller_get_profile_data(controller, &profile, &profile_index);

			c_player_widget_representation* current_player = &representations[controller];
			current_player->set_appearance(&profile.profile);
			current_player->set_player_name_from_configuration((s_player_properties*)profile.player_name.get_buffer()); //hack
			show_gamertag_text = true;
		}


		uint32 profile_name_text_id, press_a_to_join_text_id, insert_controller_text_id,
			profile_name_heading_text_id, gamertag_heading_text_id, gamertag_text_id,
			ui_player_model_id_a, ui_player_model_id_b,
			unknown_bitmap_id, controller_signin_bitmap_id;

		switch (controller)
		{
		case _controller_index_0:

			profile_name_text_id = _4way_signin_main_pane_0_text_player0_profile_name;
			press_a_to_join_text_id = _4way_signin_main_pane_0_text_player0_press_a_to_join;
			insert_controller_text_id = _4way_signin_main_pane_0_text_player0_insert_controller;
			profile_name_heading_text_id = _4way_signin_main_pane_0_text_player0_profile_name_heading;
			gamertag_heading_text_id = _4way_signin_main_pane_0_text_player0_gamertag_heading;
			gamertag_text_id = _4way_signin_main_pane_0_text_player0_gamertag;
			ui_player_model_id_a = _4way_signin_main_pane_0_model_ui_player1;
			ui_player_model_id_b = _4way_signin_main_pane_0_model_ui_player1b;
			unknown_bitmap_id = _4way_signin_main_pane_0_bitmap_3;
			controller_signin_bitmap_id = _4way_signin_main_pane_0_bitmap_player_0_controller_signin;

			break;

		case _controller_index_1:

			profile_name_text_id = _4way_signin_main_pane_0_text_player1_profile_name;
			press_a_to_join_text_id = _4way_signin_main_pane_0_text_player1_press_a_to_join;
			insert_controller_text_id = _4way_signin_main_pane_0_text_player1_insert_controller;
			profile_name_heading_text_id = _4way_signin_main_pane_0_text_player1_profile_name_heading;
			gamertag_heading_text_id = _4way_signin_main_pane_0_text_player1_gamertag_heading;
			gamertag_text_id = _4way_signin_main_pane_0_text_player1_gamertag;
			ui_player_model_id_a = _4way_signin_main_pane_0_model_ui_player2;
			ui_player_model_id_b = _4way_signin_main_pane_0_model_ui_player2b;
			unknown_bitmap_id = _4way_signin_main_pane_0_bitmap_4;
			controller_signin_bitmap_id = _4way_signin_main_pane_0_bitmap_player_1_controller_signin;

			break;

		case _controller_index_2:

			profile_name_text_id = _4way_signin_main_pane_0_text_player2_profile_name;
			press_a_to_join_text_id = _4way_signin_main_pane_0_text_player2_press_a_to_join;
			insert_controller_text_id = _4way_signin_main_pane_0_text_player2_insert_controller;
			profile_name_heading_text_id = _4way_signin_main_pane_0_text_player2_profile_name_heading;
			gamertag_heading_text_id = _4way_signin_main_pane_0_text_player2_gamertag_heading;
			gamertag_text_id = _4way_signin_main_pane_0_text_player2_gamertag;
			ui_player_model_id_a = _4way_signin_main_pane_0_model_ui_player3;
			ui_player_model_id_b = _4way_signin_main_pane_0_model_ui_player3b;
			unknown_bitmap_id = _4way_signin_main_pane_0_bitmap_5;
			controller_signin_bitmap_id = _4way_signin_main_pane_0_bitmap_player_2_controller_signin;

			break;

		case _controller_index_3:

			profile_name_text_id = _4way_signin_main_pane_0_text_player3_profile_name;
			press_a_to_join_text_id = _4way_signin_main_pane_0_text_player3_press_a_to_join;
			insert_controller_text_id = _4way_signin_main_pane_0_text_player3_insert_controller;
			profile_name_heading_text_id = _4way_signin_main_pane_0_text_player3_profile_name_heading;
			gamertag_heading_text_id = _4way_signin_main_pane_0_text_player3_gamertag_heading;
			gamertag_text_id = _4way_signin_main_pane_0_text_player3_gamertag;
			ui_player_model_id_a = _4way_signin_main_pane_0_model_ui_player4;
			ui_player_model_id_b = _4way_signin_main_pane_0_model_ui_player4b;
			unknown_bitmap_id = _4way_signin_main_pane_0_bitmap_6;
			controller_signin_bitmap_id = _4way_signin_main_pane_0_bitmap_player_3_controller_signin;

			break;


		}

		this->set_child_visible(_widget_type_text, profile_name_text_id, controller_has_joined);
		c_text_widget* press_a_to_join_text = (c_text_widget*)this->try_find_text_widget(press_a_to_join_text_id);
		if (press_a_to_join_text)
		{
			press_a_to_join_text->set_visible(false);

			if (!controller_has_joined)
			{
				string_id join_text_string;
				if (gamepad_connected)
				{
					join_text_string = HS_PRESS_A_TO_JOIN;
				}
				else
				{
					join_text_string = HS_INSERT_CONTROLLER;
				}
				press_a_to_join_text->set_visible(true);
				press_a_to_join_text->set_screen_string(join_text_string);
			}
		}

		c_text_widget* insert_controller_text = (c_text_widget*)this->try_find_text_widget(insert_controller_text_id);
		if (insert_controller_text)
		{
			if (controller_has_joined)
			{
				insert_controller_text->set_visible(true);
				insert_controller_text->set_screen_string(HS_PRESS_A_TO_CONTINUE);
			}
			else
			{
				insert_controller_text->set_visible(false);
			}
		}

		this->set_child_visible(_widget_type_text, gamertag_text_id, controller_has_joined);
		this->set_child_visible(_widget_type_text, profile_name_heading_text_id, show_gamertag_text);
		this->set_child_visible(_widget_type_text, gamertag_heading_text_id, show_gamertag_text);

		//for some reason game saves player characters as masterchief and dervish in saved profiles
		c_model_widget* ui_player_model_a = (c_model_widget*)this->try_find_model_widget(ui_player_model_id_a);
		if (ui_player_model_a)
			ui_player_model_a->apply_appearance_and_character(&profile.profile, _character_type_masterchief);
		c_model_widget* ui_player_model_b = (c_model_widget*)this->try_find_model_widget(ui_player_model_id_b);
		if (ui_player_model_b)
			ui_player_model_b->apply_appearance_and_character(&profile.profile, _character_type_dervish);


		c_bitmap_widget* unknown_bitmap = (c_bitmap_widget*)this->try_find_bitmap_widget(unknown_bitmap_id);
		if (unknown_bitmap)
			unknown_bitmap->set_visible(controller_has_joined);
		c_bitmap_widget* controller_signin_bitmap = (c_bitmap_widget*)this->try_find_bitmap_widget(controller_signin_bitmap_id);
		if (controller_signin_bitmap)
		{
			bool should_show_bitmap = controller_has_joined == false;
			controller_signin_bitmap->set_visible(should_show_bitmap);
			if (should_show_bitmap)
			{
				int16 bitmap_update_idx = 2;
				//controller_signin_bitmap->verify_and_update_bitmap_index(bitmap_update_idx);
			}
		}
	}


	this->apply_new_representations_to_players(representations, k_number_of_controllers);
	c_user_interface_widget::update();
}

char c_screen_4way_signin::handle_event(s_event_record* event)
{
	//LOG_INFO_FUNC("rejected");
	// TODO
	return c_screen_widget::handle_event(event);
}

void c_screen_4way_signin::pre_initialize(s_screen_parameters* parameters)
{
	s_interface_expected_screen_layout layout;
	csmemset(&layout, 0, sizeof(layout));
	layout.panes_count = 1;

	//LOG_INFO_FUNC("we are activated noob");
	this->verify_and_load_from_layout(NONE, &layout);
	user_interface_squad_clear_match_playlist();
}

void* c_screen_4way_signin::load_proc()
{
	switch (this->m_call_context)
	{
	case _4_way_signin_type_crossgame_invite:
		return  &c_screen_4way_signin::load_type4;
		break;
	case _4_way_signin_type_xbox_live:
		return  &c_screen_4way_signin::load_type3;
		break;

	case _4_way_signin_type_system_link:
		return  &c_screen_4way_signin::load_type2;
		break;

	case _4_way_signin_type_splitscreen:
		return  &c_screen_4way_signin::load_type1;
		break;

	case _4_way_signin_type_campaign:
		return  &c_screen_4way_signin::load_type0;
		break;

	default:
		LOG_ERROR_FUNC("unreachable type");
	}
	return nullptr;
}

void* c_screen_4way_signin::load(s_screen_parameters* parameters)
{
	c_screen_4way_signin* screen;

	//parameters->m_flags |= 4u;
	void* pool = ui_pool_allocate_space(sizeof(c_screen_4way_signin), 0);
	if (pool)
	{
		screen = new (pool) c_screen_4way_signin(
			parameters->m_channel_type,
			parameters->m_window_index,
			parameters->user_flags);

		screen->m_allocated = true;
		//user_interface_register_screen_to_channel(screen, parameters);
	}
	else
	{
		screen = 0;
	}

	return screen;
}

void* c_screen_4way_signin::load_type4(s_screen_parameters* parameters)
{
	c_screen_4way_signin* screen;
	screen = (c_screen_4way_signin*)c_screen_4way_signin::load(parameters);
	screen->m_call_context = _4_way_signin_type_crossgame_invite;
	user_interface_register_screen_to_channel(screen, parameters);
	return screen;
}
void* c_screen_4way_signin::load_type3(s_screen_parameters* parameters)
{
	c_screen_4way_signin* screen;
	screen = (c_screen_4way_signin*)c_screen_4way_signin::load(parameters);
	screen->m_call_context = _4_way_signin_type_xbox_live;
	user_interface_register_screen_to_channel(screen, parameters);
	return screen;
}
void* c_screen_4way_signin::load_type2(s_screen_parameters* parameters)
{
	c_screen_4way_signin* screen;
	screen = (c_screen_4way_signin*)c_screen_4way_signin::load(parameters);
	screen->m_call_context = _4_way_signin_type_system_link;
	user_interface_register_screen_to_channel(screen, parameters);
	return screen;
}
void* c_screen_4way_signin::load_type1(s_screen_parameters* parameters)
{
	c_screen_4way_signin* screen;
	screen = (c_screen_4way_signin*)c_screen_4way_signin::load(parameters);
	screen->m_call_context = _4_way_signin_type_splitscreen;
	user_interface_register_screen_to_channel(screen, parameters);
	return screen;
}
void* c_screen_4way_signin::load_type0(s_screen_parameters* parameters)
{
	c_screen_4way_signin* screen;
	screen = (c_screen_4way_signin*)c_screen_4way_signin::load(parameters);
	screen->m_call_context = _4_way_signin_type_campaign;
	user_interface_register_screen_to_channel(screen, parameters);
	return screen;
}
