#include "stdafx.h"
#include "screens_patches.h"

#include "screen_esrb_warning.h"
#include "screen_xbox_live_task_progress_dialog.h"
#include "screen_4way_signin.h"
#include "screen_multiplayer_pregame_lobby.h"
#include "screen_squad_settings.h"
#include "screen_loading_progress.h"

void screens_apply_patches_on_map_load()
{
	c_xbox_live_task_progress_menu::apply_patches();
	c_screen_4way_signin::apply_patches();
	c_screen_multiplayer_pregame_lobby::apply_tag_patches();
	c_screen_loading_progress::apply_tag_patches();
}

void screens_apply_patches()
{
	c_screen_esrb_warning::apply_patches();
	c_screen_squad_settings::apply_patches();
	c_screen_loading_progress::apply_patches();
}
