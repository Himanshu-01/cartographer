#include "stdafx.h"
#include "network_messages_simulation_synchronous.h"

/* public code */

void __cdecl network_message_types_register_simulation_synchronous(c_network_message_type_collection* message_collection)
{
	INVOKE(0x1ED397, 0x1CDD50, network_message_types_register_simulation_synchronous, message_collection);
	return;
}

void network_messages_simulation_synchronous_apply_patches(void)
{
	//patching network_message_types_register_simulation_synchronous
	WriteValue<int32>(Memory::GetAddress(0x1ED3AB + 1), sizeof(struct s_network_message_synchronous_update));
	WriteValue<int32>(Memory::GetAddress(0x1ED3B0 + 1), sizeof(struct s_network_message_synchronous_update));


	//patching simulation_view::synchronous_catchup_send_data
	WriteValue<uint32>(Memory::GetAddress(0x1DF3DF + 1), sizeof(s_network_message_synchronous_update));
	WriteValue<uint32>(Memory::GetAddress(0x1DF39E + 1), sizeof(s_network_message_synchronous_update));
	return;
}
