#include "stdafx.h"
#include "network_message_handler.h"
#include "simulation/simulation.h"
#include "simulation/simulation_view.h"
#include "network_messages_simulation_synchronous.h"

void c_network_message_handler::handle_synchronous_update(uint32 channel_idx, const s_network_message_synchronous_update* message)
{
	//INVOKE_TYPE(0x1E89D0, 0x0, void(__thiscall*)(c_network_message_handler*, uint32, const s_network_message_synchronous_update*), this, channel, message);

	c_network_channel* channel = c_network_channel::get(channel_idx);
	char* channel_name = channel ? channel->address.address.ipv6 : "null";
	c_simulation_view* view = simulation_get_remote_view_by_channel(channel_idx);

	if (view )
	{
		if(view->view_type() == _simulation_view_type_synchronous_to_remote_authority)
		{

			if (!view->handle_synchronous_update(message))
			{
				//"messages:synchronous-update: failed to handle #%d over channel '%s' with view mode %d/%d",				
				LOG_CRITICAL_NETWORK("messages:synchronous-update: failed to handle #{} over channel '{}' with view mode {}/{}",
					message->update.simulation_time,
					channel_name,
					view->get_view_establishment_mode(),
					view->get_view_establishment_identifier());

			}
		}
		else
		{
			//"messages:synchronous-update: view not authority for #%d over channel '%s' with view of type #%d",
			LOG_CRITICAL_NETWORK("messages:synchronous-update: view not authority for #{} over channel '{}' with view of type #{}",
				message->update.simulation_time,
				channel_name,
				view->view_type());
		}
	}
	else
	{
		//"messages:synchronous-update: no simulation view for #%d over channel '%s'",
		LOG_CRITICAL_NETWORK("messages:synchronous-update: no simulation view for #{} over channel '{}'",
			message->update.simulation_time,
			channel_name);
	}

}
void __declspec(naked) jmp_handle_synchronous_update() {
	__asm { jmp c_network_message_handler::handle_synchronous_update }
}

void c_network_message_handler::handle_synchronous_actions(uint32 channel_idx, const s_network_message_synchronous_actions* message)
{
	//INVOKE_TYPE(0x1E89F7, 0x0, void(__thiscall*)(c_network_message_handler*, uint32, const s_network_message_synchronous_actions*), this, channel, message);
	
	c_network_channel* channel = c_network_channel::get(channel_idx);
	char* channel_name = channel ? channel->address.address.ipv6 : "null";
	c_simulation_view* view = simulation_get_remote_view_by_channel(channel_idx);

	if (view)
	{
		if (view->view_type() == _simulation_view_type_synchronous_to_remote_client)
		{

			if (!view->handle_synchronous_actions(
				message->action_number,
				message->current_update_number,
				message->out_of_sync,
				message->user_flags,
				message->user_actions))
			{
				//"messages:synchronous-actions: failed to handle #%d over channel '%s' with view mode %d/%d",				
				LOG_CRITICAL_NETWORK("messages:synchronous-actions: failed to handle #{} over channel '{}' with view mode {}/{}",
					message->action_number,
					channel_name,
					view->get_view_establishment_mode(),
					view->get_view_establishment_identifier());

			}
		}
		else
		{
			//"messages:synchronous-actions: view not a client for #%d over channel '%s' with view of type #%d",
			LOG_CRITICAL_NETWORK("synchronous-actions: view not a client for #{} over channel '{}' with view of type #{}",
				message->action_number,
				channel_name,
				view->view_type());
		}
	}
	else
	{
		//"messages:synchronous-actions: no simulation view for #%d over channel '%s'",
		LOG_CRITICAL_NETWORK("messages:synchronous-actions: no simulation view for #{} over channel '{}'",
			message->action_number,
			channel_name);
	}
}
void __declspec(naked) jmp_handle_synchronous_actions() {
	__asm { jmp c_network_message_handler::handle_synchronous_actions }
}

