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


-- Return true if x is a function, or is a Symbol with a __call metamethod.
local function iscallable (x)
  if type (x) == "function" then return true end
  local mt = getmetatable (x)
  return mt and mt.__call and mt._type == "Symbol"
end

-- Recase str according to newcase.
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



-- Basic stack operations
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

return {
  iscallable = iscallable,
  recase     = recase,
  stack      = stack,
}
