/* Zile variables

   Copyright (c) 2008 Free Software Foundation, Inc.
   Copyright (c) 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004 Sandro Sigala.
   Copyright (c) 2003, 2004, 2005, 2006, 2007, 2008 Reuben Thomas.
   Copyright (c) 2004 David Capello.

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

/*
 * Add an entry to this list for declaring a new global variable.
 *
 * If you make any modification, please remember to keep in sync with
 * the documentation in the `../doc/zile.texi' file.
 *
 * The first column specifies the variable name.
 * The second column specifies the default value.
 * The third column specifies whether the variable becomes local when set.
 * The fourth column specifies the variable documentation.
 */

X ("auto-fill-mode", "nil", false, "\
If non-nil, Auto Fill Mode is automatically enabled.")
X ("backup-directory", "nil", false, "\
The directory for backup files, which must exist.\n\
If this variable is nil, the backup is made in the original file's\n\
directory.\n\
This value is used only when `make-backup-files' is `t'.")
X ("case-fold-search", "t", true, "\
Non-nil means searches ignore case.")
X ("case-replace", "t", false, "\
Non-nil means `query-replace' should preserve case in replacements.")
X ("fill-column", "72", true, "\
Column beyond which automatic line-wrapping should happen.\n\
Automatically becomes buffer-local when set in any fashion.")
X ("highlight-nonselected-windows", "nil", false, "\
If non-nil, highlight region even in nonselected windows.")
X ("indent-tabs-mode", "nil", true, "\
If non-nil, insert-tab inserts \"real\" tabs; otherwise, it always inserts\n\
spaces.")
X ("inhibit-splash-screen", "nil", false, "\
Non-nil inhibits the startup screen.\n\
It also inhibits display of the initial message in the `*scratch*' buffer.")
X ("kill-whole-line", "nil", false, "\
If non-nil, `kill-line' with no arg at beg of line kills the whole line.")
X ("make-backup-files", "t", false, "\
Non-nil means make a backup of a file the first time it is saved.\n\
This is done by appending `~' to the file name. ")
X ("ring-bell", "t", false, "\
Non-nil means ring the terminal bell on any error.")
X ("standard-indent", "4", false, "\
Default number of columns for margin-changing functions to indent.")
X ("tab-always-indent", "t", false, "\
If non-nil, the TAB key always indents, otherwise it inserts a \"real\"\n\
tab when point is in the left margin or in the line's indentation.")
X ("tab-width", "8", true, "\
Distance between tab stops (for display of tab characters), in columns.\n\
Automatically becomes buffer-local when set in any fashion.")
X ("transient-mark-mode", "t", false, "\
If non-nil, deactivates the mark when the buffer contents change.\n\
Also enables highlighting of the region whenever the mark is active.\n\
The variable `highlight-nonselected-windows' controls whether to\n\
highlight all windows or just the selected window.")
