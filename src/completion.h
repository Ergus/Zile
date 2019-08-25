#ifndef COMPLETION_H
#define COMPLETION_H

/* Completion fields

   Copyright (c) 2009 Free Software Foundation, Inc.

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

#include "main.h"

// Dynamically allocated string fields of Completion.
#define COMPLETION_FIELDS						\
  FIELD_STR(match)		/* The match buffer. */			\
									\
  /* Other fields of Completion.  */					\
  FIELD(Buffer, old_bp)		/* The buffer from which the completion was invoked. */	\
  FIELD(gl_list_t, completions)	/* The completions list. */		\
  FIELD(gl_list_t, matches)	/* The matches list. */			\
  FIELD(size_t, matchsize)	/* The match buffer size. */		\
  FIELD(int, flags)		/* Completion flags. */			\
  FIELD(astr, path)		/* Path for a filename completion. */

#include "main.h"

struct Completion
{
#define FIELD(ty, name) ty name;
#define FIELD_STR(name) char *name;
  COMPLETION_FIELDS
#undef FIELD
#undef FIELD_STR
};

#define FIELD(ty, field)                        \
  IGETTER (Completion, completion, ty, field)    \
  ISETTER (Completion, completion, ty, field)

#define FIELD_STR(field)                                  \
  IGETTER (Completion, completion, char *, field)          \
  ISTR_SETTER (Completion, completion, field)

COMPLETION_FIELDS
#undef FIELD
#undef FIELD_STR

_GL_ATTRIBUTE_PURE int completion_strcmp (const void *p1, const void *p2);
Completion completion_new (bool fileflag);
void completion_scroll_up (void);
void completion_scroll_down (void);
completion_code completion_try (Completion cp, astr search, bool popup_when_complete);

#endif
