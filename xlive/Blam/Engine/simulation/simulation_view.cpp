#include "stdafx.h"
#include "simulation_view.h"

#include "simulation.h"
#include "simulation_world.h"
#include "networking/network_event.h"
#include "networking/messages/network_messages_simulation.h"
#include "networking/messages/network_messages_simulation_synchronous.h"
#include "networking/session/network_observer.h"

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
		if (establishment_mode != _simulation_view_establishment_mode_established)
		{
			valid_mode =
				establishment_identifier == m_view_establishment_identifier &&
				establishment_mode == m_view_establishment_mode + 1;
		}
		else
		{
			valid_mode = establishment_identifier >= 0;
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

			s_network_message_view_establishment message;
			csmemset(&message, 0, sizeof(message));

			message.establishment_mode = m_view_establishment_mode;
			message.establishment_identifier = m_view_establishment_identifier;
			send_message(_network_message_type_view_establishment, sizeof(message), &message);

			update_view_activation_state();
		}
	}

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
	INVOKE_TYPE(0x1DEF6D, 0x1C642D, void(__thiscall*)(c_simulation_view*), this);
}

void c_simulation_view::synchronous_catchup_send_data(void)
{
	INVOKE_TYPE(0x1DF2E7, 0x1C67A7, void(__thiscall*)(c_simulation_view*), this);
}

bool c_simulation_view::synchronous_catchup_submit_update(struct simulation_update const* update)
{
	return INVOKE_TYPE(0x1DE96E, 0x1C5E2E, bool(__thiscall*)(c_simulation_view*, struct simulation_update const*), this, update);
}

