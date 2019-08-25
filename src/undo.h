#ifndef UNDO_H
#define UNDO_H

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

struct Undo
{
  Undo next;       /* Next undo delta in list. */
  void *type;      /* The type of undo delta. */
  size_t o;        /* Buffer offset of the undo delta. */
  bool unchanged;  /* Flag indicating that reverting this undo leaves
                      the buffer in an unchanged state. */
  estr text;       /* Old text. */
  size_t size;     /* Size of replacement text. */
};


extern bool undo_nosave;
void undo_start_sequence (void);
void undo_end_sequence (void);
void undo_save_block (size_t o, size_t osize, size_t size);
void undo_set_unchanged (Undo up);

#endif
