-- Undo facility commands.
--
-- Copyright (c) 2010-2013 Free Software Foundation, Inc.
--
-- This file is part of GNU Zile.
--
-- This program is free software; you can redistribute it and/or modify it
-- under the terms of the GNU General Public License as published by
-- the Free Software Foundation; either version 3, or (at your option)
-- any later version.
--
-- This program is distributed in the hope that it will be useful, but
-- WITHOUT ANY WARRANTY; without even the implied warranty of
-- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
-- General Public License for more details.
--
-- You should have received a copy of the GNU General Public License
-- along with this program.  If not, see <http://www.gnu.org/licenses/>.


Defun ("undo",
       {},
[[
Undo some previous changes.
Repeat this command to undo more changes.
]],
  true,
  function ()
    if cur_bp.noundo then
      minibuf_error ("Undo disabled in this buffer")
      return false
    end

    if warn_if_readonly_buffer () then
      return false
    end

    if not cur_bp.next_undop then
      minibuf_error ("No further undo information")
      cur_bp.next_undop = cur_bp.last_undop
      return false
    end

    cur_bp.next_undop = revert_action (cur_bp.next_undop)
    minibuf_write ("Undo!")
  end
)


Defun ("revert-buffer",
       {},
[[
Undo until buffer is unmodified.
]],
  true,
  function ()
    -- FIXME: save pointer to current undo action and abort if we get
    -- back to it.
    while cur_bp.modified do
      execute_function ("undo")
    end
  end
)
