

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

#include <string.h>

#include "face.h"

const attrib_t atts [NPROPS] =
  {
#define FIELD(F) {"F", A_##F},
    DEFAULT_ATTS
#undef FIELD
  };

face_t faces_list [FACE_GUARD] = {
#define FIELD(F) {F, "F", A_NORMAL},
  DEFAULT_FACES
#undef FIELD
};

void
face_set_attributes (int id, const char prop[], uint value)
{
  assert (id < FACE_GUARD);

  face_t *face = &faces_list[id];

  for (size_t i = 0; i < NPROPS; ++i)
    if (strcmp (prop, atts[i].name) == 0) {
      const uint attv = atts[i].value;
      face->value = value ? (face->value | attv) : (face->value & ~attv);
      return;
    }

  if (strcmp (prop, "background") == 0 ||
      strcmp (prop, "foreground") == 0 ) {

    assert (value <= COLOR_WHITE);
    short f, b;
    pair_content (id, &f, &b);
    face->value & ~COLOR_PAIR(id);

    if (strcmp (prop, "background") == 0)
      init_pair (id, f, value);
    else if (strcmp (prop, "foreground") == 0)
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
