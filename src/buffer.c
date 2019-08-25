/* Buffer-oriented functions

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
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "dirname.h"

#include "main.h"
#include "extern.h"
#include "memrmem.h"

#include "region.h"

#include "buffer.h"
#include "completion.h"
#include "marker.h"
#include "window.h"
// ================ Static ================================

static inline size_t
realo_to_o (Buffer bp, size_t o)
{
  if (o == SIZE_MAX)
    return o;
  else if (o < bp->pt + bp->gap)
    return MIN (o, bp->pt);
  else
    return o - bp->gap;
}

static inline size_t
o_to_realo (Buffer bp, size_t o)
{
  return o < bp->pt ? o : o + bp->gap;
}

// Move the given buffer to head.
static void
move_buffer_to_head (Buffer bp)
{
  Buffer prev = NULL;
  for (Buffer it = head_bp; it != bp; prev = it, it = it->next)
    ;
  if (prev)
    {
      prev->next = bp->next;
      bp->next = head_bp;
      head_bp = bp;
    }
}

// Go to the goal column.  Take care of expanding tabulations.
static void
goto_goalc (void)
{
  size_t i, col = 0, t = tab_width (cur_bp);

  for (i = get_buffer_line_o (cur_bp);
       i < get_buffer_line_o (cur_bp) + buffer_line_len (cur_bp, get_buffer_pt (cur_bp));
       i++)
    if (col == get_buffer_goalc (cur_bp))
      break;
    else if (get_buffer_char (cur_bp, i) == '\t')
      for (size_t w = t - col % t; w > 0 && ++col < get_buffer_goalc (cur_bp); w--)
        ;
    else
      ++col;

  set_buffer_pt (cur_bp, i);
}


// ========================================================

// Insert the character `c' at point in the current buffer.
bool
insert_char (int c)
{
  const char ch = (char) c;
  return insert_estr (estr_new (const_astr_new_nstr (&ch, 1), coding_eol_lf));
}

bool
delete_char (void)
{
  deactivate_mark ();

  if (eobp ())
    {
      minibuf_error ("End of buffer");
      return false;
    }

  if (warn_if_readonly_buffer ())
    return false;

  if (eolp ())
    {
      replace_estr (strlen (get_buffer_eol (cur_bp)), estr_empty);
      thisflag |= FLAG_NEED_RESYNC;
    }
  else
    replace_estr (1, estr_empty);

  set_buffer_modified (cur_bp, true);

  return true;
}

// Replace `del' chars after point with `es'.
bool
replace_estr (size_t del, const_estr es)
{
  if (warn_if_readonly_buffer ())
    return false;

  size_t newlen = estr_len (es, get_buffer_eol (cur_bp));
  undo_save_block (cur_bp->pt, del, newlen);

  /* Adjust gap. */
  size_t oldgap = cur_bp->gap;
  size_t added_gap = oldgap + del < newlen ? MIN_GAP : 0;
  if (added_gap > 0)
    { /* If gap would vanish, open it to MIN_GAP. */
      astr_insert (estr_get_as (cur_bp->text), cur_bp->pt, (newlen + MIN_GAP) - (oldgap + del));
      cur_bp->gap = MIN_GAP;
    }
  else if (oldgap + del > MAX_GAP + newlen)
    { /* If gap would be larger than MAX_GAP, restrict it to MAX_GAP. */
      astr_remove (estr_get_as (cur_bp->text), cur_bp->pt + newlen + MAX_GAP, (oldgap + del) - (MAX_GAP + newlen));
      cur_bp->gap = MAX_GAP;
    }
  else
    cur_bp->gap = oldgap + del - newlen;

  /* Zero any new bit of gap not produced by astr_insert. */
  if (MAX (oldgap, newlen) + added_gap < cur_bp->gap + newlen)
    astr_set (estr_get_as (cur_bp->text), cur_bp->pt + MAX (oldgap, newlen) + added_gap, '\0', newlen + cur_bp->gap - MAX (oldgap, newlen) - added_gap);

  /* Insert `newlen' chars. */
  estr_replace_estr (cur_bp->text, cur_bp->pt, es);
  cur_bp->pt += newlen;

  /* Adjust markers. */
  for (Marker m = get_buffer_markers (cur_bp); m != NULL; m = get_marker_next (m))
    if (get_marker_o (m) > cur_bp->pt - newlen)
      set_marker_o (m, MAX (cur_bp->pt - newlen, get_marker_o (m) + newlen - del));

  set_buffer_modified (cur_bp, true);
  if (estr_next_line (es, 0) != SIZE_MAX)
    thisflag |= FLAG_NEED_RESYNC;
  return true;
}

bool
insert_estr (const_estr es)
{
  return replace_estr (0, es);
}

// ========================================================

