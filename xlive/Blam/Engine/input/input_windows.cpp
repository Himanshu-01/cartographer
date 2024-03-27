#include "stdafx.h"
#include "input_windows.h"

rumble_state* controller_rumble_state_get(int32 controller_index)
{
	return Memory::GetAddress<rumble_state*>(0x47A704);
}

int32* hs_debug_simulate_gamepad_global_get(void)
{
	return Memory::GetAddress<int32*>(0x47A71C);
}

bool* input_suppress_global_get(void)
{
	return Memory::GetAddress<bool*>(0x479F52);
}

char  input_has_gamepad(__int16 gamepad_index, bool* a2)
{
	return INVOKE_TYPE(0x2F3CD, 0x0, char(__cdecl*)(__int16, bool*), gamepad_index, a2);
}