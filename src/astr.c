/* Dynamically allocated strings

   Copyright (c) 2001-2014 Free Software Foundation, Inc.

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

#include "astr.h"

#include "xvasprintf.h"


astr
astr_cat_cstr (astr as, const char *s)
{
  const size_t len = strlen (s);
  return astr_cat_nstr (as, s, len);
}


// Make a new string from a C null-terminated string.
astr
astr_new_cstr (const char *s)
{
  return astr_cpy_cstr (astr_new (), s);
}

astr
astr_vfmt (const char *fmt, va_list ap)
{
  return astr_new_cstr (xvasprintf (fmt, ap));
}

astr
astr_fmt (const char *fmt, ...)
{
  va_list ap;
  va_start (ap, fmt);
  astr as = astr_vfmt (fmt, ap);
  va_end (ap);
  return as;
}

astr
astr_recase (astr as, casing newcase)
{
  astr bs = astr_new ();

  if (newcase == case_capitalized || newcase == case_upper)
    astr_cat_char (bs, toupper (astr_get (as, 0)));
  else
    astr_cat_char (bs, tolower (astr_get (as, 0)));

  for (size_t i = 1, len = astr_len (as); i < len; i++)
    astr_cat_char (bs, ((newcase == case_upper) ? toupper : tolower) (astr_get (as, i)));

  return astr_cpy (as, bs);
}


#ifdef TEST

#include <stdio.h>
#include "progname.h"

#include "main.h"

static void
assert_eq (astr as, const char *s)
{
  if (!STREQ (astr_cstr (as), s))
    {
      printf ("test failed: \"%s\" != \"%s\"\n", astr_cstr (as), s);
      exit (EXIT_FAILURE);
    }
}

int
main (int argc _GL_UNUSED_PARAMETER, char **argv)
{
  astr as1, as2, as3;

  GC_init ();

  set_program_name (argv[0]);

  as1 = astr_new ();
  astr_cpy_cstr (as1, "hello world");
  astr_cat_char (as1, '!');
  assert_eq (as1, "hello world!");

  as3 = astr_substr (as1, 6, 5);
  assert_eq (as3, "world");

  as2 = astr_new ();
  astr_cpy_cstr (as2, "The ");
  astr_cat (as2, as3);
  astr_cat_char (as2, '.');
  assert_eq (as2, "The world.");

  as3 = astr_substr (as1, astr_len (as1) - 6, 5);
  assert_eq (as3, "world");

  astr_cpy_cstr (as1, "1234567");
  astr_replace_nstr (as1, 1, "foo", 3);
  assert_eq (as1, "1foo567");

  astr_cpy_cstr (as1, "12345");
  as2 = astr_substr (as1, astr_len (as1) - 2, 2);
  assert_eq (as2, "45");

  astr_cpy_cstr (as1, "12345");
  as2 = astr_substr (as1, astr_len (as1) - 5, 5);
  assert_eq (as2, "12345");

  as1 = astr_fmt ("%s * %d = ", "5", 3);
  astr_cat (as1, astr_fmt ("%d", 15));
  assert_eq (as1, "5 * 3 = 15");

  astr_cpy_cstr (as1, "some text");
  astr_recase (as1, case_capitalized);
  assert_eq (as1, "Some text");
  astr_recase (as1, case_upper);
  assert_eq (as1, "SOME TEXT");
  astr_recase (as1, case_lower);
  assert_eq (as1, "some text");

  printf ("astr test successful.\n");

  return EXIT_SUCCESS;
}

#endif /* TEST */
