###Copyright (C) 2018 David Anderson
###This code is public domain and can be freely used or copied.

MAINTAINERCLEANFILES = Makefile.in

DISTCLEAN = 

AUTOMAKE_OPTIONS = subdir-objects

bin_PROGRAMS = libcsearch binarysearch balancedsearch \
   eppingerdel hashsearch redblacksearch

balancedsearch_SOURCES = \
  tsearch_tester.c  \
  dwarf_tsearchbal.c \
  dwarf_incl.h \
  dwarf_tsearch.h 

libcsearch_SOURCES = \
  tsearch_tester.c  \
  dwarf_tsearch.h 
libcsearch_CFLAGS = "-DLIBC_TSEARCH=1"

binarysearch_SOURCES = \
  tsearch_tester.c  \
  dwarf_tsearchbin.c \
  dwarf_incl.h \
  dwarf_tsearch.h 

eppingerdel_SOURCES = \
  tsearch_tester.c  \
  dwarf_tsearchepp.c \
  dwarf_incl.h \
  dwarf_tsearch.h 

redblacksearch_SOURCES = \
  dwarf_tsearchred.c \
  tsearch_tester.c  \
  dwarf_incl.h \
  dwarf_tsearch.h 

hashsearch_SOURCES = \
  dwarf_tsearchhash.c \
  tsearch_tester.c  \
  dwarf_incl.h \
  dwarf_tsearch.h 
hashsearch_CFLAGS = "-DHASHSEARCH=1"
  
EXTRA_DIST = \
$(man_MANS) \
$(check_TESTS) \
COPYING \
ChangeLog \
ChangeLog2014 \
ChangeLog2015 \
ChangeLog2016 \
ChangeLog2017 \
ChangeLog2018 \
ChangeLog2019 \
ChangeLog2020 \
NEWS \
README 

CLEANFILES = \
junk \
junk.ltp
