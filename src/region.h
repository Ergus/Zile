#ifndef REGION_H
#define REGION_H

/* Region fields

   Copyright (c) 2014 Free Software Foundation, Inc.

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

#include "main.h"

#define REGION_FIELDS				\
  FIELD(size_t, start)				\
  FIELD(size_t, end)

struct Region
{
#define FIELD(ty, name) ty name;
  REGION_FIELDS
#undef FIELD
};

#define FIELD(ty, field)                         \
  IGETTER(Region, region, ty, field)             \
  ISETTER(Region, region, ty, field)

REGION_FIELDS
#undef FIELD

_GL_ATTRIBUTE_PURE Region region_new (size_t o1, size_t o2);
bool delete_region (const Region r);
_GL_ATTRIBUTE_PURE size_t get_region_size (const Region r);
_GL_ATTRIBUTE_PURE bool region_contains (Region r, size_t o);

#endif
