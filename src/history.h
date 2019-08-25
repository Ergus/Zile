#ifndef HISTORY_H
#define HISTORY_H

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

#include "gl_linked_list.h"
#include "main.h"

struct History
{
  gl_list_t elements;		/* Elements (strings). */
  ptrdiff_t sel;		/* Selected element. */
};

History history_new (void);
void add_history_element (History hp, const char *string);
void prepare_history (History hp);
const char *previous_history_element (History hp);
const char *next_history_element (History hp);

#endif
