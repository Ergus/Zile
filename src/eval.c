/* Lisp eval

   Copyright (c) 2001-2011 Free Software Foundation, Inc.

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
#include <stdio.h>
#include <stdlib.h>

#include "eval.h"

#include "main.h"
#include "extern.h"

#include "completion.h"
#include "bind.h"
#include "variables.h"
#include "minibuf.h"
#include "history.h"

// Zile Lisp functions.

static fentry fentry_table[] = {
#define X(zile_name, c_name, interactive, doc)   \
  {zile_name, F_ ## c_name, interactive, doc},
#include "tbl_funcs.h"
#undef X
  {NULL, NULL, false, NULL}
};

static _GL_ATTRIBUTE_PURE fentry *
get_fentry (const char *name)
{
  assert (name);
  for (fentry *it = fentry_table; it->name != NULL; ++it)
    if (STREQ (name, it->name))
      return it;
  return NULL;
}

Function
get_function (const char *name)
{
  fentry * f = get_fentry (name);
  return f ? f->func : NULL;
}

/* Return function's interactive flag, or -1 if not found. */
bool
get_function_interactive (const char *name)
{
  fentry * f = get_fentry (name);
  return f ? f->interactive : -1;
}

const char *
get_function_doc (const char *name)
{
  fentry * f = get_fentry (name);
  return f ? f->doc : NULL;
}

const char *
get_function_name (Function p)
{
  for (fentry *it = fentry_table; it->name != NULL; ++it)
    if (it->func == p)
      return it->name;
  return NULL;
}

size_t
countNodes (le * branch)
{
  int count;
  for (count = 0; branch; branch = branch->next, count++)
    ;
  return count;
}

static le *
evaluateBranch (le * trybranch)
{
  le *keyword;
  fentry * func;

  if (trybranch == NULL)
    return NULL;

  if (trybranch->branch)
    keyword = evaluateBranch (trybranch->branch);
  else
    keyword = leNew (trybranch->data);

  if (keyword->data == NULL)
    return leNIL;

  func = get_fentry (keyword->data);
  if (func)
    return call_command (func->func, 1, false, trybranch);

  return NULL;
}

static le *
evaluateNode (le * node)
{
  le *value;

  if (node == NULL)
    return leNIL;

  if (node->branch != NULL)
    {
      if (node->quoted)
        value = leDup (node->branch);
      else
        value = evaluateBranch (node->branch);
    }
  else
    {
      const char *s = get_variable (node->data);
      value = leNew (s ? s : node->data);
    }

  return value;
}

DEFUN_NONINTERACTIVE ("setq", setq)
/*+
(setq [sym val]...)

Set each sym to the value of its val.
The symbols sym are variables; they are literal (not evaluated).
The values val are expressions; they are evaluated.
+*/
{
  le *newvalue = leNIL;

  if (arglist != NULL && countNodes (arglist) >= 2)
    {
      for (le *current = arglist->next; current;
           current = current->next->next)
        {
          newvalue = evaluateNode (current->next);
          set_variable (current->data, newvalue->data);
          if (current->next == NULL)
            break; /* Cope with odd-length argument lists. */
        }
    }

  ok = newvalue;
}
END_DEFUN

void
leEval (le * list)
{
  for (; list; list = list->next)
    evaluateBranch (list->branch);
}

le *
execute_with_uniarg (int uniarg, bool (*forward) (void), bool (*backward) (void))
{
  if (backward && uniarg < 0)
    {
      forward = backward;
      uniarg = -uniarg;
    }
  bool ret = true;
  for (int uni = 0; ret && uni < uniarg; ++uni)
    ret = forward ();

  return bool_to_lisp (ret);
}

le *
move_with_uniarg (int uniarg, bool (*move) (ptrdiff_t dir))
{
  bool ret = true;
  for (unsigned long uni = 0; ret && uni < (unsigned) abs (uniarg); ++uni)
    ret = move (uniarg < 0 ? - 1 : 1);
  return bool_to_lisp (ret);
}

le *
execute_function (const char *name, int uniarg, bool is_uniarg)
{
  Function func = get_function (name);
  return func ? call_command (func, uniarg, is_uniarg, NULL) : NULL;
}

DEFUN ("execute-extended-command", execute_extended_command)
/*+
Read function name, then read its arguments and call it.
+*/
{
  astr msg = astr_new ();

  if (global.lastflag & FLAG_SET_UNIARG)
    {
      if (global.lastflag & FLAG_UNIARG_EMPTY)
        msg = astr_fmt ("C-u ");
      else
        msg = astr_fmt ("%ld ", uniarg);
    }
  astr_cat_cstr (msg, "M-x ");

  const_astr name = minibuf_read_function_name ("%s", astr_cstr (msg));
  if (name == NULL)
    return false;

  ok = bool_to_lisp (execute_function (astr_cstr (name), uniarg, global.lastflag & FLAG_SET_UNIARG) == leT);
}
END_DEFUN

/*
 * Read a function name from the minibuffer.
 */
static History functions_history = NULL;

const_astr
minibuf_read_function_name (const char *fmt, ...)
{
  va_list ap;
  Completion cp = completion_new (false);

  for (fentry *it = fentry_table; it->name != NULL; ++it)
    if (it->interactive)
      gl_sortedlist_add (get_completion_completions (cp),
                         completion_strcmp,
                         xstrdup (it->name));

  va_start (ap, fmt);
  const_astr ms = minibuf_vread_completion (fmt, "", cp, functions_history,
                                            "No function name given",
                                            minibuf_test_in_completions,
                                            "Undefined function name `%s'",
                                            ap);
  va_end (ap);

  return ms;
}

void
init_eval (void)
{
  functions_history = history_new ();
}
