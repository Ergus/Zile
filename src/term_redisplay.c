/* Redisplay engine

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
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#include "main.h"
#include "extern.h"

static int
make_char_printable (char *out, char c, int x, int cur_tab_width)
{
  if (c == '\t')
    return snprintf (out, STR_SIZE, "%*s", cur_tab_width - x % cur_tab_width, "") ;
  if (c >= 0 && c <= '\33')
    return snprintf (out, STR_SIZE, "^%c", '@' + c) ;

  return snprintf (out, STR_SIZE, "\\%o", c & 0xff) - 1; // The -1 is because it converts 2 elements
}

static void
draw_line (size_t line, Window wp,
           size_t o, Region r, int highlight, size_t cur_tab_width,
           size_t fill_column_indicator_column)
{
  /* Draw body of line. */
  const size_t first_column_wp = get_window_first_column (wp); // Start column to draw text
  const size_t start_column_wp = get_window_start_column (wp); // First text column to display
  // Effective portion of the window to write text
  // TODO: if get_window_ewidth (wp) <= first_column_wp this will produce an error
  const size_t window_ewidth_wp = get_window_ewidth (wp) - first_column_wp;

  size_t x = 0,
    i = start_column_wp,
    line_len = buffer_line_len (get_window_bp (wp), o);

  term_move (line, first_column_wp);
  if (i > 0) // There is horizontal scroll
    {
      term_addch ('$');
      ++x;
      ++i;
    }

  term_attrset (highlight && region_contains (r, o + i) ? FONT_REVERSE : FONT_NORMAL);

  while (i < line_len && x < window_ewidth_wp)
    {
      term_attrset (highlight && region_contains (r, o + i) ? FONT_REVERSE : FONT_NORMAL);

      char c = get_buffer_char (get_window_bp (wp), o + i);
      if (isprint (c))
        {
          term_addch (c);
          ++x;
        }
      else
        {
	  char buffer[STR_SIZE];
          const int ret = make_char_printable (buffer, c, x, cur_tab_width);
	  assert(ret > 0);
          term_addstr (buffer);
          x += ret;
        }
      ++i;
    }

  if (first_column_wp + x >= term_width ()) // We filled the whole visible line
    {
      term_move (line, term_width () - 1);
      term_attrset (FONT_NORMAL);
      term_addstr ("$");
    }
  else // There is space after the displayed text
    {
      const long indicator_column_x =
	fill_column_indicator_column - start_column_wp;

      if (0 < fill_column_indicator_column &&     // Enabled indicator
          indicator_column_x >= 0 &&              // Indicator horizontal visible
          indicator_column_x <= window_ewidth_wp) // window not too stretch
	{
	  while (x < indicator_column_x)          // Fill before indicator
	    {
	      term_addch (' ');
	      ++x;
	    }
          if (x == indicator_column_x)            // Set indicator
	    {
	      term_addch ('|');
	      ++x;
	    }
	}
      while (x <= window_ewidth_wp)               // Erase the rest of the line
	{
	  term_addch (' ');
	  ++x;
	}
    }
  term_attrset (FONT_NORMAL);                     // Restore default face
}

static int
calculate_highlight_region (Window wp, Region *rp)
{
  if ((wp != cur_wp
       && !get_variable_bool ("highlight-nonselected-windows"))
      || get_buffer_mark (get_window_bp (wp)) == NULL
      || !get_buffer_mark_active (get_window_bp (wp)))
    return false;

  *rp = region_new (window_o (wp), get_marker_o (get_buffer_mark (get_window_bp (wp))));
  return true;
}

static const char *
make_mode_line_flags (Window wp)
{
  const Buffer bp = get_window_bp (wp);

  if (get_buffer_modified (bp) && get_buffer_readonly (bp))
    return "%*";
  if (get_buffer_modified (bp))
    return "**";
  if (get_buffer_readonly (bp))
    return "%%";
  return "--";
}

static void
make_screen_pos (char * screen_pos, Window wp)
{
  const bool tv = window_top_visible (wp);
  const bool bv = window_bottom_visible (wp);

  if (tv && bv)
    strcpy (screen_pos, "All");
  else if (tv)
    strcpy (screen_pos, "Top");
  else if (bv)
    strcpy (screen_pos, "Bot");
  else
    snprintf (screen_pos, 4, "%2d%%",
              (int) ((float) 100.0 * window_o (wp) / get_buffer_size (get_window_bp (wp))));
}

