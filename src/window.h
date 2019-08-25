#ifndef WINDOW_H
#define WINDOW_H

/* Window fields

   Copyright (c) 2009-2014 Free Software Foundation, Inc.

   This file is part of GNU Zile.

   GNU Zile is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   GNU Zile is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with GNU Zile; see the file COPYING.  If not, write to the
   Free Software Foundation, Fifth Floor, 51 Franklin Street, Boston,
   MA 02111-1301, USA.  */

#define WINDOW_FIELDS							\
  FIELD(Window, next)		/* The next window in window list. */	\
  FIELD(Buffer, bp)		/* The buffer displayed in window. */	\
  FIELD(size_t, topdelta)		/* The top line delta from point. */ \
  FIELD(size_t, start_column)	/* The start column of the window (>0 if scrolled \
                                   sideways). */			\
  FIELD(Marker, saved_pt)		/* The point line pointer, line number and offset \
					   (used to hold the point in non-current windows). */ \
  FIELD(size_t, fwidth)		/* The formal width and height of the window. */ \
  FIELD(size_t, fheight)						\
  FIELD(size_t, ewidth)		/* The effective width and height of the window. */ \
  FIELD(size_t, eheight)						\
  FIELD(bool, all_displayed)	/* The bottom of the buffer is visible */ \
  FIELD(size_t, first_column)     /* First effective column. */

struct Window
{
#define FIELD(ty, name) ty name;
WINDOW_FIELDS
#undef FIELD
  int lastpointn;		/* The last point line number. */
};

#define FIELD(ty, field)                        \
  IGETTER (Window, window, ty, field)            \
  ISETTER (Window, window, ty, field)

WINDOW_FIELDS
#undef FIELD

void create_scratch_window (void);
Window find_window (const char *name);
Window popup_window (void);
void set_current_window (Window wp);
void delete_window (Window del_wp);
size_t window_o (Window wp);
bool window_top_visible (Window wp);
_GL_ATTRIBUTE_PURE bool window_bottom_visible (Window wp);
void window_resync (Window wp);
bool get_window_in_position (int x, int y, int *rx, int *ry);

#endif
