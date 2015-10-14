#ifndef VLIQ_H_
#define VLIQ_H_

#include <math.h>
#include <string.h>

typedef float vec_t;

typedef vec_t vec2_t[2];
typedef vec_t vec3_t[3];

#define vec2_copy(dest, src) { memcpy(dest, src, sizeof(vec2_t)); }
#define vec3_copy(dest, src) { memcpy(dest, src, sizeof(vec3_t)); }

#define vec2_zero(v) { memset(v, 0, sizeof(vec2_t)); }
#define vec3_zero(v) { memset(v, 0, sizeof(vec3_t)); }

#define vec2_magnitude_squared(v) ((v[0] * v[0]) + (v[1] * v[1]))
#define vec3_magnitude_squared(v) ((v[0] * v[0]) + (v[1] * v[1]) + (v[2] * v[2]))
#define vec2_magnitude(v) (sqrtf(vec2_magnitude_squared(v)))
#define vec3_magnitude(v) (sqrtf(vec3_magnitude_squared(v)))

#define vec2_add(a, b) { a[0] += b[0]; a[1] += b[1]; }
#define vec2_sub(a, b) { a[0] -= b[0]; a[1] -= b[1]; }
#define vec2_sub_scalar(a, b) { a[0] -= b; a[1] -= b; }
#define vec2_mul(a, b) { a[0] *= b[0]; a[1] *= b[1]; }
#define vec2_mul_scalar(a, b) { a[0] *= b; a[1] *= b; }
#define vec2_div(a, b) { a[0] /= b[0]; a[1] /= b[1]; }
#define vec2_div_scalar(a, b) { a[0] /= b; a[1] /= b;}

#define vec3_add(a, b) { a[0] += b[0]; a[1] += b[1]; a[2] += b[2]; }
#define vec3_sub(a, b) { a[0] -= b[0]; a[1] -= b[1]; a[2] -= b[2]; }
#define vec3_sub_scalar(a, b) { a[0] -= b; a[1] -= b; a[2] -= b; }
#define vec3_mul(a, b) { a[0] *= b[0]; a[1] *= b[1]; a[2] *= b[2]; }
#define vec3_mul_scalar(a, b) { a[0] *= b; a[1] *= b; a[2] *= b; }
#define vec3_div(a, b) { a[0] /= b[0]; a[1] /= b[1]; a[2] /= b[2]; }
#define vec3_div_scalar(a, b) { a[0] /= b; a[1] /= b; a[2] /= b; }

static inline vec_t vec2_distance_squared(vec2_t a, vec2_t b)
{
	vec_t 	xd = b[0] - a[0],
		yd = b[1] - a[1];

	return ((xd * xd) + (yd * yd));
}

static inline vec_t vec3_distance_squared(vec3_t a, vec3_t b)
{
	vec_t 	xd = b[0] - a[0],
		yd = b[1] - a[1],
		zd = b[2] - a[2];

	return ((xd * xd) + (yd * yd) + (zd * zd));
}

static inline vec_t vec2_distance(vec2_t a, vec2_t b)
{
	return sqrtf(vec2_distance_squared(a, b));
}

static inline vec_t vec3_distance(vec3_t a, vec3_t b)
{
	return sqrtf(vec3_distance_squared(a, b));
}

static inline void vec2_normalize(vec2_t* v)
{
	vec_t magnitude = vec2_magnitude((*v));
	if(magnitude) vec2_div_scalar((*v), magnitude);
}

static inline void vec3_normalize(vec3_t* v)
{
	vec_t magnitude = vec3_magnitude((*v));
	if(magnitude) vec3_div_scalar((*v), magnitude);
}

#endif
