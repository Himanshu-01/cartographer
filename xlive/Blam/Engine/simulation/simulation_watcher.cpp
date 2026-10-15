#include "stdafx.h"
#include "simulation_watcher.h"

#include "networking/Session/network_session.h"
#include "simulation_world.h"


bool c_simulation_watcher::need_to_generate_updates(void)
{
	bool result = false;

	//bool result = INVOKE_TYPE(0x1D4B42, 0x1C188C, bool(__thiscall*)(c_simulation_watcher*), this);
	//return (result || !m_world->simulation_queues_empty()) && m_world->is_distributed() && m_world->is_authority();


	if (m_world
		&& !m_world->is_dead()
		&& m_world->runs_simulation()
		&& m_world->can_generate_updates()) //h3 addon for synchronous
	{
		result = m_machine_update_pending;

		if (m_session
			&& m_session->established()
			&& m_session->session_mode() == _network_session_mode_in_game)
		{
			ASSERT(VALID_INDEX(m_session->get_session_host_peer_index(), m_session->get_peer_count()));

			if (m_player_last_local_membership_update_number != m_session->get_local_session_membership_update_number())
				result = true;
		}

		result = (result || !m_world->simulation_queues_empty());
	}


	return result;
}

bool c_simulation_watcher::get_player_is_in_game(
	int32 player_index,
	s_player_identifier const* player_identifier) const
{
	bool player_in_game = false;

	ASSERT(player_index >= 0 && player_index < k_maximum_players);
	ASSERT(player_identifier);

	if (TEST_BIT(m_player_collection.player_valid_mask, player_index))
	{
		s_player_identifier const* identifier = &m_player_collection.players[player_index].identifier;

		if (!csmemcmp(player_identifier, identifier, sizeof(*identifier)))
		{
			if (!m_player_collection.players[player_index].left_game)
			{
				player_in_game = true;
			}
		}
	}

	return player_in_game;
}
