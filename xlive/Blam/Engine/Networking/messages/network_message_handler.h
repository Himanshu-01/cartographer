#pragma once

struct s_network_message_synchronous_update;
struct s_network_message_synchronous_actions;
struct s_network_message_synchronous_join;
struct s_network_message_synchronous_gamestate;

class c_network_message_handler
{
	void handle_synchronous_update(uint32 channel, const s_network_message_synchronous_update* message);
	void handle_synchronous_actions(uint32 channel, const s_network_message_synchronous_actions* message);
	void handle_synchronous_join(uint32 channel, const s_network_message_synchronous_join* message);
	void handle_synchronous_gamestate(uint32 channel, const s_network_message_synchronous_gamestate* message, uint32 gamestate_size, uint8* gamestate_data);

public:
	//void handle_out_of_band_message();
	//void handle_channel_message();
	static void apply_patches();
};