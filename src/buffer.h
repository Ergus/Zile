#ifndef BUFFER_H
#define BUFFER_H
/* Buffer fields

   Copyright (c) 2009-2011 Free Software Foundation, Inc.

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

/* Cope with bad definition in some system headers. */
#undef lines

#include "main.h"
#include "region.h"

#define BUFFER_FIELDS							\
    /* Dynamically allocated string fields of Buffer. */		\
    FIELD_STR(name)           /* The name of the buffer. */		\
    FIELD_STR(filename)       /* The file being edited. */		\
									\
    /* Other fields of Buffer. */					\
    FIELD(Buffer, next)       /* Next buffer in buffer list. */		\
    FIELD(size_t, goalc)      /* Goal column for previous/next-line commands. */ \
    FIELD(Marker, mark)       /* The mark. */				\
    FIELD(Marker, markers)    /* Markers list (updated whenever text is changed). */ \
    FIELD(Undo, last_undop)   /* Most recent undo delta. */		\
    FIELD(Undo, next_undop)   /* Next undo delta to apply. */		\
    FIELD(Hash_table *, vars) /* Buffer-local variables. */		\
    FIELD(bool, modified)     /* Modified flag. */			\
    FIELD(bool, nosave)       /* The buffer need not be saved. */	\
    FIELD(bool, needname)     /* On save, ask for a file name. */	\
    FIELD(bool, temporary)    /* The buffer is a temporary buffer. */	\
    FIELD(bool, readonly)     /* The buffer cannot be modified. */	\
    FIELD(bool, backup)       /* The old file has already been backed up. */ \
    FIELD(bool, noundo)       /* Do not record undo informations. */	\
    FIELD(bool, autofill)     /* The buffer is in Auto Fill mode. */	\
    FIELD(bool, isearch)      /* The buffer is in Isearch loop. */	\
    FIELD(bool, mark_active)  /* The mark is active. */			\
    FIELD(astr, dir)          /* The default directory. */		\
    FIELD(estr, text)          /* The default directory. */

#define MIN_GAP 1024 /* Minimum gap size after resize. */
#define MAX_GAP 4096 /* Maximum permitted gap size. */

struct Buffer
{
#define FIELD(ty, name) ty name;
#define FIELD_STR(name) char *name;
  BUFFER_FIELDS
#undef FIELD
#undef FIELD_STR
  size_t pt;         /* The point. */
  size_t gap;        /* Size of gap after point. */
};

#define FIELD(ty, field)                         \
  IGETTER (Buffer, buffer, ty, field)             \
  ISETTER (Buffer, buffer, ty, field)

#define FIELD_STR(field)                         \
  IGETTER (Buffer, buffer, char *, field)         \
  ISTR_SETTER (Buffer, buffer, field)

BUFFER_FIELDS
#undef FIELD
#undef FIELD_STR

bool insert_char (int c);
bool delete_char (void);
bool replace_estr (size_t del, const_estr es);
bool insert_estr (const_estr as);

const_astr get_buffer_pre_point (Buffer bp);
const_astr get_buffer_post_point (Buffer bp);
void set_buffer_pt (Buffer bp, size_t o);
_GL_ATTRIBUTE_PURE size_t get_buffer_pt (Buffer bp);

_GL_ATTRIBUTE_PURE size_t buffer_prev_line (Buffer bp, size_t o);
_GL_ATTRIBUTE_PURE size_t buffer_next_line (Buffer bp, size_t o);
_GL_ATTRIBUTE_PURE size_t buffer_start_of_line (Buffer bp, size_t o);
_GL_ATTRIBUTE_PURE size_t buffer_end_of_line (Buffer bp, size_t o);
_GL_ATTRIBUTE_PURE size_t buffer_line_len (Buffer bp, size_t o);

_GL_ATTRIBUTE_PURE size_t get_buffer_size (Buffer bp);
_GL_ATTRIBUTE_PURE char get_buffer_char (Buffer bp, size_t o);
_GL_ATTRIBUTE_PURE size_t get_buffer_line_o (Buffer bp);
_GL_ATTRIBUTE_PURE const char *get_buffer_eol (Buffer bp);

Buffer buffer_new (void);
void init_buffer (Buffer bp);
void destroy_buffer (Buffer bp);
void insert_buffer (Buffer bp);

_GL_ATTRIBUTE_PURE const char *get_buffer_filename_or_name (Buffer bp);
void set_buffer_names (Buffer bp, const char *filename);
_GL_ATTRIBUTE_PURE Buffer find_buffer (const char *name);
void switch_to_buffer (Buffer bp);
bool warn_if_no_mark (void);
bool warn_if_readonly_buffer (void);

Region calculate_the_region (void);

void set_temporary_buffer (Buffer bp);
void activate_mark (void);
void deactivate_mark (void);
size_t tab_width (Buffer bp);
estr get_buffer_region (Buffer bp, Region r);
Buffer create_auto_buffer (const char *name);
Buffer create_scratch_buffer (void);

Completion make_buffer_completion (void);
void kill_buffer (Buffer kill_bp);
bool check_modified_buffer (Buffer bp);
bool move_char (ptrdiff_t dir);
bool move_line (ptrdiff_t n);
_GL_ATTRIBUTE_PURE size_t offset_to_line (Buffer bp, size_t offset);
void goto_offset (size_t o);

#endif
