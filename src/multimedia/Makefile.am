lib_LTLIBRARIES += lib/libCsMultimedia1.la
lib_libCsMultimedia1_la_SOURCES =

lib_libCsMultimedia1_la_CXXFLAGS = $(AM_CXXFLAGS) -Iinclude -Iinclude/QtCore -Iinclude/QtGui -Iinclude/QtMultimedia \
   -Iprivateinclude/QtCore/private  -Iprivateinclude/QtMultimedia/private   \
   -DQT_BUILD_MULTIMEDIA_LIB   \
	-DQT_SHARED   \
	-DQT_MAKEDLL

lib_libCsMultimedia1_la_LDFLAGS = $(AM_LDFLAGS) -no-undefined
lib_libCsMultimedia1_la_LIBADD = lib/libCsCore1.la lib/libCsGui1.la

if OSTYPE_LINUX
lib_libCsMultimedia1_la_CXXFLAGS += -I$(top_srcdir)/mkspecs/linux-g++
endif

if OSTYPE_WIN
lib_libCsMultimedia1_la_CXXFLAGS += -I$(top_srcdir)/mkspecs/win32-g++
endif

if OSTYPE_DARWIN
lib_libCsMultimedia1_la_CXXFLAGS += -I$(top_srcdir)/mkspecs/macx-g++ -DQ_OS_UNIX
endif

MULTIMEDIA_INCLUDES =
MULTIMEDIA_PUBLIC_INCLUDES =
MULTIMEDIA_PRIVATE_INCLUDES =
MULTIMEDIA_DIRS =

include src/multimedia/audio/Makefile.am
include src/multimedia/video/Makefile.am

MULTIMEDIA_HEADERS_INSTALLED= \
	$(patsubst %,include/QtMultimedia/%,$(MULTIMEDIA_INCLUDES)) \
	$(patsubst %,include/QtMultimedia/%,$(MULTIMEDIA_PUBLIC_INCLUDES)) \
	$(patsubst %,privateinclude/QtMultimedia/private/%,$(MULTIMEDIA_PRIVATE_INCLUDES))

BUILT_SOURCES+=.multimedia-header-deps.mk

$(lib_libCsMultimedia1_la_SOURCES) : $(CORE_HEADERS_INSTALLED) $(GUI_HEADERS_INSTALLED) $(MULTIMEDIA_HEADERS_INSTALLED)

-include .multimedia-header-deps.mk

.multimedia-header-deps.mk: Makefile
	@- rm -rf $@
	for i in $(MULTIMEDIA_PUBLIC_INCLUDES); do \
		echo "include/QtMultimedia/$$i: include/QtMultimedia/$$(echo $$i | tr A-Z a-z).h" >> $@; \
		echo "	\$$(INSTALL) -D \$$^ \$$@" >> $@ ; \
	done; \
	for i in $(MULTIMEDIA_DIRS); do \
		echo "include/QtMultimedia/%: $$i/%" >> $@ ; \
		echo "	\$$(INSTALL) -D \$$^ \$$@" >> $@ ; \
	done;\
	for i in $(MULTIMEDIA_DIRS); do \
		echo "privateinclude/QtMultimedia/private/%: $$i/%" >> $@ ; \
		echo "	\$$(INSTALL) -D \$$^ \$$@" >> $@ ; \
	done
