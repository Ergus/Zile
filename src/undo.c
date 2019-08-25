/* Undo facility functions

   Copyright (c) 1997-2014 Free Software Foundation, Inc.

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

#include <config.h>

#include "main.h"
#include "extern.h"

#include "buffer.h"

/*
 * Undo action
 */
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

/*
 * Save a reverse delta for doing undo.
 */
static void
undo_save (void *type, size_t o, size_t osize, size_t size)
{
  if (get_buffer_noundo (cur_bp))
    return;

  Undo up = (Undo) XZALLOC (struct Undo);
  *up = (struct Undo) {
    .next = get_buffer_last_undop (cur_bp),
    .type = type,
  };

  up->o = o;

  if (type == undo_save_block)
    {
      up->size = size;
      up->text = get_buffer_region (cur_bp, region_new (o, o + osize));
      up->unchanged = !get_buffer_modified (cur_bp);
    }

  set_buffer_last_undop (cur_bp, up);
}

void
undo_start_sequence (void)
{
  if (cur_bp)
    undo_save (undo_start_sequence, get_buffer_pt (cur_bp), 0, 0);
}

void
undo_end_sequence (void)
{
  if (cur_bp)
    {
      Undo up = get_buffer_last_undop (cur_bp);
      if (up)
        {
          if (up->type == undo_start_sequence)
            set_buffer_last_undop (cur_bp, up->next);
          else
            undo_save (undo_end_sequence, 0, 0, 0);
        }

      /* Update list pointer */
      if (last_command () != F_undo)
        set_buffer_next_undop (cur_bp, get_buffer_last_undop (cur_bp));
    }
}

void
undo_save_block (size_t o, size_t osize, size_t size)
{
  undo_save (undo_save_block, o, osize, size);
}

/*
 * Revert an action.  Return the next undo entry.
 */
static Undo
revert_action (Undo up)
{
  if (up->type == undo_end_sequence)
    for (up = up->next; up->type != undo_start_sequence; up = revert_action (up))
      ;

  if (up->type != undo_end_sequence)
    goto_offset (up->o);
  if (up->type == undo_save_block)
    replace_estr (up->size, up->text);
  if (up->unchanged)
    set_buffer_modified (cur_bp, false);

  return up->next;
}

DEFUN ("undo", undo)
/*+
Undo some previous changes.
Repeat this command to undo more changes.
+*/
{
  if (get_buffer_noundo (cur_bp))
    {
      minibuf_error ("Undo disabled in this buffer");
      return leNIL;
    }

  if (warn_if_readonly_buffer ())
    return leNIL;

  if (get_buffer_next_undop (cur_bp) == NULL)
    {
      minibuf_error ("No further undo information");
      set_buffer_next_undop (cur_bp, get_buffer_last_undop (cur_bp));
      return leNIL;
    }

  set_buffer_next_undop (cur_bp, revert_action (get_buffer_next_undop (cur_bp)));
  minibuf_write ("Undo!");
}
END_DEFUN

/*
 * Set unchanged flags to false.
 */
void
undo_set_unchanged (Undo up)
{
  for (; up; up = up->next)
    up->unchanged = false;
}
