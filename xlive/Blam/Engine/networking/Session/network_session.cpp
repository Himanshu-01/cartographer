#include "stdafx.h"
#include "network_session.h"

#include "network_observer.h"

#include "game/game.h"
#include "interface/user_interface_controller.h"
#include "networking/logic/life_cycle_manager.h"
#include "networking/network_event.h"
#include "networking/network_time.h"
#include "networking/messages/network_messages_session_parameters.h"
#include "shell/shell.h"

/* constants */

const char* const k_network_protocols_text[] =
{
	"<disconnected>",
	"System-Link",
	"LIVE",
};

/* prototypes */

CLASS_HOOK_DECLARE_LABEL(c_network_session__handle_parameters_update, c_network_session::handle_parameters_update);
static __declspec(naked) void jmp_c_network_session__handle_parameters_update()
{
	CLASS_HOOK_JMP(c_network_session__handle_parameters_update, c_network_session::handle_parameters_update);
}

CLASS_HOOK_DECLARE_LABEL(c_network_session__handle_mode_acknowledge, c_network_session::handle_mode_acknowledge);
static __declspec(naked) void jmp_c_network_session__handle_mode_acknowledge()
{
	CLASS_HOOK_JMP(c_network_session__handle_mode_acknowledge, c_network_session::handle_mode_acknowledge);
}



/* gloabls */

static char g_network_session_peer_description[2][35] = {};
static int32 g_network_session_peer_description_index = 0;

c_network_session_cartographer g_cartographer_network_session;

/* public code */

/* PRIVATE NAMESPACE, acts like static keyword */
// 
namespace NetworkSession
{
	c_network_session* GetNetworkSessions();
	c_network_session* GetActiveNetworkSession();
}

bool NetworkSession::PlayerIsActive(datum player_index)
{
	return GetActiveNetworkSession()->is_session_player_active(player_index);
}

std::vector<int32> NetworkSession::GetActivePlayerIndicesList()
{
	std::vector<int32> activePlayersIndices;
	if (GetPlayerCount() > 0)
	{
		for (int32 playerIndex = 0; playerIndex < k_maximum_players; playerIndex++)
		{
			if (PlayerIsActive(playerIndex))
				activePlayersIndices.emplace_back(playerIndex);
		}
	}

	return activePlayersIndices;
}

c_network_session* NetworkSession::GetNetworkSessions()
{
	return *Memory::GetAddress<c_network_session**>(0x51C474, 0x520B94);
}

c_network_session* NetworkSession::GetActiveNetworkSession()
{
	// ### FIXME replace this function wherever this is still used
	// for now, return the pointer to the active squad session regardless to avoid crashing the game
	/*c_network_session* session = NULL;
	network_life_cycle_in_squad_session(&session);*/
	return c_game_life_cycle_manager::get()->m_active_squad_session;
}

bool NetworkSession::LocalPeerIsSessionHost()
{
	return GetActiveNetworkSession()->is_host();
}

bool NetworkSession::LocalPeerIsSessionLeader()
{
	return GetActiveNetworkSession()->is_local_peer_session_leader();
}

bool NetworkSession::LocalPeerIsEstablished()
{
	return GetActiveNetworkSession()->established();
}

bool NetworkSession::GetMapFileLocation(wchar_t* buffer, size_t size)
{
	// host-only
	typedef bool(__thiscall* get_map_file_location_t)(c_network_session* session, wchar_t* buffer, size_t size);
	auto p_get_map_file_location = Memory::GetAddress<get_map_file_location_t>(0x1C5678, 0x19CD4A);
	return p_get_map_file_location(GetActiveNetworkSession(), buffer, size);
}

int32 NetworkSession::GetPeerCount()
{
	return GetActiveNetworkSession()->m_session_membership.peer_count;
}

int32 NetworkSession::GetLocalPeerIndex()
{
	return GetActiveNetworkSession()->get_local_peer_index();
}

IN_ADDR NetworkSession::GetLocalNetworkAddress()
{
	return GetActiveNetworkSession()->m_session_membership.membership_peers[GetLocalPeerIndex()].secure_address.addr.inaOnline;
}

int32 NetworkSession::GetPeerIndex(datum player_index)
{
	return GetActiveNetworkSession()->get_player_membership(player_index)->peer_index;
}

int32 NetworkSession::GetPlayerCount()
{
	return GetActiveNetworkSession()->get_player_count();
}

const wchar_t* NetworkSession::GetPlayerName(datum player_index)
{
	return GetActiveNetworkSession()->get_player_name(player_index);
}

