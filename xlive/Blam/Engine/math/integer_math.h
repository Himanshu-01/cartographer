#pragma once

union point2d
{
	int16 v[2];
	struct { int16 x, y; };
};
CHECK_STRUCT_SIZE(point2d, sizeof(int16) * 2);

union short_bounds
{
	int16 v[2];
	struct { int16 lower, upper; };
};
CHECK_STRUCT_SIZE(short_bounds, sizeof(int16) * 2);

union rectangle2d
{
	int16 v[4];
	struct { int16 top, left, bottom, right; };
};
CHECK_STRUCT_SIZE(rectangle2d, sizeof(int16) * 4);

static BLAM_MATH_INL int16 rectangle2d_width(const rectangle2d* rect)
{
	return rect->right - rect->left;
}

static BLAM_MATH_INL int16 rectangle2d_height(const rectangle2d* rect)
{
	return rect->bottom - rect->top;
}

static BLAM_MATH_INL void point2d_scale(point2d* point, int16 scale)
{
	point->v[0] *= scale;
	point->v[1] *= scale;
}

static BLAM_MATH_INL void rectangle2d_scale(rectangle2d* rect, int16 scale)
{
	rect->v[0] *= scale;
	rect->v[1] *= scale;
	rect->v[2] *= scale;
	rect->v[3] *= scale;
}
