#include "stdafx.h"
#include "simulation.h"

#include "simulation_queue_global_events.h"
#include "simulation_entity_database.h"
#include "simulation_event_handler.h"
#include "simulation_watcher.h"

#include "cseries/debug_memory.h"
#include "debug/debug_gamestate.h"
#include "debug/debug_simulation_globals.h"
#include "debug/debug_update.h"
#include "game/game.h"
#include "game/game_time.h"
#include "objects/objects.h"
#include "simulation/game_interface/simulation_game_action.h"
#include "Networking/messages/network_messages_simulation_synchronous.h"
#include "Networking/NetworkMessageTypeCollection.h"



#define GAME_STATE_ALLOCATION_SIZE 0x3FE000
#define GAME_STATE_ALLOCATION_BASE 0x30000000

s_network_message_synchronous_update g_host_synchronous_message;

s_simulation_globals* simulation_get_globals()
{
    return Memory::GetAddress<s_simulation_globals*>(0x5178D0, 0x520B60);
}

c_simulation_world* simulation_get_world()
{
    return simulation_get_globals()->world;
}

bool simulation_engine_initialized()
{
    return simulation_get_globals()->initialized;
}

bool simulation_aborted()
{
    //return INVOKE(0x1ADD8E, 0x0);  
    return simulation_engine_initialized() && simulation_get_globals()->aborted;
}

bool simulation_reset_in_progress()
{
    return simulation_get_globals()->simulation_reset_in_progress;
}

bool simulation_starting_up(void)
{
    bool result = false;
    s_simulation_globals* simulation_globals = simulation_get_globals();
    if (simulation_globals->initialized)
    {
        ASSERT(simulation_globals->world);
        if (!simulation_globals->aborted && simulation_globals->world->exists())
        {
            result = !simulation_globals->world->is_active();
        }
    }

    return result;
}

void simulation_notify_reset_complete()
{
    s_simulation_globals* sim_globals = simulation_get_globals();
    if (!game_is_playback() && simulation_reset_in_progress())
    {
        // make sure simulation is still in resetting
        if (sim_globals->world->exists())
        {
            if (!sim_globals->world->is_authority())
            {
                // dont need this if we are not authority
                sim_globals->world->send_player_acknowledgements(true);
            }
        }
    }
    sim_globals->simulation_reset_in_progress = false;
}

void simulation_reset_immediate()
{
    s_simulation_globals* sim_globals = simulation_get_globals();

    sim_globals->simulation_reset_in_progress = true;
    sim_globals->world->reset_world();
    simulation_queue_game_global_event_insert(_simulation_queue_game_global_event_main_reset_map);
    // ### TODO figure out these
    // simulation_gamestate_entities_build_clear_flags();
    // simulation_queue_gamestates_delete_insert();
    simulation_queue_game_global_event_insert(_simulation_queue_game_global_event_notify_reset_complete);
}


void __cdecl simulation_reset()
{
    s_simulation_globals* sim_globals = simulation_get_globals();
    if (sim_globals->simulation_invalidate)
    {
        sim_globals->simulation_invalidate = false;
    }
    else
    {
        // this will use the main game simulation reset code
        // but we don't need it
        //sim_globals->simulation_reset_pending = true;

        // instead, call reset directly
        simulation_reset_immediate();
    }
}

void simulation_fatal_error(void)
{
    //INVOKE(0x1ADEFE, 0x0, simulation_fatal_error);
    simulation_get_globals()->fatal_error = true;
}

bool simulation_in_progress()
{
    bool result = false;

    if (simulation_engine_initialized()
        && game_in_progress()
        && game_get_active_structure_bsp_index() != NONE
        && !simulation_aborted()
        && simulation_get_world()->is_active())
    {
        result = true;
    }

    return result;
}

bool simulation_query_object_is_predicted(datum object_datum)
{
    return game_is_predicted() && object_get_fast_unsafe(object_datum)->simulation_entity_index != NONE;
}

void __cdecl simulation_process_input(uint32 player_action_mask, const player_action* player_actions)
{
    INVOKE(0x1ADDA9, 0x1A8160, simulation_process_input, player_action_mask, player_actions);
    return;
}

void __cdecl simulation_start()
{
    INVOKE(0x1ADCE3, 0x0, simulation_start);

    debug_simulation_initialize();
}

