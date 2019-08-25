#ifndef TERM_CURSES_H
#define TERM_CURSES_H

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

size_t term_buf_len (void);
void term_init (void);

void term_attron (int pair);
void term_attroff (int pair);
void term_attr_reset ();

void term_close (void);
void term_move (size_t y, size_t x);
void term_clrtoeol (void);
void term_refresh (void);
void term_clear (void);
void term_addch (char c);
void term_addstr (const char *s);
void term_attrset (size_t attr);
void term_beep (void);
_GL_ATTRIBUTE_PURE size_t term_width (void);
_GL_ATTRIBUTE_PURE size_t term_height (void);
size_t term_getkey (int delay);
int term_getkey_unfiltered (int delay);
void term_ungetkey (size_t key);

#endif
