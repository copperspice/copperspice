lib_LTLIBRARIES += lib/libCsCore@BUILD_ABI@.la
lib_libCsCore@BUILD_ABI@_la_SOURCES =
lib_libCsCore@BUILD_ABI@_la_LIB = lib/core/libkernel.la

lib_libCsCore@BUILD_ABI@_la_CXXFLAGS = $(AM_CXXFLAGS) -Iinclude -Iinclude/QtCore -Iprivateinclude/QtCore \
   -Iprivateinclude/QtCore/private \
   -I$(top_srcdir)/src/core/io -Isrc/core/animation -Isrc/core/kernel \
   -Isrc/core/io -Isrc/core/statemachine \
   -I$(top_srcdir)/src/3rdparty/harfbuzz/src   \
   -DBUILD_DATE=\"`date +%Y-%m-%d`\"   \
   -DQT_BUILD_CORE_LIB  \
   -DBUILDING_LIB_CS_SIGNAL \
   -DBUILDING_LIB_CS_STRING

lib_libCsCore@BUILD_ABI@_la_CFLAGS = $(AM_CFLAGS) -DQT_BUILD_CORE_LIB
lib_libCsCore@BUILD_ABI@_la_OBJCXXFLAGS = $(lib_libCsCore@BUILD_ABI@_la_CXXFLAGS)
lib_libCsCore@BUILD_ABI@_la_LDFLAGS = $(AM_LDFLAGS) -no-undefined

if OSTYPE_LINUX
lib_libCsCore@BUILD_ABI@_la_LDFLAGS += -ldl -lpthread
lib_libCsCore@BUILD_ABI@_la_CXXFLAGS += -I$(top_srcdir)/src/3rdparty/forkfd
endif

if OSTYPE_BSD
lib_libCsCore@BUILD_ABI@_la_LDFLAGS += -lpthread -L/usr/local/lib -liconv
lib_libCsCore@BUILD_ABI@_la_CXXFLAGS += -I$(top_srcdir)/src/3rdparty/forkfd
endif

if OSTYPE_WIN
lib_libCsCore@BUILD_ABI@_la_LDFLAGS += -lole32 -lws2_32 -Wl,-luuid
endif

if OSTYPE_DARWIN
lib_libCsCore@BUILD_ABI@_la_LDFLAGS += -framework CoreFoundation -framework Foundation \
   -framework Security -framework ApplicationServices -lobjc
lib_libCsCore@BUILD_ABI@_la_CXXFLAGS += -I$(top_srcdir)/src/3rdparty/forkfd
endif

if ZLIB
lib_libCsCore@BUILD_ABI@_la_CXXFLAGS += $(ZLIB_CFLAGS)
lib_libCsCore@BUILD_ABI@_la_LDFLAGS += $(ZLIB_LIBS)
else
lib_libCsCore@BUILD_ABI@_la_CXXFLAGS +=  -I$(top_srcdir)/src/3rdparty/zlib
lib_libCsCore@BUILD_ABI@_la_CFLAGS += -Iinclude/QtCore -I$(top_srcdir)/src/3rdparty/zlib
lib_libCsCore@BUILD_ABI@_la_SOURCES += \
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

include src/core/animation/Makefile.am
include src/core/codecs/Makefile.am
include src/core/concurrent/Makefile.am
include src/core/global/Makefile.am
include src/core/io/Makefile.am
include src/core/json/Makefile.am
include src/core/kernel/Makefile.am
include src/core/plugin/Makefile.am
include src/core/signal/Makefile.am
include src/core/statemachine/Makefile.am
include src/core/string/Makefile.am
include src/core/thread/Makefile.am
include src/core/tools/Makefile.am
include src/core/xml/Makefile.am

EXTRA_DIST+= \
	src/core/CMakeLists.txt \
	src/core/animation/animation.cmake \
	src/core/codecs/codecs.cmake \
	src/core/concurrent/concurrent.cmake \
	src/core/global/global.cmake \
	src/core/io/io.cmake \
	src/core/json/json.cmake \
	src/core/kernel/kernel.cmake \
	src/core/plugin/plugin.cmake \
	src/core/signal/signal.cmake \
	src/core/statemachine/statemachine.cmake \
	src/core/string/string.cmake \
	src/core/thread/thread.cmake \
	src/core/tools/tools.cmake \
	src/core/xml/xml.cmake


CORE_HEADERS_INSTALLED= \
	$(patsubst %,include/QtCore/%,$(CORE_INCLUDES)) \
	$(patsubst %,include/QtCore/%,$(CORE_PUBLIC_INCLUDES)) \
	$(patsubst %,privateinclude/QtCore/private/%,$(CORE_PRIVATE_INCLUDES)) \
	include/QtCore \
	include/qt-acconfig.h \
	privateinclude/QtCore/private


BUILT_SOURCES+=.core-header-deps.mk

$(lib_libCsCore@BUILD_ABI@_la_SOURCES): $(CORE_HEADERS_INSTALLED)

-include .core-header-deps.mk

.core-header-deps.mk: Makefile
	@- rm -rf $@
	echo "include/QtCore:" >> $@ ; \
	echo "	\$$(MKDIR_P) \$$@" >> $@ ; \
	echo "	\$$(MKDIR_P) \$$@/regex" >> $@ ; \
	echo "privateinclude/QtCore/private:" >> $@; \
	echo "	\$$(MKDIR_P) \$$@" >> $@ ; \
	for i in $(CORE_PUBLIC_INCLUDES); do \
		echo "include/QtCore/$$i: include/QtCore/$$(echo $$i | tr A-Z a-z).h | include/QtCore" >> $@ ; \
		echo "	\$$(INSTALL) \$$< \$$@" >> $@ ; \
	done; \
	for i in $(CORE_DIRS); do \
		echo "include/QtCore/%: $$i/% | include/QtCore" >> $@ ; \
		echo "	\$$(INSTALL) \$$< \$$@" >> $@ ; \
	done;\
	for i in $(CORE_DIRS); do \
		echo "privateinclude/QtCore/private/%: $$i/% | privateinclude/QtCore/private" >> $@ ; \
		echo "	\$$(INSTALL) \$$< \$$@" >> $@ ; \
	done

EXTRA_DIST+= \
	src/3rdparty/zlib/adler32.c \
	src/3rdparty/zlib/compress.c \
	src/3rdparty/zlib/crc32.c \
	src/3rdparty/zlib/crc32.h \
	src/3rdparty/zlib/deflate.c \
	src/3rdparty/zlib/deflate.h \
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
	src/3rdparty/zlib/trees.c \
	src/3rdparty/zlib/trees.h \
	src/3rdparty/zlib/uncompr.c \
	src/3rdparty/zlib/zconf.h \
	src/3rdparty/zlib/zlib.3 \
	src/3rdparty/zlib/zlib.h \
	src/3rdparty/zlib/zutil.c \
	src/3rdparty/zlib/zutil.h \
	src/3rdparty/forkfd/forkfd.h \
	src/3rdparty/forkfd/forkfd.c \
	src/3rdparty/forkfd/forkfd_gcc.h

macdir=$(prefix)/mac
dist_mac_DATA=
if OSTYPE_DARWIN
dist_mac_DATA+=src/core/global/Info.plist.app
endif
