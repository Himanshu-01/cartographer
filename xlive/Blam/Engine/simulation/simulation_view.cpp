#include "stdafx.h"
#include "simulation_view.h"
#include "simulation.h"

#include "debug/debug_simulation_globals.h"
#include "Networking/Transport/network_observer.h"
#include "Networking/NetworkMessageTypeCollection.h"
#include "Networking/messages/network_messages_simulation_synchronous.h"


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

void c_simulation_view::get_view_description(static_string64* out_description)
{
	sprintf(out_description->get_buffer(), "v%d/m%d/%s", m_view_index, m_remote_machine_index, "unknown");
}

void c_simulation_view::dispatch_synchronous_update(struct simulation_update* host_update)
{
	//INVOKE_TYPE(0x1DFB6C, 0x0, void(__thiscall*)(c_simulation_view*, simulation_update*), this, host_update);
	s_network_message_synchronous_update packet;
	s_network_message_synchronous_update* authority_message = simulation_get_synchronous_message();
	//packet = authority_message;
	csmemcpy(&packet.update, &authority_message->update, sizeof(struct simulation_update));
	packet.simulation_bookkeeping_queue.duplicate(&authority_message->simulation_bookkeeping_queue);
	packet.game_simulation_queue.duplicate(&authority_message->game_simulation_queue);

	if (packet.game_simulation_queue.queued_count() > 0)
		LOG_TRACE_SIM(" {} game_simulation_queue  has count : {} ", __FUNCTION__, packet.game_simulation_queue.queued_count());
	if (packet.simulation_bookkeeping_queue.queued_count() > 0)
		LOG_TRACE_SIM(" {} simulation_bookkeeping_queue has count : {} ", __FUNCTION__, packet.simulation_bookkeeping_queue.queued_count());

	if (m_view_establishment_mode == _simulation_view_establishment_mode_active)
	{
		//if (m_observer_channel_index != NONE) {
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

void c_simulation_view::go_out_of_sync(void)
{
	ASSERT(exists());
	ASSERT(view_type() == _simulation_view_type_synchronous_to_remote_client);
	ASSERT(m_world != nullptr);

	static_string64 view_description;
	get_view_description(&view_description);

	//simulation:view: view %s is OUT OF SYNC at update/time [#%d]/[#%d]
	LOG_CRITICAL_SIM("simulation:view: view {} is OUT OF SYNC at update/time [#{}]/[#{}]",
		view_description.get_buffer(),
		m_world->get_next_update_number(),
		m_world->get_time());

	kill_view(_simulation_view_reason_out_of_sync);
}

bool c_simulation_view::handle_synchronous_update(const s_network_message_synchronous_update* update)
{
	//return INVOKE_TYPE(0x1DE7FB, 0x0, bool(__thiscall*)(c_simulation_view*, const s_network_message_synchronous_update*), this, update);


	ASSERT(exists());
	ASSERT(view_type() == _simulation_view_type_synchronous_to_remote_authority);
	ASSERT(m_world != nullptr);
	ASSERT(update != nullptr);

	if (m_world->time_running())
	{
		return m_world->handle_synchronous_update(update);
	}

	return false;


}

bool c_simulation_view::handle_synchronous_actions(int32 action_no, int32 update_no, bool is_out_of_sync, uint32 user_flags, player_action const* actions)
{
	//return INVOKE_TYPE(0x1DFB08, 0x0, char(__thiscall*)(c_simulation_view*, int32, int32, char, int, player_action const*), this, action_no, update_no, is_out_of_sync, user_flags, actions);

	ASSERT(exists());
	ASSERT(view_type() == _simulation_view_type_synchronous_to_remote_client);
	ASSERT(m_world != nullptr);

	static_string64 view_description;
	get_view_description(&view_description);

	if (is_out_of_sync)
	{

		LOG_CRITICAL_SIM("simulation:view: view {} synchronous-actions received msg to go out-of-sync", view_description.get_buffer());

		go_out_of_sync();
		// if client is oos , tell host to go oos as well
		this->m_world->go_out_of_sync();
		debug_simulation_notify_oos();

		return true;
	}

	else if (action_no >= m_synchronous_client_action_no
		&& update_no >= m_synchronous_client_update_no)
	{
		if (update_no < m_world->get_next_update_number())
		{
			if (m_world->is_active())
			{
				m_synchronous_client_action_no = action_no;
				m_synchronous_client_update_no = update_no;
				m_world->handle_synchronous_client_actions(&m_machine_identifier, user_flags, actions);
			}
			else
			{
				//simulation:view: view %s synchronous-actions discarded action/update [#%d]/[#%d], world not active
				LOG_CRITICAL_SIM("simulation:view: view {} synchronous-actions discarded action/update [#{}]/[#{}], world not active",
					view_description.get_buffer(),
					action_no,
					update_no);
			}
		}
		else
		{
			//simulation:view: view[% s] synchronous-actions discarded action / update[#%d] / [#%d] in the future (update/time [#%d]/[#%d])",
			LOG_CRITICAL_SIM("simulation:view: view[{}] synchronous-actions discarded action / update[#{}] / [#{}] in the future (update/time [#{}]/[#{}])",
				view_description.get_buffer(),
				action_no,
				update_no,
				m_world->get_next_update_number(),
				m_world->get_time());
		}
		return true;
	}
	else
	{
		//simulation:view: view %s synchronous-actions discarded action/update [#%d]/[#%d] < most recent [#%d]/[#%d]
		LOG_CRITICAL_SIM("simulation:view: view {} synchronous-actions discarded action/update [#{}]/[#{}] < most recent [#{}]/[#{}]",
			view_description.get_buffer(),
			action_no,
			update_no,
			m_synchronous_client_action_no,
			m_synchronous_client_update_no);
	}

	return false;
}

bool c_simulation_view::handle_synchronous_join(int32 next_update_number)
{
	//return INVOKE_TYPE(0x1DFA64, 0x0, char(__thiscall*)(c_simulation_view*, int), this,next_update_number);

	ASSERT(exists());
	ASSERT(view_type() == _simulation_view_type_synchronous_to_remote_authority);
	ASSERT(m_world != nullptr);
	ASSERT(next_update_number >= 0);


	if (m_world->get_state() == _simulation_world_state_joining
		&& !m_world->synchronous_gamestate_write_in_progress())
	{
		if (m_world->synchronous_gamestate_write_start())
		{
			//"simulation:view: synchronous-join received at #%d (currently  #%d)"
			LOG_CRITICAL_SIM("simulation:view: synchronous-join received at #{} (currently  #{})",
				next_update_number,
				m_world->get_time());

			m_world->time_start(next_update_number);
			m_world->time_set_immediate_update(true);

			return true;
		}
		else
		{
			//"simulation:view: synchronous-join unable to begin gamestate write"
			LOG_CRITICAL_SIM("simulation:view: synchronous-join unable to begin gamestate write");
			kill_view(_simulation_view_reason_catchup_fail);

		}
	}
	else
	{
		//"simulation:view: synchronous-join rejected, world is in state #%d (gamestate-write %s)"
		char* state = m_world->synchronous_gamestate_write_in_progress() ? "in-progress" : "not-in-progress";
		LOG_CRITICAL_SIM("simulation:view: synchronous-join rejected, world is in state #{} (gamestate-write {})",
			m_world->get_state(),
			state);

	}

	return false;
}

bool c_simulation_view::handle_synchronous_gamestate(int32 gamestate_offset, int32 message_gamestate_size, void* gamestate_data)
{
	//return INVOKE_TYPE(0x1DFAB6, 0x0, char(__thiscall*)(c_simulation_view*, int32, int32, void*), this, gamestate_offset, message_gamestate_size, gamestate_data);

	ASSERT(exists());
	ASSERT(view_type() == _simulation_view_type_synchronous_to_remote_authority);
	ASSERT(m_world != nullptr);


	bool success = false;
	if (!m_world->synchronous_gamestate_write_in_progress())
	{
		//"simulation:view: synchronous-gamestate rejected, world has no gamestate-write in progress (state #%d)"
		LOG_CRITICAL_SIM("simulation:view: synchronous-gamestate rejected, world has no gamestate-write in progress (state #{})",
			m_world->get_state());

		return false;
	}

	if (message_gamestate_size <= 0)
	{
		success = m_world->synchronous_gamestate_decompress_and_load(gamestate_offset);
		if (success)
		{
			//"simulation:world:gamestate: successfully decompressed gamestate (%d -> %d bytes, time #%d)",
			LOG_CRITICAL_SIM("simulation:world:gamestate: successfully decompressed gamestate ({} -> {} bytes, time #{})",
				gamestate_offset,
				123456, // just a filler
				m_world->get_time());
		}
		else
		{
			//"simulation:world:gamestate: gamestate didn't decompress successfully");
			LOG_CRITICAL_SIM("simulation:world:gamestate: gamestate didn't decompress successfully");
		}
	}
	else
	{
		success = m_world->synchronous_gamestate_write_chunk(gamestate_offset, message_gamestate_size, gamestate_data);

		if(!success)
		{
			//"simulation:view: synchronous-gamestate block %d@%d rejected, gamestate write has failed",
			LOG_CRITICAL_SIM("simulation:view: synchronous-gamestate block {}@{} rejected, gamestate write has failed",
				message_gamestate_size,
				gamestate_offset);
		}
	}

	if (success)
		return true;

	kill_view(_simulation_view_reason_catchup_fail);
	return false;
}

void c_simulation_view::apply_patches()
{
	PatchCall(Memory::GetAddress(0x1DC542), jmp_view_dispatch_synchronous_update);
}


