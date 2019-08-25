#ifndef MARKER_H
#define MARKER_H

/* Marker fields

   Copyright (c) 2009-2011 Free Software Foundation, Inc.

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

#define MARKER_FIELDS							\
  FIELD(Marker, next)		/* Used to chain all markers in the buffer. */ \
  FIELD(size_t, o)		/* Marker offset within buffer. */	\
  FIELD(Buffer, bp)		/* Buffer that marker points into. */	\


typedef struct Marker
{
#define FIELD(ty, name) ty name;
  MARKER_FIELDS
#undef FIELD
} *Marker;

#define FIELD(ty, field)                         \
  IGETTER (Marker, marker, ty, field)             \
  ISETTER (Marker, marker, ty, field)

MARKER_FIELDS
#undef FIELD

Marker marker_new (void);
void unchain_marker (const Marker marker);
void move_marker (Marker marker, Buffer bp, size_t o);
Marker copy_marker (const Marker marker);
Marker point_marker (void);
void push_mark (void);
void pop_mark (void);
void set_mark (void);

#endif
