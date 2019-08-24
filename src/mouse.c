#include <config.h>

#ifdef MOUSE_ON

#include <stdlib.h>
#if defined HAVE_NCURSESW_CURSES_H
#  include <ncursesw/curses.h>
#elif defined HAVE_NCURSESW_H
#  include <ncursesw.h>
#elif defined HAVE_NCURSES_CURSES_H
#  include <ncurses/curses.h>
#elif defined HAVE_NCURSES_H
#  include <ncurses.h>
#elif defined HAVE_CURSES_H
#  include <curses.h>
#else
#  error "SysV or X/Open-compatible Curses header file required"
#endif
#include <term.h>

#include "main.h"
#include "extern.h"

static MEVENT mouse_event;

void
mouse_enable()
{
  mousemask(ALL_MOUSE_EVENTS, NULL);
}

size_t
mouse_codetokey()
{
  if (getmouse(&mouse_event) == OK)
    return KBD_MOUSE;

  return KBD_NOKEY;
}

bool
mouse_keytocodes(int *p)
{
  if (mouse_event.bstate)
    {
      *p++ = KEY_MOUSE;
      return true;
    }
  return false;
}

astr
mouse_chordtodesc (size_t key)
{
  astr as = astr_new ();

  if (mouse_event.bstate & BUTTON_CTRL)
    astr_cat_cstr (as, "C-");
  if (mouse_event.bstate & BUTTON_ALT)
    astr_cat_cstr (as, "M-");
  if (mouse_event.bstate & BUTTON_CTRL)
    astr_cat_cstr (as, "S-");

  if (mouse_event.bstate & BUTTON1_CLICKED)
    astr_cat_cstr (as, "<mouse-1>");

  mouse_event.bstate = 0;
  return as;
}

DEFUN ("mouse-set-point", mouse_set_point)
/*+
Set point in mouse position
+*/
{
  minibuf_write ("%d %d %d", mouse_event.x, mouse_event.y, mouse_event.z );
}
END_DEFUN


#endif
