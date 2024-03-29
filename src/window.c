/* Window handling functions

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

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "main.h"
#include "extern.h"

#include "window.h"
#include "buffer.h"
#include "marker.h"
#include "minibuf.h"
#include "term_curses.h"


#define FIELD(ty, field)			 \
  DGETTER (Window, window, ty, field)            \
  DSETTER (Window, window, ty, field)

WINDOW_FIELDS
#undef FIELD


/*
 * Set the current window and his buffer as the current buffer.
 */
void
set_current_window (Window wp)
{
  /* Save buffer's point in a new marker.  */
  if (global.cur_wp->saved_pt)
    unchain_marker (global.cur_wp->saved_pt);

  global.cur_wp->saved_pt = point_marker ();

  global.cur_wp = wp;
  global.cur_bp = wp->bp;

  /* Update the buffer point with the window's saved point
     marker.  */
  if (global.cur_wp->saved_pt)
    {
      goto_offset (get_marker_o (global.cur_wp->saved_pt));
      unchain_marker (global.cur_wp->saved_pt);
      global.cur_wp->saved_pt = NULL;
    }
}

DEFUN ("split-window", split_window)
/*+
Split current window into two windows, one above the other.
Both windows display the same buffer now current.
+*/
{
  /* Windows smaller than 4 lines cannot be split. */
  if (global.cur_wp->fheight < 4)
    {
      minibuf_error ("Window height %zu too small (after splitting)",
                     global.cur_wp->fheight);
      return leNIL;
    }

  Window newwp = (Window) XZALLOC (struct Window);
  *newwp = *global.cur_wp;
  newwp->fheight = global.cur_wp->fheight / 2 + global.cur_wp->fheight % 2;
  newwp->eheight = newwp->fheight - 1;
  newwp->saved_pt = point_marker ();

  global.cur_wp->next = newwp;
  global.cur_wp->fheight = global.cur_wp->fheight / 2;
  global.cur_wp->eheight = global.cur_wp->fheight - 1;
  if (global.cur_wp->topdelta >= global.cur_wp->eheight)
    recenter (global.cur_wp);
}
END_DEFUN

void
delete_window (Window del_wp)
{
  Window wp;

  if (del_wp == global.head_wp)
    wp = global.head_wp = global.head_wp->next;
  else
    for (wp = global.head_wp; wp != NULL; wp = wp->next)
      if (wp->next == del_wp)
        {
          wp->next = wp->next->next;
          break;
        }

  if (wp != NULL)
    {
      wp->fheight += del_wp->fheight;
      wp->eheight += del_wp->eheight + 1;
      set_current_window (wp);
    }

  if (del_wp->saved_pt)
    unchain_marker (del_wp->saved_pt);
}

DEFUN ("delete-window", delete_window)
/*+
Remove the current window from the screen.
+*/
{
  if (global.cur_wp == global.head_wp && global.cur_wp->next == NULL)
    {
      minibuf_error ("Attempt to delete sole ordinary window");
      return leNIL;
    }

  delete_window (global.cur_wp);
}
END_DEFUN

DEFUN ("enlarge-window", enlarge_window)
/*+
Make current window one line bigger.
+*/
{
  if (global.cur_wp == global.head_wp && (global.cur_wp->next == NULL ||
                            global.cur_wp->next->fheight < 3))
    return leNIL;

  Window wp = global.cur_wp->next;
  if (wp == NULL || wp->fheight < 3)
    for (wp = global.head_wp; wp != NULL; wp = wp->next)
      if (wp->next == global.cur_wp)
        {
          if (wp->fheight < 3)
            return leNIL;
          break;
        }

  --wp->fheight;
  --wp->eheight;
  if (wp->topdelta >= wp->eheight)
    recenter (wp);
  ++global.cur_wp->fheight;
  ++global.cur_wp->eheight;
}
END_DEFUN

DEFUN ("shrink-window", shrink_window)
/*+
Make current window one line smaller.
+*/
{

  if ((global.cur_wp == global.head_wp && global.cur_wp->next == NULL) || global.cur_wp->fheight < 3)
    return leNIL;

  Window wp = global.cur_wp->next;
  if (wp == NULL)
    for (wp = global.head_wp; wp != NULL; wp = wp->next)
      if (wp->next == global.cur_wp)
        break;

  ++(wp->fheight);
  ++(wp->eheight);
  --(global.cur_wp->fheight);
  --(global.cur_wp->eheight);
  if (global.cur_wp->topdelta >= global.cur_wp->eheight)
    recenter (global.cur_wp);
}
END_DEFUN

