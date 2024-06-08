#include "stdafx.h"
#include "simulation_queue.h"

#include "simulation_queue_global_events.h"

void c_simulation_queue::allocate(int32 data_size, s_simulation_queue_element** out_allocated_elem)
{
	*out_allocated_elem = NULL;

	if (initialized())
	{
		uint32 required_data_size = sizeof(s_simulation_queue_element) + data_size;

		if (allocated_count() + 1 < k_simulation_queue_count_max)
		{
			if (allocated_size_in_bytes() + required_data_size < k_simulation_queue_size_max)
			{
				if (data_size < k_simulation_queue_element_data_size_max)
				{
					if (allocated_new_encoded_size_bytes(data_size) < k_simulation_queue_max_encoded_size)
					{
						uint8* net_heap_block = network_heap_allocate_block(required_data_size);
						if (net_heap_block)
						{
							csmemset(net_heap_block, 0, required_data_size);
							s_simulation_queue_element* allocated_elem = (s_simulation_queue_element*)net_heap_block;
							allocated_elem->type = _simulation_queue_element_type_none;
							allocated_elem->data = net_heap_block + sizeof(s_simulation_queue_element);
							allocated_elem->data_size = data_size;
							allocated_elem->next = NULL;

							m_allocated_count++;
							m_allocated_size_in_bytes += required_data_size;
							*out_allocated_elem = allocated_elem;
						}
						else
						{
							// DEBUG
						}
					}
				}
			}
		}
	}
}

void c_simulation_queue::transfer_elements(c_simulation_queue* source_queue)
{
	// transefrs data from the source into ours

	int32 queued_count = source_queue->queued_count();
	if (queued_count > 0)
	{
		LOG_TRACE_NETWORK(" {} transfer count : {} ", __FUNCTION__, queued_count);
	}
	for (int32 i = 0; i < queued_count; i++)
	{
		s_simulation_queue_element* element = NULL;
		source_queue->dequeue(&element);
		source_queue->m_allocated_count--;
		source_queue->m_allocated_size_in_bytes -= get_element_size_in_bytes(element);
		this->m_allocated_count++;
		this->m_allocated_size_in_bytes += get_element_size_in_bytes(element);
		this->enqueue(element);
		// if the element type is a global event
		// check if we need to cut the update here
		if (element->type == _simulation_queue_element_type_game_global_event
			&& simulation_queue_game_global_event_requires_cutoff(element))
		{
			// cut the queue here, keep the rest for the next update
			break;
		}
	}
}

void c_simulation_queue::deallocate(s_simulation_queue_element* element)
{
	if (initialized())
	{
		m_allocated_size_in_bytes -= get_element_size_in_bytes(element);
		m_allocated_count--;
		network_heap_free_block((uint8*)element);
	}
}

void c_simulation_queue::enqueue(s_simulation_queue_element* element)
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

void c_simulation_queue::dequeue(s_simulation_queue_element** out_deq_elem)
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

void c_simulation_queue::clear()
{
	if (initialized())
	{
		while (get_head() != NULL)
		{
			s_simulation_queue_element* element_to_deque = NULL;
			dequeue(&element_to_deque);
			deallocate(element_to_deque);
			// ### TODO clear allocated but not queued too??
		}
	}
}

void c_simulation_queue::dispose()
{
	if (initialized())
	{
		clear();
		m_initialized = false;
	}
}

void c_simulation_queue::duplicate(c_simulation_queue* source_queue)
{
	// retains source_queue and allocates new data for cloned elements
	// queue must be initialized before using this	

	int32 queued_count = source_queue->queued_count();
	if (queued_count > 0)
	{
		LOG_TRACE_NETWORK(" {} duplicating elements count : {} ", __FUNCTION__, queued_count);
	}
	
	for (const s_simulation_queue_element* element = source_queue->get_first_element();
		element != nullptr;
		element = source_queue->get_next_element(element))
	{
		s_simulation_queue_element* clone = nullptr;
		this->allocate(element->data_size, &clone);

		clone->type = element->type;
		csmemcpy(clone->data, element->data, element->data_size);
		this->enqueue(clone);
	}
	
	//for (int32 i = 0; i < queued_count; i++)
	//{
	//	s_simulation_queue_element* element = nullptr;
	//	source_queue->dequeue(&element);


	//	s_simulation_queue_element* clone = nullptr;
	//	this->allocate(element->data_size, &clone);

	//	clone->type = element->type;
	//	csmemcpy(clone->data, element->data, element->data_size);
	//	this->enqueue(clone);
	//	



	//	////add back to the source
	//	//source_queue->enqueue(element);
	//}
}

void c_simulation_queue::encode(c_bitstream* stream)
{
	ASSERT(initialized());
	ASSERT(stream);

	if (queued_count() || queued_size())
		LOG_TRACE_NETWORK(" {} encoding count : {}  total size : {}", __FUNCTION__, queued_count(), queued_size());

	stream->write_integer("queue-count", this->queued_count(), 0xC);
	stream->write_integer("queue-size", this->queued_size(), 0x11);

	for (const s_simulation_queue_element* element = this->get_first_element();
		element != nullptr;
		element = this->get_next_element(element))
	{
		stream->write_integer("type", element->type, k_simulation_queue_type_encoded_size_in_bits);
		stream->write_integer("size", element->data_size, k_simulation_queue_payload_encoded_size_in_bits);
		stream->write_raw_data("data", element->data, CHAR_BITS * element->data_size);
	}
}

void c_simulation_queue::decode(c_bitstream* stream)
{
	ASSERT(!initialized());
	ASSERT(stream);

	this->initialize();
	int32 queue_count = stream->read_integer("queue-count", 0xC);
	int32 queue_size = stream->read_integer("queue-size", 0x11);

	if (queue_count || queue_size)
		LOG_TRACE_NETWORK(" {} received count : {}  total size : {}", __FUNCTION__, queue_count, queue_size);

	if (VALID_COUNT(queue_count, k_simulation_queue_count_max))
	{
		if (VALID_COUNT(queue_size, k_simulation_queue_size_max))
		{
			for (int32 i = 0; i < queue_count; i++)
			{
				int16 type = stream->read_integer("type", k_simulation_queue_type_encoded_size_in_bits);
				uint32 data_size = stream->read_integer("size", k_simulation_queue_payload_encoded_size_in_bits);
				s_simulation_queue_element* element = nullptr;
				if (VALID_INDEX(type, k_simulation_queue_element_type_count))
				{
					this->allocate(data_size, &element);
					if (element)
					{
						element->type = (e_event_queue_type)type;
						stream->read_raw_data("data", element->data, CHAR_BITS * data_size);
						this->enqueue(element);
					}
					// else 
					// allocation failed for this element
					// should probably add to critical errors
				}
			}
		}

	}
}
