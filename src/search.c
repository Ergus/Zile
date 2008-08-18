/* Search and replace functions

   Copyright (c) 2008 Free Software Foundation, Inc.
   Copyright (c) 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004 Sandro Sigala.
   Copyright (c) 2004 David A. Capello.
   Copyright (c) 2004, 2005, 2006, 2007, 2008 Reuben Thomas.

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

#include "config.h"

#include <assert.h>
#include <limits.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#include "regex.h"

#include "zile.h"
#include "extern.h"

/* Downcasing table for case-folding search */
static char id[UCHAR_MAX + 1];
static char downcase[UCHAR_MAX + 1];

void
init_search (void)
{
  int i;

  for (i = 0; i <= UCHAR_MAX; i++)
    {
      id[i] = i;
      downcase[i] = tolower (i);
    }
}

/* Return true if there are no upper-case letters in the given string.
   If `regex' is true, ignore escaped letters. */
static int
no_upper (const char *s, size_t len, int regex)
{
  int quote_flag = 0;
  size_t i;

  for (i = 0; i < len; i++)
    {
      if (regex && s[i] == '\\')
	quote_flag = !quote_flag;
      else if (!quote_flag && isupper ((int) s[i]))
	return false;
    }

  return true;
}

static const char *
fold_table (const char *s, int regex)
{
  if (get_variable_bool ("case-fold-search")
      && no_upper (s, strlen (s), regex))
    return downcase;
  else
    return id;
}

static const char *
find_substr (const char *s1, size_t s1size,
	     const char *s2, size_t s2size,
	     const char translate[UCHAR_MAX + 1])
{
  const char *e1 = s1 + s1size, *e2 = s2 + s2size;

  for (; s1 <= e1 - s2size; s1++)
    {
      const char *sp1 = s1, *sp2 = s2;

      while (translate[(unsigned char) *sp1++] ==
	     translate[(unsigned char) *sp2++])
	if (sp2 == e2)
	  return sp1;
    }

  return NULL;
}

static const char *
rfind_substr (const char *s1, size_t s1size,
	      const char *s2, size_t s2size,
	      const char translate[UCHAR_MAX + 1])
{
  const char *e1 = s1 + s1size, *e2 = s2 + s2size;

  for (; e1 >= s1 + s2size; e1--)
    {
      const char *sp1 = e1, *sp2 = e2;

      while (translate[(unsigned char) *--sp1] ==
	     translate[(unsigned char) *--sp2])
	if (sp2 == s2)
	  return sp1;
    }

  return NULL;
}

static const char *re_find_err = NULL;

static char *
re_find_substr (const char *s1, size_t s1size,
		const char *s2, size_t s2size,
		int bol, int eol, int backward,
		const char translate[UCHAR_MAX + 1])
{
  struct re_pattern_buffer pattern;
  struct re_registers search_regs;
  char *ret = NULL;
  int index;
  reg_syntax_t old_syntax;

  old_syntax = re_set_syntax (RE_SYNTAX_EMACS);

  search_regs.num_regs = 1;
  search_regs.start = XZALLOC (regoff_t);
  search_regs.end = XZALLOC (regoff_t);

  /* translate table is never written to, so this cast is safe */
  pattern.translate = (unsigned char *) translate;
  pattern.fastmap = NULL;
  pattern.buffer = NULL;
  pattern.allocated = 0;

  re_find_err = re_compile_pattern (s2, (int) s2size, &pattern);
  if (!re_find_err)
    {
      pattern.not_bol = !bol;
      pattern.not_eol = !eol;

      if (!backward)
	index = re_search (&pattern, s1, (int) s1size, 0, (int) s1size,
			   &search_regs);
      else
	index =
	  re_search (&pattern, s1, (int) s1size, (int) s1size, -(int) s1size,
		     &search_regs);

      if (index >= 0)
	{
	  if (!backward)
	    ret = ((char *) s1) + search_regs.end[0];
	  else
	    ret = ((char *) s1) + search_regs.start[0];
	}
      else if (index == -1)
	{
	  /* no match */
	}
      else
	{
	  /* error */
	}
    }

  re_set_syntax (old_syntax);
  pattern.translate = NULL;	/* regfree requires translate to be NULL
				   or malloced */
  regfree (&pattern);

  free (search_regs.start);
  free (search_regs.end);

  return ret;
}

static void
goto_linep (Line * lp)
{
  cur_bp->pt = point_min ();
  resync_redisplay ();
  while (cur_bp->pt.p != lp)
    next_line ();
}