s_player_identifier NetworkSession::GetPlayerId(datum player_index)
{
	return GetActiveNetworkSession()->get_player_id(player_index);
}

int8 NetworkSession::GetPlayerTeam(datum player_index)
{
	return GetActiveNetworkSession()->get_player_membership(player_index)->configuration.team_index;
}

void NetworkSession::KickPeer(int32 peer_index)
{
	typedef void(__thiscall* game_session_boot_t)(c_network_session*, int, bool);
	auto p_game_session_boot = Memory::GetAddress<game_session_boot_t>(0x1CCE9B);

	if (peer_index < GetPeerCount())
	{
		event(_event_message, "h2mod:network_session: %s - about to kick peer index = %d", __FUNCTION__, peer_index);
		p_game_session_boot(NetworkSession::GetActiveNetworkSession(), peer_index, true);
	}
}

void NetworkSession::EndGame()
{
	INVOKE(0x215470, 0x197F32, NetworkSession::EndGame);
}

wchar_t* NetworkSession::GetGameVariantName()
{
	return GetActiveNetworkSession()->m_session_parameters.game_variant.variant_name;
}

bool NetworkSession::IsVariantTeamPlay()
{
	return TEST_BIT(GetActiveNetworkSession()->m_session_parameters.game_variant.flags, _game_engine_teams_bit);
}

void NetworkSession::LeaveSession()
{
	if (shell_is_dedicated_server())
		return;

	if (game_is_ui_shell())
	{
		// request_squad_browser
		*Memory::GetAddress<bool*>(0x978BAC) = true;

		typedef void(__cdecl* load_main_menu_with_context_t)(int context);
		auto p_load_main_menu_with_context = Memory::GetAddress<load_main_menu_with_context_t>(0x08EAF);
		p_load_main_menu_with_context(0);
	}

	typedef int(__cdecl* leave_game_type_t)(int a1);
	auto p_leave_session = Memory::GetAddress<leave_game_type_t>(0x216388);
	p_leave_session(0);
}

s_session_interface_globals* s_session_interface_globals::get()
{
	return Memory::GetAddress<s_session_interface_globals*>(0x51A590, 0x520408);
}

s_session_interface_user* session_interface_get_local_user_properties(int32 user_index)
{
	return &s_session_interface_globals::get()->users[user_index];
}

e_network_session_class network_squad_session_get_session_class()
{
	//return INVOKE(0x1B1643, 0x0, network_squad_session_get_session_class);

	e_network_session_class out_class = _network_session_class_unknown;
	c_network_session* session = NULL;
	if (network_life_cycle_in_squad_session(&session))
	{
		if (session->established())
		{
			out_class = session->m_session_class;
		}
	}
	return out_class;
}


bool network_session_interface_set_local_user_character_type(int32 user_index, e_character_type character_type)
{
	s_session_interface_user* user_properties = session_interface_get_local_user_properties(user_index);
	
	// Don't change the character type if the user doesn't exist
	if (user_properties->user_exists)
	{
		user_properties->properties.profile_traits.profile.player_character_type = character_type;
		return true;
	}

	return false;
}

bool network_session_interface_get_local_user_identifier(int32 user_index, s_player_identifier* out_identifier)
{
	s_session_interface_user* user_properties = session_interface_get_local_user_properties(user_index);
	if (user_properties->user_exists)
	{
		*out_identifier = user_properties->network_user_identifier;
		return true;
	}
	return false;
}

void network_session_interface_set_local_user_rank(int32 user_index, int8 rank)
{
	s_session_interface_user* user_properties = session_interface_get_local_user_properties(user_index);
	user_properties->properties.player_displayed_skill = rank;
	user_properties->properties.player_overall_skill = rank;
	return;
}

bool __cdecl network_session_interface_get_local_user_properties(int32 user_index, int32* out_controller_index, s_player_configuration* out_properties, int32* out_player_voice, int32* out_player_text_chat)
{
	return INVOKE(0x1B10E0, 0x1970A8, network_session_interface_get_local_user_properties, user_index, out_controller_index, out_properties, out_player_voice, out_player_text_chat);
}

void __cdecl network_globals_switch_environment(int32 a1, bool a2)
{
	INVOKE(0x1B54CF, 0x1A922D, network_globals_switch_environment, a1, a2);
}

void network_session_apply_patches()
{
	//PatchCall(Memory::GetAddress(0x1E8961), c_network_session::handle_parameters_update);
	WriteJmpTo(Memory::GetAddress(0x1CEB3E,0), jmp_c_network_session__handle_parameters_update);
	WriteJmpTo(Memory::GetAddress(0x1C5A7F,0), jmp_c_network_session__handle_mode_acknowledge);

}

