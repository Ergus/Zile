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
     57005	48879	65261	65261

 All the indexes passed to methods use 1-based counting.

 @classmod zile.Array
]]


local alien  = require "alien"
local Object = require "std.object"

local lib    = require "zile.lib"

local prototype = Object.prototype
local typeof = type
local calloc, memset = alien.array, alien.memset
local argcheck, argscheck = lib.argcheck, lib.argscheck

local element_chunk_size = 16


--- Fast zeroing of a contiguous block of array elements.
-- @tparam alien.array array an array
-- @int from index of first element to zero out
-- @int n number of elements to zero out
-- @treturn alien.array array
local function setzero (array, from, n)
  if n > 0 then
    local size = array.size
    local b = (from - 1) * size + 1
    memset (array.buffer:topointer (b), 0, n * size)
  end
  return array
end


local _functions = {
  --- Remove the right-most element.
  -- @function pop
  -- @return the right-most element
  pop = function (self)
    argscheck ("pop", {"Array"}, {self})

    local used = self.used
    if used > 0 then
      local a = self.array
      local elem = a[used]
      self:realloc (used - 1)
      return elem
    end
  end,


  --- Add elem as the new right-most element.
  -- @function push
  -- @param elem new element to be pushed
  -- @treturn Array the array
  push = function (self, elem)
    argscheck ("push", {"Array", "number"}, {self, elem})

    local a, used = self.array, self.used + 1
    self:realloc (used)
    a[used] = elem
    return self
  end,


  --- Change the number of elements allocated to be at least `n`.
  -- @function realloc
  -- @int n the number of elements required
  -- @treturn Array the array
  realloc = function (self, n)
    argscheck ("realloc", {"Array", "number"}, {self, n})

    local a, used = self.array, self.used
    if n > a.length or n < a.length / 2 then
      a:realloc (n + element_chunk_size)
    end

    -- Zero padding for uninitialised elements.
    setzero (a, used + 1, n - used)

    self.used = n
    return self
  end,
}


--- Copy an alien.array.
-- @tparam alien.array array array to be copied
-- @treturn alien.array a new array identical to `array`
local function copy (array)
  local a = calloc (array.type, array.length)
  local n = array.size * array.length
  alien.memmove (a.buffer:topointer (1), array.buffer:topointer (1), n)
  return a
end


------
-- An efficient array of homogenous objects.
-- @table Array
-- @int length number of elements currently allocated
-- @tfield alien.array array a block of indexable memory
local Array = Object {
  _type = "Array",


  -- Prototype initial values.
  used  = 0,
  array = calloc ("int", {0}),


  -- Module functions.
  _functions = _functions,


  --- Instantiate a newly cloned Array.
  -- @function __call
  -- @string[opt] type the C type of each element
  -- @tparam[opt] int|table init initial size or list of initial elements
  -- @treturn Array a new Array object
  _init = function (self, type, init)
    if init ~= nil then
      -- When called with 2 arguments:
      argcheck ("Array", 1, {"string"}, type)
      argcheck ("Array", 2, {"number", "table"}, init)
    elseif type ~= nil then
      -- When called with 1 argument:
      argcheck ("Array", 1, {"number", "string", "table"}, type)
    end

    local parray, pused  = self.array, self.used
    local plength, ptype = parray.length, parray.type

    -- Non-string argument 1 is really an init argument.
    if typeof (type) ~= "string" then type, init = nil, type end

    -- New array type is copied from prototype if not specified.
    if type == nil then type = ptype end

    local a
    if init == nil then
      -- 1a. A fast clone of prototype's array memory.
      if type == ptype then
        a = copy (parray)

      -- 1b. Size of elements changed, so we have to manually copy them
      --     over where they are truncated or expanded as necessary.
      else
        a = calloc (type, plength)
        for i = 1, plength do
          a[i] = parray[i]
        end
      end

    elseif typeof (init) == "number" then
      -- 2a. Clone a number of elements from the prototype, padding with
      --     zeros if we have more elements than the prototype.
      if type == ptype then
        a = copy (parray)
        a:realloc (init) -- alien.array.realloc, not zile.Array.realloc!!

      -- 2b. Same, but element-wise copying with a different sized type.
      else
        a = calloc (type, init)
        for i = 1, math.min (init, plength) do
          a[i] = parray[i]
        end
      end

      setzero (a, pused + 1, init - pused)
      self.used = init

    elseif typeof (init) == "table" then
      -- 3. With an initialisation table, ignore prototype elements.
      a = calloc (type, init)
      self.used = #init
    end

    self.array = a
    return self
  end,


  --- Return the number of elements in this Array.
  -- @function __len
  -- @treturn int number of elements
  __len = function (self)
    argscheck ("__len", {"Array"}, {self})
    return self.used
  end,


  --- Return the `n`th character in this Array.
  -- @function __index
  -- @int n 1-based index, or negative to index starting from the right
  -- @treturn string the element at index `n`
  __index = function (self, n)
    argscheck ("__index", {"Array", {"number", "string"}}, {self, n})

    if typeof (n) == "number" then
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
    argscheck ("__newindex", {"Array", "number", "number"}, {self, n, elem})

    if typeof (n) == "number" then
      local a, used = self.array, self.used
      if n == 0 or math.abs (n) > used then
	return a[0] -- guaranteed to be out of bounds
      end
      if n < 0 then n = n + used + 1 end
      a[n] = elem
    else
      rawset (self, n, elem)
    end
    return self
  end,


  --- Return a string representation of the contents of this Array.
  -- @function __tostring
  -- @treturn string string representation
  __tostring = function (self)
    argscheck ("__tostring", {"Array"}, {self})

    local a = self.array
    local t = {'"' .. a.type .. '"'}
    for i = 1, self.used do
      t[#t + 1] = tostring (a[i])
    end
    return prototype (self) .. " (" .. table.concat (t, ", ") .. ")"
  end,
}

return Array
