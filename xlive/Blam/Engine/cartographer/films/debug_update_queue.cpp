#include "stdafx.h"
#include "debug_update_queue.h"

#include "cseries/cseries.h"
#include "cseries/cseries_system_memory.h"

/* enums */

/* structures */

/* globals */

/* prototypes */

/* public code */

c_debug_update_queue::c_debug_update_queue()
{
	initialize();
}

c_debug_update_queue::~c_debug_update_queue()
{
	dispose();
}

bool c_debug_update_queue::initialized() const
{
	return m_initialized;
}

const c_debug_update_node* c_debug_update_queue::get_head() const
{
	if (initialized())
	{
		return m_head;
	}
	return NULL;
}

const c_debug_update_node* c_debug_update_queue::get_first_element() const
{
	if (initialized())
	{
		return get_head();
	}
	return NULL;
}

const c_debug_update_node* c_debug_update_queue::get_next_element(const c_debug_update_node* element) const
{
	if (initialized())
	{
		return element->next;
	}
	return NULL;
}

int32 c_debug_update_queue::allocated_count() const
{
	if (initialized())
	{
		return m_allocated_count;
	}

	return 0;
}

int32 c_debug_update_queue::allocated_size_in_bytes() const
{
	if (initialized())
	{
		return m_allocated_size_in_bytes;
	}

	return 0;
}

int32 c_debug_update_queue::get_element_size_in_bytes(const c_debug_update_node* element) const
{
	return element->data_size + sizeof(c_debug_update_node);
}

int32 c_debug_update_queue::queued_count() const
{
	if (initialized())
	{
		return m_queued_count;
	}

	return 0;
}

int32 c_debug_update_queue::queued_size() const
{
	if (initialized())
	{
		return m_queued_size;
	}

	return 0;
}

void c_debug_update_queue::initialize()
{
	m_allocated_count = 0;
	m_allocated_size_in_bytes = 0;
	m_queued_count = 0;
	m_queued_size = 0;
	m_head = NULL;
	m_tail = NULL;
	m_initialized = true;
}

//allocate memory for encoded sync-update
void c_debug_update_queue::allocate(int32 data_size, c_debug_update_node** out_allocated_elem)
{
	*out_allocated_elem = NULL;

	if (initialized())
	{
		uint32 required_data_size = sizeof(c_debug_update_node) + data_size;

		//if (allocated_count() + 1 < K_SIMULATION_DEBUG_MAX_UPDATES)
		//{
		uint8* heap_block = (uint8*)CSERIES_MALLOC(required_data_size);
		if (heap_block)
		{
			csmemset(heap_block, 0, required_data_size);
			c_debug_update_node* allocated_elem = (c_debug_update_node*)heap_block;
			allocated_elem->data = heap_block + sizeof(c_debug_update_node);
			allocated_elem->data_size = data_size;
			allocated_elem->next = nullptr;
			allocated_elem->allocated = true;

			m_allocated_count++;
			m_allocated_size_in_bytes += required_data_size;
			*out_allocated_elem = allocated_elem;
		}
		else
		{
			// DEBUG
		}

		//}
	}
}


void c_debug_update_queue::deallocate(c_debug_update_node* element)
{
	if (initialized())
	{
		m_allocated_size_in_bytes -= get_element_size_in_bytes(element);
		m_allocated_count--;
		CSERIES_FREE(element);
	}
}


void c_debug_update_queue::enqueue(c_debug_update_node* element)
{
	if (initialized())
	{
		if (m_tail)
		{
			m_tail->next = element;
		}
		else
		{
			m_head = element;
		}

		m_tail = element;
		m_queued_count++;
		m_queued_size += get_element_size_in_bytes(element);

	}
}

void c_debug_update_queue::dequeue(c_debug_update_node** out_deq_elem)
{
	*out_deq_elem = NULL;
	if (initialized())
	{
		if (m_head)
		{
			m_queued_count--;
			m_queued_size -= get_element_size_in_bytes(m_head);
			*out_deq_elem = m_head;
			m_head = m_head->next;
			(*out_deq_elem)->next = NULL;
		}
		if (m_head == NULL)
		{
			m_tail = NULL;
		}
	}
}

void c_debug_update_queue::clear()
{
	if (initialized())
	{
		while (get_head() != NULL)
		{
			c_debug_update_node* element_to_deque = NULL;
			dequeue(&element_to_deque);
			deallocate(element_to_deque);
		}
	}
}

void c_debug_update_queue::dispose()
{
	if (initialized())
	{
		clear();
		m_initialized = false;
	}
}
