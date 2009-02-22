/* Program invocation, startup and shutdown

   Copyright (c) 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009 Free Software Foundation, Inc.

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

#include <ctype.h>
#include <limits.h>
#include <locale.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <signal.h>
#include "dirname.h"
#include "gl_linked_list.h"

#include "zile.h"
#include "extern.h"

#define ZILE_COPYRIGHT_STRING \
  "Copyright (C) 2009 Free Software Foundation, Inc."

/* The executable name */
char *prog_name = PACKAGE;

/* The current window; the first window in list. */
Window *cur_wp = NULL, *head_wp = NULL;
/* The current buffer; the first buffer in list. */
Buffer *cur_bp = NULL, *head_bp = NULL;

/* The global editor flags. */
int thisflag = 0, lastflag = 0;
/* The universal argument repeat count. */
int last_uniarg = 1;

static void
loop (void)
{
  for (;;)
    {
      size_t key;

      if (lastflag & FLAG_NEED_RESYNC)
        resync_redisplay ();
      term_redisplay ();
      term_refresh ();

      thisflag = lastflag & FLAG_DEFINING_MACRO;
      key = getkey ();
      minibuf_clear ();
      process_key (root_bindings, key);

      if (thisflag & FLAG_QUIT)
        break;
      if (!(thisflag & FLAG_SET_UNIARG))
        last_uniarg = 1;

      if (last_command () != F_undo)
        cur_bp->next_undop = cur_bp->last_undop;

      lastflag = thisflag;
    }
}

static char about_splash_str[] = "\
" ZILE_VERSION_STRING "\n\
\n\
" ZILE_COPYRIGHT_STRING "\n\
\n\
Type `C-x C-c' to exit " PACKAGE_NAME ".\n\
Type `C-x u; to undo changes.\n\
Type `C-g' at any time to quit the current operation.\n\
\n\
`C-x' means hold the CTRL key while typing the character `x'.\n\
`M-x' means hold the META or ALT key down while typing `x'.\n\
If there is no META or ALT key, instead press and release\n\
the ESC key and then type `x'.\n\
Combinations like `C-x u' mean first press `C-x', then `u'.\n\
";

static char about_minibuf_str[] = "Welcome to " PACKAGE_NAME "!";

static void
about_screen (void)
{
  minibuf_write (about_minibuf_str);
  if (!get_variable_bool ("inhibit-splash-screen"))
    {
      show_splash_screen (about_splash_str);
      term_refresh ();
      waitkey (20 * 10);
    }
}

static void
load_files (gl_list_t files)
{
  size_t i;
  for (i = 0; i < gl_list_size (files); i++)
    {
      char *file = (char *) gl_list_get_at (files, i);
      term_redisplay ();
      lastflag |= FLAG_NEED_RESYNC;
      if (!lisp_loadfile (file))
        {
          minibuf_error ("Cannot open load file: %s\n", file);
          break;
        }
      if (thisflag & FLAG_QUIT)
        break;
    }
}

static void
setup_main_screen (int argc)
{
  Buffer *bp, *last_bp = NULL;
  int c = 0;

  for (bp = head_bp; bp; bp = bp->next)
    {
      /* Last buffer that isn't *scratch*. */
      if (bp->next && !bp->next->next)
        last_bp = bp;
      c++;
    }

  /* *scratch* and two files. */
  if (c == 3)
    {
      FUNCALL (split_window);
      switch_to_buffer (last_bp);
      FUNCALL (other_window);
    }
  /* More than two files. */
  else if (c > 3)
    FUNCALL (list_buffers);
  else
    {
      if (argc < 1)
        {
          undo_nosave = true;

          if (!get_variable_bool ("inhibit-splash-screen"))
            {
              astr as = astr_new_cstr ("\
This buffer is for notes you don't want to save.\n\
If you want to create a file, visit that file with C-x C-f,\n\
then enter the text in that file's own buffer.\n\
\n");
              insert_astr (as);
              astr_delete (as);
            }

          undo_nosave = false;
          cur_bp->flags &= ~BFLAG_MODIFIED;
          lastflag |= FLAG_NEED_RESYNC;
        }
    }
}

static void
segv_sig_handler (int signo GCC_UNUSED)
{
  fprintf (stderr,
           "%s: " PACKAGE_NAME
           " crashed.  Please send a bug report to <"
           PACKAGE_BUGREPORT ">.\r\n",
           prog_name);
  zile_exit (true);
}

static void
other_sig_handler (int signo GCC_UNUSED)
{
  fprintf (stderr, "%s: terminated with signal %d.\r\n",
           prog_name, signo);
  zile_exit (false);
}

static void
signal_init (void)
{
  /* Set up signal handling */
  signal (SIGSEGV, segv_sig_handler);
  signal (SIGBUS, segv_sig_handler);
  signal (SIGHUP, other_sig_handler);
  signal (SIGINT, other_sig_handler);
  signal (SIGQUIT, other_sig_handler);
  signal (SIGTERM, other_sig_handler);
}

