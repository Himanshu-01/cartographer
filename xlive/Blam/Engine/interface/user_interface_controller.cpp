#include "Util/Memory.h"
#include "user_interface_controller.h"

s_user_interface_controller_globals* get_user_interface_controller_globals(void)
{
	return Memory::GetAddress<s_user_interface_controller_globals*>(0x96C858);
}
