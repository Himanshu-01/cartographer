#pragma once

/* structures */

struct s_network_adapter
{
	char adapter_name[64];
	wchar_t friendly_name[64];
	wchar_t description[64];
};

struct s_network_adapter_configuration
{
	uint32 adapter_count;
	uint8 network_adapters_available;
	int32 network_adapter_index;
	s_network_adapter network_adapters[16];
};

struct s_simulation_world_configuration
{
	int32 maximum_catchup_views;
	int32 join_timeout;
	int32 join_total_wait_timeout;
	real32 pause_game_required_machines_fraction;
	int32 maximum_catchup_attempts;
	int32 catchup_failure_timeout;
	int32 client_join_failure_count;
	int32 client_activation_failure_timeout;
};
ASSERT_STRUCT_SIZE(s_simulation_world_configuration, 32);

struct s_simulation_configuration
{
	int8 gap_0[96];
	s_simulation_world_configuration world;
	int8 gap_80[1832];
};
ASSERT_STRUCT_SIZE(s_simulation_configuration, 0x7a8);

// TODO: properly reverse this
struct s_network_configuration
{
	int8 gap_0[16];
	real32 field_10;
	int8 gap_14[3276];
	s_simulation_configuration simulation;
	int8 gap_1488[716];
	bool registry_single_instance_only;
	int8 gap_1755[3];
	WORD registry_network_port;
	int8 gap_175A[82];
	s_network_adapter_configuration network_adapter;
};
ASSERT_STRUCT_SIZE(s_network_configuration, 11192);

/* prototypes */

void network_configuration_apply_patches(void);

s_network_configuration* global_network_configuration_get(void);

int32 __cdecl network_adapter_index_get(void);

const char* __cdecl network_adapter_name_get(int32 network_adapter_index);

// initializes some interface that's used to download the network config from the bungie's website but just a stub in release
void __cdecl network_configuration_initialize(void);

void __cdecl get_network_adapters(void);
