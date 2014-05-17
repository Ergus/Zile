-- Zile-specific library functions
--
-- Copyright (c) 2006-2010, 2012-2014 Free Software Foundation, Inc.
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
 Miscellaneous helper functions.

 @module zile.lib
]]

local Object = require "std.object"

local prototype = Object.prototype
local typeof    = type

--- Concatenate a table of strings using ", " and " or " delimiters.
-- @tparam table alternatives a table of strings
-- @treturn string string of elements from alternatives delimited by ", "
--   and " or "
local function concat (alternatives)
  local t, i = {}, 1
  while i < #alternatives do
    t[i] = alternatives[i]
    i = i + 1
  end
  if #alternatives > 1 then
    t[#t] = t[#t] .. " or " .. alternatives[#alternatives]
  else
    t = alternatives
  end
  return table.concat (t, ", ")
end


--- Raise a bad argument error.
-- Equivalent to luaL_argerror in the Lua C API. This function does not
-- return.
-- @string name function to callout in error message
-- @int i argument number
-- @string extramsg additional text to append to message inside parentheses
-- @int[opt] level call stack level to blame for the error
local function argerror (name, i, extramsg, level)
  local s = string.format ("bad argument #%d to '%s'", i, name)
  if extramsg ~= nil then
    s = s .. " (" .. extramsg .. ")"
  end
  return error (s, level or 1)
end


--- Check the type of an argument against expected types.
-- Equivalent to luaL_argcheck in the Lua C API.
-- Argument `actual` must match one of the types from in `expected`, each
-- of which can be the name of a primitive Lua type, a stdlib object type,
-- or one of the special options below:
--
--    #table    accept any non-empty table
--    object    accept any std.Object derived type
--    any       accept any argument type
--
-- Call lib.argerror if there is a type mismatch.
-- @string name function to blame in error message
-- @int i argument number to blame in error message
-- @tparam table|string expected a list of acceptable argument types
-- @param actual argument passed
-- @int[opt=2] level call stack level to blame for the error
local function argcheck (name, i, expected, actual, level)
  if prototype (expected) ~= "table" then expected = {expected} end

  -- Check actual has one of the types from expected
  local ok, actualtype = false, prototype (actual)
  for _, check in ipairs (expected) do
    if check == "any" then
      ok = true

    elseif check == "#table" then
      if actualtype == "table" and next (actual) then
        ok = true
      end

    elseif check == "object" then
      if actualtype ~= "table" and typeof (actual) == "table" then
        ok = true
      end

    elseif check == actualtype then
      ok = true
    end

    if ok then break end
  end

  if not ok then
    if actualtype == "nil" then actualtype = "no value" end
    expected = concat (expected):gsub ("#table", "non-empty table")
    return argerror (name, i, expected .. " expected, got " .. actualtype, level or 2)
  end
end


--- Check that all arguments match specified types.
-- @string name function to blame in error message
-- @tparam table|string expected a list of lists of acceptable argument types
-- @tparam table|any actual argument value, or table of argument values
local function argscheck (name, expected, actual)
  if typeof (expected) ~= "table" then expected = {expected} end
  if typeof (actual) ~= "table" then actual = {actual} end

  for i, v in ipairs (expected) do
    if v ~= "any" then
      argcheck (name, i, expected[i], actual[i])
    end
  end
end


--- Is `x` a zile.Symbol with a __call metamethod, or a plain function?
-- @param x anything
-- @treturn boolean true if x is a function, or a Symbol with a __call
--   metamethod, otherwise false
local function iscallable (x)
  if type (x) == "function" then return true end
  local mt = getmetatable (x)
  return mt and mt.__call and mt._type == "Symbol"
end


--- Recase str according to newcase.
-- @string s a string
-- @string newcase literal "upper", "lower" or "capitalized"
-- @treturn string a copy of `s` with case altered appropriately
local function recase (s, newcase)
  local bs = ""
  local i, len

  if newcase == "capitalized" or newcase == "upper" then
    bs = bs .. string.upper (s[1])
  else
    bs = bs .. string.lower (s[1])
  end

  for i = 2, #s do
    bs = bs .. (newcase == "upper" and string.upper or string.lower) (s[i])
  end

  return bs
end



--- Basic stack operations
-- @table stack
local stack, metatable

stack = {
  -- Pops and Pushes must balance, so instead of pushing `nil', which
  -- already means "no entry" and cause the matching pop to remove an
  -- unmatched value beneath the "missing" nil, use stack.empty:
  empty = math.huge,

  -- Return a new stack, optionally initialised with elements from t.
  new = function (t)
    return setmetatable (t or {}, metatable)
  end,

  -- Push v on top of a stack, creating an empty cell when v is nil.
  push = function (self, v)
    table.insert (self, v or stack.empty)
    return self[#self]
  end,

  -- Pop and return the top of a stack, or nil for empty cells.
  pop = function (self)
    local v = table.remove (self)
    return v ~= stack.empty and v or nil
  end,

  -- Return the value from the top of a stack, ignoring empty cells.
  top = function (self)
    if #self < 1 then return nil end
    local n = 0
    while n + 1 < #self and self[#self - n] == stack.empty do
      n = n + 1
    end
    assert (n < #self)
    local v = self[#self - n]
    return v ~= stack.empty and v or nil
  end,
}


-- Metamethods for stack tables
-- stack:method ()
metatable = { __index = stack }


--- @export
return {
  argcheck   = argcheck,
  argerror   = argerror,
  argscheck  = argscheck,
  iscallable = iscallable,
  recase     = recase,
  stack      = stack,
}
