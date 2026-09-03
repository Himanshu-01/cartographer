#include "stdafx.h"
#include "simulation_view.h"

#include "simulation.h"
#include "simulation_world.h"
#include "networking/network_event.h"
#include "networking/network_time.h"
#include "networking/messages/network_messages_simulation.h"
#include "networking/messages/network_messages_simulation_synchronous.h"
#include "networking/session/network_observer.h"
#include "networking/replication/replication_game_results.h"

/* enums */

enum e_synchronous_catchup_block_header
{
	_synchronous_catchup_join_initiate_block,
	_synchronous_catchup_gamestate_block,
	_synchronous_catchup_update_block,
};

/* structures */

struct s_synchronous_block_header
{
	e_synchronous_catchup_block_header block_type;
	uint32 block_size;
};
ASSERT_STRUCT_SIZE(s_synchronous_block_header, 8);


/* globals */

const char* g_simulation_view_reason_strings[k_simulation_view_reason_count]
{
	"none",
	"disconnected",
	"out-of-sync",
	"failed-to-join",
	"blocking",
	"catchup-fail",
	"ended",
	"mode-error",
	"player-error",
	"replication-entity",
	"replication-event",
	"replication-game-results",
	"no-longer-authority"
};

static char g_view_description_buffer[32];
static const char* g_remote_machine_name = "unknown";


/* public code */

char const* c_simulation_view::get_view_description(
	void) const
{
	csprintf(g_view_description_buffer, 
		0x20,
		"v%d/m%d/%s",
		m_world_view_index,
		m_remote_machine_index,
		g_remote_machine_name);

	return g_view_description_buffer;
}

char const* c_simulation_view::get_death_reason_string(
	uint32 death_reason) const
{
	const char* result = "<unknown>";
	if (VALID_INDEX(death_reason, k_simulation_view_reason_count))
	{
		result = g_simulation_view_reason_strings[death_reason];
	}

	return result;
}



void c_simulation_view::send_message(
	e_network_message_type message_type,
	int32 message_size,
	const void* message_payload)
{
	if (m_observer_channel_index != NONE)
	{
		ASSERT(m_observer != NULL);

		m_observer->send_message(_network_observer_owner_simulation, m_observer_channel_index, false, message_type, message_size, message_payload);
	}
	else
	{
		ASSERT(is_dead(NULL));
	}

	return;
}

void c_simulation_view::send_establishment_message(void)
{
	s_network_message_view_establishment message;
	csmemset(&message, 0, sizeof(message));

	message.establishment_mode = m_view_establishment_mode;
	message.establishment_identifier = m_view_establishment_identifier;
	send_message(_network_message_type_view_establishment, sizeof(message), &message);
}


bool c_simulation_view::observer_channel_backlogged(e_network_message_type message_type)
{
	bool backlogged;
	if (m_observer_channel_index != NONE)
	{
		ASSERT(m_observer != NULL);

		backlogged = m_observer->observer_channel_backlogged(
			_network_observer_owner_simulation,
			m_observer_channel_index,
			message_type);
	}
	else
	{
		ASSERT(is_dead(NULL));
		backlogged = true;
	}

	return backlogged;
}

void c_simulation_view::observer_channel_set_waiting_on_backlog(
	e_network_message_type message_type)
{
	//INVOKE_TYPE(0x1DE77C, 0x0, void(__thiscall*)(c_simulation_view*, e_network_message_type), this, message_type);
	if (m_observer_channel_index != NONE)
	{
		ASSERT(m_observer != NULL);

		m_observer->observer_channel_set_waiting_on_backlog(
			_network_observer_owner_simulation,
			m_observer_channel_index,
			message_type);
	}

	return;
}

void c_simulation_view::kill_view(
	e_simulation_view_reason death_reason)
{
	if (is_dead(NULL))
	{
		event(
			_event_message,
			"simulation:view: view %s dying (%s) in mode %d/%d",
			get_view_description(),
			get_death_reason_string(death_reason),
			m_view_establishment_mode,
			m_view_establishment_identifier
		);

		set_view_establishment(_simulation_view_establishment_mode_none, NONE);
		m_view_death_reason = death_reason;
	}

	return;
}

