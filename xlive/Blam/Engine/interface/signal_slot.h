#pragma once
#include "Blam/Engine/cseries/cseries.h"

// forward declarations for some classes used by this file

class c_screen_widget;
class c_list_widget;
class s_event_record;


class _slot
{
	_slot* m_previous;
	_slot* m_next;
	void* m_signal;

public:
	_slot()
	{
		m_previous = nullptr;
		m_next = nullptr;
		m_signal = nullptr;
	}
};
CHECK_STRUCT_SIZE(_slot, 0xC);


template <typename type = short>
class _slot1 :_slot {};
template <class X = s_event_record*, typename type = short>
class _slot2 :_slot {};


template <class X = c_screen_widget, typename type = short>
class c_slot1 : _slot1<type>
{
	X* m_class_ptr;
	char(__thiscall* m_handler)(X*, type*);

public:
	c_slot1()
	{
		m_class_ptr = 0;
		m_handler = 0;
	}
	c_slot1(X* _class, void* handler)
	{

		m_class_ptr = _class;
		m_handler = handler;
	}
	virtual char event_handler(type* id)
	{
		return this->m_handler(m_class_ptr, id);
	}
};
//CHECK_STRUCT_SIZE(class c_slot1<c_screen_widget,long>, 0x18);


template <class X = c_list_widget, typename Y = s_event_record*, typename type = short>
class c_slot2 : _slot2<Y, type>
{
	X* m_class_ptr;
	char(__thiscall* m_handler)(X*, Y*, type*);

public:
	c_slot2()
	{
		m_class_ptr = 0;
		m_handler = 0;
	}
	c_slot2(X* _class, void* handler)
	{

		m_class_ptr = _class;
		m_handler = handler;
	}
	virtual char event_handler(Y* event, type* id)
	{
		return this->m_handler(m_class_ptr, event, id);
	}
};
//CHECK_STRUCT_SIZE(class c_slot2<class c_search_option_max_players_edit_list, struct s_event_record *, long>, 0x18);