#include "stdafx.h"

#include "H2MOD/Tags/TagInterface.h"
#include "interface/user_interface.h"
#include "interface/user_interface_bitmap_block.h"
#include "interface/user_interface_controller.h"
#include "interface/user_interface_memory.h"
#include "interface/user_interface_model_block.h"
#include "interface/user_interface_networking.h"
#include "interface/user_interface_player_widget.h"
#include "interface/user_interface_screen_widget_definition.h"
#include "interface/user_interface_utilities.h"
#include "interface/user_interface_globals.h"
#include "Networking/online/online_account_xbox.h"
#include "screen_4way_signin.h"
#include "tag_files/global_string_ids.h"

enum e_4way_signin_main_text_blocks
{

	_4way_signin_main_pane_0_text_player0_profile_name = 0,
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
			show_gamertag_text = online_connected_to_xbox_live();
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

		this->set_child_visible(_widget_type_text, TEXT_BLOCK_INDEX_TO_WIDGET_INDEX(profile_name_text_id), controller_has_joined);
		c_text_widget* press_a_to_join_text = (c_text_widget*)this->try_find_text_widget(TEXT_BLOCK_INDEX_TO_WIDGET_INDEX(press_a_to_join_text_id));
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

		c_text_widget* insert_controller_text = (c_text_widget*)this->try_find_text_widget(TEXT_BLOCK_INDEX_TO_WIDGET_INDEX(insert_controller_text_id));
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

		//this->set_child_visible(_widget_type_text, gamertag_text_id, controller_has_joined);
		//this->set_child_visible(_widget_type_text, profile_name_heading_text_id, show_gamertag_text);
		//this->set_child_visible(_widget_type_text, gamertag_heading_text_id, show_gamertag_text);

		this->set_child_visible(_widget_type_text, TEXT_BLOCK_INDEX_TO_WIDGET_INDEX(gamertag_text_id), show_gamertag_text);
		this->set_child_visible(_widget_type_text, TEXT_BLOCK_INDEX_TO_WIDGET_INDEX(gamertag_heading_text_id), show_gamertag_text);
		this->set_child_visible(_widget_type_text, TEXT_BLOCK_INDEX_TO_WIDGET_INDEX(profile_name_heading_text_id), controller_has_joined);

		//for some reason game saves player characters as masterchief and dervish in saved profiles
		c_model_widget* ui_player_model_a = (c_model_widget*)this->try_find_model_widget(ui_player_model_id_a);
		if (ui_player_model_a)
		{
			if (controller_has_joined)
				ui_player_model_a->apply_appearance_and_character(&profile.profile, _character_type_masterchief);
			else
				ui_player_model_a->set_visible(false);
		}
		c_model_widget* ui_player_model_b = (c_model_widget*)this->try_find_model_widget(ui_player_model_id_b);
		if (ui_player_model_b)
		{
			if (controller_has_joined)
				ui_player_model_b->apply_appearance_and_character(&profile.profile, _character_type_dervish);
			else
				ui_player_model_b->set_visible(false);

		}


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
				int16 bitmap_update_idx = gamepad_connected ? 1 : 0;
				controller_signin_bitmap->verify_and_update_bitmap_index(bitmap_update_idx);
			}
		}
	}


	this->apply_new_representations_to_players(representations, k_number_of_controllers);
	c_user_interface_widget::update();
}
string_id g_change_animation;
char c_screen_4way_signin::handle_event(s_event_record* event)
{
	//LOG_INFO_FUNC("rejected");
	// TODO
	// 
	// 
	//  if ( user_interface_channel_is_busy(1) )
	//		return true;
	// 
	//  if( user_interface_controller_is_player_profile_valid(event->controller)
	//  {
	//		this->handle_main_events(event);
	//	}
	//	if (event->type == _user_interface_event_type_gamepad_button_pressed)
	//	{
	//		this->handle_no_live_signin_event(event);
	//	}

	
	char result = false;

	if (user_interface_channel_is_busy(_user_interface_channel_type_game_error))
		return true;

	if (event->type == _user_interface_event_type_gamepad_button_pressed)
	{
		if (!user_interface_controller_is_player_profile_valid(event->controller))
		{
			/*user_interface_controller_pick_profile_dialog(event->controller, has_live_privileges);
			result = true;*/

			result = this->handle_default_events(event);
		}
		else
		{
			//uint32 ui_player_model_id_a, ui_player_model_id_b;
			//switch (event->controller)
			//{
			//case _controller_index_0:
			//	ui_player_model_id_a = _4way_signin_main_pane_0_model_ui_player1;
			//	ui_player_model_id_b = _4way_signin_main_pane_0_model_ui_player1b;
			//	break;

			//case _controller_index_1:
			//	ui_player_model_id_a = _4way_signin_main_pane_0_model_ui_player2;
			//	ui_player_model_id_b = _4way_signin_main_pane_0_model_ui_player2b;
			//	break;

			//case _controller_index_2:
			//	ui_player_model_id_a = _4way_signin_main_pane_0_model_ui_player3;
			//	ui_player_model_id_b = _4way_signin_main_pane_0_model_ui_player3b;
			//	break;

			//case _controller_index_3:
			//	ui_player_model_id_a = _4way_signin_main_pane_0_model_ui_player4;
			//	ui_player_model_id_b = _4way_signin_main_pane_0_model_ui_player4b;
			//	break;
			//}
			//c_model_widget* ui_player_model_a = (c_model_widget*)this->try_find_model_widget(ui_player_model_id_a);
			//c_model_widget* ui_player_model_b = (c_model_widget*)this->try_find_model_widget(ui_player_model_id_b);
			////if (ui_player_model_a)
			//ui_player_model_a->set_model_animation_mode(g_change_animation);
			////if (ui_player_model_b)
			//ui_player_model_b->set_model_animation_mode(g_change_animation);
			//
			// 
			// stupid code refuses to change animation to anything but HS_UI
			// rip my dream of getting elites to dive in menu
			//LOG_INFO_FUNC("going berserk thank you");

			result = this->handle_main_events(event);

		}
	}
	if (result)
		return result;
	return c_screen_widget::handle_event(event);
}

