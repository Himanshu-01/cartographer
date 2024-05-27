#include "stdafx.h"
#include "user_interface_utilities.h"
#include "user_interface_widget_window.h"

datum __cdecl get_wgit_tag_datum_from_menu_id(e_user_interface_screen_id screen_id)
{
	return INVOKE(0x20C701, 0x0, get_wgit_tag_datum_from_menu_id, screen_id);
}
