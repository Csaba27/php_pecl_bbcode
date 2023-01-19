PHP_ARG_ENABLE(bbcode,whether to enable C based BBCode support,
[  --enable-bbcode         Enable BBCode support])

if test "$PHP_BBCODE" = "yes"; then
  AC_DEFINE(HAVE_BBCODE,1,[Whether you want BBCode support])
  PHP_NEW_EXTENSION(bbcode, php_bbcode.c lib/bstrlib.c lib/bbcode2.c, $ext_shared,, "-I@ext_srcdir@/lib")
  PHP_ADD_BUILD_DIR(lib)
fi
