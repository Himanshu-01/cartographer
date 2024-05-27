#pragma once


#define K_NUMBER_OF_WINDOWS_INPUT_VIRTUAL_CODES 256

class input_device
{
public:
	virtual void nullsub_0(void) = 0;
	virtual void nullsub_1(void) = 0;
	virtual void update_state_error_checking(void) = 0;
	virtual uint32 get_state(XINPUT_STATE* state) = 0;
	virtual void set_state(XINPUT_VIBRATION* state) = 0;
	virtual void update_state(void) = 0;

protected:
	uint32 controller_index;
	uint32 controller_state;
	XINPUT_STATE state;
};

struct rumble_state
{
	int16 left_rumble;
	int16 right_rumble;
};

#pragma pack(push,1)

struct s_key_state
{
	uint8 gap[0x8];
};
CHECK_STRUCT_SIZE(s_key_state, 0x8);


enum e_xinput_gamepad_buttons
{
	_xinput_gamepad_dpad_up = 0x0,
	_xinput_gamepad_dpad_down = 0x1,
	_xinput_gamepad_dpad_left = 0x2,
	_xinput_gamepad_dpad_right = 0x3,
	_xinput_gamepad_start = 0x4,
	_xinput_gamepad_back = 0x5,
	_xinput_gamepad_left_thumb = 0x6,
	_xinput_gamepad_right_thumb = 0x7,
	_xinput_gamepad_left_shoulder = 0x8,
	_xinput_gamepad_right_shoulder = 0x9,
	_xinput_gamepad_a = 0xa,
	_xinput_gamepad_b = 0xb,
	_xinput_gamepad_x = 0xc,
	_xinput_gamepad_y = 0xd,
	_xinput_gamepad_left_trigger = 0xe,
	_xinput_gamepad_right_trigger = 0xf,

	K_NUMBER_OF_XINPUT_BUTTONS = 0xE,
};

struct s_gamepad_input_button_state
{
	uint8 trigger_msec_down[2];
	uint8 max_trigger_msec_down[2];
	uint8 trigger_button_frames_down[2];
	uint8 button_frames_down[K_NUMBER_OF_XINPUT_BUTTONS];
	uint16 trigger_button_msec_down[2];
	uint16 button_msec_down[K_NUMBER_OF_XINPUT_BUTTONS];
	point2d thumb_left;
	point2d thumb_right;
};
CHECK_STRUCT_SIZE(s_gamepad_input_button_state, 0x3C);


struct s_gamepad_input_state
{
	bool connected;
	bool m_device_just_joined;
	bool m_device_just_left;
	uint8 gap_3;
	s_gamepad_input_button_state state;
};
CHECK_STRUCT_SIZE(s_gamepad_input_state, 0x40);

struct s_keyboard_input_state
{
	uint8 frames_down[K_NUMBER_OF_WINDOWS_INPUT_VIRTUAL_CODES];
	uint16 msec_down[K_NUMBER_OF_WINDOWS_INPUT_VIRTUAL_CODES];
	bool key_bool[K_NUMBER_OF_WINDOWS_INPUT_VIRTUAL_CODES];
};
CHECK_STRUCT_SIZE(s_keyboard_input_state, 0x400);


class c_input_dx9_mouse_cursor; //TODO

struct s_input_globals
{
	bool initialized;
	bool mouse_acquired;
	bool input_suppressed;
	bool feedback_suppress;
	uint32 update_time;
	uint32 update_msec;
	IDirectInput8A* dinput;
	s_keyboard_input_state keyboard;
	int16 buffered_key_read_index;
	int16 buffered_key_read_count;
	s_key_state buffered_keys[64];
	LPDIRECTINPUTDEVICE8A mouse_dinput_device;
	bool mouse_show;
	uint8 gap_619[3];
	uint32 field_61C;
	DIMOUSESTATE2 mouse_state;
	int16 mouse_buttons[8];
	DIMOUSESTATE2 suppressed_mouse_state;
	uint8 gap_658[24];
	uint32 mouse_cursor_state;
	void* mouse_cursor_dx9;
	s_gamepad_input_state gamepad_states[4];
	s_gamepad_input_button_state suppressed_gamepad_state;
	XINPUT_VIBRATION rumble_states[4];
	uint32 main_controller_index;
	bool hardware_device_changed;
	char gap[3];
	int debug_simulate_gamepad;
	int field7D0;
	int field7D8;
};
CHECK_STRUCT_SIZE(s_input_globals, 0x7D8);

#pragma pack(pop)




extern s_input_globals* input_globals;



rumble_state* controller_rumble_state_get(int32 controller_index);

int32* hs_debug_simulate_gamepad_global_get(void);

bool* input_suppress_global_get(void);

char  input_has_gamepad(__int16 gamepad_index, bool* a2);
DIMOUSESTATE2* __cdecl input_get_mouse_state();

s_gamepad_input_state*  input_get_gamepad(uint16 gamepad_index);
bool __cdecl input_has_gamepad_plugged(uint16 gamepad_index);
s_gamepad_input_button_state*  input_get_gamepad_state(uint16 gamepad_index);


void input_windows_apply_patches(void);