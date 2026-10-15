#include "stdafx.h"
#include "replication_entity_manager.h"
#include "replication_entity.h"

#include "simulation/simulation_entity_database.h"

void c_replication_entity_manager::reset(void)
{
	for (uint32 i = 0; i < 16; i++)
	{
		if (this->m_views[i] != NULL)
			this->m_views[i]->reset();
	}
	csmemset(this->m_entity_data, 0, sizeof(m_entity_data));
	m_entity_creation_start_position = 0;
	return;
}

void c_replication_entity_manager::initialize(void)
{
	this->m_client = NULL;
	this->m_view_mask = 0;

	for (uint32 i = 0; i < 16; i++)
	{
		this->m_views[i] = NULL;
	}

	this->reset();
	return;
}

s_replication_entity_data* c_replication_entity_manager::get_entity(int32 entity_index)
{
	ASSERT(entity_index != NONE);

	const int32 absolute_index = ENTITY_INDEX_TO_ABSOLUTE_INDEX(entity_index);
	ASSERT(VALID_INDEX(absolute_index, NUMBEROF(m_entity_data)));

	s_replication_entity_data* entity = &m_entity_data[absolute_index];

	ASSERT(TEST_BIT(entity->flags, _replication_entity_allocated_flag));
	ASSERT(entity->seed == ENTITY_INDEX_TO_SEED(entity_index));

	return entity;
}

s_replication_entity_data* c_replication_entity_manager::try_and_get_entity(int32 entity_index)
{
	s_replication_entity_data* result = NULL;
	const int32 absolute_index = ENTITY_INDEX_TO_ABSOLUTE_INDEX(entity_index);

	if (VALID_INDEX(absolute_index ,NUMBEROF(m_entity_data)))
	{
		s_replication_entity_data* entity = &m_entity_data[absolute_index];
		if (TEST_BIT(entity->flags, _replication_entity_allocated_flag)
			&& entity->seed == ENTITY_INDEX_TO_SEED(entity_index))
		{
			result = entity;
		}
	}

	return result;
}

bool c_replication_entity_manager::is_entity_local(int32 entity_index)
{
	const s_replication_entity_data* entity = get_entity(entity_index);
	return TEST_BIT(entity->flags, _replication_entity_local_flag);
}

bool c_replication_entity_manager::is_entity_allocated(int32 entity_index)
{
	const s_replication_entity_data* entity = try_and_get_entity(entity_index);
	ASSERT(entity);
	return TEST_BIT(entity->flags, _replication_entity_allocated_flag);
}

bool c_replication_entity_manager::is_entity_being_deleted(int32 entity_index)
{
	const s_replication_entity_data* entity = get_entity(entity_index);
	return TEST_BIT(entity->flags, _replication_entity_marked_for_deletion_flag);
}

bool c_replication_entity_manager::entity_is_master_collection(int32 entity_index)
{
	const s_replication_entity_data* entity = get_entity(entity_index);
	return TEST_BIT(entity->flags, _replication_entity_collection_master_flag);
}
bool c_replication_entity_manager::entity_is_slave_collection(int32 entity_index)
{
	const s_replication_entity_data* entity = get_entity(entity_index);
	return TEST_BIT(entity->flags, _replication_entity_collection_slave_flag);
}



