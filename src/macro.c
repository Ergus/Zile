/* Macro facility functions

   Copyright (c) 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2008, 2009, 2010 Free Software Foundation, Inc.

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
#include <string.h>
#include "gl_array_list.h"

#include "main.h"
#include "extern.h"


struct Macro
{
#define FIELD(ty, name) ty name;
#include "macro.h"
#undef FIELD
};

#define FIELD(ty, field)                              \
  static GETTER (Macro, macro, ty, field)             \
  static SETTER (Macro, macro, ty, field)

#include "macro.h"
#undef FIELD
#undef FIELD_STR


static Macro *cur_mp, *cmd_mp = NULL, *head_mp = NULL;

static Macro *
macro_new (void)
{
  Macro * mp = XZALLOC (Macro);
  set_macro_keys (mp, gl_list_create_empty (GL_ARRAY_LIST,
                                            NULL, NULL, NULL, true));
  return mp;
}

static void
macro_delete (Macro * mp)
{
  if (mp)
    {
      gl_list_free (get_macro_keys (mp));
      free (get_macro_name (mp));
      free (mp);
    }
}

static void
add_macro_key (Macro * mp, size_t key)
{
  gl_list_add_last (get_macro_keys (mp), (void *) key);
}

static void
remove_macro_key (Macro * mp)
{
  gl_list_remove_at (get_macro_keys (mp), gl_list_size (get_macro_keys (mp)) - 1);
}

static void
append_key_list (Macro *to, Macro *from)
{
  size_t i;

  for (i = 0; i < gl_list_size (from->keys); i++)
    add_macro_key (to, (size_t) gl_list_get_at (from->keys, i));
}

void
add_cmd_to_macro (void)
{
  assert (cmd_mp);
  append_key_list (cur_mp, cmd_mp);
  macro_delete (cmd_mp);
  cmd_mp = NULL;
}

void
add_key_to_cmd (size_t key)
{
  if (cmd_mp == NULL)
    cmd_mp = macro_new ();

  add_macro_key (cmd_mp, key);
}

void
remove_key_from_cmd (void)
{
  assert (cmd_mp);
  remove_macro_key (cmd_mp);
}

void
cancel_kbd_macro (void)
{
  macro_delete (cmd_mp);
  macro_delete (cur_mp);
  cmd_mp = cur_mp = NULL;
  thisflag &= ~FLAG_DEFINING_MACRO;
}

DEFUN ("start-kbd-macro", start_kbd_macro)
/*+
Record subsequent keyboard input, defining a keyboard macro.
The commands are recorded even as they are executed.
Use @kbd{C-x )} to finish recording and make the macro available.
Use @kbd{M-x name-last-kbd-macro} to give it a permanent name.
+*/
{
  if (thisflag & FLAG_DEFINING_MACRO)
    {
      minibuf_error ("Already defining a keyboard macro");
      return leNIL;
    }

  if (cur_mp)
    cancel_kbd_macro ();

  minibuf_write ("Defining keyboard macro...");

  thisflag |= FLAG_DEFINING_MACRO;
  cur_mp = macro_new ();
}
END_DEFUN

DEFUN ("end-kbd-macro", end_kbd_macro)
/*+
Finish defining a keyboard macro.
The definition was started by @kbd{C-x (}.
The macro is now available for use via @kbd{C-x e}.
+*/
{
  if (!(thisflag & FLAG_DEFINING_MACRO))
    {
      minibuf_error ("Not defining a keyboard macro");
      return leNIL;
    }

  thisflag &= ~FLAG_DEFINING_MACRO;
}
END_DEFUN

DEFUN ("name-last-kbd-macro", name_last_kbd_macro)
/*+
Assign a name to the last keyboard macro defined.
Argument SYMBOL is the name to define.
The symbol's function definition becomes the keyboard macro string.
Such a \"function\" cannot be called from Lisp, but it is a valid editor command.
+*/
{
  Macro *mp;
  char *ms = minibuf_read ("Name for last kbd macro: ", "");

  if (ms == NULL)
    {
      minibuf_error ("No command name given");
      return leNIL;
    }

  if (cur_mp == NULL)
    {
      minibuf_error ("No keyboard macro defined");
      return leNIL;
    }

  mp = get_macro (ms);
  if (mp)
    /* If a macro with this name already exists, update its key list */
    free (get_macro_keys (mp));
  else
    {
      /* Add a new macro to the list */
      mp = macro_new ();
      set_macro_next (mp, head_mp);
      set_macro_name (mp, xstrdup (ms));
      head_mp = mp;
    }

  /* Copy the keystrokes from cur_mp. */
  append_key_list (mp, cur_mp);

  free(ms);
}
END_DEFUN

static void
process_keys (gl_list_t keys)
{
  size_t i, len = gl_list_size (keys), cur;

  (void) CLUE_DO (L, "cur = term_buf_len ()");
  (void) CLUE_DO (L, "io.stderr:write ('term_buf_len ' .. tostring(cur) .. '\\n')");
  CLUE_GET (L, cur, integer, cur);

  for (i = 0; i < len; i++)
    pushkey ((size_t) gl_list_get_at (keys, len - i - 1));

  while (true)
    {
      size_t newcur;
      (void) CLUE_DO (L, "newcur = term_buf_len ()");
      CLUE_GET (L, newcur, integer, newcur);
      if (newcur <= cur)
        break;
      process_command ();
    }
}

void
call_macro (Macro * mp)
{
  assert (mp);
  assert (get_macro_keys (mp));
  process_keys (get_macro_keys (mp));
}

DEFUN ("call-last-kbd-macro", call_last_kbd_macro)
/*+
Call the last keyboard macro that you defined with @kbd{C-x (}.
A prefix argument serves as a repeat count.

To make a macro permanent so you can call it even after
defining others, use @kbd{M-x name-last-kbd-macro}.
+*/
{
  int uni;

  if (cur_mp == NULL)
    {
      minibuf_error ("No kbd macro has been defined");
      return leNIL;
    }

  undo_save (UNDO_START_SEQUENCE, get_buffer_pt (cur_bp), 0, 0);
  for (uni = 0; uni < uniarg; ++uni)
    call_macro (cur_mp);
  undo_save (UNDO_END_SEQUENCE, get_buffer_pt (cur_bp), 0, 0);
}
END_DEFUN

DEFUN_NONINTERACTIVE_ARGS ("execute-kbd-macro", execute_kbd_macro,
                   STR_ARG (keystr))
/*+
Execute macro as string of editor command characters.
+*/
{
  gl_list_t keys;

  STR_INIT (keystr);
  keys = keystrtovec (keystr);
  if (keys)
    {
      process_keys (keys);
      gl_list_free (keys);
    }
  else
    ok = leNIL;
  STR_FREE (keystr);
}
END_DEFUN

/*
 * Find a macro given its name.
 */
Macro *
get_macro (const char *name)
{
  Macro *mp;
  assert (name);
  for (mp = head_mp; mp; mp = get_macro_next (mp))
    if (!strcmp (get_macro_name (mp), name))
      return mp;
  return NULL;
}

/*
 * Add macro names to a list.
 */
void
add_macros_to_list (int l)
{
  Macro *mp;

  lua_rawgeti (L, LUA_REGISTRYINDEX, l);
  lua_setglobal (L, "cp");
  for (mp = head_mp; mp; mp = get_macro_next (mp))
    {
      CLUE_SET (L, s, string, get_macro_name (mp));
      (void) CLUE_DO (L, "table.insert (cp.completions, s)");
    }
}
