#pragma once
#include "machine_id.h"
#include "simulation_players.h"

#include "memory/bitstream.h"
#include "Networking/Session/NetworkSession.h"

struct simulation_machine_update
{
	uint32 machine_valid_mask;
	s_machine_identifier identifiers[NETWORK_SESSION_PEERS_MAX];
};

void __cdecl simulation_player_update_encode(c_bitstream* stream, const simulation_player_update* player_update);
bool __cdecl simulation_player_update_decode(c_bitstream* stream, simulation_player_update* player_update);

struct s_network_message_synchronous_update;
void __cdecl synchronous_update_encode(c_bitstream* stream, s_network_message_synchronous_update* update);
bool __cdecl synchronous_update_decode(c_bitstream* stream, s_network_message_synchronous_update* update);
bool __cdecl synchronous_update_read_from_buffer(s_network_message_synchronous_update* message, uint32 data_len, uint8* buffer);
bool __cdecl synchronous_update_write_to_buffer(s_network_message_synchronous_update* update, uint32 data_len, uint8* buffer, uint32* out_size);
