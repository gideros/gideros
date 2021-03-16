/*
 * path.cpp
 *
 *  Created on: 1 sept. 2015
 *      Author: Nico
 */

#define _USE_MATH_DEFINES
#include <math.h>
#include "path.h"
#include "Shaders.h"
#include "graphicsbase.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <glog.h>
#include "kvec.h"
#include "khash.h"
#include "prpath.h"
#include "color.h"

#define PATHFILLMODE_COUNT_UP   	0
#define PATHFILLMODE_COUNT_DOWN   	1
#define PATHFILLMODE_INVERT   		2
#define PATHFILLMODE_DIRECT   		3

#define MIN(x, y) ((x) < (y) ? (x) : (y))
#define MAX(x, y) ((x) > (y) ? (x) : (y))

/* begin of mesh merger */

struct vector2f {
	float x, y;
};

struct vector4f {
	float x, y, z, w;
};

#define kh_vector2f_hash_equal(a, b) ((a.x) == (b.x) && (a.y) == (b.y))
#define kh_vector4f_hash_equal(a, b) ((a.x) == (b.x) && (a.y) == (b.y) && (a.z) == (b.z) && (a.w) == (b.w))
#define kh_vector2f_hash_func(a) ((khint_t)((a.x) * 1e6f) ^ (khint_t)((a.y) * 1e6f))
#define kh_vector4f_hash_func(a) ((khint_t)((a.x) * 1e6f) ^ (khint_t)((a.y) * 1e6f) ^ (khint_t)((a.z) * 1e6f) ^ (khint_t)((a.w) * 1e6f))

KHASH_INIT(vector2f, struct vector2f, unsigned short, 1, kh_vector2f_hash_func,
		kh_vector2f_hash_equal)
KHASH_INIT(vector4f, struct vector4f, unsigned short, 1, kh_vector4f_hash_func,
		kh_vector4f_hash_equal)

/* TODO: better hash functions */

struct merger4f {
	khash_t(vector4f) *h;kvec_t(struct vector4f)
	vertices;kvec_t(unsigned short)
	indices;
};

static void init_merger4f(struct merger4f *m) {
	m->h = kh_init(vector4f);
	kv_init(m->vertices);
	kv_init(m->indices);
}

static void free_merger4f(struct merger4f *m) {
	kh_destroy(vector4f, m->h);
	kv_free(m->vertices);
	kv_free(m->indices);
}

static void merge4f(struct merger4f *m, const struct vector4f *vertices,
		unsigned short *indices, int count) {
	int i;

	for (i = 0; i < count; ++i) {
		struct vector4f v;
		khiter_t k;

		v = vertices[indices[i]];

		k = kh_get(vector4f, m->h, v);

		if (k == kh_end(m->h)) {
			int tmp;

			kv_push_back(m->indices, kv_size(m->vertices));

			k = kh_put(vector4f, m->h, v, &tmp);
			kh_value(m->h, k) = kv_size(m->vertices);

			kv_push_back(m->vertices, v);
		} else {
			kv_push_back(m->indices, kh_value(m->h, k));
		}
	}
}

/* end of mesh merger*/

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

/* begin of path name management */

struct segment {
	unsigned int start;
	int length;
	struct segment *next;
};

static struct segment *segments = NULL;

static void init_segments(void) {
	segments = (struct segment*) malloc(sizeof(struct segment));
	segments->next = NULL;
}

static unsigned int gen_paths(int range) {
	unsigned int start = 1;

	struct segment *prev = segments;
	struct segment *curr = segments->next;

	while (curr) {
		int length = curr->start - start;
		if (length >= range) {
			struct segment *s = (struct segment*) malloc(
					sizeof(struct segment));

			s->start = start;
			s->length = range;

			s->next = curr;
			prev->next = s;

			return start;
		}

		start = curr->start + curr->length;

		prev = curr;
		curr = curr->next;
	}

	{
		struct segment *s = (struct segment*) malloc(sizeof(struct segment));

		s->start = start;
		s->length = range;

		prev->next = s;
		s->next = NULL;

		return start;
	}
}

static void delete_path_helper(unsigned int min, unsigned int max);

static void delete_paths(unsigned int path, int range) {
	struct segment *prev = segments;
	struct segment *curr = segments->next;

	unsigned int min1 = path;
	unsigned int max1 = path + range - 1;

	while (curr) {
		unsigned int min2 = curr->start;
		unsigned int max2 = curr->start + curr->length - 1;

		if (min1 > min2 && max1 < max2) {
			struct segment *s = (struct segment*) malloc(
					sizeof(struct segment));

			s->start = max1 + 1;
			s->length = max2 - max1;

			curr->length = min1 - min2;

			s->next = curr->next;
			curr->next = s;

			delete_path_helper(min1, max1);
		} else if (min1 <= min2 && max1 >= max2) {
			prev->next = curr->next;
			free(curr);
			curr = prev;

			delete_path_helper(min2, max2);
		} else if (min1 > min2 && min1 <= max2) {
			curr->length = min1 - min2;

			delete_path_helper(min1, max2);
		} else if (max1 >= min2 && max1 < max2) {
			curr->start = max1 + 1;
			curr->length = max2 - max1;

			delete_path_helper(min2, max1);
		}

		prev = curr;
		curr = curr->next;
	}
}

static void set_path(unsigned int path) {
	struct segment *prev = segments;
	struct segment *curr = segments->next;

	while (curr) {
		unsigned int min = curr->start;
		unsigned int max = curr->start + curr->length - 1;

		if (min <= path && path <= max)
			return;

		if (path < min) {
			struct segment *s = (struct segment*) malloc(
					sizeof(struct segment));

			s->start = path;
			s->length = 1;

			s->next = curr;
			prev->next = s;

			return;
		}

		prev = curr;
		curr = curr->next;
	}

	{
		struct segment *s = (struct segment*) malloc(sizeof(struct segment));

		s->start = path;
		s->length = 1;

		prev->next = s;
		s->next = NULL;
	}
}

static void cleanup_segments(void) {
	struct segment *curr = segments;

	while (curr) {
		struct segment *temp = curr;
		delete_path_helper(curr->start, curr->start + curr->length - 1);
		curr = curr->next;
		free(temp);
	}

	segments = NULL;
}

/* end of path name management */

static void subdivide_cubic(const double c[8], double c1[8], double c2[8]) {
	double p1x = (c[0] + c[2]) / 2;
	double p1y = (c[1] + c[3]) / 2;
	double p2x = (c[2] + c[4]) / 2;
	double p2y = (c[3] + c[5]) / 2;
	double p3x = (c[4] + c[6]) / 2;
	double p3y = (c[5] + c[7]) / 2;
	double p4x = (p1x + p2x) / 2;
	double p4y = (p1y + p2y) / 2;
	double p5x = (p2x + p3x) / 2;
	double p5y = (p2y + p3y) / 2;
	double p6x = (p4x + p5x) / 2;
	double p6y = (p4y + p5y) / 2;

	double p0x = c[0];
	double p0y = c[1];
	double p7x = c[6];
	double p7y = c[7];

	c1[0] = p0x;
	c1[1] = p0y;
	c1[2] = p1x;
	c1[3] = p1y;
	c1[4] = p4x;
	c1[5] = p4y;
	c1[6] = p6x;
	c1[7] = p6y;

	c2[0] = p6x;
	c2[1] = p6y;
	c2[2] = p5x;
	c2[3] = p5y;
	c2[4] = p3x;
	c2[5] = p3y;
	c2[6] = p7x;
	c2[7] = p7y;
}

static void subdivide_cubic2(const double cin[8], double cout[16]) {
	subdivide_cubic(cin, cout, cout + 8);
}

static void subdivide_cubic4(const double cin[8], double cout[32]) {
	subdivide_cubic(cin, cout, cout + 16);
	subdivide_cubic2(cout, cout);
	subdivide_cubic2(cout + 16, cout + 16);
}

static void subdivide_cubic8(const double cin[8], double cout[64]) {
	subdivide_cubic(cin, cout, cout + 32);
	subdivide_cubic4(cout, cout);
	subdivide_cubic4(cout + 32, cout + 32);
}

static void cubic_to_quadratic(const double c[8], double q[6]) {
	q[0] = c[0];
	q[1] = c[1];
	q[2] = (3 * (c[2] + c[4]) - (c[0] + c[6])) / 4;
	q[3] = (3 * (c[3] + c[5]) - (c[1] + c[7])) / 4;
	q[4] = c[6];
	q[5] = c[7];
}

/*
 M: 0
 L: 1
 Z: 2

 M -> M no
 M -> L no
 M -> Z no
 L -> M yes
 L -> L no
 L -> Z no
 Z -> M yes
 Z -> L yes
 Z -> Z no
 */
static const int new_path_table[3][3] =
		{ { 0, 0, 0 }, { 1, 0, 0 }, { 1, 1, 0 } };

// always starts with 'M'
// after 'M', only contains 'L' and 'Q'
// optionally finishes with 'Z'
struct reduced_path {
	kvec_t(unsigned char)
	commands;kvec_t(float)
	coords;
};

typedef kvec_t(struct reduced_path)
reduced_path_vec;

struct geometry {
	kvec_t(float)
	vertices;kvec_t(unsigned short)
	indices;

	VertexBuffer<float> *vertex_buffer;
	VertexBuffer<unsigned short> *index_buffer;
	int count;
};

struct path {
	int num_commands;
	unsigned char *commands;

	int num_coords;
	float *coords;

	reduced_path_vec reduced_paths;

	float stroke_width;
	float stroke_feather;
    float stroke_margin;
    float stroke_flatness;
	int join_style;
	int initial_end_cap;
	int terminal_end_cap;
	float miter_limit;

	int num_dashes;
	float *dashes;
	float dash_length;

	struct geometry fill_geoms[4]; /* 0: front-solid, 1:back-solid 2:front-quad 3:back-quad */
	struct geometry stroke_geoms[2]; /* 1: solid 2: quad */

	VertexBuffer<vector4f> *fill_vertex_buffer;
	VertexBuffer<unsigned short> *fill_index_buffer;
	int fill_counts[2];
	int fill_starts[2];

	VertexBuffer<float> *fill_bounds_vbo;
	float fill_bounds[4];
	float stroke_bounds[4];

	int is_stroke_dirty;
	int is_fill_dirty;
	int is_reduced_paths_dirty;
};

KHASH_MAP_INIT_INT(path, struct path *)

static khash_t(path) *paths = NULL;

static void delete_path(struct path *p) {
	size_t i;

	free(p->commands);
	free(p->coords);

	free(p->dashes);

	for (i = 0; i < kv_size(p->reduced_paths); ++i) {
		kv_free(kv_a(p->reduced_paths, i).commands);
		kv_free(kv_a(p->reduced_paths, i).coords);
	}
	kv_free(p->reduced_paths);

#if 0
	for (i = 0; i < 4; ++i)
	{
		glDeleteBuffers(1, &p->fill_geoms[i].vertex_buffer);
		glDeleteBuffers(1, &p->fill_geoms[i].index_buffer);
	}
#endif

	for (i = 0; i < 2; ++i) {
		if (p->stroke_geoms[i].vertex_buffer)
			delete p->stroke_geoms[i].vertex_buffer;
		if (p->stroke_geoms[i].index_buffer)
			delete p->stroke_geoms[i].index_buffer;
	}

	if (p->fill_vertex_buffer)
		delete p->fill_vertex_buffer;
	if (p->fill_bounds_vbo)
		delete p->fill_bounds_vbo;
	if (p->fill_index_buffer)
		delete p->fill_index_buffer;

	free(p);
}

static void delete_path_helper(unsigned int min, unsigned int max) {
	unsigned int i;
	for (i = min; i <= max; ++i) {
		khiter_t iter = kh_get(path, paths, i);
		if (iter != kh_end(paths)) {
			struct path *p = kh_value(paths, iter);
			delete_path(p);
			kh_del(path, paths, iter);
		}
	}
}

static void new_path(reduced_path_vec *paths) {
	struct reduced_path rp = { 0 };
	kv_push_back(*paths, rp);
}

static void move_to(struct reduced_path *path, float x, float y) {
	if (kv_empty(path->commands)) {
		kv_push_back(path->commands, PATHCMD_MOVE_TO);
		kv_push_back(path->coords, x);
		kv_push_back(path->coords, y);
	} else {
		kv_a(path->coords, 0) = x;
		kv_a(path->coords, 1) = y;
	}
}

static void line_to(struct reduced_path *path, float x1, float y1, float x2,
		float y2) {
	if (kv_empty(path->commands)) {
		kv_push_back(path->commands, PATHCMD_MOVE_TO);
		kv_push_back(path->coords, x1);
		kv_push_back(path->coords, y1);
	}

	kv_push_back(path->commands, PATHCMD_LINE_TO);
	kv_push_back(path->coords, x2);
	kv_push_back(path->coords, y2);
}

static void quad_to(struct reduced_path *path, float x1, float y1, float x2,
		float y2, float x3, float y3) {
	if (kv_empty(path->commands)) {
		kv_push_back(path->commands, PATHCMD_MOVE_TO);
		kv_push_back(path->coords, x1);
		kv_push_back(path->coords, y1);
	}

	kv_push_back(path->commands, PATHCMD_QUADRATIC_CURVE_TO);
	kv_push_back(path->coords, x2);
	kv_push_back(path->coords, y2);
	kv_push_back(path->coords, x3);
	kv_push_back(path->coords, y3);
}

static void cubic_to(struct reduced_path *path, float x1, float y1, float x2,
		float y2, float x3, float y3, float x4, float y4) {
	int i;

	double cin[8] = { x1, y1, x2, y2, x3, y3, x4, y4 };
	double cout[64];
	subdivide_cubic8(cin, cout);

	if (kv_empty(path->commands)) {
		kv_push_back(path->commands, PATHCMD_MOVE_TO);
		kv_push_back(path->coords, x1);
		kv_push_back(path->coords, y1);
	}

	for (i = 0; i < 8; ++i) {
		double q[6];
		cubic_to_quadratic(cout + i * 8, q);
		kv_push_back(path->commands, PATHCMD_QUADRATIC_CURVE_TO);
		kv_push_back(path->coords, q[2]);
		kv_push_back(path->coords, q[3]);
		kv_push_back(path->coords, q[4]);
		kv_push_back(path->coords, q[5]);
	}
}