void c_simulation_view::set_view_establishment(
	e_simulation_view_establishment_mode establishment_mode,
	int32 establishment_identifier)
{
	// Param validation
	ASSERT(exists());
	ASSERT(establishment_mode >= 0 && establishment_mode < k_simulation_view_establishment_mode_count);

	// Class validation
	ASSERT(m_world != NULL);
	ASSERT(m_channel_index != NONE);


	bool valid_mode;

	if (is_dead(NULL))
	{
		ASSERT(establishment_mode == _simulation_view_establishment_mode_none);
	}

	if (establishment_mode >= _simulation_view_establishment_mode_established)
	{
		if (establishment_mode == _simulation_view_establishment_mode_established)
		{
			valid_mode = establishment_identifier >= 0;
		}
#ifdef USE_H3_DROP_SIMULATION_TO_JOINING
		//h3 addon
		else if (m_view_establishment_mode == _simulation_view_establishment_mode_active
			&& establishment_mode == _simulation_view_establishment_mode_joining)
		{
			event(_event_message,
				"networking:simulation:view: we are currently active, but are going back to joining (this is allowed)"
			);
			valid_mode = establishment_identifier >= 0;
		}
#endif
		else
		{
			valid_mode =
				establishment_identifier == m_view_establishment_identifier &&
				establishment_mode == m_view_establishment_mode + 1;
		}
	}
	else
	{
		valid_mode = establishment_identifier == NONE;
	}

	if (!valid_mode)
	{
		event(
			_event_error,
			"simulation:view: view %s mode logic error: not uniformly ascending while above established (%d/%d -> %d/%d)",
			get_view_description(),
			m_view_establishment_mode,
			m_view_establishment_identifier,
			establishment_mode,
			establishment_identifier
		);
		kill_view(_simulation_view_reason_mode_error);
	}
	else
	{
		if (m_view_establishment_mode == establishment_mode &&
			m_view_establishment_identifier == establishment_identifier)
		{
			event(
				_event_status,
				"simulation:view: view %s suppressing duplicate mode %d/%d",
				get_view_description(),
				m_view_establishment_mode,
				m_view_establishment_identifier
			);
		}
		else
		{
			event(
				_event_status,
				"simulation:view: view %s mode change %d/%d -> %d/%d",
				get_view_description(),
				m_view_establishment_mode,
				m_view_establishment_identifier,
				establishment_mode,
				establishment_identifier
			);

			m_view_establishment_mode = establishment_mode;
			m_view_establishment_identifier = establishment_identifier;

			send_establishment_message();
			update_view_activation_state();
		}
	}

	return;
}

void c_simulation_view::update(void)
{
	ASSERT(exists());
	ASSERT(m_world != NULL);

	INVOKE_TYPE(0x1DF78A, 0x0, void(__thiscall*)(c_simulation_view*), this);
	return;
}


bool c_simulation_view::handle_synchronous_update(
	struct simulation_update const* update)
{
	ASSERT(exists());
	ASSERT(m_view_type == _simulation_view_type_synchronous_to_remote_authority);
	ASSERT(m_world != NULL);
	ASSERT(update);

	return m_world->time_running() ? m_world->handle_synchronous_update(update) : false;
}

