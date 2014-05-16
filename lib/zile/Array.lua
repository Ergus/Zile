-- Copyright (c) 2014 Free Software Foundation, Inc.
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
-- along with this program.  If not, see <htt://www.gnu.org/licenses/>.

--[[--
 Efficient array of homogenous objects.

 An Array is a block of contiguous memory, divided into equal sized
 elements that can be indexed quickly.

 Create a new Array with:

     > Array = require "zile.Array"
     > array = Array ("int", 0xdead, 0xbeef, 0xfeed)
     > =array[1], array[2], array[3], array[-1]
     57005	48879	nil	65261

 All the indexes passed to methods use 1-based counting.

 @classmod zile.Array
]]


local alien  = require "alien"
local Object = require "std.object"

local calloc = alien.array

local element_chunk_size = 16


local _functions = {
  --- Remove the right-most element.
  -- @function pop
  -- @return the right-most element
  pop = function (self)
    if self.used > 0 then
      local a = self.array
      local elem = a[self.used]
      self.used = self.used - 1
      if self.used < a.length / 2 then
        self:setlen (self.used)
      end
      return elem
    end
  end,

  --- Add elem as the new right-most element.
  -- @function push
  -- @param elem new element to be pushed
  -- @treturn Array the array
  push = function (self, elem)
    if type (elem) ~= "number" then
      return error ("bad argument #2 to 'push' (number expected, got " ..
                    (elem and type (elem) or "no value") .. ")", 2)
    end
    local a = self.array
    self.used = self.used + 1
    if self.used > a.length then
      self:setlen (self.used)
    end
    a[self.used] = elem
    return self
  end,


  --- Change the number of elements allocated to be at least `n`.
  -- @function setlen
  -- @int n the number of elements required
  -- @treturn Array the array
  setlen  = function (self, n)
    local a = self.array
    if n > a.length or n < a.length / 2 then
      a:realloc (n + element_chunk_size)
    end
    return self
  end,
}


local _type = type

------
-- An efficient array of homogenous objects.
-- @table Array
-- @int length number of elements currently allocated
-- @tfield alien.array array a block of indexable memory
local Array = Object {
  _type = "Array",


  -- Module functions.
  _functions = _functions,


  --- Instantiate a newly cloned Array.
  -- @function __call
  -- @string type the C type of each element
  -- @param[opt] ... initial elements to store
  -- @treturn Array a new Array object
  _init = function (self, type, ...)
    if _type (type) ~= "string" then
      return error ("bad argument #1 to 'Array' (string expected, got " ..
                    (type and  _type (type) or "no value") ..")", 2)
    end
    self.used  = select ("#", ...)
    self.array = calloc (type, math.max (self.used, 1))
    for i, v in ipairs {...} do
      if _type (v) ~= "number" then
        return error (string.format (
          "bad argument #%d to 'Array' (number expected, got %s)", i + 2, _type (v)),
          2
        )
      end
      self.array[i] = v
    end
    return self
  end,


  --- Return a string representation of the contents of this Array.
  -- @function __tostring
  -- @treturn string string representation
  __tostring = function (self)
    local a = self.array
    local t = {'"' .. a.type .. '"'}
    for i = 1, self.used do
      t[#t + 1] = tostring (a[i])
    end
    return Object.prototype (self) .. " (" .. table.concat (t, ", ") .. ")"
  end,


  --- Return the number of elements in this Array.
  -- @function __len
  -- @treturn int number of elements
  __len = function (self)
    return self.used
  end,


  --- Return the `n`th character in this Array.
  -- @function __index
  -- @int n 1-based index, or negative to index starting from the right
  -- @treturn string the element at index `n`
  __index = function (self, n)
    if type (n) == "number" then
      if n < 0 then n = n + self.used + 1 end
      if n > 0 and n <= self.used then
        return self.array[n]
      end
    else
      return _functions[n]
    end
  end,


  --- Set the `n`th element of this Array to `elem`.
  -- @function __newindex
  -- @int n 1-based index
  -- @param elem value to store at index n
  -- @treturn Array the array
  __newindex = function (self, n, elem)
    if type (n) == "number" then
      local a = self.array
      if n == 0 or math.abs (n) > self.used then
	return a[0] -- guaranteed to be out of bounds
      end
      if n < 0 then n = n + self.used + 1 end
      a[n] = elem
    else
      rawset (self, n, elem)
    end
    return self
  end,
}

return Array