static double angle(double ux, double uy, double vx, double vy) {
	return atan2(ux * vy - uy * vx, ux * vx + uy * vy);
}

/* http://www.w3.org/TR/SVG/implnote.html#ArcConversionEndpointToCenter */
static void endpoint_to_center(double x1, double y1, double x2, double y2,
		int fA, int fS, double *prx, double *pry, double phi, double *cx,
		double *cy, double *theta1, double *dtheta) {
	double x1p, y1p, rx, ry, lambda, fsgn, c1, cxp, cyp;

	x1p = cos(phi) * (x1 - x2) / 2 + sin(phi) * (y1 - y2) / 2;
	y1p = -sin(phi) * (x1 - x2) / 2 + cos(phi) * (y1 - y2) / 2;

	rx = *prx;
	ry = *pry;

	lambda = (x1p * x1p) / (rx * rx) + (y1p * y1p) / (ry * ry);
	if (lambda > 1) {
		lambda = sqrt(lambda);
		rx *= lambda;
		ry *= lambda;
		*prx = rx;
		*pry = ry;
	}

	fA = !!fA;
	fS = !!fS;

	fsgn = (fA != fS) ? 1 : -1;

	c1 = (rx * rx * ry * ry - rx * rx * y1p * y1p - ry * ry * x1p * x1p)
			/ (rx * rx * y1p * y1p + ry * ry * x1p * x1p);

	if (c1 < 0)	// because of floating point inaccuracies, c1 can be -epsilon.
		c1 = 0;
	else
		c1 = sqrt(c1);

	cxp = fsgn * c1 * (rx * y1p / ry);
	cyp = fsgn * c1 * (-ry * x1p / rx);

	*cx = cos(phi) * cxp - sin(phi) * cyp + (x1 + x2) / 2;
	*cy = sin(phi) * cxp + cos(phi) * cyp + (y1 + y2) / 2;

	*theta1 = angle(1, 0, (x1p - cxp) / rx, (y1p - cyp) / ry);

	*dtheta = angle((x1p - cxp) / rx, (y1p - cyp) / ry, (-x1p - cxp) / rx,
			(-y1p - cyp) / ry);

	if (!fS && *dtheta > 0)
		*dtheta -= 2 * M_PI;
	else if (fS && *dtheta < 0)
		*dtheta += 2 * M_PI;
}

static void arc_tod(struct reduced_path *path, double x1, double y1, double rh,
		double rv, double phi, int fA, int fS, double x2, double y2) {
	double cx, cy, theta1, dtheta;

	int i, nquads;

	phi *= M_PI / 180;

	endpoint_to_center(x1, y1, x2, y2, fA, fS, &rh, &rv, phi, &cx, &cy, &theta1,
			&dtheta);

	nquads = ceil(fabs(dtheta) * 4 / M_PI);

	for (i = 0; i < nquads; ++i) {
		double t1 = theta1 + (i / (double) nquads) * dtheta;
		double t2 = theta1 + ((i + 1) / (double) nquads) * dtheta;
		double tm = (t1 + t2) / 2;

		double x1 = cos(phi) * rh * cos(t1) - sin(phi) * rv * sin(t1) + cx;
		double y1 = sin(phi) * rh * cos(t1) + cos(phi) * rv * sin(t1) + cy;

		double x2 = cos(phi) * rh * cos(t2) - sin(phi) * rv * sin(t2) + cx;
		double y2 = sin(phi) * rh * cos(t2) + cos(phi) * rv * sin(t2) + cy;

		double xm = cos(phi) * rh * cos(tm) - sin(phi) * rv * sin(tm) + cx;
		double ym = sin(phi) * rh * cos(tm) + cos(phi) * rv * sin(tm) + cy;

		double xc = (xm * 4 - (x1 + x2)) / 2;
		double yc = (ym * 4 - (y1 + y2)) / 2;

		kv_push_back(path->commands, PATHCMD_QUADRATIC_CURVE_TO);
		kv_push_back(path->coords, xc);
		kv_push_back(path->coords, yc);
		kv_push_back(path->coords, x2);
		kv_push_back(path->coords, y2);
	}
}

static void arc_to(struct reduced_path *path, float x1, float y1, float rh,
		float rv, float phi, int fA, int fS, float x2, float y2) {
	if (kv_empty(path->commands)) {
		kv_push_back(path->commands, PATHCMD_MOVE_TO);
		kv_push_back(path->coords, x1);
		kv_push_back(path->coords, y1);
	}

	arc_tod(path, x1, y1, rh, rv, phi, fA, fS, x2, y2);
}

static void close_path(struct reduced_path *path) {
	if (kv_back(path->commands) != PATHCMD_CLOSE_PATH)
		kv_push_back(path->commands, PATHCMD_CLOSE_PATH);
}

static void reduce_path(int num_commands, const unsigned char *commands,
		int num_coords, const float *coords, reduced_path_vec *reduced_paths) {
#define c0 coords[icoord]
#define c1 coords[icoord + 1]
#define c2 coords[icoord + 2]
#define c3 coords[icoord + 3]
#define c4 coords[icoord + 4]
#define c5 coords[icoord + 5]
#define c6 coords[icoord + 6]

#define set(x1, y1, x2, y2) ncpx = x1; ncpy = y1; npepx = x2; npepy = y2;

#define last_path &kv_back(*reduced_paths)

	int icoord = 0;

	float bearing = 0;
	float spx = 0, spy = 0;
	float cpx = 0, cpy = 0;
	float pepx = 0, pepy = 0;
	float ncpx = 0, ncpy = 0;
	float npepx = 0, npepy = 0;
	float rx, ry, rx1, ry1, rx2, ry2;
#define relative(mx,my) if (bearing) \
    { rx=cpx+cosf(bearing)*mx-sinf(bearing)*my; ry=cpy+cosf(bearing)*my+sinf(bearing)*mx; } \
	else { rx=cpx+mx; ry=cpy+my; }

	unsigned char prev_command = 2;

	int i;

	for (i = 0; i < num_commands; ++i) {
		switch (commands[i]) {
		case PATHCMD_MOVE_TO:
		case 'M':
			if (new_path_table[prev_command][0])
				new_path(reduced_paths);
			prev_command = 0;
			move_to(last_path, c0, c1);
			set(c0, c1, c0, c1);
			spx = ncpx;
			spy = ncpy;
			icoord += 2;
			break;

			case PATHCMD_RELATIVE_MOVE_TO:
			case 'm':
			if (new_path_table[prev_command][0])
			new_path(reduced_paths);
			prev_command = 0;
			relative(c0,c1);
			move_to(last_path, rx, ry);
			set(rx,ry,rx,ry);
			spx = ncpx;
			spy = ncpy;
			icoord += 2;
			break;

			case PATHCMD_CLOSE_PATH:
			case 'z':
			case 'Z':
			if (new_path_table[prev_command][2])
			new_path(reduced_paths);
			prev_command = 2;
			close_path(last_path);
			set(spx, spy, spx, spy);
			break;

			case PATHCMD_RESTART_PATH:
			if (new_path_table[prev_command][0])
			new_path(reduced_paths);
			prev_command = 0;
			move_to(last_path, 0, 0);
			set(0, 0, 0, 0);
			spx = ncpx;
			spy = ncpy;
			break;

			case PATHCMD_LINE_TO:
			case 'L':
			if (new_path_table[prev_command][1])
			new_path(reduced_paths);
			prev_command = 1;
			line_to(last_path, cpx, cpy, c0, c1);
			set(c0, c1, c0, c1);
			icoord += 2;
			break;

			case PATHCMD_RELATIVE_LINE_TO:
			case 'l':
			if (new_path_table[prev_command][1])
			new_path(reduced_paths);
			prev_command = 1;
			relative(c0,c1);
			line_to(last_path, cpx, cpy, rx, ry);
			set(rx,ry,rx,ry);
			icoord += 2;
			break;

			case PATHCMD_HORIZONTAL_LINE_TO:
			case 'H':
			if (new_path_table[prev_command][1])
			new_path(reduced_paths);
			prev_command = 1;
			line_to(last_path, cpx, cpy, c0, cpy);
			set(c0, cpy, c0, cpy);
			icoord += 1;
			break;

			case PATHCMD_RELATIVE_HORIZONTAL_LINE_TO:
			case 'h':
			if (new_path_table[prev_command][1])
			new_path(reduced_paths);
			prev_command = 1;
			relative(c0,0);
			line_to(last_path, cpx, cpy, rx,ry);
			set(rx,ry,rx,ry);
			icoord += 1;
			break;

			case PATHCMD_VERTICAL_LINE_TO:
			case 'V':
			if (new_path_table[prev_command][1])
			new_path(reduced_paths);
			prev_command = 1;
			line_to(last_path, cpx, cpy, cpx, c0);
			set(cpx, c0, cpx, c0);
			icoord += 1;
			break;

			case PATHCMD_RELATIVE_VERTICAL_LINE_TO:
			case 'v':
			if (new_path_table[prev_command][1])
			new_path(reduced_paths);
			prev_command = 1;
			relative(0,c0);
			line_to(last_path, cpx, cpy, rx,ry);
			set(rx,ry,rx,ry);
			icoord += 1;
			break;

			case PATHCMD_QUADRATIC_CURVE_TO:
			case 'Q':
			if (new_path_table[prev_command][1])
			new_path(reduced_paths);
			prev_command = 1;
			quad_to(last_path, cpx, cpy, c0, c1, c2, c3);
			set(c2, c3, c0, c1);
			icoord += 4;
			break;

			case PATHCMD_RELATIVE_QUADRATIC_CURVE_TO:
			case 'q':
			if (new_path_table[prev_command][1])
			new_path(reduced_paths);
			prev_command = 1;
			relative(c2,c3); rx1=rx; ry1=ry;
			relative(c0,c1);
			quad_to(last_path, cpx, cpy, rx,ry,rx1,ry1);
			set(rx1,ry1,rx,ry);
			icoord += 4;
			break;

			case PATHCMD_CUBIC_CURVE_TO:
			case 'C':
			if (new_path_table[prev_command][1])
			new_path(reduced_paths);
			prev_command = 1;
			cubic_to(last_path, cpx, cpy, c0, c1, c2, c3, c4, c5);
			set(c4, c5, c2, c3);
			icoord += 6;
			break;

			case PATHCMD_RELATIVE_CUBIC_CURVE_TO:
			case 'c':
			if (new_path_table[prev_command][1])
			new_path(reduced_paths);
			prev_command = 1;
			relative(c2,c3); rx1=rx; ry1=ry;
			relative(c4,c5); rx2=rx; ry2=ry;
			relative(c0,c1);
			cubic_to(last_path, cpx, cpy, rx,ry,rx1,ry1,rx2,ry2);
			set(rx2,ry2,rx1,ry1);
			icoord += 6;
			break;

			case PATHCMD_SMOOTH_QUADRATIC_CURVE_TO:
			case 'T':
			if (new_path_table[prev_command][1])
			new_path(reduced_paths);
			prev_command = 1;
			quad_to(last_path, cpx, cpy, 2 * cpx - pepx, 2 * cpy - pepy, c0, c1);
			set(c0, c1, 2 * cpx - pepx, 2 * cpy - pepy);
			icoord += 2;
			break;

			case PATHCMD_RELATIVE_SMOOTH_QUADRATIC_CURVE_TO:
			case 't':
			if (new_path_table[prev_command][1])
			new_path(reduced_paths);
			prev_command = 1;
			relative(c0,c1);
			quad_to(last_path, cpx, cpy, 2 * cpx - pepx, 2 * cpy - pepy, rx,ry);
			set(rx,ry, 2 * cpx - pepx, 2 * cpy - pepy);
			icoord += 2;
			break;

			case PATHCMD_SMOOTH_CUBIC_CURVE_TO:
			case 'S':
			if (new_path_table[prev_command][1])
			new_path(reduced_paths);
			prev_command = 1;
			cubic_to(last_path, cpx, cpy, 2 * cpx - pepx, 2 * cpy - pepy, c0, c1, c2, c3);
			set(c2, c3, c0, c1);
			icoord += 4;
			break;

			case PATHCMD_RELATIVE_SMOOTH_CUBIC_CURVE_TO:
			case 's':
			if (new_path_table[prev_command][1])
			new_path(reduced_paths);
			prev_command = 1;
			relative(c2,c3); rx1=rx; ry1=ry;
			relative(c0,c1);
			cubic_to(last_path, cpx, cpy, 2 * cpx - pepx, 2 * cpy - pepy,rx,ry,rx1,ry1);
			set(rx1,ry1,rx,ry);
			icoord += 4;
			break;

			case PATHCMD_SMALL_CCW_ARC_TO:
			if (new_path_table[prev_command][1])
			new_path(reduced_paths);
			prev_command = 1;
			arc_to(last_path, cpx, cpy, c0, c1, c2, 0, 1, c3, c4);
			set(c3, c4, c3, c4);
			icoord += 5;
			break;

			case PATHCMD_RELATIVE_SMALL_CCW_ARC_TO:
			if (new_path_table[prev_command][1])
			new_path(reduced_paths);
			prev_command = 1;
			relative(c3,c4);
			arc_to(last_path, cpx, cpy, c0, c1, c2, 0, 1, rx,ry);
			set(rx,ry,rx,ry);
			icoord += 5;
			break;

			case PATHCMD_SMALL_CW_ARC_TO:
			if (new_path_table[prev_command][1])
			new_path(reduced_paths);
			prev_command = 1;
			arc_to(last_path, cpx, cpy, c0, c1, c2, 0, 0, c3, c4);
			set(c3, c4, c3, c4);
			icoord += 5;
			break;

			case PATHCMD_RELATIVE_SMALL_CW_ARC_TO:
			if (new_path_table[prev_command][1])
			new_path(reduced_paths);
			prev_command = 1;
			relative(c3,c4);
			arc_to(last_path, cpx, cpy, c0, c1, c2, 0, 0, rx,ry);
			set(rx,ry,rx,ry);
			icoord += 5;
			break;

			case PATHCMD_LARGE_CCW_ARC_TO:
			if (new_path_table[prev_command][1])
			new_path(reduced_paths);
			prev_command = 1;
			arc_to(last_path, cpx, cpy, c0, c1, c2, 1, 1, c3, c4);
			set(c3, c4, c3, c4);
			icoord += 5;
			break;

			case PATHCMD_RELATIVE_LARGE_CCW_ARC_TO:
			if (new_path_table[prev_command][1])
			new_path(reduced_paths);
			prev_command = 1;
			relative(c3,c4);
			arc_to(last_path, cpx, cpy, c0, c1, c2, 1, 1, rx,ry);
			set(rx,ry,rx,ry);
			icoord += 5;
			break;

			case PATHCMD_LARGE_CW_ARC_TO:
			if (new_path_table[prev_command][1])
			new_path(reduced_paths);
			prev_command = 1;
			arc_to(last_path, cpx, cpy, c0, c1, c2, 1, 0, c3, c4);
			set(c3, c4, c3, c4);
			icoord += 5;
			break;

			case PATHCMD_RELATIVE_LARGE_CW_ARC_TO:
			if (new_path_table[prev_command][1])
			new_path(reduced_paths);
			prev_command = 1;
			relative(c3,c4);
			arc_to(last_path, cpx, cpy, c0, c1, c2, 1, 0, cpx + c3, cpy + c4);
			set(rx,ry,rx,ry);
			icoord += 5;
			break;

			case PATHCMD_ARC_TO:
			case 'A':
			if (new_path_table[prev_command][1])
			new_path(reduced_paths);
			prev_command = 1;
			arc_to(last_path, cpx, cpy, c0, c1, c2, c3, c4, c5, c6);
			set(c5, c6, c5, c6);
			icoord += 7;
			break;

			case PATHCMD_RELATIVE_ARC_TO:
			case 'a':
			if (new_path_table[prev_command][1])
			new_path(reduced_paths);
			prev_command = 1;
			relative(c5,c6);
			arc_to(last_path, cpx, cpy, c0, c1, c2, c3, c4, rx,ry);
			set(rx,ry,rx,ry);
			icoord += 7;
			break;

			case PATHCMD_RECT:
			if (new_path_table[prev_command][0])
			new_path(reduced_paths);
			prev_command = 2;
			move_to(last_path, c0, c1);
			line_to(last_path, c0, c1, c0 + c2, c1);
			line_to(last_path, c0 + c2, c1, c0 + c2, c1 + c3);
			line_to(last_path, c0 + c2, c1 + c3, c0, c1 + c3);
			close_path(last_path);
			set(c0, c1, c0, c1 + c3);
			icoord += 4;
			break;

			case PATHCMD_DUP_FIRST_CUBIC_CURVE_TO:
			if (new_path_table[prev_command][1])
			new_path(reduced_paths);
			prev_command = 1;
			cubic_to(last_path, cpx, cpy, cpx, cpy, c0, c1, c2, c3);
			set(c2, c3, c0, c1);
			icoord += 4;
			break;

			case PATHCMD_DUP_LAST_CUBIC_CURVE_TO:
			if (new_path_table[prev_command][1])
			new_path(reduced_paths);
			prev_command = 1;
			cubic_to(last_path, cpx, cpy, c0, c1, c2, c3, c2, c3);
			set(c2, c3, c2, c3);
			icoord += 4;
			break;

			case 'B':
			bearing=c0*M_PI/180.0;
			icoord += 1;
			break;

			case 'b':
			bearing+=c0*M_PI/180.0;
			icoord += 1;
			break;

			case PATHCMD_CIRCULAR_CCW_ARC_TO:
			case PATHCMD_CIRCULAR_CW_ARC_TO:
			case PATHCMD_CIRCULAR_TANGENT_ARC_TO:
			// todo
			break;
		}

		cpx = ncpx;
		cpy = ncpy;
		pepx = npepx;
		pepy = npepy;
	}

#undef c0
#undef c1
#undef c2
#undef c3
#undef c4
#undef c5
#undef c6

#undef set

#undef last_path
}