void __cdecl simulation_end()
{
    INVOKE(0x1AE8D1, 0x0, simulation_end);
}

void simulation_update_hook()
{
    // call orignal 
    INVOKE(0x1AE7C5, 0x0, simulation_update_hook);

    
    if (simulation_get_globals()->initialized && !simulation_get_globals()->aborted && game_in_progress()
        && debug_simulation_active() && debug_simulation_is_replaying() && !simulation_starting_up())
    {
        bool match_remote_time;
        int32 available_updates = simulation_get_world()->time_get_available(&match_remote_time);

        if(debug_simulation_replay_has_updates())
        {
            if (!debug_simulation_retrieve_updates())
            {
                LOG_INFO_SIM("simulation:global:debug failed to fetch film updates ");
            }
        }
        else if(available_updates == NULL)
        {
            LOG_INFO_SIM("simulation:global:debug ran out updates , pausing replay! remaining film ticks : {} ", debug_simulation_replay_update_queue_length());
            debug_simulation_pause(true);
            debug_simulation_stop_replay();
        }

    }
}

void simulation_notify_players_created_hook()
{
    s_simulation_globals* simulation_globals = simulation_get_globals();
    if (simulation_globals->initialized && !simulation_globals->loading_saved_game
        && !simulation_globals->world->is_playback())
    {
        INVOKE(0x1ADC4B, 0x0, simulation_notify_players_created_hook);
    }
}



c_simulation_type_collection* simulation_get_type_collection()
{
    return c_simulation_type_collection::get();
}


void __cdecl simulation_apply_before_game(simulation_update* update)
{
    // allow global seed usage inside game_tick
    // TODO : rewrite game_tick
    random_seed_allow_use();

    c_simulation_queue simulation_bookkeeping_queue, game_simulation_queue;
    c_simulation_world* sim_world = simulation_get_world();

    simulation_get_globals()->world->queues_update_statistsics();

    // only during distributed system or server synchronous
    // but not client synchronous
	// transfer the elements to the 

	sim_world->attach_simulation_queues_to_update(
		update->simulation_in_progress,
		&simulation_bookkeeping_queue,
		&game_simulation_queue
	);
    if (sim_world->runs_simulation() && !sim_world->is_distributed())
    {
        // either local or synchronous authority
        g_host_synchronous_message.game_simulation_queue.clear();
        g_host_synchronous_message.simulation_bookkeeping_queue.clear();

        g_host_synchronous_message.game_simulation_queue.duplicate(&game_simulation_queue);
        g_host_synchronous_message.simulation_bookkeeping_queue.duplicate(&simulation_bookkeeping_queue);
        csmemcpy(&g_host_synchronous_message.update, update, sizeof(simulation_update));

        if (g_host_synchronous_message.game_simulation_queue.queued_count()
            || g_host_synchronous_message.simulation_bookkeeping_queue.queued_count())
        {
            LOG_TRACE_SIM(" {} host has bookkeeping_queue count : {} game_simulation_queue count : {} ", __FUNCTION__,
                g_host_synchronous_message.simulation_bookkeeping_queue.queued_count(), g_host_synchronous_message.game_simulation_queue.queued_count());
        }


        // really need a better way to to do this
        // probably rewrite game_tick() at some point
    }
    else if(sim_world->get_world_type() == _simulation_world_type_synchronous_client)
    {
        // if we dont run simulation means we are sychronous-client
        // applying client hacks here
        //random_math_set_seed(update->random_seed);
        //time_globals::get()->tick_count = update->game_time_ticks;
        // "if it works it works" 
    }

    if (update->game_time_ticks == 0)
    {
        // maybe not a good idea to record here?
        if(debug_simulation_is_recording())
        {
            debug_gamestate_record_current_state();
            debug_update_queue_clear();
        }
    }

    debug_update_record_update(update, &simulation_bookkeeping_queue, &game_simulation_queue);

    for (int32 i = 0; i < k_maximum_players; i++)
    {
        datum control_unit_index = update->control_unit_index[i];
        if (TEST_BIT(update->unit_control_mask, i) && object_try_and_get_and_verify_type(control_unit_index, FLAG(_object_type_vehicle) | FLAG(_object_type_biped)))
        {
            unit_control(control_unit_index, &update->unit_control[i]);
        }
    }
    
    if (update->machine_update_valid)
    {
        players_set_machines(update->machine_update.machine_valid_mask, update->machine_update.identifiers);
    }

    if (simulation_bookkeeping_queue.queued_count()>0)
    {
        LOG_TRACE_SIM(" {} simulation_bookkeeping_queue  has count : {} ", __FUNCTION__, simulation_bookkeeping_queue.queued_count());
        sim_world->apply_simulation_queue(&simulation_bookkeeping_queue, update);
    }

    // Player activation code
    /* Moved so we can activate in the queue
    s_simulation_globals* globals = simulation_get_globals();
    if (update->player_update_count > 0)
    {
        bool fatal_error = false;
        for (int32 player_update_index = 0; simulation_players_apply_update(&update->player_updates[player_update_index]); player_update_index++)
        {
            // Get out of here if we've overflown
            if (player_update_index >= update->player_update_count) 
            {
                fatal_error = true;
                break;
            }
        }
        
        //  Set bool to true ONLY if overflown is false, don't change otherwise
        if (!fatal_error)
        {
            globals->fatal_error = true;
        }
    }*/

    // #########
    // ### FIXME 
    // IMPLEMENT simulation_get_world()->queue_get(_simulation_queue_basic)->requires_application();

	if (game_simulation_queue.queued_count() > 0)
	{
        LOG_TRACE_SIM(" {} game_simulation_queue  has count : {} ", __FUNCTION__, game_simulation_queue.queued_count());
		sim_world->apply_simulation_queue(&game_simulation_queue, update);

		// purge any deletion pending object during this update
		// if simulation is not in progress
		if (!update->simulation_in_progress)
			objects_purge_deleted_objects();
	}

    if (update->flush_gamestate)
    {
        //LOG_CRITICAL_NETWORK("simulation:global: trying to flush gamestate world type {}", sim_world->get_world_type());
        //simulation_get_globals()->world->gamestate_flush_immediate();
    }

	// destroy the update exactly after we applied the queues to the gamestate
	simulation_bookkeeping_queue.dispose();
	game_simulation_queue.dispose();
	//simulation_destroy_update();

    return;
}


