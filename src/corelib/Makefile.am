lib_LTLIBRARIES += lib/libCsCore1.la
lib_libCsCore1_la_SOURCES =
lib_libCsCore1_la_LIB = lib/core/libkernel.la

lib_libCsCore1_la_CXXFLAGS = $(AM_CXXFLAGS) -Iinclude -Iinclude/QtCore -Iprivateinclude/QtCore \
   -Iprivateinclude/QtCore/private \
   -I$(top_srcdir)/src/corelib/io -Isrc/corelib/animation -Isrc/corelib/kernel \
   -Isrc/corelib/io -Isrc/corelib/statemachine \
   -I$(top_srcdir)/src/3rdparty/harfbuzz/src   \
   -DBUILD_DATE=\"`date +%Y-%m-%d`\"   \
   -DQT_BUILD_CORE_LIB  \
   -DQT_SHARED   \
   -DQT_MAKEDLL

lib_libCsCore1_la_CFLAGS = $(AM_CFLAGS)
lib_libCsCore1_la_OBJCXXFLAGS = $(lib_libCsCore1_la_CXXFLAGS)
lib_libCsCore1_la_LDFLAGS = $(AM_LDFLAGS) -no-undefined

EXTRA_DIST += mkspecs/common/posix/qplatformdefs.h \
              mkspecs/common/c89/qplatformdefs.h \
              mkspecs/common/mac/qplatformdefs.h \
              mkspecs/linux-g++/qplatformdefs.h \
              mkspecs/win32-g++/qplatformdefs.h \
              mkspecs/macx-g++/qplatformdefs.h

if OSTYPE_LINUX
lib_libCsCore1_la_CXXFLAGS += -I$(top_srcdir)/mkspecs/linux-g++
lib_libCsCore1_la_LDFLAGS += -ldl -lpthread
endif

if OSTYPE_WIN
lib_libCsCore1_la_CXXFLAGS += -I$(top_srcdir)/mkspecs/win32-g++
lib_libCsCore1_la_CFLAGS += -I$(top_srcdir)/mkspecs/win32-g++ -DQT_MAKEDLL
lib_libCsCore1_la_LDFLAGS += -lole32 -lws2_32 -Wl,-luuid
endif

if OSTYPE_DARWIN
lib_libCsCore1_la_CFLAGS += -I$(top_srcdir)/mkspecs/macx-g++
lib_libCsCore1_la_CXXFLAGS += -I$(top_srcdir)/mkspecs/macx-g++
lib_libCsCore1_la_OBJCXXFLAGS += -I$(top_srcdir)/mkspecs/macx-g++
lib_libCsCore1_la_LDFLAGS += -framework CoreFoundation -framework Security -framework ApplicationServices -lObjC
endif

if ZLIB
lib_libCsCore1_la_CXXFLAGS += $(ZLIB_CFLAGS)
lib_libCsCore1_la_LDFLAGS += $(ZLIB_LIBS)
else
lib_libCsCore1_la_CXXFLAGS +=  -I$(top_srcdir)/src/3rdparty/zlib
lib_libCsCore1_la_CFLAGS += -Iinclude/QtCore -I$(top_srcdir)/src/3rdparty/zlib
lib_libCsCore1_la_SOURCES += \
	src/3rdparty/zlib/adler32.c \
	src/3rdparty/zlib/compress.c \
	src/3rdparty/zlib/crc32.c \
	src/3rdparty/zlib/deflate.c \
	src/3rdparty/zlib/gzclose.c \
	src/3rdparty/zlib/gzlib.c \
	src/3rdparty/zlib/gzread.c \
	src/3rdparty/zlib/gzwrite.c \
	src/3rdparty/zlib/infback.c \
	src/3rdparty/zlib/inffast.c \
	src/3rdparty/zlib/inflate.c \
	src/3rdparty/zlib/inftrees.c \
	src/3rdparty/zlib/trees.c \
	src/3rdparty/zlib/uncompr.c \
	src/3rdparty/zlib/zutil.c