static int
search_forward (Line * startp, size_t starto, const char *s, int regexp)
{
  Line *lp;
  const char *sp, *sp2, *translate = fold_table (s, regexp);
  size_t s1size, s2size = strlen (s);

  if (s2size < 1)
    return false;

  for (lp = startp; lp != cur_bp->lines; lp = lp->next)
    {
      if (lp == startp)
	{
	  sp = astr_char (lp->text, (ptrdiff_t) starto);
	  s1size = astr_len (lp->text) - starto;
	}
      else
	{
	  sp = astr_cstr (lp->text);
	  s1size = astr_len (lp->text);
	}
      if (s1size < 1)
	continue;

      if (regexp)
	sp2 = re_find_substr (sp, s1size, s, s2size,
			      sp == astr_cstr (lp->text), true, false,
			      translate);
      else
	sp2 = find_substr (sp, s1size, s, s2size, translate);

      if (sp2 != NULL)
	{
	  goto_linep (lp);
	  cur_bp->pt.o = sp2 - astr_cstr (lp->text);
	  return true;
	}
    }

  return false;
}

static int
search_backward (Line * startp, size_t starto, const char *s, int regexp)
{
  Line *lp;
  const char *sp, *sp2, *translate = fold_table (s, regexp);
  size_t s1size, ssize = strlen (s);

  if (ssize < 1)
    return false;

  for (lp = startp; lp != cur_bp->lines; lp = lp->prev)
    {
      sp = astr_cstr (lp->text);
      if (lp == startp)
	s1size = starto;
      else
	s1size = astr_len (lp->text);
      if (s1size < 1)
	continue;

      if (regexp)
	sp2 = re_find_substr (sp, s1size, s, ssize,
			      true, s1size == astr_len (lp->text), true,
			      translate);
      else
	sp2 = rfind_substr (sp, s1size, s, ssize, translate);

      if (sp2 != NULL)
	{
	  goto_linep (lp);
	  cur_bp->pt.o = sp2 - astr_cstr (lp->text);
	  return true;
	}
    }

  return false;
}

static char *last_search = NULL;

typedef int (*Searcher) (Line * startp, size_t starto, const char *s, int regexp);

static le *
search (Searcher searcher, bool regexp, const char *search_msg)
{
  char *ms = minibuf_read (search_msg, last_search);

  if (ms == NULL)
    return FUNCALL (keyboard_quit);
  if (ms[0] == '\0')
    {
      free (ms);
      return leNIL;
    }

  free (last_search);
  last_search = xstrdup (ms);

  if (!searcher (cur_bp->pt.p, cur_bp->pt.o, ms, regexp))
    {
      minibuf_error ("Search failed: \"%s\"", ms);
      free (ms);
      return leNIL;
    }

  free (ms);
  return leT;
}

DEFUN ("search-forward", search_forward)
/*+
Search forward from point for the user specified text.
+*/
{
  return search (search_forward, false, "Search: ");
}
END_DEFUN

DEFUN ("search-backward", search_backward)
/*+
Search backward from point for the user specified text.
+*/
{
  return search (search_backward, false, "Search backward: ");
}
END_DEFUN

DEFUN ("search-forward-regexp", search_forward_regexp)
/*+
Search forward from point for regular expression REGEXP.
+*/
{
  return search (search_forward, true, "RE search: ");
}
END_DEFUN

DEFUN ("search-backward-regexp", search_backward_regexp)
/*+
Search backward from point for match for regular expression REGEXP.
+*/
{
  return search (search_backward, true, "RE search backward: ");
}
END_DEFUN

#define ISEARCH_FORWARD		1
#define ISEARCH_BACKWARD	2
/*
 * Incremental search engine.
 */
