/* Main types and definitions

   Copyright (c) 2008 Free Software Foundation, Inc.
   Copyright (c) 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004 Sandro Sigala.
   Copyright (c) 2003, 2004, 2005, 2006, 2007 Reuben Thomas.

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

#ifndef ZILE_H
#define ZILE_H

#include <stdlib.h>
#include <stdbool.h>
#include <limits.h>
#include "xalloc.h"
#include "size_max.h"
#include "minmax.h"
#include "hash.h"
#include "gl_list.h"

#include "astr.h"
#include "lists.h"

#define ZILE_VERSION_STRING	"GNU " PACKAGE_NAME " " VERSION

/*--------------------------------------------------------------------------
 * Main editor structures.
 *--------------------------------------------------------------------------*/

/* Opaque types. */
typedef struct History History;
typedef struct Undo Undo;
typedef struct Macro Macro;
typedef struct Binding *Binding;

/* Types which should really be opaque. */
typedef struct Line Line;
typedef struct Point Point;
typedef struct Marker Marker;
typedef struct Region Region;
typedef struct Buffer Buffer;
typedef struct Window Window;
typedef struct Completion Completion;

/*
 * A line is a doubly-linked list of astrs.
 * The newline at the end of each line is implicit.
 */
struct Line
{
  Line *prev;
  Line *next;
  astr text;
};

struct Point
{
  Line *p;			/* Line pointer. */
  size_t n;			/* Line number. */
  size_t o;			/* Offset. */
};

struct Marker
{
  Buffer *bp;			/* Buffer that points into. */
  Point pt;			/* Point position. */
  Marker *next;			/* Used to chain all markers in the buffer. */
};

/* Undo delta types. */
enum
{
  UNDO_REPLACE_BLOCK,		/* Replace a block of characters. */
  UNDO_START_SEQUENCE,		/* Start a multi operation sequence. */
  UNDO_END_SEQUENCE,		/* End a multi operation sequence. */
};

struct Region
{
  Point start;			/* The region start. */
  Point end;			/* The region end. */
  size_t size;			/* The region size. */
};

/* Buffer flags or minor modes. */
#define BFLAG_MODIFIED  0000001	/* The buffer has been modified. */
#define BFLAG_NOSAVE    0000002	/* The buffer need not to be saved. */
#define BFLAG_NEEDNAME  0000004	/* On save, ask for a file name. */
#define BFLAG_TEMPORARY 0000010	/* The buffer is a temporary buffer. */
#define BFLAG_READONLY  0000020	/* The buffer cannot be modified. */
#define BFLAG_OVERWRITE 0000040	/* The buffer is in overwrite mode. */
#define BFLAG_BACKUP    0000100	/* The old file has already been
                                   backed up. */
#define BFLAG_NOUNDO    0000200	/* Do not record undo informations. */
#define BFLAG_AUTOFILL  0000400	/* The buffer is in Auto Fill mode. */
#define BFLAG_ISEARCH   0001000	/* The buffer is in Isearch loop. */
#define BFLAG_MARK      0002000	/* The mark is active. */

struct Buffer
{
  Buffer *next;		/* Next buffer in buffer list. */
  char *name;		/* The name of the buffer. */
  char *filename;	/* The file being edited. */
  char *eol;		/* EOL string (up to 2 chars). */
  Line *lines;		/* The lines of text. */
  size_t last_line;	/* The number of the last line in the buffer. */
  Point pt;		/* The point. */
  Marker *mark;		/* The mark. */
  Marker *markers;	/* Markers list (updated whenever text is changed). */
  int flags;		/* Buffer flags. */
  Undo *last_undop;	/* Most recent undo delta. */
  Undo *next_undop;	/* Next undo delta to apply. */
  Hash_table *vars;	/* Buffer-local variables. */
};

struct Window
{
  Window *next;		/* The next window in window list. */
  Buffer *bp;		/* The buffer displayed in window. */
  size_t topdelta;	/* The top line delta. */
  int lastpointn;	/* The last point line number. */
  size_t start_column;	/* The start column of the window (>0 if scrolled
                           sideways). */
  Marker *saved_pt;	/* The point line pointer, line number and offset
                           (used to hold the point in non-current windows). */
  size_t fwidth, fheight; /* The formal width and height of the window. */
  size_t ewidth, eheight; /* The effective width and height of the window. */
};

