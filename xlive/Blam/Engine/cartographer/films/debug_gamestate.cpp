#include "stdafx.h"
#include "debug_gamestate.h"
#include "debug_simulation_globals.h"

#include "cseries/debug_memory.h"
#include "memory/data_compress.h"
#include "networking/network_event.h"
#include "saved_games/game_state.h"
#include "saved_games/game_state_procs.h"

/* constants */

enum
{
    k_simulation_debug_gamestate_save_flag = 0,
    k_simulation_debug_gamestate_load_flag = 0,
    k_simulation_debug_data_compression_scratch_size = 0x4b000,
    k_simulation_debug_data_decompression_scratch_size = 0x10000
};

/* enums */

/* structures */

/* globals */

/* prototypes */

/* public code */

bool debug_gamestate_read_header(game_state_header* out_header)
{
    ASSERT(out_header);

    if(g_simulation_debug_globals.replay_has_gamestate)
    {
        if (out_header != nullptr && g_simulation_debug_globals.gamestate_write_buffer != nullptr)
        {
            csmemcpy(out_header, g_simulation_debug_globals.gamestate_write_buffer, sizeof(game_state_header));
            return true;
        }
    }
    return false;
}

bool debug_gamestate_read_header_runtime(game_state_header* out_header)
{
    bool result = false;
    uint32 buffer_size = 0;
    uint8* gamestate_buffer = (uint8*)game_state_get_buffer_address(&buffer_size);

    game_state_call_before_save_procs(k_simulation_debug_gamestate_save_flag);
    if (out_header != nullptr && gamestate_buffer != nullptr)
    {
        csmemcpy(out_header, gamestate_buffer, sizeof(game_state_header));
        result = true;
    }
    game_state_call_after_save_procs(k_simulation_debug_gamestate_save_flag);
    return result;
}

bool debug_gamestate_write_compressed_gamestate_to_buffer(uint8* temporary_buffer, uint32 temporary_buffer_size , uint32* compressed_size_out)
{
    const int32 opaque_data = 9;
    uint8* scratch_buffer = (uint8*)CSERIES_MALLOC(k_simulation_debug_data_compression_scratch_size);
    uint32 gamestate_buffer_size = NULL;
    uint8* gamestate_buffer = g_simulation_debug_globals.gamestate_write_buffer;
    game_state_get_buffer_address(&gamestate_buffer_size);

    ASSERT(gamestate_buffer != nullptr);
    ASSERT(scratch_buffer != nullptr);

    bool success = false;

    if (runtime_data_compress(
        gamestate_buffer,
        gamestate_buffer_size,
        temporary_buffer,
        compressed_size_out,
        temporary_buffer_size,
        opaque_data,
        k_simulation_debug_data_compression_scratch_size,
        scratch_buffer))
    {
        event(_event_message,
            "cartographer:saved_film: gamestate compressed successfully,  0x:%08x ----> 0x%08X bytes",
            gamestate_buffer_size,
            *compressed_size_out            
            );

        success = true;
    }
    else
    {
        event(_event_error, "cartographer:saved_film: failed to compress gamestate!");
        //compression failed check size etc..
        success = false;
    }

    if (scratch_buffer)
        CSERIES_FREE(scratch_buffer);

    return success;
}


bool debug_gamestate_read_compressed_gamestate_from_buffer(uint8* temporary_buffer, uint32 temporary_buffer_size, uint32* decompressed_size_out)
{
    const int32 opaque_data = 9;
    uint8* scratch_buffer = (uint8*)CSERIES_MALLOC(k_simulation_debug_data_compression_scratch_size);
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
        k_simulation_debug_data_compression_scratch_size,
        scratch_buffer)

        && (*decompressed_size_out == gamestate_buffer_size))
    {
        event(_event_message,
            "cartographer:saved_film: gamestate decompressed successfully ,  0x:%08x ----> 0x%08X bytes",
            temporary_buffer_size,
            *decompressed_size_out
        );

        success = true;
    }
    else
    {
        event(_event_error, "cartographer:saved_film: failed to decompress gamestate!");
        //decompression failed check size etc..
        success = false;
    }

    CSERIES_FREE(scratch_buffer);

    return success;
}

void debug_gamestate_read_from_chunk(s_simulation_debug_chunk* chunk)
{
    if (debug_simulation_active())
    {
        uint8* gamestate_compressed_buffer = (uint8*)CSERIES_MALLOC(chunk->chunk_size);
        if (file_read_from_position(
            &g_simulation_debug_globals.save_file,
            chunk->file_offset,
            chunk->chunk_size,
            true,
            gamestate_compressed_buffer))
        {
            event(_event_verbose, "cartographer:saved_film: successfully read gamestate chunk from save file ");

            //  initialize gamestate_write_buffer if we havent already
            debug_gamestate_memory_initialize();

            uint32 out_decompressed_size = NULL;
            if (debug_gamestate_read_compressed_gamestate_from_buffer(
                gamestate_compressed_buffer,
                chunk->chunk_size,
                &out_decompressed_size))
            {
                g_simulation_debug_globals.replay_has_gamestate = true;

                game_state_header saved_header;
                debug_gamestate_read_header(&saved_header);
                debug_gamestate_compare_header_with_runtime(&saved_header);

                event(_event_verbose, "cartographer:saved_film: successfully read gamestate from saved film ");
            }
            else
            {
                event(_event_warning, "cartographer:saved_film: failed to decompress gamestate from saved chunk");
            }

        }
        else
        {
            event(_event_error, "cartographer:saved_film: failed in file_read_from_position");
        }


        if (gamestate_compressed_buffer)
        {
            CSERIES_FREE(gamestate_compressed_buffer);
            event(_event_verbose, "cartographer:saved_film: clearing compressed buffer ");
        }
    }
}

