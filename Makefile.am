#
#	COMOE NETWORKS COPYRIGHT
#

##	Makefile template for automake
##
##
## Double '#' will not be ouput to Makefile
##
AM_CFLAGS = $(WARNINGCFLAGS)
bin_PROGRAMS= yasock
yasock_SOURCES= main.c server.c client.c socket_opt.c io_interactive.c tcp_info.c io.c yasock.h
man1_MANS= yasock.man
dist_man_MANS = $(man1_MANS)
## TO ADD library
## LDADD for all programs generated by this makefile
## yasock_LDADD for specific program yasock
AM_CPPFLAGS=
## DEBUG is set (or not) in configure.ac with AM_CONDITIONAL
if DEBUG
AM_CFLAGS += -g2 -DYASOCK_DEBUG
else
endif

yasock.man: main.c
	-$(HELP2MAN) --output=$@ -N --no-discard-stderr ./yasock
