# Makefile.am for Hdrs
#
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.

AM_CPPFLAGS = \
  $(iocore_include_dirs) \
  -I$(top_srcdir)/lib \
  -I$(top_srcdir)/lib/records \
  -I$(top_srcdir)/lib/ts

noinst_LIBRARIES = libhdrs.a
EXTRA_PROGRAMS = load_http_hdr

# Http library source files.
libhdrs_a_SOURCES = \
  HdrHeap.cc \
  HdrHeap.h \
  HdrToken.cc \
  HdrToken.h \
  HdrTSOnly.cc \
  HdrUtils.cc \
  HdrUtils.h \
  HTTP.cc \
  HTTP.h \
  HttpCompat.cc \
  HttpCompat.h \
  MIME.cc \
  MIME.h \
  URL.cc \
  URL.h

if BUILD_TESTS
  libhdrs_a_SOURCES += HdrTest.cc \
    HdrTest.h
endif

load_http_hdr_SOURCES = \
  HdrHeap.h \
  HTTP.h \
  load_http_hdr.cc \
  MIME.h

load_http_hdr_LDADD = -L. -lhdrs \
  $(top_builddir)/lib/ts/libtsutil.la \
  @LIBTCL@
load_http_hdr_LDFLAGS = @EXTRA_CXX_LDFLAGS@ @LIBTOOL_LINK_FLAGS@