endif

CORE_INCLUDES =
CORE_PUBLIC_INCLUDES =
CORE_PRIVATE_INCLUDES =
CORE_DIRS =

include src/corelib/arch/Makefile.am
include src/corelib/animation/Makefile.am
include src/corelib/codecs/Makefile.am
include src/corelib/concurrent/Makefile.am
include src/corelib/global/Makefile.am
include src/corelib/io/Makefile.am
include src/corelib/json/Makefile.am
include src/corelib/kernel/Makefile.am
include src/corelib/plugin/Makefile.am
include src/corelib/statemachine/Makefile.am
include src/corelib/thread/Makefile.am
include src/corelib/tools/Makefile.am
include src/corelib/xml/Makefile.am

CORE_HEADERS_INSTALLED= \
	$(patsubst %,include/QtCore/%,$(CORE_INCLUDES)) \
	$(patsubst %,include/QtCore/%,$(CORE_PUBLIC_INCLUDES)) \
	$(patsubst %,privateinclude/QtCore/private/%,$(CORE_PRIVATE_INCLUDES))

BUILT_SOURCES+=.core-header-deps.mk

$(lib_libCsCore1_la_SOURCES): $(CORE_HEADERS_INSTALLED)

-include .core-header-deps.mk

.core-header-deps.mk: Makefile
	@- rm -rf $@
	for i in $(CORE_PUBLIC_INCLUDES); do \
		echo "include/QtCore/$$i: include/QtCore/$$(echo $$i | tr A-Z a-z).h" >> $@; \
		echo "	\$$(INSTALL) -D \$$^ \$$@" >> $@ ; \
	done; \
	for i in $(CORE_DIRS); do \
		echo "include/QtCore/%: $$i/%" >> $@ ; \
		echo "	\$$(INSTALL) -D \$$^ \$$@" >> $@ ; \
	done;\
	for i in $(CORE_DIRS); do \
		echo "privateinclude/QtCore/private/%: $$i/%" >> $@ ; \
		echo "	\$$(INSTALL) -D \$$^ \$$@" >> $@ ; \
	done

EXTRA_DIST+= \
	src/3rdparty/zlib/adler32.c \
	src/3rdparty/zlib/compress.c \
	src/3rdparty/zlib/crc32.c \
	src/3rdparty/zlib/crc32.h \
	src/3rdparty/zlib/deflate.c \
	src/3rdparty/zlib/deflate.h \
	src/3rdparty/zlib/example.c \
	src/3rdparty/zlib/gzclose.c \
	src/3rdparty/zlib/gzguts.h \
	src/3rdparty/zlib/gzlib.c \
	src/3rdparty/zlib/gzread.c \
	src/3rdparty/zlib/gzwrite.c \
	src/3rdparty/zlib/infback.c \
	src/3rdparty/zlib/inffast.c \
	src/3rdparty/zlib/inffast.h \
	src/3rdparty/zlib/inffixed.h \
	src/3rdparty/zlib/inflate.c \
	src/3rdparty/zlib/inflate.h \
	src/3rdparty/zlib/inftrees.c \
	src/3rdparty/zlib/inftrees.h \
	src/3rdparty/zlib/minigzip.c \
	src/3rdparty/zlib/trees.c \
	src/3rdparty/zlib/trees.h \
	src/3rdparty/zlib/uncompr.c \
	src/3rdparty/zlib/zconf.h \
	src/3rdparty/zlib/zlib.3 \
	src/3rdparty/zlib/zlib.h \
	src/3rdparty/zlib/zutil.c \
	src/3rdparty/zlib/zutil.h

macdir=$(prefix)/mac
dist_mac_DATA=
if OSTYPE_DARWIN
dist_mac_DATA+=mkspecs/macx-g++/Info.plist.app
endif