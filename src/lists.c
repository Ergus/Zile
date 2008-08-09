/* Lisp lists

   Copyright (c) 2008 Free Software Foundation, Inc.
   Copyright (c) 2001 Scott "Jerry" Lawrence.
   Copyright (c) 2005 Reuben Thomas.

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

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "zile.h"
#include "extern.h"
#include "lists.h"


le *
leNew (const char *text)
{
  le *new = (le *) XZALLOC (le);

  if (text)
    new->data = xstrdup (text);

  return new;
}

void
leReallyWipe (le * list)
{
  if (list)
    {
      /* free descendants */
      leWipe (list->branch);
      leWipe (list->next);

      /* free ourself */
      free (list->data);
      free (list);
    }
}

void
leWipe (le * list)
{
  if (list && list != leNIL && list != leT)
    leReallyWipe (list);
}

static le *
leAddTail (le * list, le * element)
{
  le *temp = list;

  /* if either element or list doesn't exist, return the `new' list */
  if (!element)
    return list;
  if (!list)
    return element;

  /* find the end element of the list */
  while (temp->next)
    temp = temp->next;

  /* tack ourselves on */
  temp->next = element;

  /* return the list */
  return list;
}

le *
leAddBranchElement (le * list, le * branch, int quoted)
{
  le *temp = leNew (NULL);
  temp->branch = branch;
  temp->quoted = quoted;
  return leAddTail (list, temp);
}

le *
leAddDataElement (le * list, const char *data, int quoted)
{
  le *newdata = leNew (data);
  newdata->quoted = quoted;
  return leAddTail (list, newdata);
}

le *
leDup (le * list)
{
  le *temp;

  if (!list)
    return NULL;

  temp = leNew (list->data);
  temp->branch = leDup (list->branch);
  temp->next = leDup (list->next);

  return temp;
}