const_astr
get_buffer_pre_point (Buffer bp)
{
  return const_astr_new_nstr (astr_cstr (estr_get_as (bp->text)), bp->pt);
}

const_astr
get_buffer_post_point (Buffer bp)
{
  size_t post_gap = bp->pt + bp->gap;
  const_astr as = estr_get_as (bp->text);
  return const_astr_new_nstr (astr_cstr (as) + post_gap, astr_len (as) - post_gap);
}

void
set_buffer_pt (Buffer bp, size_t o)
{
  if (o < bp->pt)
    {
      astr_move (estr_get_as (bp->text), o + bp->gap, o, bp->pt - o);
      astr_set (estr_get_as (bp->text), o, '\0', MIN (bp->pt - o, bp->gap));
    }
  else if (o > bp->pt)
    {
      astr_move (estr_get_as (bp->text), bp->pt, bp->pt + bp->gap, o - bp->pt);
      astr_set (estr_get_as (bp->text), o + bp->gap - MIN (o - bp->pt, bp->gap), '\0', MIN (o - bp->pt, bp->gap));
    }
  bp->pt = o;
}

size_t
get_buffer_pt (Buffer bp)
{
  return bp->pt;
}

//  =======================================================

size_t
buffer_prev_line (Buffer bp, size_t o)
{
  return realo_to_o (bp, estr_prev_line (bp->text, o_to_realo (bp, o)));
}

size_t
buffer_next_line (Buffer bp, size_t o)
{
  return realo_to_o (bp, estr_next_line (bp->text, o_to_realo (bp, o)));
}

size_t
buffer_start_of_line (Buffer bp, size_t o)
{
  return realo_to_o (bp, estr_start_of_line (bp->text, o_to_realo (bp, o)));
}

size_t
buffer_end_of_line (Buffer bp, size_t o)
{
  return realo_to_o (bp, estr_end_of_line (bp->text, o_to_realo (bp, o)));
}

size_t
buffer_line_len (Buffer bp, size_t o)
{
  return realo_to_o (bp, estr_end_of_line (bp->text, o_to_realo (bp, o))) -
    realo_to_o (bp, estr_start_of_line (bp->text, o_to_realo (bp, o)));
}

// ========================================================

size_t
get_buffer_size (Buffer bp)
{
  return realo_to_o (bp, astr_len (estr_get_as (bp->text)));
}

char
get_buffer_char (Buffer bp, size_t o)
{
  return astr_get (estr_get_as (bp->text), o_to_realo (bp, o));
}

size_t
get_buffer_line_o (Buffer bp)
{
  return realo_to_o (bp, estr_start_of_line (bp->text, o_to_realo (bp, bp->pt)));
}

// Buffer methods that don't know about the gap.
const char *
get_buffer_eol (Buffer bp)
{
  return estr_get_eol (bp->text);
}

// Get the buffer region as an estr.
estr
get_buffer_region (Buffer bp, Region r)
{
  astr as = astr_new ();
  if (get_region_start (r) < bp->pt)
    astr_cat (as, astr_substr (get_buffer_pre_point (bp), get_region_start (r), MIN (get_region_end (r), bp->pt) - get_region_start (r)));
  if (get_region_end (r) > bp->pt)
    {
      size_t from = MAX (get_region_start (r), bp->pt);
      astr_cat (as, astr_substr (get_buffer_post_point (bp), from - bp->pt, get_region_end (r) - from));
    }
  return estr_new (as, get_buffer_eol (bp));
}


// ========================================================

/*
 * Allocate a new buffer structure, set the default local
 * variable values, and insert it into the buffer list.
 */
Buffer
buffer_new (void)
{
  Buffer bp = (Buffer) XZALLOC (struct Buffer);
  bp->text = estr_new_astr (astr_new ());
  bp->dir = agetcwd ();

  /* Insert into buffer list. */
  bp->next = head_bp;
  head_bp = bp;

  init_buffer (bp);

  return bp;
}

// Initialise a buffer
void
init_buffer (Buffer bp)
{
  if (get_variable_bool ("auto-fill-mode"))
    set_buffer_autofill (bp, true);
}

// Unchain the buffer's markers.
void
destroy_buffer (Buffer bp)
{
  while (bp->markers)
    unchain_marker (bp->markers);
}

void
insert_buffer (Buffer bp)
{
  /* Copy text to avoid problems when bp == cur_bp. */
  insert_estr (estr_new (get_buffer_pre_point (bp), get_buffer_eol (bp)));
  insert_estr (estr_new (get_buffer_post_point (bp), get_buffer_eol (bp)));
}

// ========================================================

// Get filename, or buffer name if NULL.
const char *
get_buffer_filename_or_name (Buffer bp)
{
  const char *fname = get_buffer_filename (bp);
  return fname ? fname : get_buffer_name (bp);
}

