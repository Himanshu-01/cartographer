#include "stdafx.h"
#include "simulation_view.h"
#include "simulation.h"
#include "Networking/Transport/network_observer.h"
#include "Networking/NetworkMessageTypeCollection.h"


void c_simulation_view::kill_view(e_simulation_view_reason reason)
{
	INVOKE_TYPE(0x1DF286, 0x0, void(__thiscall*)(c_simulation_view*, e_simulation_view_reason), this, reason);
}

bool c_simulation_view::synchronous_catchup_submit_update(s_network_message_synchronous_update* update)
{
	//TODO : rewrite
	return INVOKE_TYPE(0x1DE96E, 0x0, bool(__thiscall*)(c_simulation_view*, s_network_message_synchronous_update*), this, update);

	//uint8 buffer[UINT16_MAX];
	//uint32 out_size;

	//if (!synchronous_update_write_to_buffer(update, 0xFFFF, buffer, &out_size))
	//	return false;

	////*(_DWORD*)a3 = 0;
	////v6 = 0;
	////v3 = *a2;
	////a3[0] = 2;
	////a3[1] = out_size;
	////v6 = v3;
	////if (sub_781507((int)&this->field_9C, 8, (int)a3) == 0xFFFFFFFF
	////	|| sub_781507((int)&this->field_9C, out_size, (int)buffer) == 0xFFFFFFFF)
	////{
	////	return false;
	////}

	//++this->m_synchronous_catchup_buffers_count;
	//return true;
}

void c_simulation_view::synchronous_catchup_send_data()
{
	//TODO : rewrite
	INVOKE_TYPE(0x1DF2E7, 0x0, void(__thiscall*)(c_simulation_view*), this);
}

void c_simulation_view::dispatch_synchronous_update(simulation_update* host_update)
{
	//INVOKE_TYPE(0x1DFB6C, 0x0, void(__thiscall*)(c_simulation_view*, simulation_update*), this, host_update);
	s_network_message_synchronous_update packet;
	s_network_message_synchronous_update* authority_message = simulation_get_synchronous_message();
	//packet = authority_message;
	csmemcpy(&packet.update, &authority_message->update, sizeof(simulation_update));
	packet.simulation_bookkeeping_queue.duplicate(&authority_message->simulation_bookkeeping_queue);
	packet.game_simulation_queue.duplicate(&authority_message->game_simulation_queue);

	if (packet.game_simulation_queue.queued_count() > 0)
		LOG_TRACE_NETWORK(" {} game_simulation_queue  has count : {} ", __FUNCTION__, packet.game_simulation_queue.queued_count());
	if (packet.simulation_bookkeeping_queue.queued_count() > 0)
		LOG_TRACE_NETWORK(" {} simulation_bookkeeping_queue has count : {} ", __FUNCTION__, packet.simulation_bookkeeping_queue.queued_count());

	if (m_view_establishment_mode == _simulation_view_establishment_mode_active)
	{

		//if (m_observer_channel_index != 0xFFFFFFFF) {
		//	m_observer->send_message(2, m_observer_channel_index, false, _synchronous_update, sizeof(s_network_message_synchronous_update), &packet);
		//	//LOG_TRACE_FUNC("sent");
		//}

		send_message(_synchronous_update, sizeof(s_network_message_synchronous_update), &packet, false);

	}
	else if (synchronous_catchup_in_progress())
	{
		if (synchronous_catchup_submit_update(&packet))
		{
			// this handles synchronous_join
			// this is broken in h2v dont really need this
			// but clients will go brrr without this
			synchronous_catchup_send_data();
		}
		else
		{
			kill_view(_simulation_view_reason_catchup_fail);
		}
	}
}

void __declspec(naked) jmp_view_dispatch_synchronous_update() {
	__asm { jmp c_simulation_view::dispatch_synchronous_update }
}


void c_simulation_view::send_message(int32 message_type, uint32 message_size, void* data, bool out_of_band)
{
	if (!DATUM_IS_NONE(m_observer_channel_index))
	{
		m_observer->send_message(2, m_observer_channel_index, out_of_band, message_type, message_size, data);
	}
}


void c_simulation_view::apply_patches()
{
	PatchCall(Memory::GetAddress(0x1DC542), jmp_view_dispatch_synchronous_update);
}