bool debugging_last_tick = false;
simulation_update debug_update;
uint8* g_debug_gamestate_buffer = nullptr;

void __cdecl simulation_build_update(simulation_update* update)
{
    //INVOKE(0x1ADDF3, 0x1A81AA, simulation_build_update, update);

    ASSERT(simulation_engine_initialized());
    if (simulation_aborted())
    {
        LOG_ERROR_SIM("simulation aborted inside game update!");
    }
    ASSERT(game_in_progress());
    ASSERT(update);

    s_simulation_globals* globals = simulation_get_globals();
    csmemset(update, 0, sizeof(simulation_update));

    if (debug_simulation_active() && debug_simulation_is_replaying())
    {
        if(globals->world->update_queue_length() >0)
        {
            globals->world->update_queue_retrieve_update(update);
            LOG_DEBUG_SIM("simulation:global:debug fetching update from m_synchronous_client_queue now /update->time {}/{}",
                globals->world->get_time(), update->game_time_ticks);
            g_simulation_debug_globals.current_replaying_tick = update->game_time_ticks;
        }
        else
        {
            LOG_ERROR_SIM("simulation:global:debug we dont have any more updates to fetch! errrrrrrrrrrrrrror");
            ASSERT(false);
        }
        //ASSERT(false);
        return;
    }

    globals->world->build_update(update);


    bool go_oos = false;

    if ((!globals->world->is_authority() || globals->world->is_playback())
        && (!globals->world->is_distributed() || globals->world->is_playback())
        && !globals->world->is_out_of_sync())
    {

        //set_random_seed(update->random_seed);
        //time_globals::get()->tick_count = update->game_time_ticks;

        if (update->simulation_time != globals->world->get_next_update_number())
        {
            //"simulation:global: OUT OF SYNC, update number differs, update [#%d] != next [#%d]",
            LOG_CRITICAL_SIM("simulation:global: OUT OF SYNC, update number differs, update [#{}] != next [#{}]", update->simulation_time, globals->world->get_next_update_number());
            go_oos = true;
        }

        if (update->game_time_ticks != globals->world->get_time())
        {
            //simulation:global: OUT OF SYNC, update time differs, update [#%d] time [%d] != local time %d"
            LOG_CRITICAL_SIM("simulation:global: OUT OF SYNC, update time differs, update [#{}] time [{}] != local time {}", update->simulation_time, update->game_time_ticks, globals->world->get_time());
            go_oos = true;
        }
        if (update->random_seed != get_random_seed())
        {
            //simulation:global: OUT OF SYNC, random seed differs, update [#%d] time [%d] seed [0x%08X] (local seed [0x%08X])"
            int32 seed_off_index;
            seed_iterate_until_point(get_random_seed(), update->random_seed, &seed_off_index);
            LOG_CRITICAL_SIM("simulation:global: OUT OF SYNC, random seed differs, update [#{}] time [{}] seed [0x{:08X}] (local seed [0x{:08X}]), seed off by: {}",
                update->simulation_time, 
                update->game_time_ticks, 
                update->random_seed, 
                get_random_seed(),
                seed_off_index);
            go_oos = true;
        }
    }
    if (go_oos)
    {

        globals->world->go_out_of_sync();
        debug_simulation_notify_oos();
        
        //tell the authority we are going oos
        s_network_message_synchronous_actions sync_actions;
        csmemset(&sync_actions, NULL, sizeof(s_network_message_synchronous_actions));
        sync_actions.out_of_sync = true;
        globals->world->get_authority_view()->send_message(_synchronous_actions, sizeof(s_network_message_synchronous_actions), &sync_actions, false);

        //// doesnt really help much
        //set_random_seed(update->random_seed);
        //time_globals::get()->tick_count = update->game_time_ticks;
    }

    return;
}

