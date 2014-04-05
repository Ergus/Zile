# zemacs Makefile.am
#
# Copyright (c) 1997-2014 Free Software Foundation, Inc.
#
# This file is part of GNU Zile.
#
# This program is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3, or (at your option)
# any later version.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.


## ------------- ##
## Declarations. ##
## ------------- ##

zemacsdatadir = $(datadir)/zemacs
zemacscmdsdir = $(zemacsdatadir)/zlisp


## ------ ##
## Build. ##
## ------ ##

doc_DATA += doc/dotzemacs.sample

dist_bin_SCRIPTS += bin/zemacs

man_MANS += doc/zemacs.1

## $(srcdir) prefixes are required when passing $(dist_zemacscmds_DATA)
## to zlc in the build tree with a VPATH build, otherwise it fails to
## find them in $(builddir)/zemacs/commands/*.
dist_zemacscmds_DATA =					\
	$(srcdir)/lib/zemacs/zlisp/bind.zl		\
	$(srcdir)/lib/zemacs/zlisp/buffer.zl		\
	$(srcdir)/lib/zemacs/zlisp/edit.zl		\
	$(srcdir)/lib/zemacs/zlisp/file.zl		\
	$(srcdir)/lib/zemacs/zlisp/killring.zl		\
	$(srcdir)/lib/zemacs/zlisp/help.zl		\
	$(srcdir)/lib/zemacs/zlisp/line.zl		\
	$(srcdir)/lib/zemacs/zlisp/lisp.zl		\
	$(srcdir)/lib/zemacs/zlisp/macro.zl		\
	$(srcdir)/lib/zemacs/zlisp/marker.zl		\
	$(srcdir)/lib/zemacs/zlisp/minibuf.zl		\
	$(srcdir)/lib/zemacs/zlisp/move.zl		\
	$(srcdir)/lib/zemacs/zlisp/registers.zl		\
	$(srcdir)/lib/zemacs/zlisp/search.zl		\
	$(srcdir)/lib/zemacs/zlisp/undo.zl		\
	$(srcdir)/lib/zemacs/zlisp/variables.zl		\
	$(srcdir)/lib/zemacs/zlisp/window.zl		\
	$(NOTHING_ELSE)

dist_zemacsdata_DATA =					\
	lib/zemacs/default-bindings-el.lua		\
	lib/zemacs/bind.lua				\
	lib/zemacs/callbacks.lua			\
	lib/zemacs/commands.lua				\
	lib/zemacs/eval.lua				\
	lib/zemacs/keymaps.lua				\
	lib/zemacs/main.lua				\
	$(NOTHING_ELSE)

zemacs_zemacs_DEPS =					\
	Makefile					\
	lib/zemacs/zemacs.in				\
	$(dist_zemacscmds_DATA)				\
	$(dist_zemacsdata_DATA)				\
	$(NOTHING_ELSE)


# AM_SILENT_RULES pretty printing.
ZM_V_ZLC    = $(zm__v_ZLC_@AM_V@)
zm__v_ZLC_  = $(zm__v_ZLC_@AM_DEFAULT_V@)
zm__v_ZLC_0 = @echo "  ZLC     " $@;
zm__v_ZLC_1 =

lib/zemacs/commands.lua: $(srcdir)/lib/zemacs/zlc $(dist_zemacscmds_DATA)
	@d=`echo '$@' |sed 's|/[^/]*$$||'`;			\
	test -d "$$d" || $(MKDIR_P) "$$d"
	$(ZM_V_ZLC)LUA_INIT= LUA_INIT_5_2=			\
	  LUA_PATH='$(ZILE_PATH);$(LUA_PATH)'			\
	  $(LUA) $(srcdir)/lib/zemacs/zlc $(dist_zemacscmds_DATA) > '$@' || rm -f '$@'
	test -f '$@'

