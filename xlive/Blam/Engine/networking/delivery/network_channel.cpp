#include "stdafx.h"
#include "network_channel.h"

s_network_channel* s_network_channel::get(int32 channel_index)
{
	s_network_channel* network_channels = *Memory::GetAddress<s_network_channel**>(0x4FADBC, 0x525274);
	return &network_channels[channel_index];
}

bool s_network_channel::get_network_address(transport_address* address_out)
{
	bool result = false;
	if (channel_state >= _network_channel_state_2)
	{
		csmemcpy(address_out, &address, sizeof(transport_address));
		result = true;
	}

	return result;
}

void c_network_channel_simulation_interface::set_established(
	bool established)
{
	ASSERT(m_initialized);
	m_established = established;

	return;
}

bool c_network_channel_simulation_interface::established(
	void) const
{
	ASSERT(m_initialized);

	return m_established;
}