Window
popup_window (void)
{
  if (global.head_wp && global.head_wp->next == NULL)
    {
      /* There is only one window on the screen, so split it. */
      FUNCALL (split_window);
      return global.cur_wp->next;
    }

  /* Use the window after the current one, or first window if none. */
  return global.cur_wp->next ? global.cur_wp->next : global.head_wp;
}

DEFUN ("delete-other-windows", delete_other_windows)
/*+
Make the selected window fill the screen.
+*/
{
  for (Window wp = global.head_wp, nextwp; wp != NULL; wp = nextwp)
    {
      nextwp = wp->next;
      if (wp != global.cur_wp)
        delete_window (wp);
    }
}
END_DEFUN

DEFUN ("other-window", other_window)
/*+
Select the first different window on the screen.
All windows are arranged in a cyclic order.
This command selects the window one step away in that order.
+*/
{
  set_current_window ((global.cur_wp->next != NULL) ? global.cur_wp->next : global.head_wp);
}
END_DEFUN


bool
get_window_in_position (int x, int y, int *rx, int *ry)
{
  //minibuf_write ("%d %d %d", mouse_event.x, mouse_event.y, mouse_event.z );

  int first_line = 0;
  int last_line = 0;
  int cont = 0;
  for (Window wp = global.head_wp; wp != NULL; wp = get_window_next (wp)) {
    last_line += get_window_fheight(wp);

    if (last_line >= y) {
      minibuf_write ("Selected window %d",  cont);
      set_current_window (wp);
      *rx = x - get_window_first_column(wp);
      *ry = y - first_line;
      return true;
    }

    first_line = last_line;
    ++cont;
  }

  return false;
}

/*
 * This function creates the scratch buffer and window when there are
 * no other windows (and possibly no other buffers).
 */
void
create_scratch_window (void)
{
  Buffer bp = create_scratch_buffer ();
  Window wp = (Window) XZALLOC (struct Window);
  global.cur_wp = global.head_wp = wp;
  wp->fwidth = wp->ewidth = term_width ();
  /* Save space for minibuffer. */
  wp->fheight = term_height () - 1;
  /* Save space for status line. */
  wp->eheight = wp->fheight - 1;
  wp->bp = global.cur_bp = bp;
}

Window
find_window (const char *name)
{
  for (Window wp = global.head_wp; wp != NULL; wp = wp->next)
    if (STREQ (get_buffer_name (wp->bp), name))
      return wp;

  return NULL;
}

size_t
window_o (Window wp)
{
  /* The current window uses the current buffer point; all other
     windows have a saved point, except that if a window has just been
     killed, it needs to use its new buffer's current point. */
  assert (wp != NULL);
  if (wp == global.cur_wp)
    {
      assert (wp->bp == global.cur_bp);
      assert (wp->saved_pt == NULL);
      assert (global.cur_bp);
      return get_buffer_pt (global.cur_bp);
    }
  else
    {
      if (wp->saved_pt != NULL)
        return get_marker_o (wp->saved_pt);
      else
        return get_buffer_pt (wp->bp);
    }
}

bool
window_top_visible (Window wp)
{
  return offset_to_line (get_window_bp (wp), window_o (wp)) == get_window_topdelta (wp);
}

bool
window_bottom_visible (Window wp)
{
  return get_window_all_displayed (wp);
}

void
window_resync (Window wp)
{
  size_t n = offset_to_line (wp->bp, get_buffer_pt (wp->bp));
  ptrdiff_t delta = n - wp->lastpointn;

  if (delta)
    {
      if ((delta > 0 && wp->topdelta + delta < wp->eheight) ||
          (delta < 0 && wp->topdelta >= (size_t) (-delta)))
        wp->topdelta += delta;
      else if (n > wp->eheight / 2)
        wp->topdelta = wp->eheight / 2;
      else
        wp->topdelta = n;
    }
  wp->lastpointn = n;
}
