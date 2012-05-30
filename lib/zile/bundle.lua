-- Copyright (c) 2012-2014 Free Software Foundation, Inc,
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


local posix    = require "posix"
local rex_onig = require "rex_onig"
local empty    = require "std.table".empty

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


-- Figure out where we're looking for grammar files.
local PATH_GRAMMARDIR

for m in package.path:gmatch ("([^" .. esc (pathsep) .. "]+)") do
  local sentinel = ("zile.grammar.lua-bundle"):gsub ("%.", dirsep)
  local path = m:gsub (esc (path_mark), sentinel)
  local fh, err = io.open (path, "r")
  if fh then
    PATH_GRAMMARDIR = path:match ("^(.*)" .. dirsep .. "lua%-bundle.*$")
    break
  end
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


--- Return attributes for most complete match against scope name.
-- @string name full scope name
-- @treturn int attributes for closest matching scope.
local function scopetoattr (name)
  if not name then return nil end

  local key = {}
  for w in name:gmatch "[^%.]+" do
    table.insert (key, w)
  end

  repeat
    local scope = table.concat (key, ".")
    if colors[scope] then
      return colors[scope]
    end
    table.remove (key)
  until #key == 0
end


--- Calculate attributes for capture elements.
-- @tparam table captures capture elements
local function capturestoattr (captures)
  if not captures then return nil end

  for i, t in pairs (captures) do
    captures[i] = scopetoattr (t.name)
  end

  if not empty (captures) then return captures end
end


--- Compile a valid oniguruma match expression.
-- @string match an oniguruma regular expression string
-- @treturn userdata compiled expression if valid, otherwise `nil`
local function compile_rex (match)
  local ok, rex = pcall (rex_onig.new, match, 0)
  return ok and rex or nil
end


--- Does match have any back-references?
-- @string match uncompiled regular expression
-- @treturn boolean true if any unescaped back-references are found
local function has_backrefs (match)
  return (nil ~= match:gsub ("\\[^%d]", ""):find ("\\%d"))
end


--- Precompile grammar expressions.
-- @tparam table patterns a grammar bundle `patterns` sub-table
-- @treturn table patterns, with match expressions compiled
local function compile_patterns (patterns)
  if not patterns then return nil end

  for i, v in ipairs (patterns) do
    patterns[i] = {
      rex      = compile_rex (v.match or v.begin),
      captures = capturestoattr (v.captures or v.beginCaptures),
      colors   = scopetoattr (v.name),
      patterns = compile_patterns (v.patterns),
    }

    -- Append optional sentinel to end of patterns.
    if v["end"] then
      patterns[i].patterns = patterns[i].patterns or {}

      table.insert (patterns[i].patterns, {
        finish   = has_backrefs (v["end"]) or compile_rex (v["end"]),
        captures = v.endCaptures and capturestoattr (v.endCaptures) or v.captures,
        match    = has_backrefs (v["end"]) and v["end"],
      })
    end
  end

  return patterns
end


--- Load the grammar description for modename.
-- @string modename name of a mode with this grammar
-- @treturn table grammar
function load_grammar (modename)
  local fullname = "zile.grammar." .. modename

  if package.loaded[fullname] == nil then
    local g = require (fullname)

    if g and g.patterns then
      g.patterns = compile_patterns (g.patterns)
    end
  end

  return package.loaded[fullname]
end


--- Return a map from filename extension to syntax name.
-- @treturn table filename extension to syntax name
function load_file_associations ()
  local auto_mode_alist = {}

  for _, filename in pairs (posix.dir (PATH_GRAMMARDIR)) do
    local modename = filename:match ("^(.*)%-bundle%.lua$")
    if modename then
      local g = load_grammar (modename)
      if g then
        if type (g.fileTypes) == "string" then
          auto_mode_alist[g.fileTypes] = modename
        elseif type (g.fileTypes) == "table" then
          for _,v in pairs (g.fileTypes) do
            auto_mode_alist[v] = modename
          end
        end
      end
    end
  end

  return auto_mode_alist
end


--- @export
return {
  compile_rex            = compile_rex,
  load_file_associations = load_file_associations,
  load_grammar           = load_grammar,
  set_colors             = set_colors,
}