static le *
isearch (int dir, int regexp)
{
  int c;
  int last = true;
  astr buf = astr_new ();
  astr pattern = astr_new ();
  Point start, cur;
  Marker *old_mark = cur_wp->bp->mark ? copy_marker (cur_wp->bp->mark) : NULL;

  start = cur_bp->pt;
  cur = cur_bp->pt;

  /* I-search mode. */
  cur_wp->bp->flags |= BFLAG_ISEARCH;

  for (;;)
    {
      /* Make the minibuf message. */
      astr_truncate (buf, 0);
      astr_afmt (buf, "%sI-search%s: %s",
		 (last ?
		  (regexp ? "Regexp " : "") :
		  (regexp ? "Failing regexp " : "Failing ")),
		 (dir == ISEARCH_FORWARD) ? "" : " backward",
		 astr_cstr (pattern));

      /* Regex error. */
      if (re_find_err)
	{
	  if ((strncmp (re_find_err, "Premature ", 10) == 0) ||
	      (strncmp (re_find_err, "Unmatched ", 10) == 0) ||
	      (strncmp (re_find_err, "Invalid ", 8) == 0))
	    {
	      re_find_err = "incomplete input";
	    }
	  astr_afmt (buf, " [%s]", re_find_err);
	  re_find_err = NULL;
	}

      minibuf_write ("%s", astr_cstr (buf));

      c = getkey ();

      if (c == KBD_CANCEL)
	{
	  cur_bp->pt = start;
	  thisflag |= FLAG_NEED_RESYNC;

	  /* Quit. */
	  FUNCALL (keyboard_quit);

	  /* Restore old mark position. */
	  if (cur_bp->mark)
	    free_marker (cur_bp->mark);

	  if (old_mark)
	    cur_bp->mark = copy_marker (old_mark);
	  else
	    cur_bp->mark = old_mark;
	  break;
	}
      else if (c == KBD_BS)
	{
	  if (astr_len (pattern) > 0)
	    {
	      astr_truncate (pattern, -1);
	      cur = cur_bp->pt = start;
	      thisflag |= FLAG_NEED_RESYNC;
	    }
	  else
	    ding ();
	}
      else if (c & KBD_CTRL && ((c & 0xff) == 'r' || (c & 0xff) == 's'))
	{
	  /* Invert direction. */
	  if ((c & 0xff) == 'r' && dir == ISEARCH_FORWARD)
	    dir = ISEARCH_BACKWARD;
	  else if ((c & 0xff) == 's' && dir == ISEARCH_BACKWARD)
	    dir = ISEARCH_FORWARD;
	  if (astr_len (pattern) > 0)
	    {
	      /* Find next match. */
	      cur = cur_bp->pt;
	      /* Save search string. */
              free (last_search);
	      last_search = xstrdup (astr_cstr (pattern));
	    }
	  else if (last_search != NULL)
	    astr_cpy_cstr (pattern, last_search);
	}
      else if (c & KBD_META || c & KBD_CTRL || c > KBD_TAB)
	{
	  if (c == KBD_RET && astr_len (pattern) == 0)
	    {
	      if (dir == ISEARCH_FORWARD)
		{
		  if (regexp)
		    FUNCALL (search_forward_regexp);
		  else
		    FUNCALL (search_forward);
		}
	      else
		{
		  if (regexp)
		    FUNCALL (search_backward_regexp);
		  else
		    FUNCALL (search_backward);
		}
	    }
	  else
	    {
	      if (astr_len (pattern) > 0)
		{
		  /* Save mark. */
		  set_mark ();
		  cur_bp->mark->pt = start;

		  /* Save search string. */
                  free (last_search);
		  last_search = xstrdup (astr_cstr (pattern));

		  minibuf_write ("Mark saved when search started");
		}
	      else
		minibuf_clear ();
	      if (c != KBD_RET)
		ungetkey (c);
	    }
	  break;
	}
      else
	astr_cat_char (pattern, c);

      if (astr_len (pattern) > 0)
	{
	  if (dir == ISEARCH_FORWARD)
	    last = search_forward (cur.p, cur.o, astr_cstr (pattern), regexp);
	  else
	    last =
	      search_backward (cur.p, cur.o, astr_cstr (pattern), regexp);
	}
      else
	last = true;

      if (thisflag & FLAG_NEED_RESYNC)
	resync_redisplay ();
    }

  /* done */
  cur_wp->bp->flags &= ~BFLAG_ISEARCH;

  astr_delete (buf);
  astr_delete (pattern);

  if (old_mark)
    free_marker (old_mark);

  return leT;
}

DEFUN ("isearch-forward", isearch_forward)
/*+
Do incremental search forward.
With a prefix argument, do an incremental regular expression search instead.
As you type characters, they add to the search string and are found.
Type return to exit, leaving point at location found.
Type C-s to search again forward, C-r to search again backward.
C-g when search is successful aborts and moves point to starting point.
+*/
{
  return isearch (ISEARCH_FORWARD, (lastflag & FLAG_SET_UNIARG));
}
END_DEFUN

DEFUN ("isearch-backward", isearch_backward)
/*+
Do incremental search backward.
With a prefix argument, do a regular expression search instead.
As you type characters, they add to the search string and are found.
Type return to exit, leaving point at location found.
Type C-r to search again backward, C-s to search again forward.
C-g when search is successful aborts and moves point to starting point.
+*/
{
  return isearch (ISEARCH_BACKWARD, (lastflag & FLAG_SET_UNIARG));
}
END_DEFUN

DEFUN ("isearch-forward-regexp", isearch_forward_regexp)
/*+
Do incremental search forward for regular expression.
With a prefix argument, do a regular string search instead.
Like ordinary incremental search except that your input
is treated as a regexp.  See M-x isearch-forward for more info.
+*/
{
  return isearch (ISEARCH_FORWARD, !(lastflag & FLAG_SET_UNIARG));
}
END_DEFUN

