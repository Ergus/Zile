/* An empty config.h, to override the main config.h, so that assumptions
   about the host platform are not used when building mkdoc on the build
   platform. */

#include <stdarg.h>

#define PACKAGE "zile"

extern int asprintf(char **strp, const char *fmt, ...);
extern int vasprintf(char **strp, const char *fmt, va_list ap);
extern char *strrstr(const char *, const char *);
