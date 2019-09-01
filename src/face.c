

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

#include "face.h"

#if defined HAVE_NCURSESW_CURSES_H
#  include <ncursesw/curses.h>
#elif defined HAVE_NCURSESW_H
#  include <ncursesw.h>
#elif defined HAVE_NCURSES_CURSES_H
#  include <ncurses/curses.h>
#elif defined CURSES_HAVE_NCURSES_H
#  include <ncurses.h>
#elif defined CURSES_HAVE_CURSES_H
#  include <curses.h>
#else
#  error "SysV or X/Open-compatible Curses header file required"
#endif
#include <term.h>

#include <string.h>

#include "completion.h"
#include "minibuf.h"
#include "astr.h"

#include "minibuf.h"
#include "extern.h"

attrib_t attribs [NATTS] = {
#define FIELD(F) {#F, A_##F},
    DEFAULT_ATTS
#undef FIELD
  };

color_t colors [NCOLORS] = {
#define FIELD(F) {#F, COLOR_##F},
  DEFAULT_COLORS
#undef FIELD
};

face_t faces_list [FACE_GUARD] = {
#define FIELD(F) {F, #F, FACE_NORMAL},
  DEFAULT_FACES
#undef FIELD
};

CREATE_FIND(attrib_t);
CREATE_FIND(color_t);
CREATE_FIND(face_t);

void
face_set_attribute (face_t *face, attrib_t *attrib, int value)
{
  const int attrib_value = attrib->value;

  if (attrib_value >= 0)
    {
      if (value)
	face->value |= attrib_value;
      else
	face->value &= ~attrib_value;
      return;
    }
  else if (attrib_value == -1 || attrib_value == -2)
    {
      const int id = face->id;
      assert (value <= COLOR_WHITE);
      short f, b;
      pair_content (id, &f, &b);
      face->value & ~COLOR_PAIR(id);

      if (attrib_value == -1)
	init_pair (id, f, value);
      else if (attrib_value == -2)
	init_pair (id, value, b);

      face->value |= COLOR_PAIR(id);
      return;
    }

  return;
}

void
term_init_attrs ()
{
  const bool colors = has_colors();
  if (colors)
    {
      start_color ();

      init_pair(FACE_NORMAL, COLOR_WHITE, COLOR_BLACK);
      init_pair(FACE_REGION, COLOR_WHITE, COLOR_BLUE);
      init_pair(FACE_STATUS, COLOR_WHITE, COLOR_BLUE);
      init_pair(FACE_REVERSE, COLOR_BLACK, COLOR_WHITE);
      init_pair(FACE_LINUM, COLOR_CYAN, COLOR_BLACK);
      init_pair(FACE_INDICATOR, COLOR_CYAN, COLOR_BLACK);
    }

  faces_list[FACE_NORMAL].value = (colors ? COLOR_PAIR(FACE_NORMAL) : FACE_NORMAL);
  faces_list[FACE_REGION].value = (colors ? COLOR_PAIR(FACE_REGION) : A_STANDOUT);
  faces_list[FACE_STATUS].value = (colors ? COLOR_PAIR(FACE_STATUS) : A_REVERSE);
  faces_list[FACE_REVERSE].value = (faces_list[FACE_NORMAL].value | A_REVERSE);
  faces_list[FACE_LINUM].value = (colors ? COLOR_PAIR(FACE_LINUM) | A_DIM : A_DIM);
  faces_list[FACE_INDICATOR].value = (colors ? COLOR_PAIR(FACE_INDICATOR) | A_DIM : A_DIM);
}

int
get_face_value(int id)
{
  assert (id < FACE_GUARD);
  return faces_list[id].value;
}

DEFUN ("set-face-attribute", set_face_attribute)
/*+
Set a face attribute.
+*/
{
  // Read face
  const_astr face_name = minibuf_read_face_t_name (faces_list, FACE_GUARD, "Face name: ");
  if (face_name == NULL)
    return leNIL;
  face_t *face = find_face_t_by_name(face_name, faces_list, FACE_GUARD);
  if (!face)
    return leNIL;

  // Read attributes
  const_astr attrib_name = minibuf_read_attrib_t_name (attribs, NATTS, "attribute name: ");
  if (attrib_name == NULL)
    ok = FUNCALL (keyboard_quit);
  attrib_t *attrib = find_attrib_t_by_name(attrib_name, attribs, NATTS);
  if (!attrib)
    return leNIL;

  // Value
  int val = -1;
    {
      if (attrib->value < 0)
	{
	  const_astr color_name = minibuf_read_color_t_name (colors, NCOLORS, "Color name: ");
	  color_t *color = find_color_t_by_name(color_name, colors, NCOLORS);
	  val = color->value;
	}
      else
	val = minibuf_read_number ("Insert value number: ");
    }
  if (val < 0)
    ok = FUNCALL (keyboard_quit);

  face_set_attribute (face, attrib, val);
}
END_DEFUN