void network_session_membership_update_local_players_teams()
{
	c_network_session* session = NULL;
	if (network_life_cycle_in_squad_session(&session))
	{
		if (session->established() || session->peer_joining())
		{
			int32 local_peer_index = session->get_local_peer_index();

			for (int32 i = 0; i < k_number_of_users; i++)
			{
				datum player_index = session->get_peer_membership(local_peer_index)->local_players_indexes[i];
				if (player_index != NONE)
				{
					const s_membership_player* membership_player = session->get_player_membership(player_index);
					user_interface_controller_set_desired_team_index((e_controller_index)i, (e_game_team)membership_player->configuration.team_index);
					user_interface_controller_update_network_properties((e_controller_index)i);
				}
			}
		}
	}
}

void network_session_set_player_team(datum player_index, e_game_team team)
{
	c_network_session* session = NULL;
	if (network_life_cycle_in_squad_session(&session))
	{
		if (session->established() 
			&& session->is_host())
		{
			s_membership_player* membership_player = session->get_player_membership(player_index);
			membership_player->configuration.team_index = (int8)team;

			if (session->peer_index_local_peer(membership_player->peer_index))
			{
				user_interface_controller_set_desired_team_index((e_controller_index)membership_player->controller_index, (e_game_team)membership_player->configuration.team_index);
				user_interface_controller_update_network_properties((e_controller_index)membership_player->controller_index);
			}
		}
	}
}

bool c_network_session::initialize_session(
	int32 session_index,
	e_network_session_type session_type,
	int32 session_transport_index,
	c_network_message_gateway* message_gateway,
	c_network_observer* observer,
	c_network_session_manager* session_manager,
	c_network_text_chat_manager* text_chat_manager)
{
	return INVOKE_TYPE(0x1C1E65, 0x199578, bool(__thiscall*)(c_network_session*, int32, e_network_session_type, int32, c_network_message_gateway*, c_network_observer*, c_network_session_manager*, c_network_text_chat_manager*),
		this,
		session_index,
		session_type,
		session_transport_index,
		message_gateway,
		observer,
		session_manager,
		text_chat_manager);
}

bool c_network_session::channel_is_authoritative(int32 network_channel_index) const
{
	bool result = false;

	if ((established() || peer_joining()) && !is_host())
	{
		int32 observer_index = m_network_observer->observer_channel_find_by_network_channel(m_session_index, network_channel_index);
		int32 peer_index = get_peer_index_by_observer_index(observer_index);

		if (peer_index != NONE && is_peer_session_host(peer_index))
		{
			result = true;
		}
	}

	return result;
}

void c_network_session::switch_players_to_teams(datum* player_indexes, int32 player_count, e_game_team* team_indexes)
{
	if (is_host())
	{
		for (int32 i = 0; i < player_count; i++)
		{
			s_membership_player* player_membership = get_player_membership(player_indexes[i]);
			player_membership->configuration.team_index = (int8)team_indexes[i];
		}
		request_membership_update();
		network_session_membership_update_local_players_teams();
	}
}

bool c_network_session::get_secure_key(s_transport_secure_identifier* out_session_id, XNKEY* out_session_key, int32* out_session_key_index, e_transport_platform* transport_platform) const
{
	bool result = false;

	if (!disconnected() && m_field_48)
	{
		if (out_session_id != NULL)
		{
			*out_session_id = m_session_id;
		}

		if (out_session_key != NULL)
		{
			*out_session_key = m_session_key;
		}

		if (out_session_key_index != NULL)
		{
			*out_session_key_index = m_session_transport_index;
		}

		if (transport_platform != NULL)
		{
			*transport_platform = m_session_transport_platform;
		}

		result = true;
	}

	return result;
}

bool c_network_session::get_transport_session_id(s_transport_secure_identifier* out_session_id) const
{
	return get_secure_key(out_session_id, NULL, NULL, NULL);
}

uint32 c_network_session::time_get(void) const
{
	ASSERT(m_time_exists);
	return m_time + network_time_get_exact();
}

bool c_network_session::handle_leave_request(const transport_address* incoming_address)
{
	const int32 peer = get_peer_from_incoming_address(incoming_address);
	ASSERT(is_host());

	bool result;
	if (peer == NONE || peer == m_local_peer_index)
	{
		event(
			_event_warning,
			"networking:session:membership: [%s] leave-request received from an incorrect peer [%s] (invalid or local)",
			managed_session_get_id_string(&m_session_id),
			get_peer_description(peer)
		);
		result = false;
	}
	else
	{
		event(
			_event_message,
			"session:membership: [%s] leave-request received from peer [%s]",
			managed_session_get_id_string(&m_session_id),
			get_peer_description(peer)
		);
		result = handle_leave_internal(peer);
	}

	return result;
}