void simulation_record_and_apply_state(simulation_update* update)
{
    s_simulation_globals* globals = simulation_get_globals();
    if(globals->world->is_out_of_sync())
    {
        LOG_CRITICAL_NETWORK("simulation:global:debug calling dump random seed on client");
        //random_math_dump_call_stack();
        
        LOG_CRITICAL_NETWORK("simulation:global:debug we are pausing the game.. waiting on you to start debugging");
        time_globals::get()->paused = true;
        debugging_last_tick = true;

        //break the game here wait on debugger to resume
        ASSERT(false);


        typedef void(__cdecl game_state_call_before_load_procs)(int context);
        auto p_game_state_call_before_load_procs = Memory::GetAddress<game_state_call_before_load_procs*>(0x8C245);

        typedef void(__cdecl game_state_call_after_load_procs)(int context);
        auto p_game_state_call_after_load_procs = Memory:: GetAddress<game_state_call_after_load_procs*>(0x8C269);

        LOG_CRITICAL_NETWORK("simulation:global:debug reverting gamestate");
        ASSERT(g_debug_gamestate_buffer);

        p_game_state_call_before_load_procs(0);
        csmemcpy((void*)GAME_STATE_ALLOCATION_BASE, g_debug_gamestate_buffer, GAME_STATE_ALLOCATION_SIZE);
        p_game_state_call_after_load_procs(0);

        csmemcpy(update, &debug_update, sizeof(simulation_update));
        time_globals::get()->paused = false;

        LOG_CRITICAL(rng_math_log, "simulation:global:debug starting debug tick calls for tick {} ", time_globals::get_game_time());
    }
    else
    {
        typedef void(__cdecl game_state_call_before_save_procs)(int context);
        auto p_game_state_call_before_save_procs = Memory::GetAddress<game_state_call_before_save_procs*>(0x8C21B);

        typedef void(__cdecl game_state_call_after_save_procs__)(int context);
        auto p_game_state_call_after_save_procs = Memory::GetAddress<game_state_call_after_save_procs__*>(0x8C23F);

        if (g_debug_gamestate_buffer == nullptr)
        {
            LOG_CRITICAL_NETWORK("simulation:global:debug initializing save memory");
            g_debug_gamestate_buffer = new uint8[GAME_STATE_ALLOCATION_SIZE];
        }
        else
        {
            ASSERT(g_debug_gamestate_buffer);
            p_game_state_call_before_save_procs(0);
            csmemcpy(g_debug_gamestate_buffer, (void*)GAME_STATE_ALLOCATION_BASE, GAME_STATE_ALLOCATION_SIZE);
            p_game_state_call_after_save_procs(0);

            csmemcpy(&debug_update, update, sizeof(simulation_update));
            
        }
        
    }
}

