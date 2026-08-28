#pragma once

/* enums */

enum e_replication_entity_flags
{
	_replication_entity_allocated_flag = 0,
	_replication_entity_marked_for_deletion_flag = 1,
	_replication_entity_local_flag = 2,
	_replication_entity_collection_master_flag = 3,
	_replication_entity_collection_slave_flag = 4,
};

/* prototypes */

// Get the replication entity index
void replication_entity_index_decode(class c_bitstream* bitstream, int32* replication_entity_index);

// Get the replication entity index as well as the entity abs index
void replication_entity_index_decode_get_abs_entity_index(class c_bitstream* bitstream, int32* entity_index, uint32* entity_abs_index);

