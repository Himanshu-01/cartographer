#include "stdafx.h"
#include "simulation_encoding.h"
#include "simulation.h"
#include "game/game_time.h"
#include "Networking/messages/network_messages_simulation_synchronous.h"

//typedef simulation_update s_network_message_synchronous_update;

void __cdecl simulation_player_update_encode(c_bitstream* stream, const simulation_player_update* player_update)
{
	INVOKE(0x1E06AB, 0x1C7B6B, simulation_player_update_encode, stream, player_update);
	return;
}

bool __cdecl simulation_player_update_decode(c_bitstream* stream, simulation_player_update* player_update)
{
	return INVOKE(0x1E078A, 0x1C7C4A, simulation_player_update_decode, stream, player_update);
}


void __cdecl simulation_encoding_encode_action(c_bitstream* stream, void* action)
{
	INVOKE(0x1DFE4C, 0x0, simulation_encoding_encode_action, stream, action);
}

bool __cdecl simulation_encoding_decode_action(c_bitstream* a1, void* a2)
{
	return INVOKE(0x1E01CB, 0x0, simulation_encoding_decode_action, a1, a2);
}

void __cdecl simulation_encoding_encode_machines_data(c_bitstream* a1, void* a2)
{
	INVOKE(0x1E08E7, 0x0, simulation_encoding_encode_machines_data, a1, a2);
}

bool __cdecl simulation_encoding_decode_machines_data(c_bitstream* a1, void* a2)
{
	return INVOKE(0x1E0935, 0x0, simulation_encoding_decode_machines_data, a1, a2);
}

void __cdecl simulation_update_encode(c_bitstream* stream, simulation_update* update)
{
	//INVOKE(0x1E0998, 0x0, simulation_update_encode, stream, update);

	stream->write_integer("update-number", update->simulation_time, 0x20);

	//adding missing simulation_in_progress
	stream->write_bool("simulation_in_progress", update->simulation_in_progress);

	stream->write_integer("player-flags", update->player_action_mask, 0x10);
	int32 v2 = 0;
	player_action* v3 = update->player_actions;
	do
	{
		//if (((1 << v2) & update->player_action_mask) != 0)
		if (TEST_BIT(update->player_action_mask, v2))
			simulation_encoding_encode_action(stream, v3);
		++v2;
		++v3;
	} while (v2 < 0x10);

	// why is unit/actor control data never sent?????

	stream->write_bool("machine-update-exists", update->machine_update_valid);
	if (update->machine_update_valid)
		simulation_encoding_encode_machines_data(stream, &update->machine_update);
	stream->write_integer("player-update-count", update->player_update_count, 5);


	int32 v4 = 0;
	if (update->player_update_count > 0)
	{
		simulation_player_update* v5 = update->player_updates;
		do
		{
			simulation_player_update_encode(stream, v5);
			++v4;
			++v5;
		} while (v4 < update->player_update_count);
	}
	stream->write_bool("flush-gamestate", update->flush_gamestate);
	stream->write_integer("verify-game-time", update->game_time_ticks, 0x20);
	stream->write_integer("verify-random", update->random_seed, 0x20);


}

bool __cdecl simulation_update_decode(c_bitstream* stream, simulation_update* update)
{
	//return INVOKE(0x1E0AA2, 0x0, simulation_update_decode, stream, update);

	bool v2 = 1;
	update->simulation_time = stream->read_integer("update-number", 0x20);

	//added back
	update->simulation_in_progress = stream->read_bool("simulation_in_progress");

	update->player_action_mask = stream->read_integer("player-flags", 0x10);

	int32 v4 = 0;
	player_action* a2a = update->player_actions;
	do
	{
		//if (((1 << v4) & update->player_action_mask) != 0)
		if (TEST_BIT(update->player_action_mask, v4))
			v2 = v2 && simulation_encoding_decode_action(stream, a2a);
		++a2a;
		++v4;
	} while (v4 < 0x10);


	bool machine_has_update = stream->read_bool("machine-update-exists");
	update->machine_update_valid = machine_has_update;
	//LOBYTE(update->machine_update_exists) = v5;
	if (machine_has_update)
		v2 = v2 && simulation_encoding_decode_machines_data(stream, &update->machine_update);

	int32 v6 = stream->read_integer("player-update-count", 5);
	update->player_update_count = v6;
	if (v6 < 0 || v6 > 0x40)
	{
		v2 = 0;
	}
	else
	{
		int32 v7 = 0;
		if (v6 > 0)
		{
			simulation_player_update* a2b = update->player_updates;
			do
			{
				v2 = v2 && simulation_player_update_decode(stream, a2b++);
				++v7;
			} while (v7 < update->player_update_count);
		}
	}
	update->flush_gamestate = stream->read_bool("flush-gamestate");
	update->game_time_ticks = stream->read_integer("verify-game-time", 0x20);
	update->random_seed = stream->read_integer("verify-random", 0x20);

	return v2
		&& !stream->error_occured()
		&& (update->game_time_ticks >= 0)
		&& (update->simulation_time >= 0);

}

void __cdecl synchronous_update_encode(c_bitstream* stream, s_network_message_synchronous_update* host_update)
{
	// really need to rewrite some of the stuffs so i dont have to hook into encode function to transfer queues
	// 
	//update->simulation_bookkeeping_queue.transfer_elements(simulation_get_world()->queue_get(_simulation_queue_bookkeeping));
	//update->game_simulation_queue.transfer_elements(simulation_get_world()->queue_get(_simulation_queue));
	simulation_update_encode(stream, &host_update->update);

	//added simulation_queue encoding
	host_update->simulation_bookkeeping_queue.encode(stream);
	host_update->game_simulation_queue.encode(stream);
}

bool __cdecl synchronous_update_decode(c_bitstream* stream, s_network_message_synchronous_update* out_update)
{

	bool state = simulation_update_decode(stream, &out_update->update);
	if (!state)
	{
		LOG_CRITICAL_NETWORK("failed in decoding simulation_update");
	}

	//added simulation_queue decoding
	//out_update->simulation_bookkeeping_queue.clear();
	//out_update->game_simulation_queue.clear();

	out_update->simulation_bookkeeping_queue.decode(stream);
	out_update->game_simulation_queue.decode(stream);

	return state;
}



bool __cdecl synchronous_update_read_from_buffer(s_network_message_synchronous_update* message, uint32 data_len, uint8* buffer)
{
	//return INVOKE_TYPE(0x1AE03F, 0x0, char(__cdecl*)(s_network_message_synchronous_update*, uint32, uint8*), update, data_len, buffer);

	c_bitstream stream(buffer, data_len);
	csmemset(message, 0, sizeof(s_network_message_synchronous_update));
	stream.begin_reading();
	if (!synchronous_update_decode(&stream, message))
	{
		LOG_CRITICAL_NETWORK("failed in decoding synchronous_update");
		return false;
	}
	stream.finish_reading();
	return true;
}

bool __cdecl synchronous_update_write_to_buffer(s_network_message_synchronous_update* update, uint32 data_len, uint8* buffer, uint32* out_size)
{
	//return INVOKE_TYPE(0x1ADFBA, 0x0, char(__cdecl*)(s_network_message_synchronous_update*, uint32, uint8*, uint32*), update, data_len, buffer, out_size);

	c_bitstream stream(buffer, data_len);
	stream.begin_writing(k_bitstream_default_alignment);
	synchronous_update_encode(&stream, update);
	*out_size = stream.get_space_used_in_bytes();
	stream.finish_writing(NULL);

	//TODO:  add decode buffer check here for consistency and return false if it fails

	return true;

}