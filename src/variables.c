/* Zile variables handling functions

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

#include "config.h"

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "hash.h"
#include "gl_list.h"

#include "zile.h"
#include "extern.h"

/*
 * Variable type.
 */
struct var_entry
{
  const char *var;		/* Variable name. */
  const char *defval;		/* Default value. */
  const char *val;		/* Current value, if any. */
  bool local;			/* If true, becomes local when set. */
};
typedef struct var_entry var_entry;

static Hash_table *main_vars;

static size_t
var_hash (const void *v, size_t n)
{
  return hash_string (((var_entry *) v)->var, n);
}

static bool
var_cmp (const void *v, const void *w)
{
  return strcmp (((var_entry *) v)->var, ((var_entry *) w)->var) == 0;
}

static void
var_free (void *v)
{
  var_entry *p = (var_entry *) v;
  free ((char *) p->var);
  if (p->defval != p->val)
    free ((char *) p->defval);
  free ((char *) p->val);
  free (p);
}

static void
init_builtin_var (const char *var, const char *defval, bool local)
{
  var_entry *p = XMALLOC (var_entry);
  p->var = xstrdup (var);
  p->defval = p->val = xstrdup (defval);
  p->local = local;
  hash_insert (main_vars, p);
}

void
init_variables (void)
{
  /* Initial size of 32 is big enough for default variables and some
     more */
  main_vars = hash_initialize (32, NULL, var_hash, var_cmp, var_free);
#define X(var, defval, local, doc)              \
  init_builtin_var (var, defval, local);
#include "tbl_vars.h"
#undef X
}

static void
set_variable_in_list (Hash_table *var_list, const char *var, const char *val)
{
  var_entry *p = XZALLOC (var_entry), *q;

  /* Insert variable if it doesn't already exist */
  p->var = xstrdup (var);
  q = hash_insert (var_list, p);

  /* Update value */
  val = xstrdup (val);
  q->val = val;

  /* If variable is new, initialise other fields */
  if (q == p)
    {
      p->defval = val;
      p->local = false;
    }
  else
    var_free (p);
}

void
set_variable (const char *var, const char *val)
{
  set_variable_in_list (main_vars, var, val);
}

void
free_variables (void)
{
  hash_free (main_vars);
}

const char *
get_variable_bp (Buffer * bp, char *var)
{
  var_entry *p = NULL, *key = XMALLOC (var_entry);

  key->var = var;

  if (bp && bp->vars)
    p = hash_lookup (bp->vars, key);

  if (p == NULL)
    p = hash_lookup (main_vars, key);

  free (key);

  return p ? p->val : NULL;
}

const char *
get_variable (char *var)
{
  return get_variable_bp (cur_bp, var);
}

int
get_variable_number_bp (Buffer * bp, char *var)
{
  int t = 0;
  const char *s = get_variable_bp (bp, var);

  if (s)
    t = atoi (s);

  return t;
}

int
get_variable_number (char *var)
{
  return get_variable_number_bp (cur_bp, var);
}

bool
get_variable_bool (char *var)
{
  const char *p = get_variable (var);
  if (p != NULL)
    return strcmp (p, "nil") != 0;

  return false;
}

char *
minibuf_read_variable_name (char *msg)
{
  char *ms;
  Completion *cp = completion_new (false);
  var_entry *p;

  for (p = hash_get_first (main_vars);
       p != NULL;
       p = hash_get_next (main_vars, p))
    {
      gl_sortedlist_add (cp->completions, completion_strcmp,
                         xstrdup (p->var));
    }

  for (;;)
    {
      ms = minibuf_read_completion (msg, "", cp, NULL);

      if (ms == NULL)
	{
	  free_completion (cp);
	  FUNCALL (keyboard_quit);
	  return NULL;
	}

      if (ms[0] == '\0')
	{
	  free_completion (cp);
          free (ms);
	  minibuf_error ("No variable name given");
	  return NULL;
	}
      else if (get_variable (ms) == NULL)
	{
	  minibuf_error ("Undefined variable name `%s'", ms);
	  waitkey (WAITKEY_DEFAULT);
	}
      else
	{
	  minibuf_clear ();
	  break;
	}
    }

  free_completion (cp);

  return ms;
}

DEFUN ("set-variable", set_variable)
/*+
Set a variable value to the user-specified value.
+*/
{
  char *var, *val;
  struct var_entry *ent, *key = XMALLOC (var_entry);
  size_t argc = countNodes (arglist);

  if (arglist && argc >= 3)
    {
      var = arglist->next->data;
      val = arglist->next->next->data;
    }
  else
    {
      var = minibuf_read_variable_name ("Set variable: ");
      if (var == NULL)
        return leNIL;

      val = minibuf_read ("Set %s to value: ", "", var);
      if (val == NULL)
        {
          free (var);
          return FUNCALL (keyboard_quit);
        }
    }

  /* Some variables automatically become buffer-local when set in
     any fashion. */
  key->var = var;
  ent = hash_lookup (main_vars, key);
  free (key);
  if (ent->local)
    set_variable_in_list (cur_bp->vars, var, val);
  else
    set_variable (var, val);

  free (var);
  free (val);
  return leT;
}
END_DEFUN
