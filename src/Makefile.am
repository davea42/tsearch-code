###Copyright (C) 2018 David Anderson
###This code is public domain and can be freely used or copied.

MAINTAINERCLEANFILES = Makefile.in

AUTOMAKE_OPTIONS = subdir-objects

bin_PROGRAMS = tsearchstd binarysearch balancedsearch \
   eppingerdel hashsearch   gnusearch redblacksearch

TESTS = $(check_PROGRAMS) $(check_TESTS)

balancedsearch_SOURCES = \
  tsearch_tester.c  \
  dwarf_tsearchbal.c \
  dwarf_tsearch.h \
  config.h 

tsearchstd_SOURCES = \
  tsearch_tester.c  \
  dwarf_tsearch.h \
  config.h

binarysearch_SOURCES
  tsearch_tester.c  \
  dwarf_tsearchbin.c\
  dwarf_tsearch.h \
  config.h

eppingerdel_SOURCES = \
  tsearch_tester.c  \
  dwarf_tsearchepp.c\
  dwarf_tsearch.h \
  config.h

redblacksearch_SOURCES = \
  dwarf_tsearchred.c \
  tsearch_tester.c  \
  dwarf_tsearch.h \
  config.h

hashsearch_SOURCES= \
  dwarf_tsearchhash.c \
  tsearch_tester.c  \
  dwarf_tsearch.h \
  config.h
  
gnusearch_SOURCES= \
  tsearch_tester.c  \
  dwarf_tsearch.h \
  config.h

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
