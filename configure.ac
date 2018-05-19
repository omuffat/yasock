dnl
dnl 'Discard Next Line': don't output comments in generated configure script
dnl '#' will appear in generated configure script
dnl
dnl commands to run
dnl
dnl aclocal && automake --add-missing --copy --force-missing
dnl autoconf && ./configure [--enable-debug]
dnl Process this file with autoconf to produce a configure script.

dnl arguments of macro should be enclosed within '[' and ']'

dnl End of tuto

# Always start with AC_INIT
AC_INIT([Yet Another Sock Program], [0.2.1], [contact@comoe-networks.com],
	     [yasock], [https://github.com/omuffat/yasock])
dnl Set option for automake in this macro call
AM_INIT_AUTOMAKE()
AC_LANG([C])
## Automake Variables (in this file ??)
dnl AM_LDFLAGS for specifying options to linker
AC_ARG_ENABLE([debug], [  --enable-debug    Turn on debugging],
		       [case "${enableval}" in
		       yes) debug=true ;;
		       no)  debug=false ;;
		       *) AC_MSG_ERROR([bad value ${enableval} for --enable-debug]) ;;
		       esac],[debug=false])
AM_CONDITIONAL([DEBUG], [test x$debug = xtrue])
dnl Define man generator program
AM_MISSING_PROG(HELP2MAN, help2man)


## Generate a config.h from config.h.in
AC_CONFIG_HEADERS([config.h])

## Check for programs

## Checks for libraries


## Checks for header files
AC_CHECK_HEADERS([sys/types.h], AC_DEFINE([HAVE_SYS_TYPES_H], 1,
				[Define to 1 if you have the <sys/types.h> header file. MY MSG]))
AC_CHECK_HEADERS([sys/socket.h], AC_DEFINE([HAVE_SYS_SOCKET_H], 1,
				 [Define to 1 if you have the <sys/socket.h> header file. MY MSG]))
AC_CHECK_HEADERS([netinet/in.h], AC_DEFINE([HAVE_NETINET_IN_H], 1,
				 [Define to 1 if you have the <netinet/in.h> header file. MY MSG]))
AC_CHECK_HEADERS([netinet/ip.h], AC_DEFINE([HAVE_NETINET_IP_H], 1,
				 [Define to 1 if you have the <netinet/ip.h> header file. My MSG]),
		[],
		[[#ifdef HAVE_SYS_TYPES_H
	      	 # include <sys/types.h>
		 #endif
		 #ifdef HAVE_NETINET_IN_H
	      	 # include <netinet/in.h>
		 #endif
		 ]]
		 )

## Checks for types

## Checks for structures
# struct ip_mreq for multicast membership
AC_CHECK_TYPE([struct ip_mreq], [
		      AC_DEFINE([HAVE_IP_MREQ_H], 1, [Define to 1 if you have struct ip_mreq])],
          [], [[  #ifdef HAVE_SYS_SOCKET_H
	      	 # include <sys/socket.h>
		 #endif
		 #ifdef HAVE_NETINET_IN_H
	      	 # include <netinet/in.h>
		 #endif
		 ]]
		)
# struct linger for setsockopt SO_LINGER
AC_CHECK_TYPE([struct linger], [
		      AC_DEFINE([HAVE_SO_LINGER_H], 1, [Define to 1 if you have struct linger])],
          [], [[  #ifdef HAVE_SYS_SOCKET_H
	      	 # include <sys/socket.h>
		 #endif
		 ]]
		)
# TCP_MAXSEG preprocessor macro for setsockopt (MSS)
AC_CHECK_DECL([TCP_MAXSEG], [
		      AC_DEFINE([HAVE_TCP_MAXSEG_H], 1, [Define to 1 if you have TCP_MAXSEG option])],
          [], [[  #ifdef HAVE_SYS_SOCKET_H
	      	 # include <sys/socket.h>
		 #endif
		 #ifdef HAVE_NETINET_IN_H
	      	 # include <netinet/in.h>
		 #endif
		 #ifdef HAVE_NETINET_IN_H
	      	 # include <netinet/tcp.h>
		 #endif
		 ]]
		)


dnl AC_MSG_CHECKING("whether flock struct is linux ordered or not")
dnl AC_TRY_RUN([
dnl  #include <fcntl.h>
dnl  struct flock lock = { 1, 2, 3, 4, 5 };
dnl  int main() { return lock.l_type==1 ? 0 : 1; }
dnl], [
dnl    AC_DEFINE(HAVE_FLOCK_LINUX) 
dnl    AC_MSG_RESULT("yes")
dnl], AC_MSG_RESULT("no") )


## Checks for compiler characteristics


## Checks for library functions


## Checks for system services


# Generate Makefile
AC_CONFIG_FILES([Makefile])

### Always make a call to AC_OUTPUT at the end
AC_OUTPUT

dnl Any output after AC_OUTPUT will be copied verbatim at the end of configure script