bool c_simulation_view::handle_remote_establishment(
	e_simulation_view_establishment_mode remote_establishment_mode,
	int32 remote_establishment_identifier)
{
	ASSERT(exists());
	ASSERT(VALID_INDEX(remote_establishment_mode, k_simulation_view_establishment_mode_count));

	bool result = false;

	if (m_world
		&& m_channel_index != NONE)
	{
		e_simulation_view_establishment_mode old_remote_establishment_mode = m_remote_establishment_mode;
		int32 old_remote_establishment_identifier = m_remote_establishment_identifier;

		m_remote_establishment_mode = remote_establishment_mode;
		m_remote_establishment_identifier = remote_establishment_identifier;

		if (is_dead(NULL))
		{
			event(
				_event_message,
				"simulation:view: view %s remote %s mode %d/%d -> %d/%d but we are dead (currently %d/%d)",
				get_view_description(),
				is_client_view() ? "client" : "authority",
				old_remote_establishment_mode,
				old_remote_establishment_identifier,
				m_remote_establishment_mode,
				m_remote_establishment_identifier,
				m_view_establishment_mode,
				m_view_establishment_identifier
			);
		}
		else
		{
			//remote client start
			if (is_client_view())
			{

				//ready to establish
				if (m_view_establishment_mode < _simulation_view_establishment_mode_established)
				{
					ASSERT(m_view_establishment_identifier == NONE);

					if (m_remote_establishment_mode < _simulation_view_establishment_mode_established
						&& m_remote_establishment_identifier == NONE)
					{
						event(
							_event_message,
							"simulation:view: view %s remote client mode %d/%d -> %d/%d is non-established (authority %d/%d)",
							get_view_description(),
							old_remote_establishment_mode,
							old_remote_establishment_identifier,
							m_remote_establishment_mode,
							m_remote_establishment_identifier,
							m_view_establishment_mode,
							m_view_establishment_identifier
						);
					}
					else
					{
						event(
							_event_message,
							"simulation:view: view %s remote client mode %d/%d -> %d/%d is erroneously established (authority %d/%d), resending",
							get_view_description(),
							old_remote_establishment_mode,
							old_remote_establishment_identifier,
							m_remote_establishment_mode,
							m_remote_establishment_identifier,
							m_view_establishment_mode,
							m_view_establishment_identifier
						);

						send_establishment_message();
					}
				}

				// establised
				else if (m_view_establishment_mode == _simulation_view_establishment_mode_established)
				{
					ASSERT(m_view_establishment_identifier >= 0);

					if (m_remote_establishment_mode == _simulation_view_establishment_mode_established
						&& m_remote_establishment_identifier == m_view_establishment_identifier)
					{

						event(
							_event_message,
							"simulation:view: view %s remote client mode %d/%d -> %d/%d acknowledging establishment (authority %d/%d)",
							get_view_description(),
							old_remote_establishment_mode,
							old_remote_establishment_identifier,
							m_remote_establishment_mode,
							m_remote_establishment_identifier,
							m_view_establishment_mode,
							m_view_establishment_identifier
						);
					}
					else if (old_remote_establishment_mode != _simulation_view_establishment_mode_established
						&& old_remote_establishment_identifier != m_view_establishment_identifier)
					{

						if (m_remote_establishment_mode < _simulation_view_establishment_mode_connected)
						{
							event(
								_event_message,
								"simulation:view: view %s remote client mode %d/%d -> %d/%d disconnects client (authority %d/%d), disconnecting",
								get_view_description(),
								old_remote_establishment_mode,
								old_remote_establishment_identifier,
								m_remote_establishment_mode,
								m_remote_establishment_identifier,
								m_view_establishment_mode,
								m_view_establishment_identifier
							);
							set_view_establishment(_simulation_view_establishment_mode_connected, NONE);
						}
						else
						{
							event(
								_event_message,
								"simulation:view: view %s remote client mode %d/%d -> %d/%d does not establish (authority %d/%d), waiting",
								get_view_description(),
								old_remote_establishment_mode,
								old_remote_establishment_identifier,
								m_remote_establishment_mode,
								m_remote_establishment_identifier,
								m_view_establishment_mode,
								m_view_establishment_identifier
							);
							send_establishment_message();
						}
					}
					else
					{
						event(
							_event_warning,
							"simulation:view: view %s remote client mode %d/%d -> %d/%d abort establishment (authority %d/%d), dying",
							get_view_description(),
							_simulation_view_establishment_mode_established,
							old_remote_establishment_identifier,
							m_remote_establishment_mode,
							m_remote_establishment_identifier,
							m_view_establishment_mode,
							m_view_establishment_identifier
						);
						kill_view(_simulation_view_reason_mode_error);
					}
				}
				//check for errors
				else if (m_remote_establishment_mode != _simulation_view_establishment_mode_none)
				{
					ASSERT(m_view_establishment_identifier >= 0);

					//acknowledge increment
					if (m_remote_establishment_identifier == m_view_establishment_identifier
						&& m_remote_establishment_mode > _simulation_view_establishment_mode_established
						&& m_remote_establishment_mode <= m_view_establishment_mode
						&& m_remote_establishment_mode == old_remote_establishment_mode + 1)
					{
						event(
							_event_message,
							"simulation:view: view %s remote client mode %d/%d -> %d/%d acknowledged (authority %d/%d)",
							get_view_description(),
							old_remote_establishment_mode,
							old_remote_establishment_identifier,
							m_remote_establishment_mode,
							m_remote_establishment_identifier,
							m_view_establishment_mode,
							m_view_establishment_identifier
						);
					}
#ifdef USE_H3_DROP_SIMULATION_TO_JOINING
					//h3 addon
					//acknowledge active to joining
					else if (m_remote_establishment_identifier == m_view_establishment_identifier
						&& m_remote_establishment_mode == _simulation_view_establishment_mode_joining
						&& old_remote_establishment_mode == _simulation_view_establishment_mode_active)
					{
						event(
							_event_message,
							"networking:simulation:view: view %s remote client mode %d/%d -> %d/%d acknowledged (active to join) (authority %d/%d)",
							get_view_description(),
							_simulation_view_establishment_mode_active,
							old_remote_establishment_identifier,
							m_remote_establishment_mode,
							m_remote_establishment_identifier,
							m_view_establishment_mode,
							m_view_establishment_identifier
						);
					}
#endif
					else
					{
						event(
							_event_error,
							"simulation:view: view %s remote client mode %d/%d -> %d/%d establishment error (authority %d/%d), dying",
							get_view_description(),
							old_remote_establishment_mode,
							old_remote_establishment_identifier,
							m_remote_establishment_mode,
							m_remote_establishment_identifier,
							m_view_establishment_mode,
							m_view_establishment_identifier
						);
						kill_view(_simulation_view_reason_mode_error);
					}
				}
				else
				{
					event(
						_event_message,
						"simulation:view: view %s remote client mode %d/%d -> %d/%d died (authority %d/%d), dying",
						get_view_description(),
						old_remote_establishment_mode,
						old_remote_establishment_identifier,
						m_remote_establishment_mode,
						m_remote_establishment_identifier,
						m_view_establishment_mode,
						m_view_establishment_identifier
					);
					kill_view(_simulation_view_reason_remote_ended);
				}
			}
			//end remote client

			//if remote authority
			else
			{
				const bool remote_is_established = remote_establishment_mode == _simulation_view_establishment_mode_established;
				const bool remote_is_establishing = remote_establishment_mode < _simulation_view_establishment_mode_established;
				const bool remote_establishment_matches = m_view_establishment_identifier == remote_establishment_identifier;

				//established
				if (remote_is_established)
				{
					if (m_view_establishment_identifier == NONE)
					{
						event(
							_event_message,
							"simulation:view: view %s remote authority mode %d/%d -> %d/%d initiating establishment (currently %d/%d)",
							get_view_description(),
							old_remote_establishment_mode,
							old_remote_establishment_identifier,
							m_remote_establishment_mode,
							m_remote_establishment_identifier,
							m_view_establishment_mode,
							m_view_establishment_identifier
						);

						set_view_establishment(_simulation_view_establishment_mode_established, remote_establishment_identifier);
					}
				}
				//ready to establish or beyond
				else if (remote_is_establishing || !remote_establishment_matches)
				{
					if (m_view_establishment_mode < _simulation_view_establishment_mode_established)
					{
						if (old_remote_establishment_mode > _simulation_view_establishment_mode_none
							|| remote_establishment_mode <= _simulation_view_establishment_mode_none)
						{
							event(
								_event_message,
								"simulation:view: view %s remote authority mode %d/%d -> %d/%d ignored as unestablished/unrecognized (currently %d/%d)",
								get_view_description(),
								old_remote_establishment_mode,
								old_remote_establishment_identifier,
								m_remote_establishment_mode,
								m_remote_establishment_identifier,
								m_view_establishment_mode,
								m_view_establishment_identifier
							);
						}
						else
						{
							event(
								_event_message,
								"simulation:view: view %s remote authority mode %d/%d -> %d/%d newly created, resending local mode %d/%d",
								get_view_description(),
								_simulation_view_establishment_mode_none,
								old_remote_establishment_identifier,
								m_remote_establishment_mode,
								m_remote_establishment_identifier,
								m_view_establishment_mode,
								m_view_establishment_identifier
							);
							send_establishment_message();
						}
					}
					else
					{
						event(
							_event_message,
							"simulation:view: view %s remote authority mode %d/%d -> %d/%d disconnects us (currently %d/%d), dying",
							get_view_description(),
							old_remote_establishment_mode,
							old_remote_establishment_identifier,
							m_remote_establishment_mode,
							m_remote_establishment_identifier,
							m_view_establishment_mode,
							m_view_establishment_identifier
						);
						kill_view(_simulation_view_reason_remote_ended);
					}

				}
				//if remote_establishment_matches
				else
				{

					event(
						_event_message,
						"simulation:view: view %s remote authority mode %d/%d -> %d/%d continuing establishment (currently %d/%d)",
						get_view_description(),
						old_remote_establishment_mode,
						old_remote_establishment_identifier,
						m_remote_establishment_mode,
						m_remote_establishment_identifier,
						m_view_establishment_mode,
						m_view_establishment_identifier
					);
					set_view_establishment(remote_establishment_mode, remote_establishment_identifier);
				}

			}
		}


		update_view_activation_state();
		result = true;
	}
	else
	{
		event(
			_event_warning,
			"simulation:view: view %s ignoring mode %d/%d, not attached to a channel",
			get_view_description(),
			remote_establishment_mode,
			remote_establishment_identifier
		);
	}

	return result;
}


