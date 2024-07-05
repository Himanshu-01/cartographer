#include "stdafx.h"
#include "debug_gamestate.h"
#include "debug_simulation_constants.h"
#include "debug_simulation_globals.h"
#include "cseries/debug_memory.h"
#include "memory/data_compress.h"
#include "saved_games/game_state.h"
#include "saved_games/game_state_procs.h"

bool debug_gamestate_read_header(s_game_state_header* out_header)
{
    if(g_simulation_debug_globals.replay_has_gamestate)
    {
        if (out_header != nullptr && g_simulation_debug_globals.gamestate_write_buffer != nullptr)
        {
            csmemcpy(out_header, g_simulation_debug_globals.gamestate_write_buffer, sizeof(s_game_state_header));
            return true;
        }
    }
    return false;
}

bool debug_gamestate_read_header_runtime(s_game_state_header* out_header)
{
    bool result = false;
    uint8* gamestate_buffer = (uint8*)game_state_get_buffer_address(nullptr);

    game_state_call_before_save_procs(K_SIMULATION_DEBUG_GAMESTATE_SAVE_FLAG);
    if (out_header != nullptr && gamestate_buffer != nullptr)
    {
        csmemcpy(out_header, gamestate_buffer, sizeof(s_game_state_header));
        result = true;
    }
    game_state_call_after_save_procs(K_SIMULATION_DEBUG_GAMESTATE_SAVE_FLAG);
    return result;
}

bool debug_gamestate_write_compressed_gamestate_to_buffer(uint8* temporary_buffer, uint32 temporary_buffer_size , uint32* compressed_size_out)
{
    const int32 opaque_data = 9;
    uint8* scratch_buffer = debug_malloc(K_SIMULATION_DEBUG_DATA_COMPRESSION_SCRATCH_SIZE);
    uint32 gamestate_buffer_size = NULL;
    uint8* gamestate_buffer = g_simulation_debug_globals.gamestate_write_buffer;
    game_state_get_buffer_address(&gamestate_buffer_size);

    ASSERT(gamestate_buffer != nullptr);
    bool success = false;

    if (runtime_data_compress(
        gamestate_buffer,
        gamestate_buffer_size,
        temporary_buffer,
        compressed_size_out,
        temporary_buffer_size,
        opaque_data,
        K_SIMULATION_DEBUG_DATA_COMPRESSION_SCRATCH_SIZE,
        scratch_buffer))
    {
        LOG_TRACE_SIM("{} - gamestate compressed successfully.... old size : 0x{:08X} ----> new size 0x{:08X} bytes ", __FUNCTION__, gamestate_buffer_size, *compressed_size_out);
        success = true;
    }
    else
    {
        //compression failed check size etc..
        success = false;
    }
    debug_free(scratch_buffer);

    return success;
}


bool debug_gamestate_read_compressed_gamestate_from_buffer(uint8* temporary_buffer, uint32 temporary_buffer_size, uint32* decompressed_size_out)
{
    const int32 opaque_data = 9;
    uint8* scratch_buffer = debug_malloc(K_SIMULATION_DEBUG_DATA_DECOMPRESSION_SCRATCH_SIZE);
    uint32 gamestate_buffer_size = NULL;
    uint8* gamestate_buffer = g_simulation_debug_globals.gamestate_write_buffer;
    game_state_get_buffer_address(&gamestate_buffer_size);

    ASSERT(gamestate_buffer != nullptr);
    bool success = false;

    if (runtime_data_decompress(
        temporary_buffer,
        temporary_buffer_size,
        gamestate_buffer,
        decompressed_size_out,
        opaque_data,    //probably opaque data goes here
        K_SIMULATION_DEBUG_DATA_DECOMPRESSION_SCRATCH_SIZE,
        scratch_buffer)

        && (*decompressed_size_out == gamestate_buffer_size))
    {
        LOG_TRACE_SIM("{} - gamestate decompressed successfully....old size : 0x{:08X} ----> new size 0x{:08X} bytes ", __FUNCTION__, temporary_buffer_size, *decompressed_size_out);
        success = true;
    }
    else
    {
        //decompression failed check size etc..
        success = false;
    }

    debug_free(scratch_buffer);

    return success;
}

void debug_gamestate_read_from_chunk(s_simulation_debug_chunk* chunk)
{
    if (debug_simulation_active())
    {
        uint8* gamestate_compressed_buffer = debug_malloc(chunk->chunk_size);
        if (file_read_from_position(&g_simulation_debug_globals.save_file, chunk->file_offset, chunk->chunk_size, true, gamestate_compressed_buffer))
        {
            LOG_TRACE_SIM("{} - successfully read gamestate data from save file ", __FUNCTION__);

            //  initialize gamestate_write_buffer if we havent already
            debug_gamestate_memory_initialize();

            uint32 out_decompressed_size = NULL;
            if (debug_gamestate_read_compressed_gamestate_from_buffer(gamestate_compressed_buffer, chunk->chunk_size, &out_decompressed_size))
            {
                g_simulation_debug_globals.replay_has_gamestate = true;

                s_game_state_header saved_header;
                debug_gamestate_read_header(&saved_header);
                debug_gamestate_compare_header_with_runtime(&saved_header);
            }
            else
            {
                LOG_ERROR_SIM("{} - failed to decompress gamestate!", __FUNCTION__);
            }

        }
        else
        {
            LOG_ERROR_SIM("{} - failed in file_read_from_position!", __FUNCTION__);
        }


        if (gamestate_compressed_buffer)
        {
            debug_free(gamestate_compressed_buffer);
            LOG_TRACE_SIM("{} - clearing compressed buffer ", __FUNCTION__);
        }
    }
}

