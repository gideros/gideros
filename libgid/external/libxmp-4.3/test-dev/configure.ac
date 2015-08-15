AC_INIT
0>confdefs.h
AC_PROG_CC
AC_C_BIGENDIAN

dnl XMP_TRY_COMPILE(<message>,<cache-var>,<flags>,<program>,<ifyes>,<ifno>)
AC_DEFUN([XMP_TRY_COMPILE],[
  AC_CACHE_CHECK([$1],[$2],[
    oldcflags="${CFLAGS}"
    CFLAGS="${CFLAGS} $3"
    AC_COMPILE_IFELSE([AC_LANG_SOURCE([[$4]])],[$2=yes],[$2=no],[true])
    CFLAGS="${oldcflags}"])
  AS_IF([test "x$$2" = xyes], [$5], [$6])])

XMP_TRY_COMPILE(whether compiler understands -Wunused-result,
  ac_cv_c_flag_w_unused_result,
  -Wunused-result,[
  int main(){}],
  CFLAGS="${CFLAGS} -Wno-unused-result")  

AC_CHECK_FUNCS(popen mkstemp fnmatch strlcpy strlcat)
AC_CHECK_LIB(m,pow)
AC_CONFIG_FILES([Makefile])
AC_OUTPUT