enum
{
  COMPLETION_NOTMATCHED,
  COMPLETION_MATCHED,
  COMPLETION_MATCHEDNONUNIQUE,
  COMPLETION_NONUNIQUE
};

struct Completion
{
  Buffer *old_bp;		/* The buffer from which the
                                   completion was invoked. */
  gl_list_t completions;	/* The completions list. */
  gl_list_t matches;		/* The matches list. */
  const char *match;		/* The match buffer. */
  size_t matchsize;		/* The match buffer size. */
  int flags;			/* Completion flags. */
  astr path;			/* Path for a filename completion. */
};

/* Completion flags */
#define CFLAG_POPPEDUP	0000001	/* Completion window has been popped up. */
#define CFLAG_CLOSE	0000002	/* The completion window should be closed. */
#define CFLAG_FILENAME	0000004	/* This is a filename completion. */

/*--------------------------------------------------------------------------
 * Zile commands to C bindings.
 *--------------------------------------------------------------------------*/

/*
 * The type of a Zile exported function.
 * `uniarg' is the number of times to repeat the function.
 */
typedef le * (*Function) (int uniarg, le * list);

/* Turn a bool into a Lisp boolean */
#define bool_to_lisp(b) ((b) ? leT : leNIL)

/* Define an interactive function. */
#define DEFUN(zile_func, c_func) \
        le * F_ ## c_func (int uniarg, le *arglist) \
        { \
          if (arglist && arglist->data) \
            uniarg = atoi (arglist->data);
#define END_DEFUN \
        }

/* Call an interactive function. */
#define FUNCALL(c_func)                         \
        F_ ## c_func (1, NULL)

/* Call an interactive function with a universal argument. */
#define FUNCALL_ARG(c_func, uniarg)             \
        F_ ## c_func (uniarg, NULL)

/*--------------------------------------------------------------------------
 * Keyboard handling.
 *--------------------------------------------------------------------------*/

#define GETKEY_DELAYED                  0001
#define GETKEY_UNFILTERED               0002

/* Special value returned for invalid key codes, or when no key is pressed. */
#define KBD_NOKEY                       UINT_MAX

/* Key modifiers. */
#define KBD_CTRL                        01000
#define KBD_META                        02000

/* Common non-alphanumeric keys. */
#define KBD_CANCEL                      (KBD_CTRL | 'g')
#define KBD_TAB                         00402
#define KBD_RET                         00403
#define KBD_PGUP                        00404
#define KBD_PGDN                        00405
#define KBD_HOME                        00406
#define KBD_END                         00407
#define KBD_DEL                         00410
#define KBD_BS                          00411
#define KBD_INS                         00412
#define KBD_LEFT                        00413
#define KBD_RIGHT                       00414
#define KBD_UP                          00415
#define KBD_DOWN                        00416
#define KBD_F1                          00420
#define KBD_F2                          00421
#define KBD_F3                          00422
#define KBD_F4                          00423
#define KBD_F5                          00424
#define KBD_F6                          00425
#define KBD_F7                          00426
#define KBD_F8                          00427
#define KBD_F9                          00430
#define KBD_F10                         00431
#define KBD_F11                         00432
#define KBD_F12                         00433

/*--------------------------------------------------------------------------
 * Miscellaneous stuff.
 *--------------------------------------------------------------------------*/

/* Global flags, stored in thisflag and lastflag. */
#define FLAG_DONE_CPCN		0000001	/* Last command was C-p or C-n. */
#define FLAG_DONE_KILL		0000002	/* The last command was a kill. */
#define FLAG_NEED_RESYNC	0000004	/* A resync is required. */
#define FLAG_QUIT		0000010	/* The user has asked to quit. */
#define FLAG_SET_UNIARG		0000020	/* The last command modified the
                                           universal arg variable `uniarg'. */
#define FLAG_UNIARG_EMPTY	0000040	/* Current universal arg is just C-u's
                                           with no number. */
#define FLAG_DEFINING_MACRO	0000100	/* We are defining a macro. */

/* Zile font codes
 * Designed to fit in an int, leaving room for a char underneath. */
#define FONT_NORMAL		0x000
#define FONT_REVERSE		0x100

/* Default waitkey pause in ds */
#define WAITKEY_DEFAULT 20

/* Avoid warnings about unused parameters. */
#undef GCC_UNUSED
#ifdef __GNUC__
#define GCC_UNUSED __attribute__ ((unused))
#else
#define GCC_UNUSED
#endif

#endif /* !ZILE_H */
