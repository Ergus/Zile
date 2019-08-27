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

#include <stdlib.h>
#if defined HAVE_NCURSESW_CURSES_H
#  include <ncursesw/curses.h>
#elif defined HAVE_NCURSESW_H
#  include <ncursesw.h>
#elif defined HAVE_NCURSES_CURSES_H
#  include <ncurses/curses.h>
#elif defined HAVE_NCURSES_H
#  include <ncurses.h>
#elif defined HAVE_CURSES_H
#  include <curses.h>
#else
#  error "SysV or X/Open-compatible Curses header file required"
#endif


#include <astr.h>

#define NPROPS 6

// Attributes ==============================================

typedef struct attrib_t {
  const char name[ALLOCATION_CHUNK_SIZE];
  const int value;
} attrib_t;

extern const attrib_t atts [NPROPS];

#define DEFAULT_ATTS					\
  FIELD(STANDOUT)					\
  FIELD(UNDERLINE)					\
  FIELD(REVERSE)					\
  FIELD(BLINK)						\
  FIELD(DIM)						\
  FIELD(BOLD)

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

void face_set_attribute (const char prop[], int value);

void term_init_attrs ();

inline int
get_face_value(int id)
{
  assert (id < FACE_GUARD);
  return faces_list[id].value;
}

#endif
