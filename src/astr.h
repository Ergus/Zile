/* Dynamically allocated strings

   Copyright (c) 2001-2012 Free Software Foundation, Inc.

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

#ifndef ASTR_H
#define ASTR_H

#include <config.h>

#include <stddef.h>
#include <stdio.h>
#include <stdarg.h>
#include <assert.h>
#include <string.h>

#include <stdlib.h>
#include <ctype.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "xalloc.h"

#include "minmax.h"

#define ALLOCATION_CHUNK_SIZE	16

/*
 * The astr library provides dynamically allocated null-terminated C
 * strings.
 *
 * The string type, astr, is a pointer type.
 *
 * String positions start at zero, as with ordinary C strings.
 *
 * Where not otherwise specified, the functions return the first
 * argument string, usually named as in the function prototype.
 */

/*
 * The opaque string type.
 */
typedef struct astr
{
  char *text;		/* The string buffer. */
  size_t len;		/* The length of the string. */
  size_t maxlen;	/* The buffer size. */
} *astr;

typedef struct astr const *const_astr;


inline void
astr_set_len (astr as, size_t len)
{
  if (len > as->maxlen || len < as->maxlen / 2)
    {
      as->maxlen = len + ALLOCATION_CHUNK_SIZE;
      as->text = xrealloc (as->text, as->maxlen + 1);
    }
  as->len = len;
  as->text[as->len] = '\0';
}


// Allocate a new string with zero length.
inline astr
astr_new (void)
{
  astr as;
  as = (astr) XZALLOC (struct astr);
  as->maxlen = ALLOCATION_CHUNK_SIZE;
  as->len = 0;
  as->text = (char *) xzalloc (as->maxlen + 1);
  return as;
}

// Release string memory.
inline void astr_free(astr in)
{
  free(in->text);
  free(in);
}

// Make a new constant string from a counted C string.
inline const_astr
const_astr_new_nstr (const char *s, size_t n)
{
  astr as;
  as = (astr) XZALLOC (struct astr);
  as->len = n;
  as->text = (char *) s;
  return as;
}

/*
 * Convert as into a C null-terminated string.
 * as[0] to as[astr_len (as) - 1] inclusive may be read.
 */
inline _GL_ATTRIBUTE_PURE const char *
astr_cstr (const_astr as)
{
  return (const char *) (as->text);
}

// Return the length of the argument string `as'.
inline _GL_ATTRIBUTE_PURE size_t
astr_len (const_astr as)
{
  return as->len;
}

// Return the `pos'th character of `as'.
inline _GL_ATTRIBUTE_PURE char
astr_get (const_astr as, size_t pos)
{
  assert (pos <= astr_len (as));
  return (astr_cstr (as))[pos];
}

/*
 * Append the contents of the argument string or character to `as'.
 */

inline astr
astr_cat_nstr (astr as, const char *s, size_t csize)
{
  assert (as != NULL);
  size_t oldlen = as->len;
  astr_set_len (as, as->len + csize);
  memmove (as->text + oldlen, s, csize);
  return as;
}

/*
 * Return a new astr consisting of `size' characters from string `as'
 * starting from position `pos'.
 */
inline astr
astr_substr (const_astr as, size_t pos, size_t size)
{
  assert (pos + size <= astr_len (as));
  return astr_cat_nstr (astr_new (), astr_cstr (as) + pos, size);
}

astr
astr_new_cstr (const char *s);

inline astr
astr_cat (astr as, const_astr src)
{
  return astr_cat_nstr (as, astr_cstr (src), astr_len (src));
}

inline astr
astr_cat_char (astr as, int c)
{
  char c2 = c;
  return astr_cat_nstr (as, &c2, 1);
}

astr
astr_cat_cstr (astr as, const char *s);

/*
 * Overwrite `size' characters of `as', starting at `pos', with the
 * argument string.
 */
inline astr
astr_replace_nstr (astr as, size_t pos, const char *s, size_t size)
{
  assert (as != NULL);
  assert (pos <= as->len);
  assert (size <= as->len - pos);
  memmove (as->text + pos, s, size);
  return as;
}

/*
 * Remove `size' chars from `as' at position `pos'.
 */
inline astr
astr_remove (astr as, size_t pos, size_t size)
{
  assert (as != NULL);
  assert (pos <= as->len);
  assert (size <= as->len - pos);
  memmove (as->text + pos, as->text + pos + size, as->len - (pos + size));
  astr_set_len (as, as->len - size);
  return as;
}

/*
 * Insert gap of `size' characters in `as' at position `pos'.
 */
inline astr
astr_insert (astr as, size_t pos, size_t size)
{
  assert (as != NULL);
  assert (pos <= as->len);
  assert (pos + size >= MAX (pos, size));    /* Check for overflow. */
  astr_set_len (as, as->len + size);
  memmove (as->text + pos + size, as->text + pos, as->len - (pos + size));
  memset (as->text + pos, '\0', size);
  return as;
}

/*
 * Move `n' chars in `as' from position `from' to `to'.
 */
inline astr
astr_move (astr as, size_t to, size_t from, size_t n)
{
  assert (as != NULL);
  assert (to <= as->len);
  assert (from <= as->len);
  assert (n <= as->len - MAX (from, to));
  memmove (as->text + to, as->text + from, n);
  return as;
}

/*
 * Set `n' chars in `as' at position `pos' to `c'.
 */
inline astr
astr_set (astr as, size_t pos, int c, size_t n)
{
  assert (as != NULL);
  assert (pos <= as->len);
  assert (n <= as->len - pos);
  memset (as->text + pos, c, n);
  return as;
}

/*
 * Truncate `as' to position `pos'.
 */
inline astr
astr_truncate (astr as, size_t pos)
{
  return astr_remove (as, pos, astr_len (as) - pos);
}


inline astr
astr_ncpy_cstr (astr as, const char *s, size_t len)
{
  astr_truncate (as, 0);
  return astr_cat_nstr (as, s, len);
}


// Assign the contents of the argument string to the string `as'.
inline astr
astr_cpy (astr as, const_astr src)
{
  return astr_ncpy_cstr (as, astr_cstr (src), astr_len (src));
}

inline astr
astr_cpy_cstr (astr as, const char *s)
{
  return astr_ncpy_cstr (as, s, strlen (s));
}

// Make a new string from a C null-terminated string.
astr astr_new_cstr (const char *s);


/*
 * Read file contents into an astr.
 * Returns NULL if the file doesn't exist, or other error.
 */
inline astr
astr_readf (const char *filename)
{
  astr as = NULL;
  struct stat st;
  if (stat (filename, &st) == 0)
    {
      size_t size = st.st_size;
      int fd = open (filename, O_RDONLY);
      if (fd >= 0)
        {
          char buf[BUFSIZ];
          as = astr_new ();
          while ((size = read (fd, buf, BUFSIZ)) > 0)
            astr_cat_nstr (as, buf, size);
          close (fd);
        }
    }
  return as;
}


/*
 * Format text into a string and return it.
 */
_GL_ATTRIBUTE_FORMAT_PRINTF(1, 0) astr astr_vfmt (const char *fmt, va_list ap);
_GL_ATTRIBUTE_FORMAT_PRINTF(1, 2) astr astr_fmt (const char *fmt, ...);

/* Enumeration for casing. */
typedef enum {
  case_upper = 1,
  case_lower,
  case_capitalized,
} casing;

/*
 * Recase as according to newcase.
 */
astr astr_recase (astr as, casing newcase);

#endif /* !ASTR_H */
