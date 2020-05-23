AC_PREREQ(2.59)
AC_INIT([child], [1.4-svn], [David Lebrun (target0@geeknode.org)])
AC_CONFIG_AUX_DIR([autotools])
AC_CONFIG_MACRO_DIR([autotools])
AM_INIT_AUTOMAKE
AC_CONFIG_HEADER(config.h)
AC_CONFIG_SRCDIR([src/child.c])

AC_CANONICAL_HOST

AC_PROG_CC
AC_PROG_CPP

AC_MSG_CHECKING([for supported operating system])
AC_MSG_RESULT($host_os)

AC_CHECK_FUNCS([bzero getcwd gethostbyname inet_ntoa memset regcomp socket strcasecmp strerror strstr strtol],,[AC_MSG_ERROR([Required functions not found])])
AC_CHECK_FUNCS([srandomdev])
AC_C_CONST
AC_C_INLINE
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_STRUCT_TM

case "$host_os" in
linux*) AC_CHECK_LIB([dl], [dlopen], [have_libdl=yes], [have_libdl=no])
        if (test x"$have_libdl" = x"yes"); then
                LIBS="-L/usr/lib/mysql $LIBS -ldl"
        else
                AC_MSG_ERROR([can't find dl library])
        fi
        ;;
freebsd*) AC_CHECK_HEADERS([kvm.h paths.h sys/param.h sys/sysctl.h sys/user.h],,[AC_MSG_ERROR([Required headers not found])])
        LIBS="-L/usr/local/lib -L/usr/local/lib/mysql $LIBS"
	CFLAGS="-I/usr/local/include $CFLAGS"
        AC_CHECK_LIB([kvm], [kvm_open], [have_libkvm=yes], [have_libkvm=no])
        if (test x"$have_libkvm" = x"yes"); then
                LIBS="$LIBS -lkvm"
        else
                AC_MSG_ERROR([can't find kvm library])
        fi
        ;;
esac

AC_ARG_WITH(mysql-libs, [ --with-mysql-libs=dir    Specify the location of mysql shared libs], [
 LIBS="-L$withval $LIBS"
])

AC_ARG_WITH(mysql-headers, [ --with-mysql-headers=dir    Specify the location of mysql headers], [
 CFLAGS="-I$withval $CFLAGS"
])

AC_ARG_ENABLE(gnutls,
	AC_HELP_STRING([--enable-gnutls], [enable gnutls secure connections @<:@default=disabled@:>@]),
			[use_gnutls="yes"], [use_gnutls="no"])

AC_ARG_ENABLE(filter,
	AC_HELP_STRING([--enable-filter], [enable data filtering @<:@default=disabled@:>@]),
			[use_filter="yes"], [use_filter="no"])

AC_CHECK_LIB([mysqlclient], [mysql_init], [have_libmysqlclient=yes], [have_libmysqlclient=no])
if (test x"$have_libmysqlclient" = x"yes"); then
	LIBS="$LIBS -lmysqlclient"
else
	AC_MSG_ERROR([can't find mysqlclient library])
fi

have_libssl=no
have_libcrypto=no
AC_CHECK_LIB([ssl], [OpenSSL_add_all_digests], [have_libssl=yes])
AC_CHECK_LIB([ssl], [OPENSSL_init_crypto], [have_libssl=yes])
AC_CHECK_LIB([crypto], [OpenSSL_add_all_digests], [have_libcrypto=yes])
AC_CHECK_LIB([crypto], [OPENSSL_init_crypto], [have_libcrypto=yes])

if (test x"$have_libssl" = x"yes"); then
	LIBS="$LIBS -lssl"
else
	if (test x"$have_libcrypto" = x"yes"); then
		LIBS="$LIBS -lcrypto"
	else
		AC_MSG_ERROR([can't find ssl library])
	fi
fi

if (test x"$use_gnutls" = x"yes"); then
	AC_CHECK_LIB([gnutls], [gnutls_init], [have_libgnutls=yes], [have_libgnutls=no])
	if (test x"$have_libgnutls" = x"yes"); then
		LIBS="$LIBS -lgnutls"
	else
		AC_MSG_ERROR([can't find gnutls library])
	fi
	AC_CHECK_HEADERS([gnutls/gnutls.h],,[AC_MSG_ERROR([Can't find gnutls headers])])
	AC_DEFINE([USE_GNUTLS],[],[Use gnutls])
fi

if (test x"$use_filter" = x"yes"); then
	AC_DEFINE([USE_FILTER],[],[Enable data filtering support])
fi

LIBS="$LIBS -export-dynamic"

AC_CHECK_HEADERS([dlfcn.h mysql/mysql.h openssl/evp.h],,[AC_MSG_ERROR([Required headers not found])])

CFLAGS="-g -Wall -W -pipe -export-dynamic -I../include $CFLAGS"

AC_CONFIG_FILES([Makefile src/Makefile src/modules/Makefile])
AC_OUTPUT
