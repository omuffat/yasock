###
### commands to run
###
### aclocal && automake --add-missing
### autoconf && ./configure [--enable-debug]

# arguments of macro should be enclosed within '[' and ']'

## End of tuto

# Process this file with autoconf to produce a configure script.
# Always start with AC_INIT
AC_INIT([yasock], [0.1], [contact@comoe-networks.com], [Yet Another Sock Program])
AM_INIT_AUTOMAKE
AC_LANG([C])
## Automake Variables (in this file ??)
#AM_LDFLAGS for specifying options to linker
AC_ARG_ENABLE([debug], [  --enable-debug    Turn on debugging],
		       [case "${enableval}" in
		       yes) debug=true ;;
		       no)  debug=false ;;
		       *) AC_MSG_ERROR([bad value ${enableval} for --enable-debug]) ;;
		       esac],[debug=false])
AM_CONDITIONAL([DEBUG], [test x$debug = xtrue])


## Generate a config.h from config.h.in
AC_CONFIG_HEADERS([config.h])

## Check for programs

## Checks for libraries


## Checks for header files
##AC_CHECK_HEADERS([sys/types.h])
AC_CHECK_HEADERS([sys/socket.h], AC_DEFINE([HAVE_SYS_SOCKET_H], 1,
				 [Define to 1 if you have the <sys/socket.h> header file. MY MSG]))
AC_CHECK_HEADERS([netinet/in.h], AC_DEFINE([HAVE_NETINET_IN_H], 1,
				 [Define to 1 if you have the <netinet/in.h> header file. MY MSG]))
AC_CHECK_HEADERS([netinet/ip.h], AC_DEFINE([HAVE_NETINET_IP_H], 1,
				 [Define to 1 if you have the <netinet/ip.h> header file. My MSG]))

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

#AC_MSG_CHECKING("whether flock struct is linux ordered or not")
#AC_TRY_RUN([
  #include <fcntl.h>
#  struct flock lock = { 1, 2, 3, 4, 5 };
#  int main() { return lock.l_type==1 ? 0 : 1; }
#], [
#    AC_DEFINE(HAVE_FLOCK_LINUX) 
#    AC_MSG_RESULT("yes")
#], AC_MSG_RESULT("no") )


## Checks for compiler characteristics


## Checks for library functions


## Checks for system services


#AC_CONFIG_FILES([FILE...])
AC_CONFIG_FILES([Makefile])

### Always make a call to AC_OUTPUT at the end
AC_OUTPUT