// Set a new filename, and from it a name, for the buffer.
void
set_buffer_names (Buffer bp, const char *filename)
{
  if (filename[0] != '/')
    filename = astr_cstr (astr_fmt ("%s/%s", astr_cstr (agetcwd ()), filename));
  set_buffer_filename (bp, filename);

  char *s = base_name (filename);
  char *name = xstrdup (s);
  /* Note: there can't be more than SIZE_MAX buffers. */
  for (size_t i = 2; find_buffer (name) != NULL; i++)
    name = xasprintf ("%s<%zu>", s, i);
  set_buffer_name (bp, name);
}

// Search for a buffer named `name'.
Buffer
find_buffer (const char *name)
{
  for (Buffer bp = head_bp; bp != NULL; bp = bp->next)
    {
      const char *bname = get_buffer_name (bp);
      if (bname && STREQ (bname, name))
        return bp;
    }

  return NULL;
}

// Switch to the specified buffer.
void
switch_to_buffer (Buffer bp)
{
  assert (get_window_bp (cur_wp) == cur_bp);

  // The buffer is the current buffer; return safely.
  if (cur_bp == bp)
    return;

  // Set current buffer.
  cur_bp = bp;
  set_window_bp (cur_wp, cur_bp);

  // Move the buffer to head.
  move_buffer_to_head (bp);

  // Change to buffer's default directory.
  if (chdir (astr_cstr (bp->dir))) {
    // Avoid compiler warning for ignoring return value.
  }

  thisflag |= FLAG_NEED_RESYNC;
}

bool
warn_if_no_mark (void)
{
  if (!cur_bp->mark)
    {
      minibuf_error ("The mark is not set now");
      return true;
    }
  else if (!get_buffer_mark_active (cur_bp))
    {
      minibuf_error ("The mark is not active now");
      return true;
    }
  return false;
}

/*
 * Print an error message into the echo area and return true
 * if the current buffer is readonly; otherwise return false.
 */
bool
warn_if_readonly_buffer (void)
{
  if (get_buffer_readonly (cur_bp))
    {
      minibuf_error ("Buffer is readonly: %s", get_buffer_name (cur_bp));
      return true;
    }

  return false;
}

// ========================================================

/*
 * Set the specified buffer temporary flag and move the buffer
 * to the end of the buffer list.
 */

Region
calculate_the_region (void)
{
  return region_new (get_buffer_pt (cur_bp), get_marker_o (get_buffer_mark (cur_bp)));
}

void
set_temporary_buffer (Buffer bp)
{
  Buffer bp0;

  set_buffer_temporary (bp, true);

  if (bp == head_bp)
    {
      if (head_bp->next == NULL)
        return;
      head_bp = head_bp->next;
    }
  else if (bp->next == NULL)
    return;

  for (bp0 = head_bp; bp0 != NULL; bp0 = bp0->next)
    if (bp0->next == bp)
      {
        bp0->next = bp0->next->next;
        break;
      }

  for (bp0 = head_bp; bp0->next != NULL; bp0 = bp0->next)
    ;

  bp0->next = bp;
  bp->next = NULL;
}

void
activate_mark (void)
{
  set_buffer_mark_active (cur_bp, true);
}

void
deactivate_mark (void)
{
  set_buffer_mark_active (cur_bp, false);
}

/*
 * Return a safe tab width for the given buffer.
 */
size_t
tab_width (Buffer bp)
{
  long res = 0;
  lisp_to_number (get_variable_bp (bp, "tab-width"), &res);
  if (res < 1)
    res = 8;
  return res;
}

Buffer
create_auto_buffer (const char *name)
{
  Buffer bp = buffer_new ();
  set_buffer_name (bp, name);
  set_buffer_needname (bp, true);
  set_buffer_temporary (bp, true);
  set_buffer_nosave (bp, true);
  return bp;
}

Buffer
create_scratch_buffer (void)
{
  return create_auto_buffer ("*scratch*");
}

/*
 * Remove the specified buffer from the buffer list and deallocate
 * its space.  Recreate the scratch buffer when required.
 */
