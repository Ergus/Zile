-- History facility functions
--
-- Copyright (c) 2007, 2009, 2010 Free Software Foundation, Inc.
--
-- This file is part of GNU Zile.
--
-- GNU Zile is free software; you can redistribute it and/or modify it
-- under the terms of the GNU General Public License as published by
-- the Free Software Foundation; either version 3, or (at your option)
-- any later version.
--
-- GNU Zile is distributed in the hope that it will be useful, but
-- WITHOUT ANY WARRANTY; without even the implied warranty of
-- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
-- General Public License for more details.
--
-- You should have received a copy of the GNU General Public License
-- along with GNU Zile; see the file COPYING.  If not, write to the
-- Free Software Foundation, Fifth Floor, 51 Franklin Street, Boston,
-- MA 02111-1301, USA.

function history_new ()
  return history_prepare ({})
end

function history_prepare (hp)
  hp.sel = 0
  return hp
end

function add_history_element (hp, s)
  if hp[#hp] ~= s then
    table.insert (hp, s)
  end
end

function previous_history_element (hp)
  if hp.sel == 0 then -- First call for this history
    -- Select last element
    if #hp > 0 then
      hp.sel = #hp
      return hp[hp.sel]
    end
    minibuf_error ("Beginning of history; no preceding item")
    waitkey (WAITKEY_DEFAULT)
  elseif hp.sel > 1 then
    -- If there is there another element, select it
    hp.sel = hp.sel - 1
    return hp[hp.sel]
  end
end

function next_history_element (hp)
  -- Next element
  if hp.sel < #hp then
    hp.sel = hp.sel + 1
    return hp[hp.sel]
  else -- No more elements (back to original status)
    minibuf_error ("End of history; no defaults available")
    waitkey (WAITKEY_DEFAULT)
  end
end