void __cdecl simulation_update_aftermath(simulation_update* update)
{
    INVOKE(0x1ADEA9, 0x1A8260, simulation_update_aftermath, update);
    return;
}

void __cdecl simulation_update_pregame(void)
{
    simulation_update update;
    s_simulation_globals* globals = simulation_get_globals();

    if (globals->initialized && game_in_progress() && !simulation_aborted())
    {
        if (globals->watcher->need_to_generate_updates())
        {
            simulation_build_update(&update);
            simulation_apply_before_game(&update);
            random_seed_disallow_use(_random_seed_in_simulation_update_pregame);
            simulation_update_aftermath(&update);
        }
        else
        {
            globals->world->queues_update_statistsics();
        }
    }
}

void simulation_destroy_update()
{
    return;
}

bool __cdecl simulation_get_machine_active_in_game(s_machine_identifier* machine_identifier)
{
    return INVOKE(0x1AE0CB, 0x1A8482, simulation_get_machine_active_in_game, machine_identifier);
}

int32 __cdecl simulation_time_get_maximum_available(bool* match_remote_time)
{
    //return INVOKE(0x1ADCBB, 0x0, simulation_time_get_maximum_available, match_remote_time);

    *match_remote_time = 0;
    int32 available_updates = INT16_MAX;


    if (simulation_engine_initialized() && simulation_get_world()->exists())
        available_updates = simulation_get_world()->time_get_available(match_remote_time);

    return available_updates;
}

void __cdecl simulation_build_player_updates(int32* player_update_count, int32 maximum_player_update_count, simulation_player_update* player_updates)
{
    simulation_get_globals()->watcher->generate_player_updates(player_update_count, maximum_player_update_count, player_updates);
    for (int32 i = 0; i < *player_update_count; i++)
    {
        simulation_queue_player_update_insert(&player_updates[i]);
    }
    return;
}

c_simulation_view* __cdecl simulation_get_remote_view_by_channel(uint32 channel_index)
{
    return INVOKE(0x1ADF06, 0x0, simulation_get_remote_view_by_channel, channel_index);
}

s_network_message_synchronous_update* simulation_get_synchronous_message()
{
    return &g_host_synchronous_message;
}

void simulation_apply_patches(void)
{
    // ### TODO move somewhere else, network related
    network_memory_apply_patches();

    simulation_event_handler_apply_patches();
    simulation_world_apply_patches();
    simulation_watcher_apply_patches();
    simulation_entity_database_apply_patches();
    simulation_game_action_apply_patches();

    PatchCall(Memory::GetAddress(0x39D73, 0xC0F8), simulation_update_pregame);
    PatchCall(Memory::GetAddress(0x4A4D5, 0), simulation_build_update); //hook for logging oos
    PatchCall(Memory::GetAddress(0x4A4DF, 0x4375D), simulation_apply_before_game);
    PatchCall(Memory::GetAddress(0x1DD22F, 0x1C46E3), simulation_build_player_updates);
    PatchCall(Memory::GetAddress(0x39C97, 0), simulation_update_hook);
    PatchCall(Memory::GetAddress(0x7C2BD, 0), simulation_time_get_maximum_available);//inside game_time_update
    PatchCall(Memory::GetAddress(0x49F2C, 0), simulation_notify_players_created_hook); // inside game_create_players


    //PatchCall(Memory::GetAddress(0x1AE002), synchronous_update_encode); //inside simulation_update_write_to_buffer
    PatchCall(Memory::GetAddress(0x1DE9A4), synchronous_update_write_to_buffer); //inside view::synchronous_catchup_submit_update
    PatchCall(Memory::GetAddress(0x1ED08E), synchronous_update_encode); //inside c_network_message_synchronous_update::encode
    //PatchCall(Memory::GetAddress(0x1AE084), synchronous_update_decode); //inside simulation_update_read_from_buffer
    PatchCall(Memory::GetAddress(0x1DF3BF), synchronous_update_read_from_buffer); //inside view::synchronous_catchup_send_data
    PatchCall(Memory::GetAddress(0x1ED0A3), synchronous_update_decode); //inside c_network_message_synchronous_update::decode
    c_simulation_view::apply_patches();


    WriteJmpTo(Memory::GetAddress(0x1AE6D8, 0x1A8932), simulation_reset);
    return;
}