void c_screen_4way_signin::pre_initialize(s_screen_parameters* parameters)
{
	s_interface_expected_screen_layout layout;
	csmemset(&layout, 0, sizeof(layout));
	layout.panes_count = 1;

	//LOG_INFO_FUNC("we are activated noob");
	datum widget_tag_datum = get_wgit_tag_datum_from_menu_id(this->m_menu_id);
	if (!DATUM_IS_NONE(widget_tag_datum))
	{

		//c_screen_4way_signin_tag_fixes((s_user_interface_screen_widget_definition*)this->get_screen_definition());
		this->verify_and_load_from_layout(NONE, &layout);
	}
	this->setup_children();
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

char __cdecl user_interface_mainmenu_sign_out_controller_callback(e_controller_index controller_index)
{
	//return INVOKE(0xA421, 0x0, user_interface_mainmenu_sign_out_controller_callback, controller_index);
	user_interface_controller_sign_out(controller_index);
	user_interface_enter_game_shell(2);
	return true;

}
char __cdecl user_inteface_sign_out_controller_default_callback(e_controller_index controller_index)
{
	user_interface_controller_sign_out(controller_index);
	return true;
}

char c_screen_4way_signin::handle_main_events(s_event_record* event)
{
	char sucess = true;
	if (event->component == _user_interface_controller_component_button_a ||
		event->component == _user_interface_controller_component_button_start)
	{
		s_screen_parameters params;
		params.m_flags = 0;
		params.m_window_index = _window_4;
		params.field_C = 0;
		params.user_flags = FLAG(event->controller);
		params.m_channel_type = _user_interface_channel_type_gameshell;
		params.m_screen_state.field_0 = 0xFFFFFFFF;
		params.m_screen_state.field_4 = 0xFFFFFFFF;
		params.m_screen_state.field_8 = 0xFFFFFFFF;
		params.m_load_function = nullptr;
		//c_screen_4way_signin::load_type2(&params);

		switch (this->m_call_context)
		{
		case _4_way_signin_type_campaign:
			
			if (user_interface_create_new_squad(true, online_connected_to_xbox_live()))
			{
				user_interface_squad_clear_game_settings();
				user_interface_set_desired_multiplayer_mode(0);
				int32 difficulty = user_interface_globals_get_game_difficulty();
				user_interface_globals_set_game_difficulty_real(difficulty);

				user_interface_globals_campaign_unk(false);
				user_interface_squad_set_campaign_difficulty(difficulty);
			}
			break;
		case _4_way_signin_type_splitscreen:
			if (user_interface_create_new_squad(true, false))
			{
				user_interface_set_desired_multiplayer_mode(2);
			}
			break;
		case _4_way_signin_type_system_link:
			params.m_load_function = c_screen_network_squad_browser_load;
			break;
		case _4_way_signin_type_xbox_live:
			params.m_load_function = c_screen_bungie_news_load;
			break;
		case _4_way_signin_type_crossgame_invite:
			//if (connected_to_xbox_live())
			//{
			//	sub_238F3F(*(event + 4), 0, 0i64);
			//	a1->m_sign_in_context = _4_way_signin_type_xbox_live;
			//}
			//else
			//{
			//	screen_error_dialog_show(3, _ui_error_cant_join_gameinvite_without_signon, 4, 1 << *(event + 4), 0, 0);
			//}
			// i dont really know how to get the game invite part done
			// maybe merge this with discord invite??
			if (online_connected_to_xbox_live())
			{

			}

			screen_error_ok_dialog_show(_user_interface_channel_type_interface, _ui_error_cant_join_gameinvite_without_signon, _window_4, FLAG(event->controller), 0, 0);

			break;
		}

		if (params.m_load_function != nullptr)
			params.m_load_function(&params);
		
	}
	else if (event->component == _user_interface_controller_component_button_b ||
		event->component == _user_interface_controller_component_button_back)
	{
		if (user_interface_controller_get_signed_in_controller_count() == 1)
		{
			if (!user_interface_back_out_from_channel_by_id(_user_interface_channel_type_gameshell, _window_4, _screen_xbox_live_main_menu)
				&& !user_interface_back_out_from_channel_by_id(_user_interface_channel_type_gameshell, _window_4, _screen_main_menu))
			{
				user_interface_enter_game_shell(1);
			}
		}
		else
		{

			if (user_interface_controller_get_guest_controllers_count_for_master(event->controller) <= 0)
			{
				if (this->m_call_context == _4_way_signin_type_crossgame_invite)
				{
					user_interface_error_display_ok_cancle_dialog_with_ok_callback(_user_interface_channel_type_interface,
						_window_4,
						FLAG(event->controller),
						user_interface_mainmenu_sign_out_controller_callback,
						_ui_error_confirm_controller_sign_out);
				}
				else
				{
					user_interface_error_display_ok_cancle_dialog_with_ok_callback(_user_interface_channel_type_interface,
						_window_4,
						FLAG(event->controller),
						user_inteface_sign_out_controller_default_callback,
						_ui_error_confirm_controller_sign_out);
				}
			}
			else
			{
				screen_error_ok_dialog_show(_user_interface_channel_type_interface, _ui_error_cant_sign_out_master_with_guests, _window_4, FLAG(event->controller), NULL, NULL);

			}
			this->m_user_flags |= FLAG(event->controller);
		}

	}
	else
	{
		sucess = false;
	}

	return sucess;
	}

	char c_screen_4way_signin::handle_default_events(s_event_record* event)
	{
		if (event->component == _user_interface_controller_component_button_a
			|| event->component == _user_interface_controller_component_button_start)
		{
			bool online = false;
			if (online_connected_to_xbox_live() && this->m_call_context == _4_way_signin_type_crossgame_invite)
			{
				online = true;
			}
			user_interface_controller_pick_profile_dialog(event->controller, online);
		}
		else if (event->component == _user_interface_controller_component_button_b 
			|| event->component == _user_interface_controller_component_button_back)
		{
			user_interface_error_display_ok_cancle_dialog_with_ok_callback(_user_interface_channel_type_interface,
				_window_4,
				FLAG(event->controller),
				user_interface_mainmenu_sign_out_controller_callback,
				_ui_error_confirm_controller_sign_out);
		}
		return true;
	}

	void* c_screen_4way_signin::load(s_screen_parameters * parameters)
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

	void c_screen_4way_signin::apply_patches()
	{
		LOG_INFO_FUNC("applying tag fixes");

		const char* main_widget_tag_path = "ui\\screens\\game_shell\\4way_multiplayer_signin\\4way_signin_main";
		const char* player_skins_tag_path = "ui\\player_skins\\player_skin_signin";
		const int16 scale_factor = 2;

		datum main_widget_datum_index = tags::find_tag(blam_tag::tag_group_type::userinterfacescreenwidgetdefinition, main_widget_tag_path);
		datum player_skins_datum_index = tags::find_tag(blam_tag::tag_group_type::userinterfacelistskindefinition, player_skins_tag_path);

		if (DATUM_IS_NONE(main_widget_datum_index) || DATUM_IS_NONE(player_skins_datum_index))
			return;

		s_user_interface_screen_widget_definition* main_widget_tag = tags::get_tag_fast<s_user_interface_screen_widget_definition>(main_widget_datum_index);
		s_window_pane_reference* base_pane = main_widget_tag->panes[0];

		const point2d bitmap_positions[] = {
			{ -227   ,   198   },
			{ -280   ,   180   },
			{   45   ,    31   },
			{ -107   ,   123   },
			{  172   ,   123   },
			{ -107   ,   -54   },
			{  172   ,   -54   },
			{ -181   ,   145   },
			{   94   ,   145   },
			{ -181   ,   -32   },
			{   94   ,   -32   }
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

		const rectangle2d text_bounds[] = {
			{ 68    ,  -179    ,  46     ,  -4   },
			{166    ,  -179    ,  118    ,  -4   },
			{ 68    ,   100    ,  46     ,  275  },
			{166    ,   100    ,  118    ,  275  },
			{-108   ,  -179    , -130    ,  -4   },
			{-5     ,  -179    ,  -59    ,  -4   },
			{-108   ,   100    ,  -130   ,  275  },
			{-5     ,   100    ,  -59    ,  275  },
			{ 86    ,  -179    ,   68    ,  -4   },
			{ 47    ,  -179    ,   29    ,  -4   },
			{ 29    ,  -179    ,   7     ,  -4   },
			{ 86    ,   100    ,   68    ,  275  },
			{ 47    ,   100    ,   29    ,  275  },
			{ 29    ,   100    ,   7     ,  275  },
			{-90    ,  -179    ,  -108   ,  -4   },
			{-130   ,  -179    ,  -148   ,  -4   },
			{-148   ,  -179    ,  -170   ,  -4   },
			{-90    ,   100    ,  -108   ,  275  },
			{-130   ,   100    ,  -148   ,  275  },
			{-148   ,   100    ,  -170   ,  275  },
			{ 68    ,  -229    ,   14    ,  -47  },
			{ 68    ,    46    ,   14    , 	228  },
			{-108   ,  -229    ,  -162   ,  -47  },
			{-108   ,    46    ,  -162   ,  228  }
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

		const rectangle2d model_viewports[] = {
			{240 , -320,  0		, 320 }   ,
			{240 , -320,  0		, 320 }   ,
			{0   , -320,-240	, 320 }   ,
			{0   , -320,-240	, 320 }   ,
			{240 , -320,  0		, 320 }   ,
			{240 , -320,  0		, 320 }   ,
			{0   , -320,-240	, 320 }   ,
			{0   , -320,-240	, 320 }   ,
		};
		if (base_pane->model_scene_blocks.size > 0)
		{
			for (uint8 itr = 0; itr < base_pane->model_scene_blocks.size; itr++)
			{
				rectangle2d og = model_viewports[itr];
				rectangle2d_scale(&og, scale_factor);
				base_pane->model_scene_blocks[itr]->ui_viewport = og;
			}
		}
		if (base_pane->player_blocks.size > 0)
		{
			s_player_block_reference* players = base_pane->player_blocks[0];
			//players->bottomleft = { -200 , 150 };
			//players->row_height = -344;
			//players->column_width = 558;
			point2d og_pos = { -107 , 59 };
			point2d_scale(&og_pos, scale_factor);
			players->bottomleft = og_pos;
			players->row_height = -177 * scale_factor;
			players->column_width = 279 * scale_factor;

		}

		//hacky wacky until skin tag definitions are added
		void* player_skins_signin_tag = tags::get_tag_fast(player_skins_datum_index);
		tag_block<s_hud_block_reference> hud_block = *reinterpret_cast<tag_block<s_hud_block_reference>*>((char*)(player_skins_signin_tag)+0x2C);
		//LOG_INFO_FUNC("checking offset {0:x} : {0:x}", hud_block.size, hud_block.data);


		s_hud_block_reference* base_hud_block = hud_block[0];
		rectangle2d og_bounds = { 6   , -6, -58	, 58 };
		rectangle2d_scale(&og_bounds, scale_factor);
		base_hud_block->bounds = og_bounds;

		const char* shader_tag_path = "ui\\hud\\shaders\\ui_small_emblem";
		datum shader_datum_index = tags::find_tag(blam_tag::tag_group_type::shader, shader_tag_path);

		// ui_medium_emblem is bugged for guest profiles so we use ui_small_emblem shader
		// but the bitmap quality is lowered
		// fixme
		if (!DATUM_IS_NONE(shader_datum_index))
		{
			base_hud_block->shader.TagIndex = shader_datum_index;
		}

	}

	
