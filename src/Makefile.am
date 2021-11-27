###Copyright (C) 2018 David Anderson
###This code is public domain and can be freely used or copied.

MAINTAINERCLEANFILES = Makefile.in

AUTOMAKE_OPTIONS = subdir-objects

bin_PROGRAMS = readelfobj readobjmacho readobjpe object_detector

TESTS = $(check_PROGRAMS) $(check_TESTS)

readelfobj_SOURCES = \
dwarf_tsearchbal.c \
dwarf_tsearch.c \
dwarf_tsearchhash.c \
tsearch.c
dwarf_tsearchbin.c\
dwarf_tsearchepp.c\
dwarf_tsearchred.c \
tsearch_tester.c \
config.h \
dwarf_incl.h \
dwarf_tsearch.h


readelfobj_CFLAGS = $(RO_CFLAGS_WARN)
readelfobj_CPPFLAGS = \
-I$(top_srcdir) -I$(top_builddir) -I$(top_srcdir)/src

readobjmacho_SOURCES =  readobjmacho.c \
sanitized.h \
dwarf_object_read_common.c \
dwarf_object_read_common.h \
dwarf_reading.h \
dwarf_object_detector.c \
dwarf_object_detector.h \
dwarf_machoread.h \
dwarf_machoread.c \
dwarf_macho_loader.h \
errormessages.c \
sanitized.c
readobjmacho_CFLAGS = $(RO_CFLAGS_WARN)
readobjmacho_CPPFLAGS = \
-I$(top_srcdir) -I$(top_builddir) -I$(top_srcdir)/src

readobjpe_SOURCES =  readobjpe.c \
sanitized.h \
dwarf_object_read_common.c \
dwarf_object_read_common.h \
dwarf_reading.h \
dwarf_object_detector.c \
dwarf_object_detector.h \
dwarf_peread.h \
dwarf_peread.c \
dwarf_pe_descr.h \
errormessages.c \
sanitized.c
readobjpe_CFLAGS = $(RO_CFLAGS_WARN)
readobjpe_CPPFLAGS = \
-I$(top_srcdir) -I$(top_builddir) -I$(top_srcdir)/src

object_detector_SOURCES = object_detector_main.c \
   dwarf_object_detector.c \
   errormessages.c \
   dwarf_object_detector.h
object_detector_CFLAGS = $(RO_CFLAGS_WARN) 

check_PROGRAMS = test_dwstring test_linkedtopath

test_dwstring_SOURCES = test_dwstring.c \
   dwarfstring.h dwarfstring.c 
test_dwstring_CFLAGS = $(RO_CFLAGS_WARN)
test_dwstring_CPPFLAGS = \
-I$(top_srcdir) -I$(top_builddir) -I$(top_srcdir)/src

test_linkedtopath_SOURCES = test_linkedtopath.c \
   dwarfstring.h dwarfstring.c \
   dwarf_debuglink.h dwarf_debuglink.c \
   dwarf_object_read_common.c 
test_linkedtopath_CFLAGS = $(RO_CFLAGS_WARN) -DTESTING
test_linkedtopath_CPPFLAGS = \
-I$(top_srcdir) -I$(top_builddir) -I$(top_srcdir)/src

check_TESTS = testdebuglink.sh
AM_TESTS_ENVIRONMENT = DWTOPSRCDIR='$(top_srcdir)'; export DWTOPSRCDIR;

man_MANS = readelfobj.1

EXTRA_DIST = \
$(man_MANS) \
$(check_TESTS) \
COPYING \
baseline.ltp \
ChangeLog \
ChangeLog2018 \
NEWS \
README 

CLEANFILES = \
junk \
junk.ltp
