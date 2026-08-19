#pragma once
#include "network_message_type_collection.h"

#include "simulation/simulation_update.h"


/* structures */

struct s_network_message_synchronous_update
{
	struct simulation_update update;
};


/* prototypes */

void __cdecl network_message_types_register_simulation_synchronous(c_network_message_type_collection* message_collection);
