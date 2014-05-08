-- Copyright (c) 2012-2014 Free Software Foundation, Inc.
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
 Syntax Highlighting.

 This module implements a two stage syntax highlighter:

 1. Search for matches against patterns in the current grammar from
   left to right, adding appropriate color pushes and pops at the
   match begin and end locations;
 2. Step through each character cell pushing any new colors for that
   cell onto a stack according to the instructions from step 1, then
   setting the cell to the color on the top of that stack (if any)
   and then finally popping any colors as instructed by step 1.

 @module zile.syntax
]]


local empty = require "std.table".empty


-- Metamethods for syntax parsers state.
local metatable = {
  -- Queue a stack operation.
  -- @string op "push" or "pop"
  -- @int o offest into line
  -- @int attr attribute to push or pop
  push_op = function (self, op, o, attr)
    if o and attr then
      local st = self.syntax.ops
      st[o] = st[o] or stack.new ()
      st[o]:push { [op] = attr }
    end
  end,
}


-- Syntax parser state.
local state = {
  -- Return a new parser state for the bp line containing offset o.
  -- @tparam table bp buffer table
  -- @int o offset into buffer bp
  -- @treturn table lexer for buffer line containing offset o
  new = function (bp, o)
    local n = offset_to_line (bp, o)

    bp.syntax[n] = {
      -- Calculate the attributes for each cell of this line using a
      -- stack-machine with color push and pop operations.
      attrs = {},
      ops   = {},

      -- parser state for the current line
      highlight = stack.new (),
    }

    local bol    = buffer_start_of_line (bp, o)
    local eol    = bol + buffer_line_len (bp, o)
    local region = get_buffer_region (bp, {start = bol, finish = eol})
    local lexer  = {
      grammar = bp.grammar,
      s       = tostring (region),
      syntax  = bp.syntax[n],
    }

    return setmetatable (lexer, {__index = metatable})
  end,
}


--- Marshal 0-indexed buffer API into and out-of 1-indexed onig API.
-- @tparam userdata rex compiled rex_onig expression
-- @string s string to match against rex
-- @param i search start index into s
-- @treturn int offset of beginning of match
-- @treturn int offset of end of match
-- @treturn table expression capture offsets
local function rex_exec (rex, s, i)
  local b, e, caps = rex:exec (s, i + 1)

  for k, v in pairs (caps or {}) do
    -- onig stores unmatched captures as `false`.
    if v then caps[k] = v - 1 end
  end

  return b and (b - 1), e and (e - 1), caps
end


--- Find the leftmost matching expression.
-- @tparam table lexer syntax highlight matcher state
-- @int i search start index
-- @tparam table pats a list of patterns
-- @treturn int offset of beginning of match
-- @treturn int offset of end of match
-- @treturn table expression capture offsets
-- @treturn pattern matching pattern
local function leftmost_match (lexer, i, pats)
  local s = lexer.s
  local b, e, caps, matched

  for _, v in ipairs (pats) do
    if v.match then
      local _b, _e, _caps = rex_exec (v.match, s, i)
      if _b and (not b or _b < b) then
        b, e, caps, matched = _b, _e, _caps, v
      end
    end
  end

  return b, e, caps, matched
end


--- Parse a string from left-to-right for matches against pats,
-- queueing color push and pop instructions as we go.
-- @tparam table lexer syntax highlight matcher stat
local function parse (lexer)
  local pats = lexer.grammar.patterns
  local b, e, caps, matched

  local i = 0
  repeat
    b, e, caps, matched = leftmost_match (lexer, i, pats)
    if b then
      lexer:push_op ("push", b, matched.attrs)
      if caps and matched.captures then
        for k, t in pairs (matched.captures) do
          -- onig marks zero length captures with first > last
          local first, last = caps[(k * 2) - 1], caps[k * 2]

          if first and first < last then
            lexer:push_op ("push", first, t.attrs)
            lexer:push_op ("pop",  last,  t.attrs)
          end
        end
      end
      lexer:push_op ("pop",  e, matched.attrs)

      i = e + 1
    end
  until b == nil
end


--- Highlight s according to queued color operations.
-- @param lexer
-- @return lexer
local function highlight (lexer)
  parse (lexer)

  local attrs, ops = lexer.syntax.attrs, lexer.syntax.ops
  local highlight  = lexer.syntax.highlight

  for i = 0, #lexer.s do
    -- set the color at this position before it can be popped.
    attrs[i] = highlight:top ()
    for _,v in ipairs (ops[i] or {}) do
      if v.push then
        highlight:push (v.push)
        -- but, override the initial color if a new one is pushed.
        attrs[i] = highlight:top ()

      elseif v.pop then
        assert (v.pop == highlight:top ())
        highlight:pop ()
      end
    end
  end

  return lexer
end


--- Return attributes for the line in bp containing o.
-- @tparam buffer bp a buffer
-- @int o character offset into bp
-- @treturn int attributes
local function attrs (bp, o)
  if not bp.grammar then return nil end

  local lexer = highlight (state.new (bp, o))

  return lexer.syntax.attrs
end


--- @export
return {
  attrs = attrs,
}
