#
# Makefile.am
#

if ENABLE_DEBUG_INFO
AM_CFLAGS = ${my_CFLAGS} \
	${my_DEBUG_CFLAGS}
else
AM_CFLAGS = ${my_CFLAGS}
endif
AM_LDFLAGS = -Wl,--gc-sections -Wl,--as-needed

bin_PROGRAMS = baalue

baalue_SOURCES =             	\
	main.c    	        \
	threads.c    	        \
	common.h    	        \
	gettext.h

baalue_CFLAGS = $(AM_CFLAGS)	\
	$(LIBCONFIG_CFLAGS)     \
	$(LIBBAALUE_CFLAGS)     \
	-DLOCALEDIR=\"$(localedir)\" -I$(srcdir)/../lib

baalue_LDFLAGS = $(AM_LDFLAGS)

baalue_LDADD =  		\
	$(LIBCONFIG_LIBS)   	\
	$(LIBBAALUE_LIBS)      	\
	$(LIBINTL)
