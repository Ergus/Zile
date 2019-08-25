#ifndef BIND_H
#define BIND_H

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

#include "main.h"

struct Binding
{
  size_t key; /* The key code (for every level except the root). */
  Function func; /* The function for this key (if a leaf node). */

  /* Branch vector, number of items, max number of items. */
  Binding *vec;
  size_t vecnum, vecmax;
};


_GL_ATTRIBUTE_PURE Function last_command (void);
void set_this_command (Function cmd);
size_t do_binding_completion (astr as);
gl_list_t get_key_sequence (void);
Function get_function_by_keys (gl_list_t keys);
le *call_command (Function f, int uniarg, bool uniflag, le *branch);
void get_and_run_command (void);
void init_default_bindings (void);

#endif
