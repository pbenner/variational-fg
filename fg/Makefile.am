## Process this file with automake to produce Makefile.in

AM_CPPFLAGS = -I$(top_srcdir)
AM_CXXFLAGS = $(CXXFLAGS_NORTTI)

noinst_PROGRAMS = distribution-test factor-graph-test

distribution_test_SOURCES = distribution-test.cc
distribution_test_LDADD   = libfg.la
factor_graph_test_SOURCES = factor-graph-test.cc
factor_graph_test_LDADD   = libfg.la

## fg library
lib_LTLIBRARIES = libfg.la
libfg_la_SOURCES = \
	domain.hh \
	distribution.hh \
	factor-graph.cc \
	factor-graph.hh \
	hotnews.hh \
	messages.hh \
	node-set.hh \
	node-types.hh \
	queue.hh \
	variational.hh \
	variational.cc
libfg_la_LIBADD   = $(LIB_PTHREAD)
libfg_la_LIBADD  += $(BOOST_THREAD_LIB)
libfg_la_LIBADD  += $(BOOST_SYSTEM_LIB)
libfg_la_LDFLAGS  = -no-undefined
libfg_la_LDFLAGS += -version-info 0:0:0

## python interface
pkgpython_LTLIBRARIES = interface.la datatypes.la

interface_la_CPPFLAGS = $(AM_CPPFLAGS) $(PYTHON_CPPFLAGS)
interface_la_CXXFLAGS =
interface_la_SOURCES  = \
	interface.cc \
	datatypes.cc \
	exceptions.hh
interface_la_LIBADD   = libfg.la
interface_la_LIBADD  += $(BOOST_PYTHON_LIB)
interface_la_LIBADD  += -lm
interface_la_LDFLAGS  = -avoid-version
interface_la_LDFLAGS += -module

datatypes_la_CPPFLAGS = $(AM_CPPFLAGS) $(PYTHON_CPPFLAGS)
datatypes_la_CXXFLAGS =
datatypes_la_SOURCES  = \
	datatypes.cc
datatypes_la_LIBADD   = $(BOOST_PYTHON_LIB)
datatypes_la_LIBADD  += -lm
datatypes_la_LDFLAGS  = -avoid-version
datatypes_la_LDFLAGS += -module

## set python package name
pkgpythondir = $(pyexecdir)/fg

## compile python files
pkgpython_PYTHON = __init__.py sigint.py

## clean python files
clean-local:
	$(RM) *.pyc

## dot
DOT_PNG = \
	factor-graph-test-1-fg.png \
	factor-graph-test-3-fg.png \
	factor-graph-test-5-fg.png

$(DOT_PNG): %.png: %.dot
	dot -Tpng -o $@ $<