/* Options table */
struct option longopts[] = {
#define X(longname, shortname, arg, argstring, docstring) \
  {longname, arg, NULL, shortname},
#include "tbl_opts.h"
#undef X
  {0, 0, 0, 0}
};

int
main (int argc, char **argv)
{
  int c, qflag = false;
  gl_list_t l_args = gl_list_create_empty (GL_LINKED_LIST,
                                          NULL, NULL, NULL, false);
  size_t line;

  /* Set prog_name to executable name, if available */
  if (argv[0])
    prog_name = base_name (argv[0]);

  /* Set up Lisp environment now so it's available to files and
     expressions specified on the command-line. */
  init_search ();
  init_lisp ();
  init_variables ();
  init_eval ();

  opterr = 0;			/* Don't display errors for unknown options */
  for (;;)
    {
      int this_optind = optind ? optind : 1;
      char *buf;

      /* Leading : so as to return ':' for a missing arg, not '?' */
      c = getopt_long (argc, argv, ":l:q", longopts, NULL);

      if (c == -1)
        break;

      switch (c)
        {
        case 'q':
          qflag = true;
          break;
        case 'v':
          printf (ZILE_VERSION_STRING "\n"
                  ZILE_COPYRIGHT_STRING "\n"
                  "GNU " PACKAGE_NAME " comes with ABSOLUTELY NO WARRANTY.\n"
                  "You may redistribute copies of " PACKAGE_NAME "\n"
                  "under the terms of the GNU General Public License.\n"
                  "For more information about these matters, see the file named COPYING.\n");
          return 0;
        case 'h':
          printf ("Usage: %s [OPTION-OR-FILENAME]...\n"
                  "\n"
                  "Run " PACKAGE_NAME ", the lightweight Emacs clone.\n"
                  "\n"
                  "Initialization options:\n"
                  "\n",
                  prog_name);
#define X(longname, shortname, arg, argstring, docstring) \
          xasprintf (&buf, "--%s, -%c %s", longname, shortname, argstring); \
          printf ("%-24s%s\n", buf, docstring);                          \
          free (buf);
#include "tbl_opts.h"
#undef X
          printf ("\n"
                  "Action options:\n"
                  "\n"
                  "FILE                    visit FILE using find-file\n"
                  "+LINE FILE              visit FILE using find-file, then go to line LINE\n"
                  "\n"
                  "Report bugs to " PACKAGE_BUGREPORT ".\n");
          return 0;
        case 'l':
          gl_list_add_last (l_args, (void *) optarg);
          break;
        case '?':		/* Unknown option */
          minibuf_error ("Unknown option `%s'", argv[this_optind]);
          break;
        case ':':		/* Missing argument */
          fprintf (stderr, "%s: Option `%s' requires an argument\n",
                   prog_name, argv[this_optind]);
          exit (1);
        }
    }

  argc -= optind;
  argv += optind;

  signal_init ();

  setlocale (LC_ALL, "");

  init_default_bindings ();
  init_minibuf ();

  term_init ();

  /* Create the `*scratch*' buffer. */
  create_first_window ();
  term_redisplay ();

  /* Read settings after creating *scratch* buffer so that any
     buffer commands won't cause a crash. */
  if (!qflag)
    {
      astr as = get_home_dir ();
      if (as)
        {
          astr_cat_cstr (as, "/." PACKAGE);
          lisp_loadfile (astr_cstr (as));
          astr_delete (as);
        }
    }

  /* Reinitialise the scratch buffer to catch settings */
  init_buffer (cur_bp);

  /* Open files given on the command line */
  for (line = 1; *argv; argv++)
    {
      if (**argv == '+')
        line = strtoul (*argv + 1, NULL, 10);
      else
        {
          find_file (*argv);
          ngotodown (line - 1);
          line = 1;
          lastflag |= FLAG_NEED_RESYNC;
        }
    }

  /* Show the splash screen only if no files and no Lisp expression
     or load file is specified on the command line. */
  if (minibuf_contents == NULL && argc == 0 && gl_list_size (l_args) == 0)
    about_screen ();

  setup_main_screen (argc);

  /* Refresh minibuffer in case there's an error that couldn't be
     written during startup */
  if (minibuf_contents != NULL)
    {
      char *buf = xstrdup (minibuf_contents);

      minibuf_write (buf);
      free (buf);
    }

  load_files (l_args);
  gl_list_free (l_args);

  /* Run the main loop. */
  if (!(thisflag & FLAG_QUIT))
    loop ();

  /* Tidy and close the terminal. */
  term_finish ();

  free_bindings (root_bindings);
  free_eval ();

  /* Free Lisp state. */
  free_variables ();
  free_lisp ();

  /* Free all the memory allocated. */
  free_kill_ring ();
  free_registers ();
  free_macros ();
  free_windows ();
  free_buffers ();
  free_minibuf ();
  free (prog_name);

  return 0;
}