static int command_coords[256];

static void path_commands(unsigned int path, int num_commands,
		const unsigned char *commands, int num_coords, const float *coords) {
	int i;
	int num_coords2;
	khiter_t iter;
	struct path *p;
	int tmp;

	//glog_d("Build path: NC:%d NV:%d", num_commands, num_coords);
	num_coords2 = 0;
	int expanded_cmds = 0;
	for (i = 0; i < num_commands; ++i) {
		if (commands[i] == '*') // repeat last
				{
			if (i == 0)
				return;    			// TODO: set error
			int num = command_coords[commands[i - 1]];
			while (num_coords > num_coords2) {
				expanded_cmds++;
				num_coords2 += num;
			}
		} else {
			int num = command_coords[commands[i]];
			if (num == -1) {
				glog_d("Build path: Invalid Command:%d", commands[i]);
				// TODO: set error
				return;
			}
			expanded_cmds++;
			num_coords2 += num;
		}
	}

	if (num_coords != num_coords2) {
		glog_d("Build path: Wrong coord count %d!=%d", num_coords, num_coords2);
		// TODO: set error
		return;
	}

	iter = kh_get(path, paths, path);
	if (iter == kh_end(paths)) {
		iter = kh_put(path, paths, path, &tmp);
		p = (struct path*) malloc(sizeof(struct path));
		kh_value(paths, iter) = p;

		p->stroke_width = 1;
        p->stroke_margin = 1;
		p->stroke_feather = 0.25;
        p->stroke_flatness = 0;
		p->join_style = PATHJOIN_BEVEL; //PATHJOIN_MITER_REVERT;
		p->initial_end_cap = PATHEND_FLAT;
		p->terminal_end_cap = PATHEND_FLAT;
		p->miter_limit = 4;
		p->num_dashes = 0;
		p->dashes = NULL;
		p->dash_length = 0;

#if 0
		for (i = 0; i < 4; ++i)
		{
			glGenBuffers(1, &p->fill_geoms[i].vertex_buffer);
			glGenBuffers(1, &p->fill_geoms[i].index_buffer);
			p->fill_geoms[i].count = 0;
		}
#endif

		for (i = 0; i < 2; ++i) {
			p->stroke_geoms[i].vertex_buffer = new VertexBuffer<float>();
			p->stroke_geoms[i].index_buffer =
					new VertexBuffer<unsigned short>();
			p->stroke_geoms[i].count = 0;
		}

		p->fill_vertex_buffer = new VertexBuffer<vector4f>();
		p->fill_index_buffer = new VertexBuffer<unsigned short>();
		p->fill_bounds_vbo = new VertexBuffer<float>();

		set_path(path);
	} else {
		size_t i;

		p = kh_value(paths, iter);

		free(p->commands);
		free(p->coords);
		for (i = 0; i < kv_size(p->reduced_paths); ++i) {
			kv_free(kv_a(p->reduced_paths, i).commands);
			kv_free(kv_a(p->reduced_paths, i).coords);
		}
		kv_free(p->reduced_paths);
	}

	p->num_commands = expanded_cmds;
	p->commands = (unsigned char*) malloc(
			expanded_cmds * sizeof(unsigned char));
	int nc = 0;
	num_coords2 = 0;
	for (i = 0; i < num_commands; ++i) {
		if (commands[i] == '*') // repeat last
				{
			int num = command_coords[commands[i - 1]];
			while (num_coords > num_coords2) {
				p->commands[nc++] = commands[i - 1];
				num_coords2 += num;
			}
		} else {
			int num = command_coords[commands[i]];
			p->commands[nc++] = commands[i];
			num_coords2 += num;
		}
	}
	//memcpy(p->commands, commands, num_commands * sizeof(unsigned char));

	p->num_coords = num_coords;
	p->coords = (float *) malloc(num_coords * sizeof(float));
	memcpy(p->coords, coords, num_coords * sizeof(float));

	kv_init(p->reduced_paths);

	reduce_path(p->num_commands, p->commands, p->num_coords, p->coords,
			&p->reduced_paths);

	p->is_fill_dirty = 1;
	p->is_stroke_dirty = 1;
	p->is_reduced_paths_dirty = 0;
}

static void add_stroke_line(struct path *p, double x0, double y0, double x1,
		double y1) {
	struct geometry *g = &p->stroke_geoms[0];

	int index = kv_size(g->vertices) / 4;

	double dx = x1 - x0;
	double dy = y1 - y0;
	double len = sqrt(dx * dx + dy * dy);
	if (len == 0)
		return;

	dx = dx * p->stroke_width / len;
	dy = dy * p->stroke_width / len;

	kv_push_back(g->vertices, x0);
	kv_push_back(g->vertices, y0);
	kv_push_back(g->vertices, -dy);
	kv_push_back(g->vertices, dx);
	kv_push_back(g->vertices, x0);
	kv_push_back(g->vertices, y0);
	kv_push_back(g->vertices, dy);
	kv_push_back(g->vertices, -dx);
	kv_push_back(g->vertices, x1);
	kv_push_back(g->vertices, y1);
	kv_push_back(g->vertices, dy);
	kv_push_back(g->vertices, -dx);
	kv_push_back(g->vertices, x1);
	kv_push_back(g->vertices, y1);
	kv_push_back(g->vertices, -dy);
	kv_push_back(g->vertices, dx);

	kv_push_back(g->indices, index);
	kv_push_back(g->indices, index + 1);
	kv_push_back(g->indices, index + 2);

	kv_push_back(g->indices, index);
	kv_push_back(g->indices, index + 2);
	kv_push_back(g->indices, index + 3);
}

static void add_stroke_line_dashed(struct path *path, double x0, double y0,
		double x1, double y1, double *dash_offset) {
	if (path->num_dashes == 0) {
		add_stroke_line(path, x0, y0, x1, y1);
		return;
	}

	double length = sqrt((x1 - x0) * (x1 - x0) + (y1 - y0) * (y1 - y0));

	/* TODO: remove this check and make sure that 0 length lines are not passed to this function (while creaating reduced path and while closing the path) */
	if (length == 0)
		return;

	double offset = -fmod(*dash_offset, path->dash_length);

	int i = 0;
	while (offset < length) {
		double o0 = MAX(offset, 0);
		double o1 = MIN(offset + path->dashes[i], length);

		if (o1 >= 0) {
			double t0 = o0 / length;
			double t1 = o1 / length;

			add_stroke_line(path, (1 - t0) * x0 + t0 * x1,
					(1 - t0) * y0 + t0 * y1, (1 - t1) * x0 + t1 * x1,
					(1 - t1) * y0 + t1 * y1);
		}

		offset += path->dashes[i] + path->dashes[i + 1];
		i = (i + 2) % path->num_dashes;
	}

	*dash_offset = fmod(length + *dash_offset, path->dash_length);
}

static void evaluate_quadratic(double x0, double y0, double x1, double y1,
		double x2, double y2, double t, double *x, double *y) {
	double Ax = x0 - 2 * x1 + x2;
	double Ay = y0 - 2 * y1 + y2;
	double Bx = 2 * (x1 - x0);
	double By = 2 * (y1 - y0);
	double Cx = x0;
	double Cy = y0;

	*x = Ax * t * t + Bx * t + Cx;
	*y = Ay * t * t + By * t + Cy;
}

static void get_quadratic_bounds(double x0, double y0, double x1, double y1,
		double x2, double y2, double stroke_width, double *minx, double *miny,
		double *maxx, double *maxy) {
	// TODO: if control point is exactly between start and end, division by zero occurs
	double tx = (x0 - x1) / (x0 - 2 * x1 + x2);
	double ty = (y0 - y1) / (y0 - 2 * y1 + y2);

	*minx = MIN(x0, x2);
	*miny = MIN(y0, y2);
	*maxx = MAX(x0, x2);
	*maxy = MAX(y0, y2);

	if (0 < tx && tx < 1) {
		double x, y;
		evaluate_quadratic(x0, y0, x1, y1, x2, y2, tx, &x, &y);

		*minx = MIN(*minx, x);
		*miny = MIN(*miny, y);
		*maxx = MAX(*maxx, x);
		*maxy = MAX(*maxy, y);
	}

	if (0 < ty && ty < 1) {
		double x, y;
		evaluate_quadratic(x0, y0, x1, y1, x2, y2, ty, &x, &y);

		*minx = MIN(*minx, x);
		*miny = MIN(*miny, y);
		*maxx = MAX(*maxx, x);
		*maxy = MAX(*maxy, y);
	}

	*minx -= stroke_width * 0.5;
	*miny -= stroke_width * 0.5;
	*maxx += stroke_width * 0.5;
	*maxy += stroke_width * 0.5;
}

static double dot(double x0, double y0, double x1, double y1) {
	return x0 * x1 + y0 * y1;
}

static void get_quadratic_bounds_oriented(double x0, double y0, double x1,
		double y1, double x2, double y2, double stroke_width, double *pcx,
		double *pcy, double *pux, double *puy, double *pvx, double *pvy) {
	double minx, miny, maxx, maxy;
	double cx, cy;
	double ex, ey;

	double ux = x2 - x0;
	double uy = y2 - y0;

	double len = sqrt(ux * ux + uy * uy);
	if (len < 1e-6) {
		ux = 1;
		uy = 0;
	} else {
		ux /= len;
		uy /= len;
	}

	get_quadratic_bounds(0, 0, dot(ux, uy, x1 - x0, y1 - y0),
			dot(-uy, ux, x1 - x0, y1 - y0), dot(ux, uy, x2 - x0, y2 - y0),
			dot(-uy, ux, x2 - x0, y2 - y0), stroke_width, &minx, &miny, &maxx,
			&maxy);

	cx = (minx + maxx) / 2;
	cy = (miny + maxy) / 2;
	ex = (maxx - minx) / 2;
	ey = (maxy - miny) / 2;

	*pcx = x0 + ux * cx + -uy * cy;
	*pcy = y0 + uy * cx + ux * cy;
	*pux = ux * ex;
	*puy = uy * ex;
	*pvx = -uy * ey;
	*pvy = ux * ey;
}