void debug_gamestate_compare_headers(game_state_header* first, game_state_header* second)
{
    ASSERT(first);
    ASSERT(second);

    event(_event_status, "cartographer:saved_film:gamestate: - - > comparing game state headers begin ------- >");

    event(_event_verbose, " - - > base_address         0x%08X ,  0x%08X", first->base_address, second->base_address);
    event(_event_verbose, " - - > alloc_checksum       0x%08X ,  0x%08X", first->alloc_checksum, second->alloc_checksum);
    event(_event_verbose, " - - > build                %s ,  %s ", first->game_build, second->game_build);
    event(_event_verbose, " - - > map_checksum         0x%08X ,  0x%08X", first->map_checksum, second->map_checksum);
    event(_event_verbose, " - - > scenario_name        %s ,  %s", first->scenario_name, second->scenario_name);
    event(_event_verbose, " - - > structure_bsp_index  %d ,  %d", first->active_bsp_index, second->active_bsp_index);

    event(_event_verbose, " - - > options.game_mode    %d ,  %d", first->options.game_mode, second->options.game_mode);
    event(_event_verbose, " - - > options.map_id       %d ,  %d", first->options.map_id, second->options.map_id);
    event(_event_verbose, " - - > options.campaign_id  %d ,  %d", first->options.campaign_id, second->options.campaign_id);
    event(_event_verbose, " - - > options.scenario     %S ,  %S", first->options.scenario_path, second->options.scenario_path);
    event(_event_verbose, " - - > options.initial_bsp  %d ,  %d", first->options.initial_bsp_index, second->options.initial_bsp_index);
    event(_event_verbose, " - - > options.random_seed  0x%08X ,  0x%08X", first->options.verify_random_seed, second->options.verify_random_seed);


    event(_event_status, "cartographer:saved_film:gamestate: - - < comparing game state headers end ------- <");
}

void debug_gamestate_compare_header_with_runtime(game_state_header* saved)
{
    game_state_header runtime;
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
        game_state_call_before_save_procs(k_simulation_debug_gamestate_save_flag);
        csmemcpy(g_simulation_debug_globals.gamestate_write_buffer, gamestate_buffer, gamestate_buffer_size);
        game_state_call_after_save_procs(k_simulation_debug_gamestate_save_flag);

        event(_event_status, "cartographer:saved_film:gamestate: captured current gamestate");
        g_simulation_debug_globals.captured_gamestate = true;

    }
    else
    {
        event(_event_status, "cartographer:saved_film:gamestate: not allowed to capture gamestate");
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

        event(_event_status, "cartographer:saved_film:gamestate: initializing save memory");
        uint8* heap = (uint8*)CSERIES_MALLOC(gamestate_buffer_size);

        if (heap)
        {
            g_simulation_debug_globals.gamestate_write_buffer = heap;
        }
        else
        {
            event(_event_error, "cartographer:saved_film:gamestate: OUT OF MEMORY unable to allocate gamestate save memory");
            g_simulation_debug_globals.record_gamestate = false;
            g_simulation_debug_globals.captured_gamestate = false;
        }

    }
}

void debug_gamestate_memory_clear()
{
    if (g_simulation_debug_globals.gamestate_write_buffer)
    {
        CSERIES_FREE(g_simulation_debug_globals.gamestate_write_buffer);
        g_simulation_debug_globals.gamestate_write_buffer = nullptr;
        g_simulation_debug_globals.captured_gamestate = false;

        event(_event_status, "cartographer:saved_film:gamestate: freed gamestate save memory");
    }
}

void debug_gamestate_apply_saved_state()
{
    if (g_simulation_debug_globals.gamestate_write_buffer == nullptr)
    {
        event(_event_error, "cartographer:saved_film:gamestate: has no saved memory to apply!!!");
        debug_simulation_stop_replay();
    }
    else if (debug_simulation_active()
        && debug_simulation_is_replaying()
        && debug_simulation_replay_has_gamesave())
    {
        if (!g_simulation_debug_globals.replay_applied_gamestate)
        {
            event(_event_status, "cartographer:saved_film:gamestate: applying recorded gamestate...");

            uint32 gamestate_buffer_size = NULL;
            uint8* gamestate_buffer = (uint8*)game_state_get_buffer_address(&gamestate_buffer_size);
            game_state_call_before_load_procs(k_simulation_debug_gamestate_load_flag);
            csmemcpy(gamestate_buffer, g_simulation_debug_globals.gamestate_write_buffer, gamestate_buffer_size);
            game_state_call_after_load_procs(k_simulation_debug_gamestate_load_flag);

            g_simulation_debug_globals.replay_applied_gamestate = true;
            g_simulation_debug_globals.replay_reset_time = true;
        }
        else
        {
            event(_event_warning, "cartographer:saved_film:gamestate: already applied gamestate once");
        }
    }
    else
    {
        event(_event_warning, "cartographer:saved_film:gamestate: failed to apply recorded gamestate");
    }
}

