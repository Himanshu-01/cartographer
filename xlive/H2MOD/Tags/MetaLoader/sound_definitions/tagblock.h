#pragma once

/*********************************************************************
* Modified implementation of Blam::Cache::DataTypes::Reflexive to suit the internal structures utilized by the game
* 8 BYTE Tag Structure for any Memory array block
* void* MemorylockPointer;
* UINT32 MemoryBlockCount;
**********************************************************************/
template<typename T>
struct h2_memory_array
{
	//* Returns Number of elements
	uint32_t GetElementCount()
	{
		return TagBlockIndex;
	}
	//* Returns memory array element size
	std::size_t GetFieldSize()
	{
		return sizeof(T);
	}
	//* Returns Total memory array size in bytes
	std::size_t GetTotalSize()
	{
		return GetElementCount()*GetFieldSize();
	}
	//* Returns memory array Elements Actual Pointer(List)
	void* GetTagBlockElements()
	{
		return (void*)ptrTagBlock;
	}
	//* Block access via index
	T* operator[](int index)
	{
		return ((T*)GetTagBlockElements() + index);
	}
	//* Appends the block at the end of the array(does a memcpy operation)
	//* Doesnt check for allocation size(cause theres no way)
	void PushBack(T* arg0)
	{
		memcpy(ptrTagBlock + TagBlockIndex, arg0, GetFieldSize());
		TagBlockIndex++;
	}
	//* Adds a range of elements from an array
	void AddRange(T* arg0, int count)
	{
		memcpy(ptrTagBlock + TagBlockIndex, arg0, GetFieldSize()*count);
		TagBlockIndex += count;
	}
	//* Reallocates the block to a newer allocation size
	//* Doesnt modify TagBlockCount
	void Realloc(int new_count, bool delete_old_alloc = true)
	{
		T* old_alloc = ptrTagBlock;
		T* new_alloc = new T[new_count];

		if (old_alloc != nullptr)
			memcpy_s(new_alloc, new_count * GetFieldSize(), old_alloc, GetTotalSize());

		if (new_count < TagBlockIndex)
			TagBlockIndex = new_count;

		ptrTagBlock = new_alloc;
		//sometimes you dont want to delete the older pointer
		if (delete_old_alloc)
			if (old_alloc != nullptr)
				delete old_alloc;
	}
	//* Block deallocator
	void Dealloc()
	{
		delete ptrTagBlock;

		TagBlockIndex = 0x0;
		ptrTagBlock = nullptr;
	}
private:
	//* pointer to the Block Array
	T* ptrTagBlock;
	//* Number of Array elements occupied
	uint32_t TagBlockIndex;
};
/*
* A helper class to facilitate operations on h2_memory_array structures
* -allowing capabilities of capacity support and auto resizing and stuff
*/
template<class T>
class h2_memory_vector
{

	//* Pointer to the memory array declaration
	h2_memory_array<T> *ptrblock;
	//* Capacity of the memory array
	int capacity;
	//* just a resize function
	void _resize(bool delete_old = true)
	{
		capacity = ptrblock->GetElementCount();
		capacity = capacity + ceil(capacity / 2) + 1;

		ptrblock->Realloc(capacity, delete_old);
	}
	void _resize(int new_count, bool delete_old = true)
	{
		capacity = new_count;
		ptrblock->Realloc(capacity, delete_old);
	}
	
public:
	//* Default constructor
	h2_memory_vector()
	{
		ptrblock = nullptr;
		capacity = 0x0;
	}
	//* Function to setup up pointers for the superstructure
	void Init(h2_memory_array<T> *arg0)
	{
		ptrblock = arg0;
	}
	//* Function to allocate and manage independent heap memory for the block
	void Allocate_new()
	{
		_resize(false);
	}
	//* Function to free the heap memory
	void Deallocate_new()
	{
		if (capacity != 0x0)
		{
			capacity = 0x0;
			ptrblock->Dealloc();
		}
	}
	//* Returns the total capacity of the vector
	//* -1 capacity indicates that the block hasnt been allocated independently
	uint32_t GetCapacity()
	{
		return capacity;
	}
	//* Returns Number of Block Field Elements currently in use
	uint32_t GetElementCount()
	{
		return ptrblock->GetElementCount();
	}
	//* Returns field size in bytes
	std::size_t GetFieldSize()
	{
		return sizeof(T);
	}
	//* Returns Total memory occupied by the block in bytes
	std::size_t GetTotalSize()
	{
		return ptrblock->GetTotalSize();
	}
	//* Returns memory array Elements Actual Pointer(List)
	void* GetTagBlockElements()
	{
		return ptrblock->GetTagBlockElements();
	}
	//* Block access via index
	T* operator[](int index)
	{
		return ((T*)ptrblock->GetTagBlockElements() + index);
	}
	//* Appends the block at the end of the memory array
	void PushBack(T* arg0)
	{
		if (ptrblock->GetElementCount() < capacity)
			ptrblock->PushBack(arg0);
		else
		{
			_resize(true);
			ptrblock->PushBack(arg0);
		}
	}
	//* Adds a range of elements from an array of elements
	void AddRange(T* arg0, int count)
	{
		if ((ptrblock->GetElementCount() + count) < capacity)
			ptrblock->AddRange(arg0, count);
		else
		{
			_resize(ptrblock->GetElementCount() + count, true);
			ptrblock->AddRange(arg0, count);
		}
	}
};
