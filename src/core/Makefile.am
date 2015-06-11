lib_LTLIBRARIES += lib/libCsCore1.la
lib_libCsCore1_la_SOURCES =
lib_libCsCore1_la_LIB = lib/core/libkernel.la

lib_libCsCore1_la_CXXFLAGS = $(AM_CXXFLAGS) -Iinclude -Iinclude/QtCore -Iprivateinclude/QtCore \
   -Iprivateinclude/QtCore/private \
   -I$(top_srcdir)/src/core/io -Isrc/core/animation -Isrc/core/kernel \
   -Isrc/core/io -Isrc/core/statemachine \
   -I$(top_srcdir)/src/3rdparty/harfbuzz/src   \
   -DBUILD_DATE=\"`date +%Y-%m-%d`\"   \
   -DQT_BUILD_CORE_LIB  \
   -DQT_SHARED   \
   -DQT_MAKEDLL

lib_libCsCore1_la_CFLAGS = $(AM_CFLAGS)
lib_libCsCore1_la_OBJCXXFLAGS = $(lib_libCsCore1_la_CXXFLAGS)
lib_libCsCore1_la_LDFLAGS = $(AM_LDFLAGS) -no-undefined

if OSTYPE_LINUX
lib_libCsCore1_la_LDFLAGS += -ldl -lpthread
endif

if OSTYPE_WIN
lib_libCsCore1_la_CFLAGS += -DQT_MAKEDLL
lib_libCsCore1_la_LDFLAGS += -lole32 -lws2_32 -Wl,-luuid
endif

if OSTYPE_DARWIN
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

include src/core/arch/Makefile.am
include src/core/animation/Makefile.am
include src/core/codecs/Makefile.am
include src/core/concurrent/Makefile.am
include src/core/global/Makefile.am
include src/core/io/Makefile.am
include src/core/json/Makefile.am
include src/core/kernel/Makefile.am
include src/core/plugin/Makefile.am
include src/core/statemachine/Makefile.am
include src/core/thread/Makefile.am
include src/core/tools/Makefile.am
include src/core/xml/Makefile.am

CORE_HEADERS_INSTALLED= \
	$(patsubst %,include/QtCore/%,$(CORE_INCLUDES)) \
	$(patsubst %,include/QtCore/%,$(CORE_PUBLIC_INCLUDES)) \
	$(patsubst %,privateinclude/QtCore/private/%,$(CORE_PRIVATE_INCLUDES))

BUILT_SOURCES+=.core-header-deps.mk

$(lib_libCsCore1_la_SOURCES): $(CORE_HEADERS_INSTALLED)

-include .core-header-deps.mk

.core-header-deps.mk: Makefile
	@- rm -rf $@
	echo "include/QtCore:" >> $@; \
	echo "	\$$(MKDIR_P) \$$@" >> $@ ; \
	echo "privateinclude/QtCore/private:" >> $@; \
	echo "	\$$(MKDIR_P) \$$@" >> $@ ; \
	for i in $(CORE_PUBLIC_INCLUDES); do \
		echo "include/QtCore/$$i: include/QtCore/$$(echo $$i | tr A-Z a-z).h include/QtCore" >> $@; \
		echo "	\$$(INSTALL) \$$< \$$@" >> $@ ; \
	done; \
	for i in $(CORE_DIRS); do \
		echo "include/QtCore/%: $$i/% include/QtCore" >> $@ ; \
		echo "	\$$(INSTALL) \$$< \$$@" >> $@ ; \
	done;\
	for i in $(CORE_DIRS); do \
		echo "privateinclude/QtCore/private/%: $$i/% privateinclude/QtCore/private" >> $@ ; \
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
	src/3rdparty/zlib/zutil.h

macdir=$(prefix)/mac
dist_mac_DATA=
if OSTYPE_DARWIN
dist_mac_DATA+=src/core/global/Info.plist.app
endif
