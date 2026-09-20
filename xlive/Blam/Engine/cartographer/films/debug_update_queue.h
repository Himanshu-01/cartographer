#pragma once


struct c_debug_update_node
{
	bool allocated;
	uint8* data;
	uint32 data_size;
	c_debug_update_node* next;
};

class c_debug_update_queue
{
	bool	m_initialized;
	int32	m_allocated_count;
	int32	m_allocated_size_in_bytes;

	int32	m_queued_count;
	int32	m_queued_size;

	c_debug_update_node* m_head;
	c_debug_update_node* m_tail;



	//private functions
	const c_debug_update_node* get_head() const;

	int32 allocated_count() const;
	int32 allocated_size_in_bytes() const;
	int32 get_element_size_in_bytes(const c_debug_update_node* element) const;


public:
	c_debug_update_queue();
	~c_debug_update_queue();

	bool initialized() const;


	const c_debug_update_node* get_first_element() const;
	const c_debug_update_node* get_next_element(const c_debug_update_node* element) const;

	int32 queued_count() const;
	int32 queued_size() const;


	void initialize();
	void allocate(int32 data_size, c_debug_update_node** out_allocated_elem);
	void deallocate(c_debug_update_node* element);

	void enqueue(c_debug_update_node* element);
	void dequeue(c_debug_update_node** out_deq_elem);

	void clear();
	void dispose();

};