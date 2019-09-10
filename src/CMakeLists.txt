

set (BUILT_SOURCES tbl_funcs.h tbl_opts.h tbl_vars.h)

set (SRC_FUNCTION
  basic.c
  bind.c
  buffer.c
  eval.c
  file.c
  funcs.c
  help.c
  killring.c
  line.c
  face.c
  gc_veneer.c
  lisp.c
  macro.c
  mouse.c
  redisplay.c
  registers.c
  search.c
  undo.c
  variables.c
  window.c)

set(SRC_BASE
  ${SRC_FUNCTION}
  memrmem.h
  memrmem.c
  astr.c
  astr.h
  estr.c
  estr.h
  extern.h
  main.h
  tbl_vars.h
  lists.c
  lists.h
  buffer.h
  completion.h
  completion.c
  editfns.c
  getkey.c
  history.c
  keycode.c
  main.c
  marker.h
  marker.c
  minibuf.c
  region.h
  region.c
  window.h
  term_minibuf.c
  term_redisplay.c
  term_curses.c)

add_custom_command(
  OUTPUT  ${CMAKE_CURRENT_BINARY_DIR}/tbl_funcs.h
  COMMAND ZILE_PACKAGE_NAME="${PACKAGE_NAME}" ${PERL}
  ARGS -I${PROJECT_SOURCE_DIR}/build-aux
  ${PROJECT_SOURCE_DIR}/build-aux/mkfuncs.pl
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${SRC_FUNCTION}
  DEPENDS ${SRC_FUNCTION} ${PROJECT_SOURCE_DIR}/build-aux/mkfuncs.pl)

add_custom_command(
  OUTPUT  ${CMAKE_CURRENT_BINARY_DIR}/tbl_vars.h dotzile.sample
  COMMAND PACKAGE="${PACKAGE}" ${PERL}
  ARGS -I${PROJECT_SOURCE_DIR}/build-aux
  ${PROJECT_SOURCE_DIR}/build-aux/mkvars.pl
  ${PROJECT_SOURCE_DIR}/src/tbl_vars.h.in
  DEPENDS tbl_vars.h.in ${PROJECT_SOURCE_DIR}/build-aux/mkvars.pl)

configure_file(tbl_opts.h.in tbl_opts.h)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_executable(zile ${SRC_BASE} ${BUILT_SOURCES} ${PROJECT_BINARY_DIR}/config.h)
target_link_libraries(zile ${CMAKE_BINARY_DIR}/gnulib/gllib/libgnu.a ${CURSES_LIBRARIES} acl ${LIBGC_LIBRARIES})