static void calculatepq(double Ax, double Ay, double Bx, double By, double Cx,
		double Cy, double px, double py, double *p, double *q) {
	double a = -2 * dot(Ax, Ay, Ax, Ay);
	if (a == 0)
		a = 0.00000001;
	double b = -3 * dot(Ax, Ay, Bx, By);
	double c = 2 * dot(px, py, Ax, Ay) - 2 * dot(Cx, Cy, Ax, Ay)
			- dot(Bx, By, Bx, By);
	double d = dot(px, py, Bx, By) - dot(Cx, Cy, Bx, By);

	*p = (3 * a * c - b * b) / (3 * a * a);
	*q = (2 * b * b * b - 9 * a * b * c + 27 * a * a * d) / (27 * a * a * a);
}

static double arc_length_helper(double A, double B, double C, double t) {
	/* Integrate[Sqrt[A t^2 + B t + C], t] */
	return (2 * sqrt(A) * (B + 2 * A * t) * sqrt(C + t * (B + A * t))
			- (B * B - 4 * A * C)
					* log(
							B + 2 * A * t
									+ 2 * sqrt(A) * sqrt(C + t * (B + A * t))))
			/ (8 * sqrt(A * A * A));
}

static double arc_length(double Ax, double Ay, double Bx, double By, double Cx,
		double Cy, double t) {
	double A = 4 * (Ax * Ax + Ay * Ay);
	double B = 4 * (Ax * Bx + Ay * By);
	double C = Bx * Bx + By * By;

	return arc_length_helper(A, B, C, t) - arc_length_helper(A, B, C, 0);
}

static double inverse_arc_length(double Ax, double Ay, double Bx, double By,
		double Cx, double Cy, double u) {
	int i;

	double A = 4 * (Ax * Ax + Ay * Ay);
	double B = 4 * (Ax * Bx + Ay * By);
	double C = Bx * Bx + By * By;

	if (u <= 0)
		return 0;

	double length = arc_length_helper(A, B, C, 1)
			- arc_length_helper(A, B, C, 0);

	if (u >= length)
		return 1;

	double u0 = arc_length_helper(A, B, C, 0) + u;

	double a = 0;
	double b = 1;
	double fa = arc_length_helper(A, B, C, a) - u0;
	double fb = arc_length_helper(A, B, C, b) - u0;

	for (i = 0; i < 20; ++i) {
		double c = (a + b) / 2;
		double fc = arc_length_helper(A, B, C, c) - u0;

		if (fc < 0) {
			a = c;
			fa = fc;
		} else if (fc > 0) {
			b = c;
			fb = fc;
		} else
			break;
	}

	return a - fa * (b - a) / (fb - fa);
}

static void quad_segment(const double qin[6], double t0, double t1,
		double qout[6]) {
	double u0 = 1 - t0;
	double u1 = 1 - t1;

	double x0 = qin[0];
	double y0 = qin[1];
	double x1 = qin[2];
	double y1 = qin[3];
	double x2 = qin[4];
	double y2 = qin[5];

	qout[0] = (u0 * u0) * x0 + (u0 * t0 + u0 * t0) * x1 + (t0 * t0) * x2;
	qout[1] = (u0 * u0) * y0 + (u0 * t0 + u0 * t0) * y1 + (t0 * t0) * y2;
	qout[2] = (u0 * u1) * x0 + (u0 * t1 + u1 * t0) * x1 + (t0 * t1) * x2;
	qout[3] = (u0 * u1) * y0 + (u0 * t1 + u1 * t0) * y1 + (t0 * t1) * y2;
	qout[4] = (u1 * u1) * x0 + (u1 * t1 + u1 * t1) * x1 + (t1 * t1) * x2;
	qout[5] = (u1 * u1) * y0 + (u1 * t1 + u1 * t1) * y1 + (t1 * t1) * y2;
}

#if 0
static void add_stroke_quad(struct path *p, float x0,
		float y0, float x1, float y1, float x2, float y2) {
	float v0x, v0y, v1x, v1y, v2x,v2y;

	v0x = x1 - x0;
	v0y = y1 - y0;
	v1x = x2 - x0;
	v1y = y2 - y0;
	v2x = x1- x2;
	v2y = y1-y2;
	float l01 = sqrt(v0x*v0x + v0y*v0y);
	float l12 = sqrt(v2x*v2x + v2y*v2y);
	float l02 = sqrt(v1x*v1x + v1y*v1y);
	float vl = sqrt(v0x*v1x + v0y*v1y);
	float ix = v1x*vl / l02 +x0;
	float iy = v1y*vl/l02 + y0;
	float cx = (x0 + x1 + x2) / 3;
	float cy = (y0 + y1 + y2) / 3;
	float h = sqrt((ix - x1)*(ix - x1) + (iy - y1)*(iy - y1));

	float el = l01;
	if (el < l12)
	el = l12;

	float ex0 = ((x0 - x2) / l02)*el / h;
	float ey0 = ((y0 - y2) / l02)*el / h;
	float ex2 = ((x2 - x0) / l02)*el / h;
	float ey2 = ((y2 - y0) / l02)*el / h;
	float a0, b0, a1, b1;
	if (v0x == 0)
	v0x += 0.000001;
	if (v2x == 0)
	v2x += 0.000001;
	a0 = v0y / v0x;
	b0 = ey0+y0 - (ex0+x0)*a0;
	a1 = v2y / v2x;
	b1 = ey2+y2 - (ex2+x2)*a1;
	float epx = (b1 - b0) / (a0 - a1);
	float epy = a0*epx + b0;
	float ex1 = epx - x1;
	float ey1 = epy - y1;
	//float cu = 0.5;
	//float cv = 0.333f;
	float sc = vl/h;
	if (sc > 10)
	sc = 10;

	struct geometry *g = &p->stroke_geoms[1];

	int index = kv_size(g->vertices) / 8;

	kv_push_back(g->vertices, x0);
	kv_push_back(g->vertices, y0);
	kv_push_back(g->vertices, 0);
	kv_push_back(g->vertices, 0);
	kv_push_back(g->vertices, cx);
	kv_push_back(g->vertices, cy);
	kv_push_back(g->vertices, sc);
	kv_push_back(g->vertices, 0);

	kv_push_back(g->vertices, x2);
	kv_push_back(g->vertices, y2);
	kv_push_back(g->vertices, 1);
	kv_push_back(g->vertices, 0);
	kv_push_back(g->vertices, cx);
	kv_push_back(g->vertices, cy);
	kv_push_back(g->vertices, sc);
	kv_push_back(g->vertices, 0);

	kv_push_back(g->vertices, x1);
	kv_push_back(g->vertices, y1);
	kv_push_back(g->vertices, 0);
	kv_push_back(g->vertices, 1);
	kv_push_back(g->vertices, cx);
	kv_push_back(g->vertices, cy);
	kv_push_back(g->vertices, sc);
	kv_push_back(g->vertices, 0);

	kv_push_back(g->indices, index);
	kv_push_back(g->indices, index + 1);
	kv_push_back(g->indices, index + 2);
}

#else
static float lengthSq(float x1,float y1,float x2,float y2,float px,float py)
{
       // Adjust vectors relative to x1,y1
       // x2,y2 becomes relative vector from x1,y1 to end of segment
       x2 -= x1;
       y2 -= y1;
       // px,py becomes relative vector from x1,y1 to test point
       px -= x1;
       py -= y1;
       double dotprod = px * x2 + py * y2;
       double projlenSq;
       if (dotprod <= 0.0) {
           // px,py is on the side of x1,y1 away from x2,y2
           // distance to segment is length of px,py vector
           // "length of its (clipped) projection" is now 0.0
           projlenSq = 0.0;
       } else {
           // switch to backwards vectors relative to x2,y2
           // x2,y2 are already the negative of x1,y1=>x2,y2
           // to get px,py to be the negative of px,py=>x2,y2
           // the dot product of two negated vectors is the same
           // as the dot product of the two normal vectors
           px = x2 - px;
           py = y2 - py;
           dotprod = px * x2 + py * y2;
           if (dotprod <= 0.0) {
               // px,py is on the side of x2,y2 away from x1,y1
               // distance to segment is length of (backwards) px,py vector
               // "length of its (clipped) projection" is now 0.0
               projlenSq = 0.0;
           } else {
               // px,py is between x1,y1 and x2,y2
               // dotprod is the length of the px,py vector
               // projected on the x2,y2=>x1,y1 vector times the
               // length of the x2,y2=>x1,y1 vector
               projlenSq = dotprod * dotprod / (x2 * x2 + y2 * y2);
           }
       }
       // Distance to line is now the length of the relative point
       // vector minus the length of its projection onto the line
       // (which is zero if the projection falls outside the range
       //  of the line segment).
       double lenSq = px * px + py * py - projlenSq;
       if (lenSq < 0) {
           lenSq = 0;
       }
       return lenSq;
}

static float flatnessSq(float x1,float y1,float x2,float y2,float px,float py)
{
    float lsq=lengthSq(x1,y1,x2,y2,px,py);
    x2 -= x1;
    y2 -= y1;
    lsq/=(x2*x2+y2*y2);
    return lsq;
}

static void subdivide_quad(float c[],float r[]) {
        double x1 = c[0];
        double y1 = c[1];
        double ctrlx = c[2];
        double ctrly = c[3];
        double x2 = c[4];
        double y2 = c[5];

        r[4] = x2;
        r[5] = y2;
        x1 = (x1 + ctrlx) / 2.0;
        y1 = (y1 + ctrly) / 2.0;
        x2 = (x2 + ctrlx) / 2.0;
        y2 = (y2 + ctrly) / 2.0;
        ctrlx = (x1 + x2) / 2.0;
        ctrly = (y1 + y2) / 2.0;
        c[2] = x1;
        c[3] = y1;
        c[4] = ctrlx;
        c[5] = ctrly;
        r[0] = ctrlx;
        r[1] = ctrly;
        r[2] = x2;
        r[3] = y2;
}

static void add_stroke_quad_int(struct path *path, double x0, double y0, double x1,
		double y1, double x2, double y2) {
	int i;

	double Ax = x0 - 2 * x1 + x2;
	double Ay = y0 - 2 * y1 + y2;
	double Bx = 2 * (x1 - x0);
	double By = 2 * (y1 - y0);
	double Cx = x0;
	double Cy = y0;

	double cx, cy, ux, uy, vx, vy;
	get_quadratic_bounds_oriented(x0, y0, x1, y1, x2, y2,
			path->stroke_width+path->stroke_margin, &cx, &cy, &ux, &uy, &vx, &vy);

	double a = -2 * dot(Ax, Ay, Ax, Ay);
	double b = -3 * dot(Ax, Ay, Bx, By);
	if (a == 0)
		a = 0.0000001;

	double px[4], py[4];

	px[0] = cx - ux - vx;
	py[0] = cy - uy - vy;
	px[1] = cx + ux - vx;
	py[1] = cy + uy - vy;
	px[2] = cx + ux + vx;
	py[2] = cy + uy + vy;
	px[3] = cx - ux + vx;
	py[3] = cy - uy + vy;

	double p[4], q[4];
	for (i = 0; i < 4; ++i)
		calculatepq(Ax, Ay, Bx, By, Cx, Cy, px[i], py[i], &p[i], &q[i]);

	struct geometry *g = &path->stroke_geoms[1];

	int index = kv_size(g->vertices) / 12;

	for (i = 0; i < 4; ++i) {
		kv_push_back(g->vertices, px[i]);
		kv_push_back(g->vertices, py[i]);
		kv_push_back(g->vertices, p[i]);
		kv_push_back(g->vertices, q[i]);
		kv_push_back(g->vertices, Ax);
		kv_push_back(g->vertices, Ay);
		kv_push_back(g->vertices, Bx);
		kv_push_back(g->vertices, By);
		kv_push_back(g->vertices, Cx);
		kv_push_back(g->vertices, Cy);
		kv_push_back(g->vertices, -b / (3 * a));
		kv_push_back(g->vertices, path->stroke_width);
	}

	kv_push_back(g->indices, index);
	kv_push_back(g->indices, index + 1);
	kv_push_back(g->indices, index + 2);

	kv_push_back(g->indices, index);
	kv_push_back(g->indices, index + 2);
	kv_push_back(g->indices, index + 3);
}

static void add_stroke_quad(struct path *path, double x0, double y0, double x1,
		double y1, double x2, double y2,int max_sub,float flatness) {
	if ((max_sub<=0)||(flatness==0)||(flatnessSq(x0,y0,x2,y2,x1,y1)<flatness)) { //FLATNESS
		add_stroke_quad_int(path,x0,y0,x1,y1,x2,y2);
		return;
	}
	float c[6],d[6];
	c[0]=x0;
	c[1]=y0;
	c[2]=x1;
	c[3]=y1;
	c[4]=x2;
	c[5]=y2;
	subdivide_quad(c,d);
	add_stroke_quad(path,c[0],c[1],c[2],c[3],c[4],c[5],max_sub-1,flatness);
	add_stroke_quad(path,d[0],d[1],d[2],d[3],d[4],d[5],max_sub-1,flatness);
}

#endif