void debug_gamestate_compare_headers(s_game_state_header* first, s_game_state_header* second)
{
    LOG_TRACE_SIM(" - - > comparing game state headers begin ------- >");

    LOG_TRACE_SIM(" - - > base_address         0x{:08X} ,  0x{:08X}", first->base_address, second->base_address);
    LOG_TRACE_SIM(" - - > alloc_checksum       0x{:08X} ,  0x{:08X}", first->alloc_checksum, second->alloc_checksum);
    LOG_TRACE_SIM(" - - > build                {} ,  {} ", first->game_build.get_string(), second->game_build.get_string());
    LOG_TRACE_SIM(" - - > map_checksum         0x{:08X} ,  0x{:08X}", first->map_checksum, second->map_checksum);
    LOG_TRACE_SIM(" - - > scenario_name        {} ,  {}", first->scenario_name.get_string(), second->scenario_name.get_string());
    LOG_TRACE_SIM(" - - > structure_bsp_index  {} ,  {}", first->structure_bsp_index, second->structure_bsp_index);

    LOG_TRACE_SIM(" - - > options.game_mode    {} ,  {}", first->options.game_mode, second->options.game_mode);
    LOG_TRACE_SIM(" - - > options.map_id       {} ,  {}", first->options.map_id, second->options.map_id);
    LOG_TRACE_SIM(" - - > options.campaign_id  {} ,  {}", first->options.campaign_id, second->options.campaign_id);
    LOG_TRACE_SIM(L" - - > options.scenario     {} ,  {}", first->options.scenario_path.get_string(), second->options.scenario_path.get_string());
    LOG_TRACE_SIM(" - - > options.initial_bsp  {} ,  {}", first->options.initial_bsp_index, second->options.initial_bsp_index);
    LOG_TRACE_SIM(" - - > options.random_seed  0x{:08X} ,  0x{:08X}", first->options.random_seed, second->options.random_seed);


    LOG_TRACE_SIM(" - - < comparing game state headers end ------- <");
}

void debug_gamestate_compare_header_with_runtime(s_game_state_header* saved)
{
    s_game_state_header runtime;
    debug_gamestate_read_header_runtime(&runtime);
    debug_gamestate_compare_headers(&runtime, saved);
}

void debug_gamestate_record_current_state()
{

    if (debug_simulation_active()
        && debug_simulation_is_recording()
        && debug_simulation_recording_allows_gamestate())
    {
        uint32 gamestate_buffer_size = NULL;
        uint8* gamestate_buffer = (uint8*)game_state_get_buffer_address(&gamestate_buffer_size);

        debug_gamestate_memory_initialize();

        ASSERT(g_simulation_debug_globals.gamestate_write_buffer);
        game_state_call_before_save_procs(K_SIMULATION_DEBUG_GAMESTATE_SAVE_FLAG);
        csmemcpy(g_simulation_debug_globals.gamestate_write_buffer, gamestate_buffer, gamestate_buffer_size);
        game_state_call_after_save_procs(K_SIMULATION_DEBUG_GAMESTATE_SAVE_FLAG);

        LOG_TRACE_SIM("simulation:debug:gamestate has recorded a save");
        g_simulation_debug_globals.recorded_gamestate = true;

    }
    else
    {
        //cannot record in current state {init,record_allowed,update_allowed}
    }

}

void debug_gamestate_memory_initialize()
{
    if (debug_simulation_active()
        && g_simulation_debug_globals.gamestate_write_buffer == nullptr)
    {
        uint32 gamestate_buffer_size = NULL;
        game_state_get_buffer_address(&gamestate_buffer_size);
        LOG_TRACE_SIM("simulation:debug:gamestate initializing save memory");
        g_simulation_debug_globals.gamestate_write_buffer = debug_malloc(gamestate_buffer_size);
    }
}

void debug_gamestate_memory_clear()
{
    if (g_simulation_debug_globals.gamestate_write_buffer)
    {
        debug_free(g_simulation_debug_globals.gamestate_write_buffer);
        g_simulation_debug_globals.gamestate_write_buffer = nullptr;
        LOG_TRACE_SIM("simulation:debug:gamestate clearing save memory");
        g_simulation_debug_globals.recorded_gamestate = false;
    }
}

void debug_gamestate_apply_saved_state()
{
    if (g_simulation_debug_globals.gamestate_write_buffer == nullptr)
    {
        LOG_ERROR_SIM("simulation:debug:gamestate has no saved memory to apply!!!");
        debug_simulation_stop_replay();
        return;
    }

    if (debug_simulation_active() && debug_simulation_is_replaying() && debug_simulation_replay_has_gamesave())
    {
        if (!g_simulation_debug_globals.replay_applied_gamestate)
        {

            //call    game_engine_detach_from_simulation; Call Procedure
            //call    objects_detach_from_simulation; Call Procedure
            //call    breakable_surfaces_detach_from_simulation; Call Procedure
            //
            //NopFill(Memory::GetAddress(0x1AE77C, 0), 5 * 3);

            LOG_TRACE_SIM("simulation:global:debug reverting gamestate");

            uint32 gamestate_buffer_size = NULL;
            uint8* gamestate_buffer = (uint8*)game_state_get_buffer_address(&gamestate_buffer_size);
            game_state_call_before_load_procs(K_SIMULATION_DEBUG_GAMESTATE_LOAD_FLAG);
            csmemcpy(gamestate_buffer, g_simulation_debug_globals.gamestate_write_buffer, gamestate_buffer_size);
            game_state_call_after_load_procs(K_SIMULATION_DEBUG_GAMESTATE_LOAD_FLAG);

            g_simulation_debug_globals.replay_applied_gamestate = true;
        }
    }
}