bool c_network_session::handle_leave_internal(int32 peer_index)
{
	return INVOKE_TYPE(0x1CC7B4, 0x1A3D34, bool(__thiscall*)(c_network_session*, int32), this, peer_index);
}

bool c_network_session::handle_parameters_update(s_network_message_parameters_update* message)
{
	//return INVOKE_TYPE(0x1CEB3E, 0x0, bool(__thiscall*)(c_network_session*, s_network_message_parameters_update*), this, message);

	ASSERT((established() || peer_joining()) && !is_host());

	s_session_parameters new_parameters;
	const bool result = apply_parameters_update(message,&new_parameters);
	const char* handle_result = message->incremental_update_number != NONE ? "incremental" : "complete";
	if (result)
	{
		csmemcpy(&this->m_session_parameters, &new_parameters, sizeof(new_parameters));
		event(
			_event_status,
			"session:parameters: [%s] parameters-update handled %s [#%d]/[#%d]",
			managed_session_get_id_string(&m_session_id),
			handle_result,
			message->update_number,
			message->incremental_update_number
		);



		/////

		///lifecycle-status
		
		/////
		s_network_message_mode_acknowledge packet;
		if (message->session_mode_valid && message->session_mode_request_ack)
		{
			//if (
			//	m_session_parameters.session_mode == _network_session_mode_setup ||
			//	m_session_parameters.session_mode == _network_session_mode_in_game
			//	)
			//{
				event(
					_event_status,
					"session:parameters: [%s] sending acknowledgement state [#%d] to host",
					managed_session_get_id_string(&m_session_id),
					m_session_parameters.session_mode
				);
			//}

			packet.session_id = m_session_id;
			packet.session_mode = m_session_parameters.session_mode;
			packet.session_mode_sequence = m_session_parameters.session_mode_sequence;
			const s_session_peer* local_peer = &m_session_peers[m_session_host_peer_index];
			if (local_peer->is_remote_peer)
			{
				ASSERT(local_peer->observer_channel_index != NONE);

				this->m_network_observer->send_message(
					this->m_session_index,
					local_peer->observer_channel_index,
					false,
					_network_message_type_mode_acknowledge,
					sizeof(s_network_message_mode_acknowledge),
					&packet);
			}
		}

	}
	else
	{
		event(
			_event_error,
			"session:parameters: [%s] parameters-update current [#%d] couldn't process %s update [#%d]/[#%d]",
			managed_session_get_id_string(&m_session_id),
			m_session_parameters.parameters_update_number,
			handle_result,
			message->update_number,
			message->incremental_update_number
		);

		handle_disconnection();
	}
	return result;
}