static void add_stroke_quad_dashed(struct path *path, double x0, double y0,
		double x1, double y1, double x2, double y2, double *dash_offset,float flatness) {
	int divide=4;
	if (path->num_dashes == 0) {
		add_stroke_quad(path, x0, y0, x1, y1, x2, y2,divide,flatness);
		return;
	}

	double Ax = x0 - 2 * x1 + x2;
	double Ay = y0 - 2 * y1 + y2;
	double Bx = 2 * (x1 - x0);
	double By = 2 * (y1 - y0);
	double Cx = x0;
	double Cy = y0;

	double q[6] = { x0, y0, x1, y1, x2, y2 };

	double length = arc_length(Ax, Ay, Bx, By, Cx, Cy, 1);

	double offset = -fmod(*dash_offset, path->dash_length);

	int i = 0;
	while (offset < length) {
		double o0 = MAX(offset, 0);
		double o1 = MIN(offset + path->dashes[i], length);

		if (o1 >= 0) {
			double t0 = inverse_arc_length(Ax, Ay, Bx, By, Cx, Cy, o0);
			double t1 = inverse_arc_length(Ax, Ay, Bx, By, Cx, Cy, o1);

			double qout[6];
			quad_segment(q, t0, t1, qout);
			add_stroke_quad(path, qout[0], qout[1], qout[2], qout[3], qout[4],
					qout[5],divide,flatness);
		}

		offset += path->dashes[i] + path->dashes[i + 1];
		i = (i + 2) % path->num_dashes;
	}

	*dash_offset = fmod(length + *dash_offset, path->dash_length);
}

static void add_join_miter_revert(struct path *path, float x0, float y0,
		float x1, float y1, float x2, float y2) {

}

static void add_join_miter_truncate(struct path *path, float x0, float y0,
		float x1, float y1, float x2, float y2) {

}

static void add_join_bevel(struct path *path, float x0, float y0, float x1,
		float y1, float x2, float y2) {
	float v0x = x0 - x1; //P1->P0
	float v0y = y0 - y1;
	float v1x = x2 - x1; //P1->P2
	float v1y = y2 - y1;

	float len0 = sqrtf(v0x * v0x + v0y * v0y);
	float len1 = sqrtf(v1x * v1x + v1y * v1y);

	if (len0 == 0 || len1 == 0)
		return;

	struct geometry *g = &path->stroke_geoms[0];

	int index = kv_size(g->vertices) / 4;

	float angle = acosf((v0x * v1x + v0y * v1y) / (len0 * len1));
	float a2 = (M_PI - angle) / 2;
	float ext = sin(a2) * path->stroke_width;
	if (angle < (M_PI / 2))
		ext /= 2; //??? Why is this necessary ???
	float nw = cos(a2) * path->stroke_width;
	float vx = (v0x / len0 + v1x / len1) / 2;
	float vy = (v0y / len0 + v1y / len1) / 2;
	float dx = vy * ext;
	float dy = -vx * ext;
	float ow = path->stroke_width;
	path->stroke_width = nw;
	add_stroke_line(path, x1 - dx, y1 - dy, x1 + dx, y1 + dy);
	path->stroke_width = ow;

	/*
	 float w0 = nw / len0;
	 float w1 = nw / len1;

	 if (v0x * v1y - v0y * v1x < 0) { //>180�
	 kv_push_back(g->vertices, x1);
	 kv_push_back(g->vertices, y1);
	 kv_push_back(g->vertices, -v0y * w0);
	 kv_push_back(g->vertices, v0x * w0);
	 kv_push_back(g->vertices, x1);
	 kv_push_back(g->vertices, y1);
	 kv_push_back(g->vertices, 0);
	 kv_push_back(g->vertices, 0);
	 kv_push_back(g->vertices, x1);
	 kv_push_back(g->vertices, y1);
	 kv_push_back(g->vertices, v1y * w1);
	 kv_push_back(g->vertices, -v1x * w1);
	 } else { //<180�
	 kv_push_back(g->vertices, x1);
	 kv_push_back(g->vertices, y1);
	 kv_push_back(g->vertices, -v1y * w1);
	 kv_push_back(g->vertices, v1x * w1);
	 kv_push_back(g->vertices, x1);
	 kv_push_back(g->vertices, y1);
	 kv_push_back(g->vertices, 0);
	 kv_push_back(g->vertices, 0);
	 kv_push_back(g->vertices, x1);
	 kv_push_back(g->vertices, y1);
	 kv_push_back(g->vertices, v0y * w0);
	 kv_push_back(g->vertices, -v0x * w0);
	 }

	 kv_push_back(g->indices, index);
	 kv_push_back(g->indices, index + 1);
	 kv_push_back(g->indices, index + 2);
	 */
}

static void add_join_round(struct path *path, float x0, float y0, float x1,
		float y1, float x2, float y2) {

}

static int check_offset(struct path *path, float of) {
	int i;

	if (path->num_dashes == 0)
		return 1;

	float offset = 0;
	for (i = 0; i < path->num_dashes; i += 2) {
		float o0 = offset;
		float o1 = offset + path->dashes[i];

		if (o0 <= of && of <= o1)
			return 1;

		offset += path->dashes[i] + path->dashes[i + 1];
	}

	return 0;
}

static void add_join(struct path *path, float x0, float y0, float x1, float y1,
		float x2, float y2) {
	switch (path->join_style) {
	case PATHJOIN_MITER_REVERT:
		add_join_miter_revert(path, x0, y0, x1, y1, x2, y2);
		break;
	case PATHJOIN_MITER_TRUNCATE:
		add_join_miter_truncate(path, x0, y0, x1, y1, x2, y2);
		break;
	case PATHJOIN_BEVEL:
		add_join_bevel(path, x0, y0, x1, y1, x2, y2);
		break;
	case PATHJOIN_ROUND:
		add_join_round(path, x0, y0, x1, y1, x2, y2);
		break;
	case PATHJOIN_NONE:
		break;
	}
}

typedef kvec_t(float)
kvec_float_t;

static void corner_start(kvec_float_t *v, float x, float y, float offset) {
	kv_push_back(*v, x);
	kv_push_back(*v, y);
	kv_push_back(*v, offset);
}

static void corner_continue(kvec_float_t *v, float x0, float y0, float x1,
		float y1, float offset) {
	float x = kv_a(*v, kv_size(*v) - 3);
	float y = kv_a(*v, kv_size(*v) - 2);

	// TODO: check equality with epsilon
	if (x != x0 || x != x1 || x0 != x1 || y != y0 || y != y1 || y0 != y1) {
		kv_push_back(*v, x0);
		kv_push_back(*v, y0);
		kv_push_back(*v, x1);
		kv_push_back(*v, y1);
		kv_push_back(*v, offset);
	}
}

static void corner_end(kvec_float_t *v, float x, float y) {
	float x0 = kv_a(*v, 0);
	float y0 = kv_a(*v, 1);

	float x1 = kv_a(*v, kv_size(*v) - 3);
	float y1 = kv_a(*v, kv_size(*v) - 2);

	// TODO: check equality with epsilon
	if (x != x0 || x != x1 || x0 != x1 || y != y0 || y != y1 || y0 != y1) {
		kv_push_back(*v, x);
		kv_push_back(*v, y);
	} else {
		kv_pop_back(*v);
		kv_pop_back(*v);
		kv_pop_back(*v);
	}
}

static void update_bounds(float bounds[4], size_t num_vertices,
		const float *vertices, int stride) {
	size_t i;

	for (i = 0; i < num_vertices; i += stride) {
		float x = vertices[i];
		float y = vertices[i + 1];

		bounds[0] = MIN(bounds[0], x);
		bounds[1] = MIN(bounds[1], y);
		bounds[2] = MAX(bounds[2], x);
		bounds[3] = MAX(bounds[3], y);
	}
}

static void create_stroke_geometry(struct path *path) {
#define c0 coords[icoord]
#define c1 coords[icoord + 1]
#define c2 coords[icoord + 2]
#define c3 coords[icoord + 3]

#define set(x1, y1, x2, y2) ncpx = x1; ncpy = y1; npepx = x2; npepy = y2;

	reduced_path_vec *reduced_paths = &path->reduced_paths;

	size_t i, j;

	for (i = 0; i < 2; ++i) {
		kv_init(path->stroke_geoms[i].vertices);
		kv_init(path->stroke_geoms[i].indices);
	}

	double offset = 0;

	for (i = 0; i < kv_size(*reduced_paths); ++i) {
		struct reduced_path *p = &kv_a(*reduced_paths, i);

		kvec_float_t corners;
		kv_init(corners);

		size_t num_commands = kv_size(p->commands);
		unsigned char *commands = kv_data(p->commands);
		float *coords = kv_data(p->coords);

		int closed = 0;

		int icoord = 0;

		float spx = 0, spy = 0;
		float cpx = 0, cpy = 0;
		float ncpx = 0, ncpy = 0;
		float npepx = 0, npepy = 0;

		for (j = 0; j < num_commands; ++j) {
			switch (commands[j]) {
			case PATHCMD_MOVE_TO:
				corner_start(&corners, c0, c1, offset);
				set(c0, c1, c0, c1);
				spx = ncpx;
				spy = ncpy;
				icoord += 2;
				break;
				case PATHCMD_LINE_TO:
				add_stroke_line_dashed(path, cpx, cpy, c0, c1, &offset);
				corner_continue(&corners, (cpx + c0) / 2, (cpy + c1) / 2, c0, c1, offset);
				set(c0, c1, c0, c1);
				icoord += 2;
				break;
				case PATHCMD_QUADRATIC_CURVE_TO:
				add_stroke_quad_dashed(path, cpx, cpy, c0, c1, c2, c3, &offset,path->stroke_flatness);
				corner_continue(&corners, c0, c1, c2, c3, offset);
				set(c2, c3, c0, c1);
				icoord += 4;
				break;
				case PATHCMD_CLOSE_PATH:
				add_stroke_line_dashed(path, cpx, cpy, spx, spy, &offset);
				corner_end(&corners, (cpx + spx) / 2, (cpy + spy) / 2);
				set(spx, spy, spx, spy);
				closed = 1;
				break;
			}

			cpx = ncpx;
			cpy = ncpy;
		}

		size_t ncorners = kv_size(corners);
		if (!closed) {
			if (ncorners > 7)
				ncorners -= 7;
			else
				ncorners = 0;
		}
		ncorners /= 5;

		for (j = 0; j < ncorners; j++) {
			int j0 = j;
			int j1 = (j + 1) % ncorners;

			float x0 = kv_a(corners, j0 * 5 + 3);
			float y0 = kv_a(corners, j0 * 5 + 4);
			float x1 = kv_a(corners, j1 * 5 + 0);
			float y1 = kv_a(corners, j1 * 5 + 1);
			float of = kv_a(corners, j1 * 5 + 2);
			float x2 = kv_a(corners, j1 * 5 + 3);
			float y2 = kv_a(corners, j1 * 5 + 4);

			if (check_offset(path, of))
				add_join(path, x0, y0, x1, y1, x2, y2);
		}

		kv_free(corners);
	}

#undef c0
#undef c1
#undef c2
#undef c3

#undef set

	path->stroke_bounds[0] = 1e30f;
	path->stroke_bounds[1] = 1e30f;
	path->stroke_bounds[2] = -1e30f;
	path->stroke_bounds[3] = -1e30f;

	update_bounds(path->stroke_bounds, kv_size(path->stroke_geoms[0].vertices),
			kv_data(path->stroke_geoms[0].vertices), 4);
	update_bounds(path->stroke_bounds, kv_size(path->stroke_geoms[1].vertices),
			kv_data(path->stroke_geoms[1].vertices), 12);

	for (i = 0; i < 2; ++i) {

		path->stroke_geoms[i].vertex_buffer->assign(
				kv_data(path->stroke_geoms[i].vertices),
				kv_data(
						path->stroke_geoms[i].vertices)+kv_size(path->stroke_geoms[i].vertices));
		path->stroke_geoms[i].index_buffer->assign(
				kv_data(path->stroke_geoms[i].indices),
				kv_data(
						path->stroke_geoms[i].indices)+kv_size(path->stroke_geoms[i].indices));
		path->stroke_geoms[i].vertex_buffer->Update();
		path->stroke_geoms[i].index_buffer->Update();

		/*
		 glBindBuffer(PATHCMD_ARRAY_BUFFER, path->stroke_geoms[i].vertex_buffer);
		 glBufferData(PATHCMD_ARRAY_BUFFER, kv_size(path->stroke_geoms[i].vertices) * sizeof(float), kv_data(path->stroke_geoms[i].vertices), PATHCMD_STATIC_DRAW);

		 glBindBuffer(PATHCMD_ELEMENT_ARRAY_BUFFER, path->stroke_geoms[i].index_buffer);
		 glBufferData(PATHCMD_ELEMENT_ARRAY_BUFFER, kv_size(path->stroke_geoms[i].indices) * sizeof(unsigned short), kv_data(path->stroke_geoms[i].indices), PATHCMD_STATIC_DRAW);
		 */

		path->stroke_geoms[i].count = kv_size(path->stroke_geoms[i].indices);

		kv_free(path->stroke_geoms[i].vertices);
		kv_free(path->stroke_geoms[i].indices);
	}
}

static void add_fill_line(struct path *p, float xc, float yc, float x0,
		float y0, float x1, float y1) {
	float v0x = x0 - xc;
	float v0y = y0 - yc;
	float v1x = x1 - xc;
	float v1y = y1 - yc;

	struct geometry *g = NULL;
	int index;

	if (v0x * v1y - v0y * v1x < 0) {
		g = &p->fill_geoms[1];

		index = kv_size(g->vertices) / 4;

		kv_push_back(g->vertices, xc);
		kv_push_back(g->vertices, yc);
		kv_push_back(g->vertices, 0);
		kv_push_back(g->vertices, 0);

		kv_push_back(g->vertices, x1);
		kv_push_back(g->vertices, y1);
		kv_push_back(g->vertices, 0);
		kv_push_back(g->vertices, 0);

		kv_push_back(g->vertices, x0);
		kv_push_back(g->vertices, y0);
		kv_push_back(g->vertices, 0);
		kv_push_back(g->vertices, 0);
	} else {
		g = &p->fill_geoms[0];

		index = kv_size(g->vertices) / 4;

		kv_push_back(g->vertices, xc);
		kv_push_back(g->vertices, yc);
		kv_push_back(g->vertices, 0);
		kv_push_back(g->vertices, 0);

		kv_push_back(g->vertices, x0);
		kv_push_back(g->vertices, y0);
		kv_push_back(g->vertices, 0);
		kv_push_back(g->vertices, 0);

		kv_push_back(g->vertices, x1);
		kv_push_back(g->vertices, y1);
		kv_push_back(g->vertices, 0);
		kv_push_back(g->vertices, 0);
	}

	kv_push_back(g->indices, index);
	kv_push_back(g->indices, index + 1);
	kv_push_back(g->indices, index + 2);
}

