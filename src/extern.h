/* Global function prototypes

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

#include "gl_xlist.h"

#ifndef STR_SIZE
#define STR_SIZE 32
#endif

/* FIXME: Split this header up into per-module headers. Rename data structure headers to avoid clashes. */


/* buffer.c --------------------------------------------------------------- */

/* completion.c ----------------------------------------------------------- */

/* editfns.c -------------------------------------------------------------- */

/* eval.c ----------------------------------------------------------------- */

/* file.c ----------------------------------------------------------------- */

/* funcs.c ---------------------------------------------------------------- */

/* getkey.c --------------------------------------------------------------- */

/* history.c -------------------------------------------------------------- */

/* mouse.c --------------------------------------------------------------- */

/* keycode.c -------------------------------------------------------------- */
astr chordtodesc (size_t key);
gl_list_t keystrtovec (const char *key);
astr keyvectodesc (gl_list_t keys);

/* libgc_extra.c ---------------------------------------------------------- */
void *zile_calloc(size_t n, size_t s);

/* line.c ----------------------------------------------------------------- */

/* lisp.c ----------------------------------------------------------------- */



/* marker.c --------------------------------------------------------------- */

/* minibuf.c -------------------------------------------------------------- */

/* redisplay.c ------------------------------------------------------------ */
void resize_windows (void);
void recenter (Window wp);

/* search.c --------------------------------------------------------------- */
void init_search (void);

/* term_curses.c ---------------------------------------------------------- */

/* term_minibuf.c --------------------------------------------------------- */

/* term_redisplay.c ------------------------------------------------------- */
extern size_t first_column;
void term_redraw_cursor (void);
void term_redisplay (void);
void term_finish (void);

/* undo.c ----------------------------------------------------------------- */

/* window.c --------------------------------------------------------------- */
/*
 * Declare external Zile functions.
 */

