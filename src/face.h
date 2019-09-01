#ifndef FACE_H
#define FACE_H

/*
 * Copyright (C) 2019  Jimmy Aguilar Mena
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <config.h>

#define CREATE_FIND(array_type)						\
  array_type *								\
  find_ ## array_type ## _by_name (const_astr name,			\
                                   array_type *array,			\
                                   int asize)				\
  {									\
    for (int i = 0; i < asize; ++i) {					\
      if (strcmp (astr_cstr(name), array[i].name) == 0)			\
	return &array[i];						\
    }									\
    return NULL;							\
  }									\
									\
  const_astr								\
  minibuf_read_ ##array_type## _name (const array_type *array,		\
                                      int asize,			\
                                      const char *fmt, ...)		\
  {									\
    Completion cp = completion_new (false);				\
									\
    for (int i = 0; i < asize; ++i)					\
      gl_sortedlist_add (get_completion_completions (cp),		\
                         completion_strcmp,				\
                         xstrdup (array[i].name));			\
									\
    va_list ap;								\
    va_start (ap, fmt);							\
									\
    const_astr ms = minibuf_vread_completion (fmt, "", cp, NULL,	\
                                              "No valid name given",	\
                                              minibuf_test_in_completions, \
                                              "Undefined name `%s'",	\
                                              ap);			\
    va_end (ap);							\
    									\
    return ms;								\
  }

#define DECLARE_FIND(array_type)				\
  array_type *							\
  find_ ## array_type ## _by_name (const_astr name,		\
                                   array_type *array,		\
                                   int asize);			\
								\
  const_astr							\
  minibuf_read_ ##array_type## _name (const array_type *array,	\
                                      int asize,		\
                                      const char *fmt, ...);

#include "astr.h"


// Attributes ==============================================
#define NATTS 8

typedef struct attrib_t {
  const char name[ALLOCATION_CHUNK_SIZE];
  const int value;
} attrib_t;

extern attrib_t attribs [NATTS];

#define A_BACKGROUND -1
#define A_FOREGROUND -2

#define DEFAULT_ATTS					\
  FIELD(STANDOUT)					\
  FIELD(UNDERLINE)					\
  FIELD(REVERSE)					\
  FIELD(BLINK)						\
  FIELD(DIM)						\
  FIELD(BOLD)						\
  FIELD(BACKGROUND)					\
  FIELD(FOREGROUND)


// Attributes ==============================================
#define NCOLORS 8

typedef struct color_t {
  const char name[ALLOCATION_CHUNK_SIZE];
  const int value;
} color_t;

extern color_t colors [NCOLORS];

#define DEFAULT_COLORS				\
  FIELD(BLACK)					\
  FIELD(RED)					\
  FIELD(GREEN)					\
  FIELD(YELLOW)					\
  FIELD(BLUE)					\
  FIELD(MAGENTA)				\
  FIELD(CYAN)					\
  FIELD(WHITE)


// Faces ===================================================

typedef struct face_t {
  int id;
  char name[ALLOCATION_CHUNK_SIZE];
  int value;
} face_t;

#define DEFAULT_FACES				\
  FIELD(FACE_NORMAL)				\
  FIELD(FACE_REGION)				\
  FIELD(FACE_STATUS)				\
  FIELD(FACE_REVERSE)				\
  FIELD(FACE_LINUM)				\
  FIELD(FACE_INDICATOR)

/* Zile faces codes */
enum default_faces {
#define FIELD(F) F,
  DEFAULT_FACES
#undef FIELD
  FACE_GUARD
};


extern face_t faces_list[FACE_GUARD];

// Functions ===============================================

void face_set_attribute (face_t *face, attrib_t *attrib, int value);

void term_init_attrs ();

int get_face_value(int id);

DECLARE_FIND(attrib_t);
DECLARE_FIND(color_t);
DECLARE_FIND(face_t);

#undef DECLARE_FIND

#endif