static void add_fill_quad(struct path *p, float xc, float yc, float x0,
		float y0, float x1, float y1, float x2, float y2) {
	float v0x, v0y, v1x, v1y;

	v0x = x0 - xc;
	v0y = y0 - yc;
	v1x = x2 - xc;
	v1y = y2 - yc;

	if (v0x * v1y - v0y * v1x < 0) {
		struct geometry *g = &p->fill_geoms[1];

		int index = kv_size(g->vertices) / 4;

		kv_push_back(g->vertices, xc);
		kv_push_back(g->vertices, yc);
		kv_push_back(g->vertices, 0);
		kv_push_back(g->vertices, 0);

		kv_push_back(g->vertices, x2);
		kv_push_back(g->vertices, y2);
		kv_push_back(g->vertices, 0);
		kv_push_back(g->vertices, 0);

		kv_push_back(g->vertices, x0);
		kv_push_back(g->vertices, y0);
		kv_push_back(g->vertices, 0);
		kv_push_back(g->vertices, 0);

		kv_push_back(g->indices, index);
		kv_push_back(g->indices, index + 1);
		kv_push_back(g->indices, index + 2);
	} else {
		struct geometry *g = &p->fill_geoms[0];

		int index = kv_size(g->vertices) / 4;

		kv_push_back(g->vertices, xc);
		kv_push_back(g->vertices, yc);
		kv_push_back(g->vertices, 0);
		kv_push_back(g->vertices, 0);

		kv_push_back(g->vertices, x0);
		kv_push_back(g->vertices, y0);
		kv_push_back(g->vertices, 0);
		kv_push_back(g->vertices, 0);

		kv_push_back(g->vertices, x2);
		kv_push_back(g->vertices, y2);
		kv_push_back(g->vertices, 0);
		kv_push_back(g->vertices, 0);

		kv_push_back(g->indices, index);
		kv_push_back(g->indices, index + 1);
		kv_push_back(g->indices, index + 2);
	}

	v0x = x1 - x0;
	v0y = y1 - y0;
	v1x = x2 - x0;
	v1y = y2 - y0;

	if (v0x * v1y - v0y * v1x < 0) {
		struct geometry *g = &p->fill_geoms[3];

		int index = kv_size(g->vertices) / 4;

		kv_push_back(g->vertices, x0);
		kv_push_back(g->vertices, y0);
		kv_push_back(g->vertices, 0);
		kv_push_back(g->vertices, 0);

		kv_push_back(g->vertices, x2);
		kv_push_back(g->vertices, y2);
		kv_push_back(g->vertices, 1);
		kv_push_back(g->vertices, 1);

		kv_push_back(g->vertices, x1);
		kv_push_back(g->vertices, y1);
		kv_push_back(g->vertices, 0.5f);
		kv_push_back(g->vertices, 0);

		kv_push_back(g->indices, index);
		kv_push_back(g->indices, index + 1);
		kv_push_back(g->indices, index + 2);
	} else {
		struct geometry *g = &p->fill_geoms[2];

		int index = kv_size(g->vertices) / 4;

		kv_push_back(g->vertices, x2);
		kv_push_back(g->vertices, y2);
		kv_push_back(g->vertices, 1);
		kv_push_back(g->vertices, 1);

		kv_push_back(g->vertices, x0);
		kv_push_back(g->vertices, y0);
		kv_push_back(g->vertices, 0);
		kv_push_back(g->vertices, 0);

		kv_push_back(g->vertices, x1);
		kv_push_back(g->vertices, y1);
		kv_push_back(g->vertices, 0.5f);
		kv_push_back(g->vertices, 0);

		kv_push_back(g->indices, index);
		kv_push_back(g->indices, index + 1);
		kv_push_back(g->indices, index + 2);
	}
}

static void create_fill_geometry(struct path *path) {
#define c0 coords[icoord]
#define c1 coords[icoord + 1]
#define c2 coords[icoord + 2]
#define c3 coords[icoord + 3]

#define set(x1, y1, x2, y2) ncpx = x1; ncpy = y1; npepx = x2; npepy = y2;

	reduced_path_vec *reduced_paths = &path->reduced_paths;

	size_t i, j;

	for (i = 0; i < 4; ++i) {
		kv_init(path->fill_geoms[i].vertices);
		kv_init(path->fill_geoms[i].indices);
	}

	for (i = 0; i < kv_size(*reduced_paths); ++i) {
		struct reduced_path *p = &kv_a(*reduced_paths, i);

		size_t num_commands = kv_size(p->commands);
		unsigned char *commands = kv_data(p->commands);
		float *coords = kv_data(p->coords);

		int closed = 0;

		int icoord = 0;

		float spx = 0, spy = 0;
		float cpx = 0, cpy = 0;
		float ncpx = 0, ncpy = 0;
		float npepx = 0, npepy = 0;

		float xc, yc;

		for (j = 0; j < num_commands; ++j) {
			switch (commands[j]) {
			case PATHCMD_MOVE_TO:
				set(c0, c1, c0, c1)
				;
				spx = ncpx;
				spy = ncpy;
				xc = spx;
				yc = spy;
				icoord += 2;
				break;
			case PATHCMD_LINE_TO:
				add_fill_line(path, xc, yc, cpx, cpy, c0, c1);
				set(c0, c1, c0, c1);
				icoord += 2;
				break;
				case PATHCMD_QUADRATIC_CURVE_TO:
				add_fill_quad(path, xc, yc, cpx, cpy, c0, c1, c2, c3);
				set(c2, c3, c0, c1);
				icoord += 4;
				break;
				case PATHCMD_CLOSE_PATH:
				add_fill_line(path, xc, yc, cpx, cpy, spx, spy);
				set(spx, spy, spx, spy);
				closed = 1;
				break;
			}

			cpx = ncpx;
			cpy = ncpy;
		}

		if (closed == 0) {
			// TODO: close the fill geometry
		}
	}

#undef c0
#undef c1
#undef c2
#undef c3

#undef set

	path->fill_bounds[0] = 1e30f;
	path->fill_bounds[1] = 1e30f;
	path->fill_bounds[2] = -1e30f;
	path->fill_bounds[3] = -1e30f;

	update_bounds(path->fill_bounds, kv_size(path->fill_geoms[0].vertices),
			kv_data(path->fill_geoms[0].vertices), 4);
	update_bounds(path->fill_bounds, kv_size(path->fill_geoms[1].vertices),
			kv_data(path->fill_geoms[1].vertices), 4);
	update_bounds(path->fill_bounds, kv_size(path->fill_geoms[2].vertices),
			kv_data(path->fill_geoms[2].vertices), 4);
	update_bounds(path->fill_bounds, kv_size(path->fill_geoms[3].vertices),
			kv_data(path->fill_geoms[3].vertices), 4);

	path->fill_bounds_vbo->resize(8);
	(*path->fill_bounds_vbo)[0] = path->fill_bounds[0];
	(*path->fill_bounds_vbo)[1] = path->fill_bounds[1];
	(*path->fill_bounds_vbo)[2] = path->fill_bounds[2];
	(*path->fill_bounds_vbo)[3] = path->fill_bounds[1];
	(*path->fill_bounds_vbo)[4] = path->fill_bounds[2];
	(*path->fill_bounds_vbo)[5] = path->fill_bounds[3];
	(*path->fill_bounds_vbo)[6] = path->fill_bounds[0];
	(*path->fill_bounds_vbo)[7] = path->fill_bounds[3];
	path->fill_bounds_vbo->Update();

	struct merger4f m;
	init_merger4f(&m);

	path->fill_starts[0] = kv_size(m.indices);
	merge4f(&m, (const vector4f *) kv_data(path->fill_geoms[0].vertices),
			kv_data(path->fill_geoms[0].indices),
			kv_size(path->fill_geoms[0].indices));
	merge4f(&m, (const vector4f *) kv_data(path->fill_geoms[2].vertices),
			kv_data(path->fill_geoms[2].indices),
			kv_size(path->fill_geoms[2].indices));
	path->fill_counts[0] = kv_size(m.indices) - path->fill_starts[0];
	path->fill_starts[1] = kv_size(m.indices);
	merge4f(&m, (const vector4f *) kv_data(path->fill_geoms[1].vertices),
			kv_data(path->fill_geoms[1].indices),
			kv_size(path->fill_geoms[1].indices));
	merge4f(&m, (const vector4f *) kv_data(path->fill_geoms[3].vertices),
			kv_data(path->fill_geoms[3].indices),
			kv_size(path->fill_geoms[3].indices));
	path->fill_counts[1] = kv_size(m.indices) - path->fill_starts[1];

	path->fill_vertex_buffer->assign(kv_data(m.vertices),
	kv_data(m.vertices) + kv_size(m.vertices));
	path->fill_index_buffer->assign(kv_data(m.indices),
	kv_data(m.indices) + kv_size(m.indices));
	path->fill_vertex_buffer->Update();
	path->fill_index_buffer->Update();

	free_merger4f(&m);

	for (i = 0; i < 4; ++i) {
#if 0
		glBindBuffer(PATHCMD_ARRAY_BUFFER, path->fill_geoms[i].vertex_buffer);
		glBufferData(PATHCMD_ARRAY_BUFFER, kv_size(path->fill_geoms[i].vertices) * sizeof(float), kv_data(path->fill_geoms[i].vertices), PATHCMD_STATIC_DRAW);

		glBindBuffer(PATHCMD_ELEMENT_ARRAY_BUFFER, path->fill_geoms[i].index_buffer);
		glBufferData(PATHCMD_ELEMENT_ARRAY_BUFFER, kv_size(path->fill_geoms[i].indices) * sizeof(unsigned short), kv_data(path->fill_geoms[i].indices), PATHCMD_STATIC_DRAW);

		path->fill_geoms[i].count = kv_size(path->fill_geoms[i].indices);
#endif

		kv_free(path->fill_geoms[i].vertices);
		kv_free(path->fill_geoms[i].indices);
	}

}

static void init_path_rendering() {
	int i;

	paths = kh_init(path);

	init_segments();

	for (i = 0; i < 256; ++i)
		command_coords[i] = -1;

	command_coords[PATHCMD_CLOSE_PATH] = 0;
	command_coords['Z'] = 0;
	command_coords['z'] = 0;
	command_coords[PATHCMD_MOVE_TO] = 2;
	command_coords['M'] = 2;
	command_coords[PATHCMD_RELATIVE_MOVE_TO] = 2;
	command_coords['m'] = 2;
	command_coords[PATHCMD_LINE_TO] = 2;
	command_coords['L'] = 2;
	command_coords[PATHCMD_RELATIVE_LINE_TO] = 2;
	command_coords['l'] = 2;
	command_coords[PATHCMD_HORIZONTAL_LINE_TO] = 1;
	command_coords['H'] = 1;
	command_coords[PATHCMD_RELATIVE_HORIZONTAL_LINE_TO] = 1;
	command_coords['h'] = 1;
	command_coords[PATHCMD_VERTICAL_LINE_TO] = 1;
	command_coords['V'] = 1;
	command_coords[PATHCMD_RELATIVE_VERTICAL_LINE_TO] = 1;
	command_coords['v'] = 1;
	command_coords[PATHCMD_QUADRATIC_CURVE_TO] = 4;
	command_coords['Q'] = 4;
	command_coords[PATHCMD_RELATIVE_QUADRATIC_CURVE_TO] = 4;
	command_coords['q'] = 4;
	command_coords[PATHCMD_CUBIC_CURVE_TO] = 6;
	command_coords['C'] = 6;
	command_coords[PATHCMD_RELATIVE_CUBIC_CURVE_TO] = 6;
	command_coords['c'] = 6;
	command_coords[PATHCMD_SMOOTH_QUADRATIC_CURVE_TO] = 2;
	command_coords['T'] = 2;
	command_coords[PATHCMD_RELATIVE_SMOOTH_QUADRATIC_CURVE_TO] = 2;
	command_coords['t'] = 2;
	command_coords[PATHCMD_SMOOTH_CUBIC_CURVE_TO] = 4;
	command_coords['S'] = 4;
	command_coords[PATHCMD_RELATIVE_SMOOTH_CUBIC_CURVE_TO] = 4;
	command_coords['s'] = 4;
	command_coords[PATHCMD_SMALL_CCW_ARC_TO] = 5;
	command_coords[PATHCMD_RELATIVE_SMALL_CCW_ARC_TO] = 5;
	command_coords[PATHCMD_SMALL_CW_ARC_TO] = 5;
	command_coords[PATHCMD_RELATIVE_SMALL_CW_ARC_TO] = 5;
	command_coords[PATHCMD_LARGE_CCW_ARC_TO] = 5;
	command_coords[PATHCMD_RELATIVE_LARGE_CCW_ARC_TO] = 5;
	command_coords[PATHCMD_LARGE_CW_ARC_TO] = 5;
	command_coords[PATHCMD_RELATIVE_LARGE_CW_ARC_TO] = 5;
	command_coords[PATHCMD_RESTART_PATH] = 0;
	command_coords[PATHCMD_DUP_FIRST_CUBIC_CURVE_TO] = 4;
	command_coords[PATHCMD_DUP_LAST_CUBIC_CURVE_TO] = 4;
	command_coords[PATHCMD_RECT] = 4;
	command_coords[PATHCMD_CIRCULAR_CCW_ARC_TO] = 5;
	command_coords[PATHCMD_CIRCULAR_CW_ARC_TO] = 5;
	command_coords[PATHCMD_CIRCULAR_TANGENT_ARC_TO] = 5;
	command_coords[PATHCMD_ARC_TO] = 7;
	command_coords['A'] = 7;
	command_coords[PATHCMD_RELATIVE_ARC_TO] = 7;
	command_coords['a'] = 7;
	command_coords['B'] = 1;
	command_coords['b'] = 1;
}

