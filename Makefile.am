SUBDIRS = doc \
	data

bin_PROGRAMS = g3data/g3data

g3data_source_files = \
	g3data/drawing.c \
	g3data/drawing.h \
	g3data/g3data-about.c \
	g3data/g3data-about.h \
	g3data/g3data-application.c \
	g3data/g3data-application.h \
	g3data/g3data-window.c \
	g3data/g3data-window.h \
	g3data/main.c \
	g3data/main.h \
	g3data/points.c \
	g3data/points.h \
	g3data/sort.c \
	g3data/sort.h \
	g3data/strings.h \
	g3data/vardefs.h

g3data_g3data_SOURCES = $(g3data_source_files)

AM_CFLAGS = $(WARNING_FLAGS)

AM_CPPFLAGS = $(GLIB_CFLAGS)
AM_CPPFLAGS += $(GTK_CFLAGS)
g3data_g3data_LDADD = $(GLIB_LIBS)
g3data_g3data_LDADD += $(GTK_LIBS)