void c_simulation_view::dispatch_synchronous_update(
	struct simulation_update const* update)
{

	ASSERT(exists());
	ASSERT(m_view_type == _simulation_view_type_synchronous_to_remote_client);
	ASSERT(m_world != NULL);
	ASSERT(update);

	if (m_view_establishment_mode == _simulation_view_establishment_mode_active)
	{
		s_network_message_synchronous_update message;
		csmemset(&message, 0, sizeof(message));
		message.update = *update;

		send_message(_network_message_type_synchronous_update, sizeof(message), &message);
	}
	else if (synchronous_catchup_in_progress())
	{
		if (synchronous_catchup_submit_update(update))
		{
			synchronous_catchup_send_data();
		}
		else
		{
			event(
				_event_error,
				"simulation:view: view [%s] synchronous-catchup failed to submit update [#%d]",
				get_view_description(),
				update->update_number
			);

			kill_view(_simulation_view_reason_synchronous_catchup_fail);
		}
	}
	else
	{
		event(
			_event_verbose,
			"simulation:view: view %s not yet in game, skipping synchronous-update [#%d]",
			get_view_description(),
			update->update_number
		);
	}

	return;
}

void c_simulation_view::update_view_activation_state(void)
{
	//INVOKE_TYPE(0x1DEF6D, 0x1C642D, void(__thiscall*)(c_simulation_view*), this);

	bool simulation_established = false;
	bool simulation_active = false;

	if (m_world && m_channel_index != NONE && m_view_establishment_identifier == m_remote_establishment_identifier)
	{
		simulation_established =
			m_view_establishment_mode >= _simulation_view_establishment_mode_established &&
			m_remote_establishment_mode >= _simulation_view_establishment_mode_established;
		simulation_active =
			m_view_establishment_mode >= _simulation_view_establishment_mode_active &&
			m_remote_establishment_mode >= _simulation_view_establishment_mode_active;
	}

	ASSERT(!(simulation_active && !simulation_established));

	if (simulation_established != established())
	{
		event(
			_event_message,
			"simulation:view: view %s simulation is now %s",
			get_view_description(),
			simulation_established ? "ESTABLISHED" : "NON-ESTABLISHED"
		);

		m_channel_interface.set_established(simulation_established);

		if (!simulation_established)
		{
			if (is_client_view())
			{
				m_simulation_player_acknowledged_mask = NULL;
				if (m_view_type == _simulation_view_type_synchronous_to_remote_client)
				{
					m_synchronous_received_action_number = NONE;
					m_synchronous_acknowledged_update_number = NONE;
					if (synchronous_catchup_in_progress())
					{
						synchronous_catchup_terminate();
					}
				}
				else if (m_view_type == _simulation_view_type_distributed_to_remote_client)
				{
					distributed_join_abort();
				}
			}
			else if (m_view_type == _simulation_view_type_synchronous_to_remote_authority)
			{
				m_synchronous_catchup_stream_items = 0;
			}
		}

		if (is_distributed())
		{
			m_distributed_view->m_game_results_replicator.handle_view_establishment(simulation_established);
		}

		ASSERT(m_world);
		m_world->handle_view_establishment(this, simulation_established);
	}

	if (m_simulation_active != simulation_active)
	{
		event(
			_event_message,
			"simulation:view: view %s simulation is now %s",
			get_view_description(),
			simulation_active ? "ACTIVE" : "INACTIVE"
		);

		m_simulation_active = simulation_active;

		ASSERT(m_world);
		m_world->handle_view_activation(this, m_simulation_active);
	}

	return;
}

