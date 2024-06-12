#include "stdafx.h"

#include "network_channel.h"

c_network_channel* c_network_channel::get(int32 channel_index)
{
	c_network_channel* network_channels = *Memory::GetAddress<c_network_channel**>(0x4FADBC, 0x525274);
	return &network_channels[channel_index];
}

bool c_network_channel::get_network_address(network_address* address_out)
{
	bool result = false;
	if (channel_state >= _network_channel_state_2)
	{
		csmemcpy(address_out, &address, sizeof(network_address));
		result = true;
	}

	return result;
}