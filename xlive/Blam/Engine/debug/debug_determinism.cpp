#include "stdafx.h"
#include "debug_determinism.h"
#include "debug_simulation_globals.h"
#include "game/game_time.h"
#include <stack>


std::stack<std::pair<uint32, uint32>> return_addresses;
void debug_random_record_call_entry(uint32 ret_addr)
{
	if (debug_simulation_is_recording() && debug_simulation_recording_allows_random())
	{
		return_addresses.push({ time_globals::get_game_time(), ret_addr });
		LOG_CRITICAL(rng_math_log, "simulation:global:debug logging calls to tick {} , offset 0x{:X} ", time_globals::get_game_time(), ret_addr);
	}
}

void debug_random_record_clear()
{
	while (!return_addresses.empty())
		return_addresses.pop();
}

void debug_random_dump_call_stack()
{
	LOG_TRACE(rng_math_log, "Dumping random_math call stack started");
	while (!return_addresses.empty())
	{
		auto top = return_addresses.top();
		LOG_TRACE(rng_math_log, "tick {}  offset 0x{:X}", top.first, top.second);
		return_addresses.pop();

	}
	LOG_TRACE(rng_math_log, "finished dumping call stack session");
}
