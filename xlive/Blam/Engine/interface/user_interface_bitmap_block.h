#pragma once
#include "user_interface_widget.h"
#include "bitmaps/bitmap_group.h"

#pragma pack(push , 1)
class c_bitmap_widget : public c_user_interface_widget
{
protected:
	void* m_tag_block;
	int32 delay_milliseconds;
	int32 last_frame_milliseconds;
	int64 field_7C;
	real_vector2d m_progress_scale;
	real32 field_8C;
	int16 m_bitmap_block_index;
	uint8 gap_92[2];
	void* m_bitmaps_block;

public:
	void verify_and_update_bitmap_index(int16 index);
	void assign_new_bitmap_block(bitmap_data* block);
	void set_local_bitmap(int16 local_bitmap_block_index, int16 bitmap_block_index);


	virtual ~c_bitmap_widget();
	virtual void update() override;
	virtual void render_widget(rectangle2d* viewport_bounds) override;
	virtual int get_intro_delay() override;
	virtual void* sub_611703(rectangle2d* unprojected_bounds) override;
	virtual c_user_interface_text* get_interface() override;
};
#pragma pack(pop)
CHECK_STRUCT_SIZE(c_bitmap_widget, 0x98);