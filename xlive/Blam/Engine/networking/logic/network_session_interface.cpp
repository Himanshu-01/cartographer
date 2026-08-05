#include "stdafx.h"
#include "network_session_interface.h"

#include "networking/session/network_session_manager.h"
#include "networking/session/network_session.h"
#include "networking/logic/life_cycle_manager.h"
#include "game/game_engine_util.h"


/* typedefs */
typedef bool(__cdecl* network_squad_session_set_game_variant_t)(s_game_variant* variant);


/* globals */

bool debug_net_distributed_always = false;
bool debug_net_distributed_never = false;

network_squad_session_set_game_variant_t p_network_squad_session_set_game_variant;

/* public code */

bool __cdecl network_session_interface_initialize(c_network_session_manager* session_manager)
{
	return INVOKE(0x1B07B2, 0x1968DC, network_session_interface_initialize, session_manager);
}

bool __cdecl network_squad_session_set_game_variant(s_game_variant* variant)
{
    //return INVOKE(0x1B1DFB, 0x197E25, network_squad_session_set_game_variant, variant);

    e_network_game_simulation_protocol simulation_protocol = _network_game_simulation_protocol_offline;
    c_network_session* session = nullptr;

    if (!network_squad_session_can_set_game_settings())
    {
        error(_error_delayed, "%s cannot set game settings, ignoring", __FUNCTION__);
        return false;
    }

    bool in_squad_session = network_life_cycle_in_squad_session(&session);
    ASSERT(in_squad_session);

    if (debug_net_distributed_always)
    {
        simulation_protocol = _network_game_simulation_protocol_distributed;
    }
    else if (debug_net_distributed_never)
    {
        simulation_protocol = _network_game_simulation_protocol_synchronous;
    }
    else if (session->get_session_class())
    {
        if (variant)
            simulation_protocol = game_engine_get_simulation_protocol(variant);
        else
            simulation_protocol = _network_game_simulation_protocol_synchronous;
    }
    else
    {
        simulation_protocol = _network_game_simulation_protocol_synchronous;
    }


    bool success = false;
    success = session->parameters_game_variant_request_change(variant);
    ASSERT(success);
    success = session->parameters_simulation_protocol_request_change(simulation_protocol);
    ASSERT(success);
    success = session->parameters_countdown_timer_request_change(0, 0, 0, 0, nullptr);
    ASSERT(success);

    return success;

}


void network_session_interface_patches()
{
    DETOUR_ATTACH(p_network_squad_session_set_game_variant, Memory::GetAddress<network_squad_session_set_game_variant_t>(0x1B1DFB, 0x197E25), network_squad_session_set_game_variant);
}