void c_simulation_view::failed_to_join(void)
{
	ASSERT(exists());
	ASSERT(is_client_view());
	ASSERT(m_world != NULL);

	event(
		_event_message,
		"simulation:view: view %s failed to join remote authority, dying",
		get_view_description()
	);

	kill_view(_simulation_view_reason_failed_to_join);
	return;
}

void c_simulation_view::synchronous_catchup_send_data(void)
{
	INVOKE_TYPE(0x1DF2E7, 0x1C67A7, void(__thiscall*)(c_simulation_view*), this);
}

bool c_simulation_view::synchronous_catchup_submit_update(struct simulation_update const* update)
{
	return INVOKE_TYPE(0x1DE96E, 0x1C5E2E, bool(__thiscall*)(c_simulation_view*, struct simulation_update const*), this, update);
}

void c_simulation_view::synchronous_catchup_terminate(
	void)
{
	INVOKE_TYPE(0x1DE932, 0x0, void(__thiscall*)(c_simulation_view*), this);
	return;
}

void c_simulation_view::synchronous_client_block(
	bool block)
{
	ASSERT(exists());
	ASSERT(m_view_type == _simulation_view_type_synchronous_to_remote_client);

	if (block && !m_synchronous_client_blocked)
	{
		m_synchronous_client_block_timestamp = network_time_get();
	}
	m_synchronous_client_blocked = block;

	if (block && network_time_since(m_synchronous_client_block_timestamp) >= 2000)
	{
		event(
			_event_message,
			"simulation:view: view [%s] has blocked for [%dms] and is now dead at update/time [#%d]/[#%d]",
			get_view_description(),
			network_time_since(m_synchronous_client_block_timestamp),
			m_world->get_next_update_number(),
			m_world->get_time()
		);
		kill_view(_simulation_view_reason_synchronous_block);
	}

	return;
}

int32 c_simulation_view::synchronous_client_get_acknowledged_update_number(void) const
{
	ASSERT(exists());
	ASSERT(m_view_type == _simulation_view_type_synchronous_to_remote_client);

	return m_synchronous_acknowledged_update_number;
}

void c_simulation_view::distributed_join_abort(void)
{
	ASSERT(is_distributed());
	ASSERT(m_distributed_view);

	INVOKE_TYPE(0x1DEAAB, 0x0, void(__thiscall*)(c_simulation_view*), this);
	return;
}