void c_network_message_handler::handle_synchronous_join(uint32 channel_idx, const s_network_message_synchronous_join* message)
{
	//INVOKE_TYPE(0x1E8A31, 0x0, void(__thiscall*)(c_network_message_handler*, uint32, const s_network_message_synchronous_join*), this, channel, message);

	c_network_channel* channel = c_network_channel::get(channel_idx);
	char* channel_name = channel ? channel->address.address.ipv6 : "null";
	c_simulation_view* view = simulation_get_remote_view_by_channel(channel_idx);

	if (view)
	{
		if (view->view_type() == _simulation_view_type_synchronous_to_remote_authority)
		{

			if (!view->handle_synchronous_join(message->next_update_number))
			{
				//"messages:synchronous-join: failed to handle #%d over channel '%s' with view mode %d/%d",				
				LOG_CRITICAL_NETWORK("messages:synchronous-join: failed to handle #%d over channel '{}' with view mode {}/{}",
					message->next_update_number,
					channel_name,
					view->get_view_establishment_mode(),
					view->get_view_establishment_identifier());

			}
		}
		else
		{
			//"messages:synchronous-join: view not a client for #%d over channel '%s' (view type #%d)"
			LOG_CRITICAL_NETWORK("messages:synchronous-join: view not a client for #{} over channel '{}' (view type #{})",
				message->next_update_number,
				channel_name,
				view->view_type());
		}
	}
	else
	{
		//"messages:synchronous-join: no simulation view for #%d over channel '%s'",
		LOG_CRITICAL_NETWORK("messages:synchronous-join: no simulation view for #{} over channel '{}'",
			message->next_update_number,
			channel_name);
	}
}
void __declspec(naked) jmp_handle_synchronous_join() {
	__asm { jmp c_network_message_handler::handle_synchronous_join }
}

void c_network_message_handler::handle_synchronous_gamestate(uint32 channel_idx, const s_network_message_synchronous_gamestate* message, uint32 received_gamestate_size, uint8* received_gamestate_data)
{
	//INVOKE_TYPE(0x1E8A5A, 0x0, void(__thiscall*)(c_network_message_handler*, uint32, const s_network_message_synchronous_gamestate* , uint32,uint8*), this, channel, message, received_gamestate_size, received_gamestate_data);


	c_network_channel* channel = c_network_channel::get(channel_idx);
	char* channel_name = channel ? channel->address.address.ipv6 : "null";
	c_simulation_view* view = simulation_get_remote_view_by_channel(channel_idx);

	if (received_gamestate_size == message->gamestate_size)
	{
		if (view)
		{
			if (view->view_type() == _simulation_view_type_synchronous_to_remote_authority)
			{

				if (!view->handle_synchronous_gamestate(message->gamestate_offset,received_gamestate_size,received_gamestate_data))
				{
					//"messages:synchronous-gamestate: failed to handle (%d@%d) over channel '%s' with view mode %d/%d"				
					LOG_CRITICAL_NETWORK("messages:synchronous-gamestate: failed to handle ({}@{}) over channel '{}' with view mode {}/{}",
						message->gamestate_size,
						message->gamestate_offset,
						channel_name,
						view->get_view_establishment_mode(),
						view->get_view_establishment_identifier());

				}
			}
			else
			{
				//"messages:synchronous-gamestate: view not a client over channel '%s' with view of type #%d",
				LOG_CRITICAL_NETWORK("messages:synchronous-gamestate: view not a client over channel '{}' with view of type #{}",
					channel_name,
					view->view_type());
			}
		}
		else
		{
			//"messages:synchronous-gamestate: no simulation view on channel '%s'",
			LOG_CRITICAL_NETWORK("messages:synchronous-gamestate: no simulation view on channel '{}'", channel_name);
		}
	}
	else
	{
		//"messages:synchronous-gamestate: had corrupt sizes #%d!=#%d over channel '%s'"
		LOG_CRITICAL_NETWORK("messages:synchronous-gamestate: had corrupt sizes #{}!=#{} over channel '{}'",
			received_gamestate_size,
			message->gamestate_size,
			channel_name);
	}
}

void __declspec(naked) jmp_handle_synchronous_gamestate() {
	__asm { jmp c_network_message_handler::handle_synchronous_gamestate }
}


void c_network_message_handler::apply_patches()
{
	PatchCall(Memory::GetAddress(0x1E97D6), jmp_handle_synchronous_update);
	PatchCall(Memory::GetAddress(0x1E97FD), jmp_handle_synchronous_actions);
	PatchCall(Memory::GetAddress(0x1E9824), jmp_handle_synchronous_join);
	PatchCall(Memory::GetAddress(0x1E98B6), jmp_handle_synchronous_gamestate);

}