bool c_network_session::handle_mode_acknowledge(int32 channel_index, s_network_message_mode_acknowledge* message)
{
	bool result = false;
	const int32 observer_channel_index = m_network_observer->observer_channel_find_by_network_channel(observer_owner(), channel_index);
	const int32 peer_index = get_peer_index_by_observer_index(observer_channel_index);

	ASSERT(message);
	ASSERT(established() && is_host());

	if (peer_index == NONE || peer_index == m_local_peer_index)
	{
		event(
			_event_warning,
			"session:mode: [%s] mode-acknowledge received from invalid peer [%s]",
			managed_session_get_id_string(&m_session_id),
			get_peer_description(peer_index)
		);
	}
	else if (m_local_pending_mode_transition)
	{
		if (message->session_mode_sequence == m_session_parameters.session_mode_sequence
			&& message->session_mode == m_session_parameters.session_mode)
		{
			s_session_peer* local_peer = &m_session_peers[peer_index];
			ASSERT(local_peer->is_remote_peer);

			if(local_peer->mode_transition_pending)
			{
				event(
					_event_message,
					"session:mode: [%s] received mode-acknowledge from peer [%s] for current mode transition [#%d](%d)",
					managed_session_get_id_string(&m_session_id),
					get_peer_description(peer_index),
					m_session_parameters.session_mode_sequence,
					m_session_parameters.session_mode
				);

				local_peer->mode_transition_pending = false;
				result = true;


				if (m_session_membership.peer_count <= 0)
				{
					complete_host_pending_transition();
				}
				else
				{
					int32 other_peer_index = 0;
					while (true)
					{
						
						const s_session_peer* other_peer = &m_session_peers[other_peer_index];
						if (other_peer->is_remote_peer && other_peer->mode_transition_pending)
							break;
						if (++other_peer_index >= m_session_membership.peer_count)
							complete_host_pending_transition();
					}
					ASSERT(other_peer_index != peer_index);
					ASSERT(other_peer_index != m_local_peer_index);
				}
			}
			else
			{
				event(
					_event_warning,
					"session:mode: [%s] duplicate mode-acknowledge received from peer [%s] for current mode transition [#%d](%d)",
					managed_session_get_id_string(&m_session_id),
					get_peer_description(peer_index),
					m_session_parameters.session_mode_sequence,
					m_session_parameters.session_mode
				);
			}
		}
		else
		{
			event(
				_event_warning,
				"session:mode: [%s] mismatched mode-acknowledge received peer [%s] mode [#%d](%d) != current transition to [#%d](%d)",
				managed_session_get_id_string(&m_session_id),
				get_peer_description(peer_index),
				message->session_mode_sequence,
				message->session_mode,
				m_session_parameters.session_mode_sequence,
				m_session_parameters.session_mode
			);

		}

	}
	else
	{
		event(
			_event_warning,
			"session:mode: [%s] mode-acknowledge received from peer [%s] mode [#%d](%d), but current mode [#%d](%d) is not transitioning",
			managed_session_get_id_string(&m_session_id),
			get_peer_description(peer_index),
			message->session_mode_sequence,
			message->session_mode,
			m_session_parameters.session_mode_sequence,
			m_session_parameters.session_mode
		);
	}

	return result;
}


/* private code */

const char* c_network_session::get_peer_description(int32 peer_index) const
{
	char* result = g_network_session_peer_description[g_network_session_peer_description_index];
	g_network_session_peer_description_index = (g_network_session_peer_description_index + 1) % 2;

	if (established() && VALID_INDEX(peer_index, m_session_membership.peer_count) && m_session_membership.membership_peers[peer_index].description[0] != '\0')
	{
		const char* mac_string = transport_secure_address_get_mac_string(&m_session_membership.membership_peers[peer_index].secure_address);
		csprintf(result, 35, "#%02d:%S:%s", peer_index, m_session_membership.membership_peers[peer_index].description[0], mac_string);
	}
	else
	{
		csprintf(result, 35, "#%02d", peer_index);
	}
	return result;
}

int32 c_network_session::get_peer_from_incoming_address(const transport_address* incoming_address) const
{
	ASSERT(incoming_address);

	int32 result = NONE;
	if (!disconnected() && m_field_48)
	{
		s_transport_secure_address secure_address;
		if (transport_secure_identifier_retrieve(incoming_address, m_session_transport_platform, NULL, NULL, NULL, &secure_address))
		{
			result = get_peer_from_secure_address(&secure_address);
		}
	}
	return result;
}

int32 c_network_session::get_peer_from_secure_address(const s_transport_secure_address* secure_address) const
{
	ASSERT(secure_address);
	
	int32 result = NONE;
	if (!disconnected() && has_membership())
	{
		for (int32 i = 0; i < m_session_membership.peer_count; ++i)
		{
			if (transport_secure_address_compare(secure_address, &m_session_membership.membership_peers[i].secure_address))
			{
				result = i;
				break;
			}
		}
	}
	return result;
}

bool c_network_session::apply_parameters_update(s_network_message_parameters_update* message, s_session_parameters* out_params)
{
	return INVOKE_TYPE(0x1C2BFC, 0x0, bool(__thiscall*)(c_network_session*, s_network_message_parameters_update*, s_session_parameters*), this, message, out_params);
}

void c_network_session::handle_disconnection()
{
	return INVOKE_TYPE(0x1CD920, 0x0, void(__thiscall*)(c_network_session*), this);
}

void c_network_session::complete_host_pending_transition()
{
	ASSERT(established() && is_host());

	if (!m_local_pending_mode_transition)
		return;

	for (int32 i = 0; i < m_session_membership.peer_count; ++i)
	{
		const s_session_peer* test_local_peer = &m_session_peers[i];
		ASSERT(!test_local_peer->mode_transition_pending);
	}

	event(
		_event_message,
		"session:mode: [%s] mode transition complete to [#%d](%d)",
		managed_session_get_id_string(&m_session_id),
		m_session_parameters.session_mode_sequence,
		m_session_parameters.session_mode
	);

	m_local_pending_mode_transition = false;
}
