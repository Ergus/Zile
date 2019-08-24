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
  int rx, ry;

  get_window_in_position (mouse_event.x, mouse_event.y, &rx, &ry);

  move_line (ry - get_window_topdelta (cur_wp));

  size_t col = 0, i;
  const size_t t = tab_width (cur_bp);
  const size_t lino = get_buffer_line_o (cur_bp);
  const size_t len  = buffer_line_len (cur_bp, get_buffer_pt (cur_bp));

  for (i = lino; i < lino + len; i++)
    if (col >= rx)
      break;
    else if (get_buffer_char (cur_bp, i) == '\t')
      for (size_t w = t - col % t; w > 0 && ++col < rx; w--);
    else
      ++col;

  set_buffer_pt (cur_bp, i);

}
END_DEFUN


#endif