static void cleanup_path_rendering() //TODO Call this
{
	cleanup_segments();
	kh_destroy(path, paths);
}

static struct path *get_path(unsigned int path) {
	khiter_t iter = kh_get(path, paths, path);
	if (iter == kh_end(paths))
		return NULL;

	return kh_val(paths, iter);
}

static void stroke_path(unsigned int path, const Matrix4 *xform, Sprite *spr) {
	khiter_t iter = kh_get(path, paths, path);
	if (iter == kh_end(paths))
		return;

	struct path *p = kh_val(paths, iter);

	if (p->is_stroke_dirty) {
		create_stroke_geometry(p);
		p->is_stroke_dirty = 0;
	}

	if (p->stroke_geoms[0].count > 0) {
		VertexBuffer<float> *vb = p->stroke_geoms[0].vertex_buffer;
		VertexBuffer<unsigned short> *ib = p->stroke_geoms[0].index_buffer;
		ShaderProgram *shp=spr->getShader(ShaderEngine::STDP_PATHSTROKELINE);
		shp->setConstant(1,	ShaderProgram::CMATRIX, 1, xform->data());
		shp->setConstant(3, ShaderProgram::CFLOAT, 1, &p->stroke_feather);
		shp->setData(ShaderProgram::DataVertex,
				ShaderProgram::DFLOAT, 4, &((*vb)[0]), vb->size() / 4,
				vb->modified, &vb->bufferCache);
		shp->drawElements(
				ShaderProgram::Triangles, p->stroke_geoms[0].count,
				ShaderProgram::DUSHORT, &((*ib)[0]), ib->modified,
				&ib->bufferCache);
		vb->modified = false;
		ib->modified = false;
	}

	if (p->stroke_geoms[1].count > 0) {
		VertexBuffer<float> *vb = p->stroke_geoms[1].vertex_buffer;
		VertexBuffer<unsigned short> *ib = p->stroke_geoms[1].index_buffer;

#if 0
		float w0x, w0y,w1x,w1y;
		ShaderEngine::Engine->getModel().transformPoint(p->stroke_width, 0, &w1x, &w1y);
		ShaderEngine::Engine->getModel().transformPoint(0, 0, &w0x, &w0y);
		float wx = w1x - w0x;
		float wy = w1y - w0y;

		float width = sqrt(wx*wx + wy*wy);
		float lwidth = p->stroke_width;

		ShaderProgram::pathShaderStrokeC->setConstant(1, ShaderProgram::CFLOAT, 1, &lwidth);
		ShaderProgram::pathShaderStrokeC->setConstant(4, ShaderProgram::CFLOAT, 1, &width);
		ShaderProgram::pathShaderStrokeC->setConstant(3, ShaderProgram::CFLOAT, 1, &p->stroke_feather);
		ShaderProgram::pathShaderStrokeC->setData(ShaderProgram::DataVertex,
				ShaderProgram::DFLOAT, 4, &((*vb)[0]), vb->size() / 4,
				vb->modified, &vb->bufferCache,32,0);
		vb->modified = false;
		ShaderProgram::pathShaderStrokeC->setData(1,
				ShaderProgram::DFLOAT, 4, &((*vb)[0]), vb->size() / 4,
				vb->modified, &vb->bufferCache,32,16);
		ShaderProgram::pathShaderStrokeC->drawElements(ShaderProgram::Triangles,
				p->stroke_geoms[1].count, ShaderProgram::DUSHORT, &((*ib)[0]),
				ib->modified, &ib->bufferCache);
		ib->modified = false;

#else
		ShaderProgram *shp=spr->getShader(ShaderEngine::STDP_PATHSTROKECURVE);
		shp->setConstant(1, ShaderProgram::CMATRIX, 1, xform->data());
		shp->setConstant(3, ShaderProgram::CFLOAT, 1, &p->stroke_feather);
		shp->setData(0, ShaderProgram::DFLOAT, 4,
				&((*vb)[0]), vb->size() / 4, vb->modified, &vb->bufferCache, 48,
				0);
		vb->modified = false;
		shp->setData(1, ShaderProgram::DFLOAT, 4,
				&((*vb)[0]), vb->size() / 4, vb->modified, &vb->bufferCache, 48,
				16);
		shp->setData(2, ShaderProgram::DFLOAT, 4,
				&((*vb)[0]), vb->size() / 4, vb->modified, &vb->bufferCache, 48,
				32);
		shp->drawElements(ShaderProgram::Triangles,
				p->stroke_geoms[1].count, ShaderProgram::DUSHORT, &((*ib)[0]),
				ib->modified, &ib->bufferCache);
		ib->modified = false;
#endif
	}

}

static void fill_path(unsigned int path, int fill_mode,
		ShaderEngine::DepthStencil stencil, const Matrix4 *xform, Sprite *spr) {
	struct path *p = NULL;

	khiter_t iter = kh_get(path, paths, path);
	if (iter == kh_end(paths)) {
		// TODO: check if we should set error here
		return;
	}

	ShaderEngine::StencilOp front, back;
	switch (fill_mode) {
	case PATHFILLMODE_COUNT_UP:
		front = ShaderEngine::STENCIL_INCR_WRAP;
		back = ShaderEngine::STENCIL_DECR_WRAP;
		break;
	case PATHFILLMODE_COUNT_DOWN:
		front = ShaderEngine::STENCIL_DECR_WRAP;
		back = ShaderEngine::STENCIL_INCR_WRAP;
		break;
	case PATHFILLMODE_INVERT:
	case PATHFILLMODE_DIRECT:
		front = ShaderEngine::STENCIL_INVERT;
		back = ShaderEngine::STENCIL_INVERT;
		break;
	default:
		// TODO: check if we should set error here
		return;
	}

	//glog_d("Fill path: Start");
	p = kh_val(paths, iter);

	if (p->is_fill_dirty) {
		//glog_d("Fill path: Generating");
		create_fill_geometry(p);
		p->is_fill_dirty = 0;
	}

	ShaderProgram *shp=spr->getShader(ShaderEngine::STDP_PATHFILLCURVE);

	shp->setConstant(1, ShaderProgram::CMATRIX, 1,	xform->data());

	VertexBuffer<vector4f> *vb = p->fill_vertex_buffer;
	VertexBuffer<unsigned short> *ib = p->fill_index_buffer;
	/*
	 glog_d("Fill path: VB Size:%d",vb->size());
	 for (int k=0;k<vb->size();k++)
	 glog_d("Fill path: VB[%d]=%f,%f,%f,%f",k,(*vb)[k].x,(*vb)[k].y,(*vb)[k].z,(*vb)[k].w);
	 glog_d("Fill path: IB Size:%d",ib->size());
	 for (int k=0;k<ib->size();k++)
	 glog_d("Fill path: IB[%d]=%d",k,(*ib)[k]);
	 */
	if (p->fill_counts[0] > 0) {
		stencil.sFail = front;
		if (fill_mode != PATHFILLMODE_DIRECT)
			ShaderEngine::Engine->setDepthStencil(stencil);
		stencil.sClear = false;
		//glog_d("Fill path: Fill0=%d S=%d",p->fill_counts[0],p->fill_starts[0]);
        shp->setData(0, ShaderProgram::DFLOAT, 4,
                                                vb->size() ? &((*vb)[0]) : NULL, vb->size(), vb->modified,
                                                &vb->bufferCache);
        vb->modified = false;
		shp->drawElements(ShaderProgram::Triangles,
				ib->size(), ShaderProgram::DUSHORT, &((*ib)[0]), ib->modified,
				&ib->bufferCache, p->fill_starts[0], p->fill_counts[0]);
		ib->modified = false;
	}

	if (p->fill_counts[1] > 0) {
		stencil.sFail = back;
		if (fill_mode != PATHFILLMODE_DIRECT)
			ShaderEngine::Engine->setDepthStencil(stencil);
		stencil.sClear = false;
		//glog_d("Fill path: Fill1=%d S=%d",p->fill_counts[1],p->fill_starts[1]);
        shp->setData(0, ShaderProgram::DFLOAT, 4,
                                                vb->size() ? &((*vb)[0]) : NULL, vb->size(), vb->modified,
                                                &vb->bufferCache);
        vb->modified = false;
		shp->drawElements(ShaderProgram::Triangles,
				ib->size(), ShaderProgram::DUSHORT, &((*ib)[0]), ib->modified,
				&ib->bufferCache, p->fill_starts[1], p->fill_counts[1]);
		ib->modified = false;
	}
	stencil.sClear = false;
}

static void get_path_points_line(float sx, float sy, float dx, float dy,
		float &offset, float interval, int &maxpts,
		std::vector<Path2D::PathPoint> &pts, float &toffset, bool last) {
	if (!maxpts)
		return;
	float dist = sqrtf((dx - sx)*(dx - sx) + (dy - sy)*(dy - sy));
	if (!dist)
		return;
	if (interval) {
		if (offset < dist) {
			float ey = (dy - sy) / dist;
			float ex = (dx - sx) / dist;
			float ea = atan2f(ex, -ey);
			while (offset < dist) {
				Path2D::PathPoint p;
				p.x = sx + offset * ex;
				p.y = sy + offset * ey;
				p.angle = ea;
				p.offset = toffset;
				pts.push_back(p);
				if (!--maxpts)
					break;
				offset += interval;
				toffset += interval;
			}
		}
	}
	else {
		float ey = (dy - sy) / dist;
		float ex = (dx - sx) / dist;
		float ea = atan2f(ex, -ey);
		Path2D::PathPoint p;
		p.x = sx;
		p.y = sy;
		p.angle = ea;
		p.offset = toffset;
		pts.push_back(p);
		toffset += dist;
		if (last) {
			p.x = dx;
			p.y = dy;
			p.offset = toffset;
			pts.push_back(p);
		}
	}
	offset -= dist;
}

static void get_path_points_curve(float sx, float sy, float tx, float ty,
		float dx, float dy, float &offset, float interval, int &maxpts,
		std::vector<Path2D::PathPoint> &pts, float flatSq,int maxsub, float &toffset, bool last) {
	if (!maxpts)
		return;
	if ((maxsub<=0)||(lengthSq(sx,sy,dx,dy,tx,ty)<flatSq)) {
		get_path_points_line(sx,sy,dx,dy,offset,interval,maxpts,pts,toffset,last);
		return;
	}
	float c[6],d[6];
	c[0]=sx;
	c[1]=sy;
	c[2]=tx;
	c[3]=ty;
	c[4]=dx;
	c[5]=dy;
	subdivide_quad(c,d);
	get_path_points_curve(c[0],c[1],c[2],c[3],c[4],c[5],offset,interval,maxpts,pts,flatSq,maxsub-1,toffset,false);
	get_path_points_curve(d[0],d[1],d[2],d[3],d[4],d[5],offset,interval,maxpts,pts,flatSq,maxsub-1,toffset,last);
}


static void get_path_points(struct path *path, float &offset, float interval,
		int &maxpts, float flatSq,int maxsub,std::vector<Path2D::PathPoint> &pts) {
#define c0 coords[icoord]
#define c1 coords[icoord + 1]
#define c2 coords[icoord + 2]
#define c3 coords[icoord + 3]

#define set(x1, y1, x2, y2) ncpx = x1; ncpy = y1; npepx = x2; npepy = y2;

	reduced_path_vec *reduced_paths = &path->reduced_paths;

	size_t i, j;
	float toffset=offset;

	for (i = 0; i < kv_size(*reduced_paths); ++i) {
		struct reduced_path *p = &kv_a(*reduced_paths, i);

		size_t num_commands = kv_size(p->commands);
		unsigned char *commands = kv_data(p->commands);
		float *coords = kv_data(p->coords);

		int icoord = 0;

		float spx = 0, spy = 0;
		float cpx = 0, cpy = 0;
		float ncpx = 0, ncpy = 0;
		float npepx = 0, npepy = 0;

		for (j = 0; j < num_commands; ++j) {
			bool last=(j==(num_commands-1));
			switch (commands[j]) {
			case PATHCMD_MOVE_TO:
				set(c0, c1, c0, c1)
				;
				spx = ncpx;
				spy = ncpy;
				icoord += 2;
				break;
			case PATHCMD_LINE_TO:
				get_path_points_line(cpx, cpy, c0,c1,offset,interval,maxpts,pts,toffset,last);
				set(c0, c1, c0, c1);
				icoord += 2;
				break;
				case PATHCMD_QUADRATIC_CURVE_TO:
				get_path_points_curve(cpx,cpy, c0,c1,c2,c3,offset,interval,maxpts,pts,flatSq,maxsub,toffset,last);
				set(c2, c3, c0, c1);
				icoord += 4;
				break;
				case PATHCMD_CLOSE_PATH:
				get_path_points_line(cpx,cpy, spx,spy,offset,interval,maxpts,pts,toffset,last);
				set(spx, spy, spx, spy);
				break;
			}

			cpx = ncpx;
			cpy = ncpy;
		}

	}

#undef c0
#undef c1
#undef c2
#undef c3

#undef set

}

int Path2D::buildPath(PrPath *ppath) {
	int path = gen_paths(1);
	path_commands(path, ppath->numCommands, ppath->commands, ppath->numCoords,
			ppath->coords);
	return path;
}

void Path2D::removePath(int path) {
	delete_paths(path, 1);
}

bool Path2D::initialized = false;
VertexBuffer<unsigned short> *Path2D::quadIndices = NULL;

