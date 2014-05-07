-- Copyright (c) 2012-214 Free Software Foundation, Inc,
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

--[[--
 Bundle loading utilities.

 A bundle file is just a nested table written in Lua read-syntax. This
 module adds a bundle loader to `package.loaders` so that:

     require "foo"

 will return a table loaded from the file `foo-bundle.lua` somewhere
 along `package.loadpath`.

 @module zile.bundle
]]

require "zile.term_curses"

local dirsep, pathsep, path_mark =
  string.match (package.config, "^([^\n]+)\n([^\n]+)\n([^\n]+)\n")


--- Ensure pattern meta-characters are escaped with a leading '%'.
-- @string s a string
-- @return s with pattern meta-characters escaped
local function esc (s)
  return (string.gsub (s, "[%^%$%(%)%%%.%[%]%*%+%-%?]", "%%%0"))
end


--- Search package.path for name, after passing through transform.
-- @string name required bundle name
-- @func transform make changes to name before searching
-- @treturn function returns the loaded bundle table when called
local function searcher (name, transform)
  local errbuf = {}
  for m in package.path:gmatch ("([^" .. esc (pathsep) .. "]+)") do
    local path = transform (m:gsub (esc (path_mark), (name:gsub ("%.", dirsep))))
    local fh, err = io.open (path, "r")
    if fh == nil then
      errbuf[#errbuf + 1] = "\topen file '" .. path .."' failed: " .. err
    else

      -- Found and opened...
      local bundle = fh:read "*a"
      fh:close ()
      local loadfn, err = loadstring ("return " .. bundle)
      if type (loadfn) ~= "function" then
        errbuf[#errbuf + 1] = "\tloadstring '" .. path .. "' failed: " .. err
      else
        -- ...and successfully loaded!
        return loadfn
      end
    end
  end

  -- Paths exhausted, return the list of failed attempts.
  return table.concat (errbuf, "\n")
end


-- Add a searcher that loads 'foo-bundle.lua' for `require "foo"` if
-- none of the built-in searchers could find anything to load.
package.loaders[#package.loaders] = function (name)
  return searcher (name, function (name) return name:gsub ("%.lua", "-bundle.lua") end)
end


--- Set up the display attributes for name.
-- @string name bundle to load colors from
-- @treturn table map of color names to curses attributes
local function set_colors (name)
  local t = require (name)

  -- Transform the bundle table format into a faster attribute
  -- lookup tree in `colors`.
  if t then
    local colors = {}

    colors.normal = term_get_attribute (t.settings)
    if t.settings then
      local selection = t.settings.selection
      if type (selection) ~= "table" then
	selection = { background = selection, fontStyle = 'bold' }
      end
      colors.selection = term_get_attribute (selection)
    end

    for _, v in ipairs (t) do
      colors[v.scope] = term_get_attribute (v.settings)
    end

    return colors
  end
end

--- @export
return {
  set_colors = set_colors,
}