DEFUN ("isearch-backward-regexp", isearch_backward_regexp)
/*+
Do incremental search forward for regular expression.
With a prefix argument, do a regular string search instead.
Like ordinary incremental search except that your input
is treated as a regexp.  See M-x isearch-forward for more info.
+*/
{
  return isearch (ISEARCH_BACKWARD, !(lastflag & FLAG_SET_UNIARG));
}
END_DEFUN

DEFUN ("replace-string", replace_string)
/*+
Replace occurrences of a string with other text.
+*/
{
  int count = 0, find_no_upper;
  size_t find_len, repl_len;
  char *find = minibuf_read ("Replace string: ", "");
  char *repl;

  if (find == NULL)
    return FUNCALL (keyboard_quit);
  if (find[0] == '\0')
    {
      free (find);
      return leNIL;
    }
  find_len = strlen (find);
  find_no_upper = no_upper (find, find_len, false);

  repl = minibuf_read ("Replace `%s' with: ", "", find);
  if (repl == NULL)
    {
      free (find);
      return FUNCALL (keyboard_quit);
    }
  repl_len = strlen (repl);

  while (search_forward (cur_bp->pt.p, cur_bp->pt.o, find, false))
    {
      ++count;
      undo_save (UNDO_REPLACE_BLOCK,
		 make_point (cur_bp->pt.n,
			     cur_bp->pt.o - find_len), find_len, repl_len);
      line_replace_text (&cur_bp->pt.p, cur_bp->pt.o - find_len,
			 find_len, repl, repl_len, find_no_upper);
    }

  free (find);
  free (repl);

  if (thisflag & FLAG_NEED_RESYNC)
    resync_redisplay ();
  term_redisplay ();

  minibuf_write ("Replaced %d occurrences", count);

  return leT;
}
END_DEFUN

DEFUN ("query-replace", query_replace)
/*+
Replace occurrences of a string with other text.
As each match is found, the user must type a character saying
what to do with it.
+*/
{
  int count = 0, noask = false, exitloop = false, find_no_upper;
  size_t find_len, repl_len;
  char *find = minibuf_read ("Query replace string: ", "");
  char *repl;

  if (find == NULL)
    return FUNCALL (keyboard_quit);
  if (*find == '\0')
    {
      free (find);
      return leNIL;
    }
  find_len = strlen (find);
  find_no_upper = no_upper (find, find_len, false);

  repl = minibuf_read ("Query replace `%s' with: ", "", find);
  if (repl == NULL)
    {
      free (find);
      return FUNCALL (keyboard_quit);
    }
  repl_len = strlen (repl);

  /* Spaghetti code follows... :-( */
  while (search_forward (cur_bp->pt.p, cur_bp->pt.o, find, false))
    {
      if (!noask)
	{
	  int c;
	  if (thisflag & FLAG_NEED_RESYNC)
	    resync_redisplay ();
	  for (;;)
	    {
	      minibuf_write
		("Query replacing `%s' with `%s' (y, n, !, ., q)? ", find,
		 repl);
	      c = getkey ();
	      if (c == KBD_CANCEL || c == KBD_RET || c == ' ' || c == 'y'
		  || c == 'n' || c == 'q' || c == '.' || c == '!')
		goto exitloop;
	      minibuf_error ("Please answer y, n, !, . or q.");
	      waitkey (WAITKEY_DEFAULT);
	    }
	exitloop:
	  minibuf_clear ();

	  switch (c)
	    {
	    case KBD_CANCEL:	/* C-g */
	      return FUNCALL (keyboard_quit);
	    case 'q':		/* Quit immediately. */
	      goto endoffunc;
	    case '.':		/* Replace and quit. */
	      exitloop = true;
	      goto replblock;
	    case '!':		/* Replace all without asking. */
	      noask = true;
	      goto replblock;
	    case ' ':		/* Replace. */
	    case 'y':
	      goto replblock;
	      break;
	    case 'n':		/* Do not replace. */
	    case KBD_RET:
	    case KBD_DEL:
	      goto nextmatch;
	    }
	}

    replblock:
      ++count;
      undo_save (UNDO_REPLACE_BLOCK,
		 make_point (cur_bp->pt.n,
			     cur_bp->pt.o - find_len), find_len, repl_len);
      line_replace_text (&cur_bp->pt.p, cur_bp->pt.o - find_len,
			 find_len, repl, repl_len, find_no_upper);
    nextmatch:
      if (exitloop)
	break;
    }

endoffunc:
  free (find);
  free (repl);

  if (thisflag & FLAG_NEED_RESYNC)
    resync_redisplay ();
  term_redisplay ();

  minibuf_write ("Replaced %d occurrences", count);

  return leT;
}
END_DEFUN