Path2D::Path2D(Application* application) :
		Sprite(application) {
	if (!initialized) {
		init_path_rendering();
		quadIndices = new VertexBuffer<unsigned short>();
		quadIndices->resize(4);
		(*quadIndices)[0] = 0;
		(*quadIndices)[1] = 1;
		(*quadIndices)[2] = 3;
		(*quadIndices)[3] = 2;
		quadIndices->Update();
		initialized = true;
	}
	path = gen_paths(1);
	texturebase_ = NULL;
	convex_ = false;
	setFillColor(0xFFFFFF, 1);
	setLineColor(0x0, 1);
}

Path2D::~Path2D() {
	if (texturebase_ != NULL)
		texturebase_->unref();
	delete_paths(path, 1);
}

void Path2D::getPathPoints(float offset, float advance,int max, float flatness, int maxsub,std::vector<PathPoint> &points)
{
	struct path *p = get_path(path);
	if (p) {
		float ro=offset;
		int rm=max;
		get_path_points(p,ro,advance,rm,flatness*flatness,maxsub,points);
	}
}

void Path2D::setFillColor(unsigned int color, float alpha) {
	int r = (color >> 16) & 0xff;
	int g = (color >> 8) & 0xff;
	int b = color & 0xff;

	fillr_ = r / 255.f;
	fillg_ = g / 255.f;
	fillb_ = b / 255.f;
	filla_ = alpha;

	getPathBounds(path, filla_ > 0, linea_ > 0, &minx_, &miny_, &maxx_, &maxy_);
}

void Path2D::setLineColor(unsigned int color, float alpha) {
	int r = (color >> 16) & 0xff;
	int g = (color >> 8) & 0xff;
	int b = color & 0xff;

	liner_ = r / 255.f;
	lineg_ = g / 255.f;
	lineb_ = b / 255.f;
	linea_ = alpha;

	getPathBounds(path, filla_ > 0, linea_ > 0, &minx_, &miny_, &maxx_, &maxy_);
}

void Path2D::setLineThickness(float thickness, float feather, float margin, float flatness) {
	struct path *p = get_path(path);
	if (p) {
		p->stroke_width = thickness;
		if ((feather >= 0) && (feather <= 1.0))
			p->stroke_feather = feather;
        if (margin>=0)
            p->stroke_margin=margin;
        if (flatness>=0)
            p->stroke_flatness=flatness;
		p->is_stroke_dirty = 1;
		getPathBounds(path, filla_ > 0, linea_ > 0, &minx_, &miny_, &maxx_,
				&maxy_);
	}
}

void Path2D::setConvex(bool convex) {
    convex_ = convex;
}

void Path2D::impressPath(int path, Matrix4 xform,
		ShaderEngine::DepthStencil stencil) {
	struct path *p = get_path(path);
	if (!p)
		return; //No PATH

	stencil.sFunc = ShaderEngine::STENCIL_NEVER;
	fill_path(path, PATHFILLMODE_COUNT_UP, stencil, &xform, this);
}

void Path2D::fillBounds(VertexBuffer<float> *vb, float *fill,
		TextureData *texture, ShaderEngine::DepthStencil stencil,
		const Matrix4 *textureMatrix,VertexBuffer<unsigned char> *cb) {
	glPushColor();
	glMultColor(fill[0], fill[1], fill[2], fill[3]);

	stencil.sFunc = ShaderEngine::STENCIL_NOTEQUAL;
	stencil.sFail = ShaderEngine::STENCIL_KEEP;
	stencil.sRef = 0;
	stencil.sMask = 0xFF;
	stencil.sWMask = 0xFF;
	ShaderEngine::Engine->setDepthStencil(stencil);
	ShaderProgram *shp;
	VertexBuffer<unsigned short> *ib = quadIndices;
	if (texture) {
		float sx = ((float) texture->width) / texture->exwidth;
		float sy = ((float) texture->height) / texture->exheight;
		float texcoords[8] = { 0, 0, sx, 0, sx, sy, 0, sy };
		if (textureMatrix)
			for (int k = 0; k < 8; k += 2)
				textureMatrix->transformPoint(texcoords[k], texcoords[k + 1],
						texcoords + k, texcoords + k + 1);
		shp=getShader(ShaderEngine::STDP_TEXTURE);
		ShaderEngine::Engine->bindTexture(0, texture->id());
		shp->setData(ShaderProgram::DataTexture, ShaderProgram::DFLOAT, 2,
				texcoords, 4, true, NULL);
	} else {
	    if (cb&&(!cb->empty()))
	    {
			shp=getShader(ShaderEngine::STDP_COLOR);
	        shp->setData(ShaderProgram::DataColor,ShaderProgram::DUBYTE,4,&((*cb)[0]),cb->size()/4,cb->modified,&cb->bufferCache);
	        cb->modified=false;
	    }
	    else {
			shp=getShader(ShaderEngine::STDP_BASIC);
	    }
	}

	shp->setData(ShaderProgram::DataVertex, ShaderProgram::DFLOAT, 2,
			&((*vb)[0]), vb->size() / 2, vb->modified, &vb->bufferCache);
	shp->drawElements(ShaderProgram::TriangleStrip, ib->size(),
			ShaderProgram::DUSHORT, &((*ib)[0]), ib->modified,
			&ib->bufferCache);
	vb->modified = false;
	ib->modified = false;

	glPopColor();
}

void Path2D::fillPath(int path, Matrix4 xform, float fill[4],
		TextureData *texture, bool convex,
		const Matrix4 *textureMatrix,VertexBuffer<unsigned char> *cb) {
	struct path *p = get_path(path);
	if (!p)
		return; //No PATH
	if (fill[3] > 0) {
		if (p->is_fill_dirty) {
			//glog_d("Fill path: Generating");
			create_fill_geometry(p);
			p->is_fill_dirty = 0;
		}

		if ((texture == NULL) && ((!cb)||(cb->empty()))
				&& (convex || (p->fill_counts[0] == 0)
						|| (p->fill_counts[1] == 0))) {
			ShaderEngine::DepthStencil stencil;
			memset(&stencil, 0, sizeof(stencil));
			glPushColor();
			glMultColor(fill[0], fill[1], fill[2], fill[3]);
			fill_path(path, PATHFILLMODE_DIRECT, stencil, &xform,this);
			glPopColor();
		} else {
			ShaderEngine::DepthStencil stencil =
					ShaderEngine::Engine->pushDepthStencil();
			ShaderEngine::Engine->pushClip(p->fill_bounds[0], p->fill_bounds[1],
					p->fill_bounds[2] - p->fill_bounds[0] + 1,
					p->fill_bounds[3] - p->fill_bounds[1] + 1);
			stencil.sClear = true;
			impressPath(path, xform, stencil);
			stencil.sClear = false;
			fillBounds(p->fill_bounds_vbo, fill, texture, stencil,
					textureMatrix,cb);
			ShaderEngine::Engine->popClip();
			ShaderEngine::Engine->popDepthStencil();
		}
	}
}

void Path2D::strokePath(int path, Matrix4 xform, float line[4]) {
	struct path *p = get_path(path);
	if (!p)
		return; //No PATH
	if (line[3] > 0) {
		glPushColor();
		glMultColor(line[0], line[1], line[2], line[3]);
		stroke_path(path, &xform, this);
		glPopColor();
	}
}

void Path2D::drawPath(int path, Matrix4 xform, float fill[4], float line[4],
		TextureData *texture, bool convex,
		const Matrix4 *textureMatrix,VertexBuffer<unsigned char> *cb) {
	fillPath(path, xform, fill, texture, convex, textureMatrix,cb);
	strokePath(path, xform, line);
}

void Path2D::getPathBounds(int path, bool fill, bool stroke, float *iminx,
		float *iminy, float *imaxx, float *imaxy) {
	float minx = 0;
	float miny = 0;
	float maxx = 0;
	float maxy = 0;
	struct path *p = get_path(path);
	if ((stroke || fill) && p) {
		minx = 1e30f;
		miny = 1e30f;
		maxx = -1e30f;
		maxy = -1e30f;
		if (stroke) {
			if (p->is_stroke_dirty) {
				create_stroke_geometry(p);
				p->is_stroke_dirty = 0;
			}
			minx = MIN(minx, p->stroke_bounds[0]);
			miny = MIN(miny, p->stroke_bounds[1]);
			maxx = MAX(maxx, p->stroke_bounds[2]);
			maxy = MAX(maxy, p->stroke_bounds[3]);
		}
		if (fill) {
			if (p->is_fill_dirty) {
				create_fill_geometry(p);
				p->is_fill_dirty = 0;
			}
			minx = MIN(minx, p->fill_bounds[0]);
			miny = MIN(miny, p->fill_bounds[1]);
			maxx = MAX(maxx, p->fill_bounds[2]);
			maxy = MAX(maxy, p->fill_bounds[3]);
		}
	}
	if (iminx)
		*iminx = minx;
	if (iminy)
		*iminy = miny;
	if (imaxx)
		*imaxx = maxx;
	if (imaxy)
		*imaxy = maxy;
}

void Path2D::doDraw(const CurrentTransform&, float sx, float sy, float ex,
		float ey) {
	float fill[4] = { fillr_, fillg_, fillb_, filla_ };
	float line[4] = { liner_, lineg_, lineb_, linea_ };
	drawPath(path, Matrix4(), fill, line,
			texturebase_ ? (texturebase_->data) : NULL, convex_,
			&textureMatrix_,&colors_);
}

void Path2D::extraBounds(float* minx, float* miny, float* maxx,
		float* maxy) const {
	if (minx)
		*minx = minx_;
	if (miny)
		*miny = miny_;
	if (maxx)
		*maxx = maxx_;
	if (maxy)
		*maxy = maxy_;
}

void Path2D::setTexture(TextureBase *texturebase, const Matrix4* matrix) {
	TextureBase *originaltexturebase = texturebase_;

	texturebase_ = texturebase;
	texturebase_->ref();

	if (originaltexturebase)
		originaltexturebase->unref();

	if (matrix)
		textureMatrix_ = matrix->inverse();

}

void Path2D::setPath(int num_commands, const unsigned char *commands,
		int num_coords, const float *coords) {
	path_commands(path, num_commands, commands, num_coords, coords);
	getPathBounds(path, filla_ > 0, linea_ > 0, &minx_, &miny_, &maxx_, &maxy_);
}

void Path2D::setPath(const PrPath *ppath) {
	path_commands(path, ppath->numCommands, ppath->commands, ppath->numCoords,
			ppath->coords);
	getPathBounds(path, filla_ > 0, linea_ > 0, &minx_, &miny_, &maxx_, &maxy_);
}

void Path2D::setGradient(int c1, float a1, int c2, float a2, int c3, float a3, int c4, float a4)
{
	c1_ = c1, a1_ = a1, c2_ = c2, a2_ = a2, c3_ = c3, a3_ = a3, c4_ = c4, a4_ = a4;
    colors_.resize(16);
    colors_[0] = ((c1 >> 16) & 0xff) * a1;
    colors_[1] = ((c1 >> 8) & 0xff) * a1;
    colors_[2] = (c1 & 0xff) * a1;
    colors_[3] = 255 * a1;
    colors_[4] = ((c2 >> 16) & 0xff) * a2;
    colors_[5] = ((c2 >> 8) & 0xff) * a2;
    colors_[6] = (c2 & 0xff) * a2;
    colors_[7] = 255 * a2;
    colors_[8] = ((c3 >> 16) & 0xff) * a3;
    colors_[9] = ((c3 >> 8) & 0xff) * a3;
    colors_[10] = (c3 & 0xff) * a3;
    colors_[11] = 255 * a3;
    colors_[12] = ((c4 >> 16) & 0xff) * a4;
    colors_[13] = ((c4 >> 8) & 0xff) * a4;
    colors_[14] = (c4 & 0xff) * a4;
    colors_[15] = 255 * a4;
    colors_.Update();
}

int Path2D::getMixedColor(int c1, int c2, float a1,float a2,float a,float &am)
{
    int b1 = c1 % 256;
    int g1 = int(c1/256)%256;
    int r1 = int(c1/65536)%256;
    int b2 = c2 % 256;
    int g2 = int(c2/256)%256;
    int r2 = int(c2/65536)%256;
    int r = r1*a+r2*(1-a);
    int g = g1*a+g2*(1-a);
    int b = b1*a+b2*(1-a);
    am= a1*a+a2*(1-a);
    return int(r)*65536+int(g)*256+int(b);
}

void Path2D::setGradientWithAngle(int co1, float a1, int co2, float a2, float angle)
{
    const float PI =3.141592653589793238463;

    float dirx = cos(angle/180*PI)/2;
    float diry = sin(angle/180*PI)/2;

    float f1 = 0.5-dirx-diry;
    float f2 = 0.5+dirx-diry;
    float f3 = 0.5+dirx+diry;
    float f4 = 0.5-dirx+diry;

    float fmin = f1 < f2 ? f1 : f2;
    fmin = fmin < f3 ? fmin : f3;
    fmin = fmin < f4 ? fmin : f4;

    float fmax = f1 > f2 ? f1 : f2;
    fmax = fmax > f3 ? fmax : f3;
    fmax = fmax > f4 ? fmax : f4;

    float fscl = 1/(fmax-fmin);
    f1 = (f1-fmin)*fscl;
    f2 = (f2-fmin)*fscl;
    f3 = (f3-fmin)*fscl;
    f4 = (f4-fmin)*fscl;

    float ao1,ao2,ao3,ao4;
    float c1 = getMixedColor(co1,co2,a1,a2,f1,ao1);
    float c2 = getMixedColor(co1,co2,a1,a2,f2,ao2);
    float c3 = getMixedColor(co1,co2,a1,a2,f3,ao3);
    float c4 = getMixedColor(co1,co2,a1,a2,f4,ao4);

    setGradient(c1, ao1, c2, ao2, c3, ao3, c4, ao4);
}

extern "C" void prFreePath(struct PrPath *svgPath) {
	if (svgPath == NULL)
		return;

	free(svgPath->coords);
	free(svgPath->commands);
	free(svgPath);
}