doc/dotzemacs.sample: lib/zemacs/mkdotzemacs.lua
	@d=`echo '$@' |sed 's|/[^/]*$$||'`;			\
	test -d "$$d" || $(MKDIR_P) "$$d"
	$(AM_V_GEN)PACKAGE='$(PACKAGE)'				\
	LUA_PATH='$(ZILE_PATH);$(LUA_PATH)'			\
	  $(LUA) $(srcdir)/lib/zemacs/mkdotzemacs.lua > '$@'

doc/zemacs.1: lib/zemacs/man-extras lib/zemacs/help2man-wrapper $(zemacs_zemacs_DEPS)
	@d=`echo '$@' |sed 's|/[^/]*$$||'`;			\
	test -d "$$d" || $(MKDIR_P) "$$d"
## Exit gracefully if zemacs.1.in is not writeable, such as during distcheck!
	$(AM_V_GEN)if ( touch $@.w && rm -f $@.w; ) >/dev/null 2>&1; \
	then							\
	  builddir='$(builddir)'				\
	  $(srcdir)/build-aux/missing --run			\
	    $(HELP2MAN)						\
	      '--output=$@'					\
	      '--no-info'					\
	      '--name=Zemacs'					\
	      --include '$(srcdir)/lib/zemacs/man-extras'	\
	      '$(srcdir)/lib/zemacs/help2man-wrapper';		\
	fi



## --------------------------- ##
## Interactive help resources. ##
## --------------------------- ##

# There's no portable way to install and then access from zemacs
# plain text resources, so we convert them to Lua modules here.

zemacsdocdatadir = $(zemacsdatadir)/doc

dist_zemacsdocdata_DATA =					\
	$(srcdir)/lib/zemacs/doc/COPYING.lua			\
	$(srcdir)/lib/zemacs/doc/FAQ.lua			\
	$(srcdir)/lib/zemacs/doc/NEWS.lua			\
	$(NOTHING_ELSE)

$(srcdir)/lib/zemacs/doc:
	@test -d '$@' || $(MKDIR_P) '$@'

$(dist_zemacsdocdata_DATA): $(srcdir)/lib/zemacs/doc
## Exit gracefully if target is not writeable, such as during distcheck!
	$(AM_V_GEN)if ( touch $@.w && rm -f $@.w; ) >/dev/null 2>&1; \
	then							\
	  {							\
	    src=`echo '$@' |sed -e 's|^.*/||' -e 's|\.lua$$||'`;\
	    echo 'return [==[';					\
	    cat "$(srcdir)/$$src";				\
	    echo ']==]';					\
	  } > '$@';						\
	fi


## -------------- ##
## Documentation. ##
## -------------- ##

ldoc_DEPS += $(zemacs_zemacs_DEPS)


## ------------- ##
## Installation. ##
## ------------- ##

install_exec_hook += zemacs-install-exec-hook
zemacs-install-exec-hook:
	sed -e 's|@datadir[@]|$(datadir)|g' $(srcdir)/bin/zemacs >.zemacsT
	$(INSTALL_SCRIPT) .zemacsT $(DESTDIR)$(bindir)/zemacs
	rm -f .zemacsT


## ------------- ##
## Distribution. ##
## ------------- ##

EXTRA_DIST +=						\
	doc/dotzemacs.sample				\
	lib/zemacs/commands.lua				\
	lib/zemacs/help2man-wrapper			\
	lib/zemacs/man-extras				\
	lib/zemacs/mkdotzemacs.lua			\
	lib/zemacs/zlc					\
	lib/zemacs/zemacs.in				\
	$(NOTHING_ELSE)


## ------------ ##
## Maintenance. ##
## ------------ ##

CLEANFILES +=						\
	doc/zemacs.1					\
	$(NOTHING_ELSE)

MAINTAINERCLEANFILES +=					\
	$(srcdir)/lib/zemacs/commands.lua		\
	$(srcdir)/lib/zemacs/zemacs.1.in			\
	$(dist_zemacsdocdata_DATA)			\
	$(NOTHING_ELSE)
