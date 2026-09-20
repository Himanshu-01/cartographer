#pragma once


/* public code */

bool debug_update_record_update(struct simulation_update* update);
bool debug_update_retrieve_latest_update(struct simulation_update* out_update);
void debug_update_read_from_chunk(struct s_simulation_debug_chunk* chunk);

void debug_update_queue_initialize_for_load();
void debug_update_queue_clear();
void debug_update_queue_dispose();
void debug_update_memory_initialize_for_playback();
void debug_update_memory_clear();
