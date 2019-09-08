GNU Zile
========

GNU Zile is free software, licensed under the GNU GPL.

Copyright (c) 1997-2016 Free Software Foundation, Inc.

**Copying and distribution of this file, with or without modification,
are permitted in any medium without royalty provided the copyright
notice and this notice are preserved.**

------------------------------------------------------------------------

INTRODUCTION
============

GNU Zile (_Zile Is Lossy Emacs_) is a lightweight [Emacs][] clone. Every
Emacs user should feel at home with Zile. Zile is aimed at small
footprint systems (a typical binary is about 100Kb) and quick editing
sessions (it starts up and shuts down instantly).

Zile is written in **ISO C99** using **POSIX.1-2008** APIs, using
[libgc][]. It can be built on many other platforms thanks to its use
of [gnulib][] (version 7.2 or later seems to be required for testing
with Valgrind, but older versions seem to work fine for normal use).

  [Emacs]:   http://www.gnu.org/s/emacs
  [libgc]:   http://www.hpl.hp.com/personal/Hans_Boehm/gc
  [gnulib]:  http://www.gnu.org/s/gnulib

Source Layout
-------------

 * See file `AUTHORS` for the names of maintainers past and present.
 * See file `FAQ` for a selection of a Frequently Answered Questions.
 * See file `NEWS` for a list of major changes in each Zile release.
 * See file `THANKS` for a list of important contributors.

 * Directory `build-aux` contains helper scripts used to build Zile.
 * Directory `doc` contains files used to create Zile's documentation.
 * Directory `gnulib` contains [gnulib]() source modules used by Zile.
 * Directory `src` contains the source code used to build Zile.
 * Directory `tests` contains a small suite of tests used by


Web Pages
---------

Original GNU Zile's home page is at: <http://www.gnu.org/s/zile/>

GNU Zile development is co-ordinated from Zile's project page at GNU
Savannah: <http://savannah.gnu.org/projects/zile/>

OBTAINING THE LATEST SOURCES
============================

To install this version of Zile you need first to get the sources and
the gnulib sources:

```shell
git clone https://github.com/Ergus/Zile.git
cd Zile
git submodule update --init --recursive

```

With the sources you can now build Zile. You need cmake to do so and the
dependencies mentioned before: **libgc**.

```shell
mkdir build
cd build
cmake ..
make
```

The resulting Zile executable will be located inside src.