static void
draw_status_line (size_t line, Window wp)
{
  Buffer bp = get_window_bp (wp);
  term_attrset (FONT_REVERSE);

  term_move (line, 0);
  for (size_t i = 0; i < get_window_ewidth (wp); ++i)
    term_addstr ("-");

  const char *eol_type;
  if (get_buffer_eol (cur_bp) == coding_eol_cr)
    eol_type = "(Mac)";
  else if (get_buffer_eol (cur_bp) == coding_eol_crlf)
    eol_type = "(DOS)";
  else
    eol_type = ":";

  term_move (line, 0);
  size_t n = offset_to_line (bp, window_o (wp));

  char screen_pos[4];   // Position
  make_screen_pos (screen_pos, wp);

  char line_col[STR_SIZE];
  snprintf(line_col, STR_SIZE, "(%zu,%zu)", n + 1, get_goalc_bp (bp, window_o (wp)));

  astr as = astr_fmt ("--%s%2s  %-15s   %s %-9s (Fundamental",
                      eol_type, make_mode_line_flags (wp), get_buffer_name (bp),
                      screen_pos, line_col);

  if (get_buffer_autofill (bp))
    astr_cat_cstr (as, " Fill");
  if (thisflag & FLAG_DEFINING_MACRO)
    astr_cat_cstr (as, " Def");
  if (get_buffer_isearch (bp))
    astr_cat_cstr (as, " Isearch");

  astr_cat_char (as, ')');
  term_addstr (astr_cstr (as));

  term_attrset (FONT_NORMAL);

  astr_free(as);
}

static void
draw_window (size_t topline, Window wp)
{
  size_t line, j, o;
  Region r;
  int highlight = calculate_highlight_region (wp, &r);
  Buffer bp = get_window_bp (wp);

  /* Find the first line to display on the first screen line. */
  for (o = buffer_start_of_line (bp, window_o (wp)), line = get_window_topdelta (wp);
       line > 0 && o > 0;
       assert ((o = buffer_prev_line (bp, o)) != SIZE_MAX), --line)
    ;

  /* Draw the window lines. */
  size_t cur_tab_width = tab_width (bp);

  const size_t window_eheight = get_window_eheight (wp);
  const bool linum_mode = get_variable_bool("linum-mode");
  const size_t buffer_first_line = offset_to_line (bp, o);
  size_t first_column = 0;
  size_t fill_column_indicator = 0;

  if (get_variable_bool ("fill-column-indicator-mode"))
    {
      long res = 0;
      if (lisp_to_number (get_variable_bp (bp, "fill-column"), &res))
	fill_column_indicator = res;
    }

  if (linum_mode)
    {
      char buff[STR_SIZE];
      const size_t buffer_last_line = buffer_first_line + window_eheight;

      snprintf(buff, STR_SIZE, "%lu", buffer_last_line);

      first_column += strlen(buff) + 1;
    }

  set_window_first_column(wp, first_column);

  for (j = 0; j < window_eheight; ++j)
    {
      line = j + topline;
      /* Clear the line. */
      term_move (line, 0);
      term_clrtoeol ();

      /* If at the end of the buffer, don't write any text. */
      if (o == SIZE_MAX)
        continue;

      if (linum_mode)
	{
	  char buff[STR_SIZE];
	  snprintf (buff, STR_SIZE, "%*lu",
	            first_column - 1, buffer_first_line + j);
	  term_addstr(buff);
	}

      draw_line (line, wp, o, r, highlight, cur_tab_width, fill_column_indicator);

      o = buffer_next_line (bp, o);
    }

  set_window_all_displayed (wp, o >= get_buffer_size (bp));

  /* Draw the status line only if there is available space after the
     buffer text space. */
  if (get_window_fheight (wp) - get_window_eheight (wp) > 0)
    draw_status_line (topline + get_window_eheight (wp), wp);
}

static size_t col, cur_topline = 0;

void
term_redisplay (void)
{
  /* Calculate the start column if the line at point has to be truncated. */
  Buffer bp = get_window_bp (cur_wp);

  const size_t tab_width_bp = tab_width (bp);
  const size_t ew = get_window_ewidth (cur_wp) - get_window_first_column (cur_wp);

  size_t o = window_o (cur_wp);
  const size_t lineo = o - get_buffer_line_o (bp);
  size_t lastcol = 0;
  col = 0;
  o -= lineo;
  set_window_start_column (cur_wp, 0);

  for (size_t lp = lineo; lp != SIZE_MAX; --lp)
    {
      col = 0;
      for (size_t p = lp; p < lineo; ++p)
        {
          char c = get_buffer_char (bp, o + p);
          if (isprint (c))
            col++;
          else
	    {
	      char buff[STR_SIZE];
	      const int ret =
		make_char_printable (buff, get_buffer_char (bp, o + p), col, tab_width_bp);
	      assert(ret > 0);
	      col += ret;
	    }
        }

      if (col >= ew - 1 ||        // Col crosses window border
          (lp / (ew / 3)) + 2 < lineo / (ew / 3))
        {
          set_window_start_column (cur_wp, lp + 1);
          col = lastcol;
          break;
        }

      lastcol = col;
    }

  /* Draw the windows. */
  cur_topline = 0;
  size_t topline = 0;
  for (Window wp = head_wp; wp != NULL; wp = get_window_next (wp))
    {
      if (wp == cur_wp)
        cur_topline = topline;

      draw_window (topline, wp);

      topline += get_window_fheight (wp);
    }

  term_redraw_cursor ();
}

void
term_redraw_cursor (void)
{
  term_move (cur_topline + get_window_topdelta (cur_wp),
             get_window_first_column(cur_wp) + col);
}

/*
 * Tidy and close the terminal ready to leave Zile.
 */
void
term_finish (void)
{
  term_move (term_height () - 1, 0);
  term_clrtoeol ();
  term_attrset (FONT_NORMAL);
  term_refresh ();
  term_close ();
}
