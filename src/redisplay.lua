-- Terminal independent redisplay routines
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

function recenter (wp)
  local n = offset_to_line (wp.bp, window_o (wp))

  if n > wp.eheight / 2 then
    wp.topdelta = math.floor (wp.eheight / 2)
  else
    wp.topdelta = n
  end
end

Defun ("recenter",
       {},
[[
Center point in selected window and redisplay frame.
]],
  true,
  function ()
    recenter (cur_wp)
    term_clear ()
    term_redisplay ()
    term_refresh ()
    return true
  end
)

function resize_windows ()
  local wp

  -- Resize windows horizontally.
  for _, wp in ipairs (windows) do
    wp.fwidth = term_width ()
    wp.ewidth = wp.fwidth
  end

  -- Work out difference in window height; windows may be taller than
  -- terminal if the terminal was very short.
  local hdelta = term_height () - 1
  for _, wp in ipairs (windows) do
    hdelta = hdelta - wp.fheight
  end

  -- Resize windows vertically.
  if hdelta > 0 then
    -- Increase windows height.
    local w = #windows
    while hdelta > 0 do
      windows[w].fheight = windows[w].fheight + 1
      windows[w].eheight = windows[w].eheight + 1
      hdelta = hdelta - 1
      w = w - 1
      if w == 0 then
        w = #windows
      end
    end
  else
    -- Decrease windows' height, and close windows if necessary.
    local decreased
    repeat
      local w = #windows
      decreased = false
      while w > 0 and hdelta < 0 do
        local wp = windows[w]
        if wp.fheight > 2 then
          wp.fheight = wp.fheight - 1
          wp.eheight = wp.eheight - 1
          hdelta = hdelta + 1
          decreased = true
        elseif #windows > 1 then
          delete_window (wp)
          w = w - 1
          decreased = true
        end
      end
    until decreased == false
  end

  execute_function ("recenter")
end