void
kill_buffer (Buffer kill_bp)
{
  Buffer next_bp;

  if (get_buffer_next (kill_bp) != NULL)
    next_bp = get_buffer_next (kill_bp);
  else
    next_bp = (head_bp == kill_bp) ? NULL : head_bp;

  /* Search for windows displaying the buffer to kill. */
  for (Window wp = head_wp; wp != NULL; wp = get_window_next (wp))
    if (get_window_bp (wp) == kill_bp)
      {
        set_window_bp (wp, next_bp);
        set_window_topdelta (wp, 0);
        set_window_saved_pt (wp, NULL);
      }

  /* Remove the buffer from the buffer list. */
  if (cur_bp == kill_bp)
    cur_bp = next_bp;
  if (head_bp == kill_bp)
    head_bp = get_buffer_next (head_bp);
  for (Buffer bp = head_bp; bp != NULL && get_buffer_next (bp) != NULL; bp = get_buffer_next (bp))
    if (get_buffer_next (bp) == kill_bp)
      {
        set_buffer_next (bp, get_buffer_next (get_buffer_next (bp)));
        break;
      }

  destroy_buffer (kill_bp);

  /* If no buffers left, recreate scratch buffer and point windows at
     it. */
  if (next_bp == NULL)
    {
      cur_bp = head_bp = next_bp = create_scratch_buffer ();
      for (Window wp = head_wp; wp != NULL; wp = get_window_next (wp))
        set_window_bp (wp, head_bp);
    }

  /* Resync windows that need it. */
  for (Window wp = head_wp; wp != NULL; wp = get_window_next (wp))
    if (get_window_bp (wp) == next_bp)
      window_resync (wp);
}

DEFUN_ARGS ("kill-buffer", kill_buffer,
            STR_ARG (buf))
/*+
Kill buffer BUFFER.
With a nil argument, kill the current buffer.
+*/
{
  Buffer bp;

  STR_INIT (buf)
  else
    {
      Completion cp = make_buffer_completion ();
      buf = minibuf_read_completion ("Kill buffer (default %s): ",
                                     "", cp, NULL, get_buffer_name (cur_bp));
      if (buf == NULL)
        ok = FUNCALL (keyboard_quit);
    }

  if (buf && astr_len (buf) > 0)
    {
      bp = find_buffer (astr_cstr (buf));
      if (bp == NULL)
        {
          minibuf_error ("Buffer `%s' not found", astr_cstr (buf));
          ok = leNIL;
        }
    }
  else
    bp = cur_bp;

  if (ok == leT)
    {
      if (!check_modified_buffer (bp))
        ok = leNIL;
      else
        kill_buffer (bp);
    }
}
END_DEFUN

Completion
make_buffer_completion (void)
{
  Completion cp = completion_new (false);
  for (Buffer bp = head_bp; bp != NULL; bp = get_buffer_next (bp))
    gl_sortedlist_add (get_completion_completions (cp), completion_strcmp,
                       xstrdup (get_buffer_name (bp)));

  return cp;
}

/*
 * Check if the buffer has been modified.  If so, asks the user if
 * he/she wants to save the changes.  If the response is positive, return
 * true, else false.
 */
bool
check_modified_buffer (Buffer bp)
{
  if (get_buffer_modified (bp) && !get_buffer_nosave (bp))
    for (;;)
      {
        int ans = minibuf_read_yesno
          ("Buffer %s modified; kill anyway? (yes or no) ", get_buffer_name (bp));
        if (ans == -1)
          {
            FUNCALL (keyboard_quit);
            return false;
          }
        else if (!ans)
          return false;
        break;
      }

  return true;
}


/* Basic movement routines */

bool
move_char (ptrdiff_t offset)
{
  int dir = offset >= 0 ? 1 : -1;
  for (size_t i = 0; i < (size_t) (abs (offset)); i++)
    {
      if (dir > 0 ? !eolp () : !bolp ())
        set_buffer_pt (cur_bp, cur_bp->pt + dir);
      else if (dir > 0 ? !eobp () : !bobp ())
        {
          thisflag |= FLAG_NEED_RESYNC;
          set_buffer_pt (cur_bp, cur_bp->pt + dir * strlen (get_buffer_eol (cur_bp)));
          if (dir > 0)
            FUNCALL (beginning_of_line);
          else
            FUNCALL (end_of_line);
        }
      else
        return false;
    }

  return true;
}

bool
move_line (ptrdiff_t n)
{
  size_t (*func) (Buffer bp, size_t o) = buffer_next_line;
  if (n < 0)
    {
      n = -n;
      func = buffer_prev_line;
    }

  if (last_command () != F_next_line && last_command () != F_previous_line)
    set_buffer_goalc (cur_bp, get_goalc ());

  for (; n > 0; n--)
    {
      size_t o = func (cur_bp, cur_bp->pt);
      if (o == SIZE_MAX)
        break;
      set_buffer_pt (cur_bp, o);
    }

  goto_goalc ();
  thisflag |= FLAG_NEED_RESYNC;

  return n == 0;
}

size_t
offset_to_line (Buffer bp, size_t offset)
{
  size_t n = 0;
  for (size_t o = 0; buffer_end_of_line (bp, o) < offset; o = buffer_next_line (bp, o))
    n++;
  return n;
}

void
goto_offset (size_t o)
{
  size_t old_lineo = get_buffer_line_o (cur_bp);
  set_buffer_pt (cur_bp, o);
  if (get_buffer_line_o (cur_bp) != old_lineo)
    {
      set_buffer_goalc (cur_bp, get_goalc ());
      thisflag |= FLAG_NEED_RESYNC;
    }
}
