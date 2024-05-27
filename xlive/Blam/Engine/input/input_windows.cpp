#include "stdafx.h"
#include "input_windows.h"
#include "input_xinput.h"
#include "input_abstraction.h"
#include "shell/shell_windows.h"

extern xinput_device** g_xinput_devices;
s_input_globals* input_globals;

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
DIMOUSESTATE2* __cdecl input_get_mouse_state()
{
	return INVOKE(0x2E404, 0x0, input_get_mouse_state);
}
s_gamepad_input_state* input_get_gamepad(uint16 gamepad_index)
{
	//s_gamepad_input_state* global = Memory::GetAddress<s_gamepad_input_state*>(0x47A5C8);
	//return &global[gamepad_index];
	return &input_globals->gamepad_states[gamepad_index];
}
bool __cdecl input_has_gamepad_plugged(uint16 gamepad_index)
{
	return INVOKE_TYPE(0x2E186, 0x0, bool(__cdecl*)(uint16), gamepad_index);
}

s_gamepad_input_button_state* input_get_gamepad_state(uint16 gamepad_index)
{
	return INVOKE(0x2F433, 0x0, input_get_gamepad_state, gamepad_index);
}

void input_update_main_device_state()
{
	//remove dependence on g_main_controller_index

	uint8 device_index = 0;
	do
	{
		xinput_device* device = g_xinput_devices[device_index];
		XINPUT_STATE state;
		s_gamepad_input_state* gamepad = input_get_gamepad(device_index);
		uint32 error_code = ERROR_DEVICE_NOT_CONNECTED;


		if (!device
			|| (error_code = device->get_state(&state)) == ERROR_SEVERITY_SUCCESS
			|| error_code == ERROR_DEVICE_NOT_CONNECTED)
		{

			bool dev_connected = gamepad->connected;
			bool success = error_code == ERROR_SEVERITY_SUCCESS;
			bool initially_not_connected = !gamepad->connected;
			bool dev_state_joined = initially_not_connected && error_code == ERROR_SEVERITY_SUCCESS;
			bool dev_state_left = dev_connected && !success;

			gamepad->connected = error_code == ERROR_SEVERITY_SUCCESS;
			gamepad->m_device_just_joined = dev_state_joined;
			gamepad->m_device_just_left = dev_state_left;
		}

		uint32 device_flags = 0;
		if (gamepad->m_device_just_left)
			device_flags = 1;
		if (gamepad->m_device_just_joined)
			device_flags |= 0x2000;

		input_abstraction_handle_device_change(device_flags);
		device_index++;

	} while (device_index < 4); // only iterate over first 4 xinput devices

}

void input_update_gamepads(uint32 duration)
{
	bool input_handled = false;
	for (uint16 gamepad_index = 0; gamepad_index < k_number_of_controllers; gamepad_index++)
	{
		if (input_has_gamepad(gamepad_index, nullptr))
		{

			s_gamepad_input_button_state* gamepad_state = input_get_gamepad_state(gamepad_index);

			if (input_xinput_update_gamepad(gamepad_index, duration, gamepad_state))
			{
				//handled successfully for any device
				input_handled = true;
			}
		}
	}

	if (!input_handled)
		return;


	HWND g_window_handle = *Memory::GetAddress<HWND*>(0x46D9C4);

	if (input_handled
		&& g_window_handle == GetFocus()
		&& g_window_handle == GetForegroundWindow()
		&& !game_is_minimized())
	{
		if ((input_globals->field7D8 & 1) == 0)
		{
			input_globals->field7D8 |= 1u;
			//v26 = 0;
			input_globals->field7D0 = system_milliseconds();
			//v26 = 0xFFFFFFFF;
		}
		uint32 time = system_milliseconds();
		if (time - input_globals->field7D0 > 15000 || time - input_globals->field7D0 < 0)
		{
			input_globals->field7D0 = time;
			tagINPUT pInputs;
			//csmemset(&pInputs, 0, sizeof(pInputs));
			pInputs.type = INPUT_KEYBOARD;
			pInputs.ki.wVk = 0;
			pInputs.ki.wScan = 0;
			pInputs.ki.dwFlags = KEYEVENTF_KEYUP;
			pInputs.ki.dwExtraInfo = 0;
			SendInput(1, &pInputs, sizeof(pInputs));
		}
	}

}

void input_windows_apply_patches(void)
{
	input_globals = Memory::GetAddress<s_input_globals*>(0x479F50);

	PatchCall(Memory::GetAddress(0x2FA62), input_update_main_device_state);
	PatchCall(Memory::GetAddress(0x2FC2F), input_update_main_device_state);

	PatchCall(Memory::GetAddress(0x2FBD2), input_update_gamepads);
	return;
}