#ifndef EVAL_H
#define EVAL_H

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

#include "main.h"
#include "lists.h"
#include "astr.h"
#include "lisp.h"

typedef struct fentry
{
  const char *name;		/* The function name. */
  Function func;		/* The function pointer. */
  bool interactive;             /* Whether function is interactively. */
  const char *doc;		/* Documentation string. */
} fentry;


#define X(zile_name, c_name, interactive, doc)			\
  le *F_ ## c_name (long uniarg, bool is_uniarg, le * l);
#include "tbl_funcs.h"
#undef X

le *leNIL, *leT;

_GL_ATTRIBUTE_PURE size_t countNodes (le * branch);
void leEval (le * list);
le *execute_with_uniarg (int uniarg, bool (*forward) (void), bool (*backward) (void));
le *move_with_uniarg (int uniarg, bool (*move) (ptrdiff_t dir));
le *execute_function (const char *name, int uniarg, bool is_uniarg);
_GL_ATTRIBUTE_PURE Function get_function (const char *name);
_GL_ATTRIBUTE_PURE const char *get_function_doc (const char *name);
_GL_ATTRIBUTE_PURE bool get_function_interactive (const char *name);
_GL_ATTRIBUTE_PURE const char *get_function_name (Function p);
const_astr minibuf_read_function_name (const char *fmt, ...) __attribute__((format(printf, 1, 2)));
void init_eval (void);

#endif
