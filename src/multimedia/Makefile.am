if MULTIMEDIA
lib_LTLIBRARIES += lib/libCsMultimedia@BUILD_ABI@.la
endif

lib_libCsMultimedia@BUILD_ABI@_la_SOURCES =

lib_libCsMultimedia@BUILD_ABI@_la_CXXFLAGS = $(AM_CXXFLAGS) -Iinclude -Iinclude/QtCore -Iinclude/QtGui -Iinclude/QtMultimedia \
   -Iprivateinclude/QtCore/private  -Iprivateinclude/QtMultimedia/private   \
   -DQT_BUILD_MULTIMEDIA_LIB

lib_libCsMultimedia@BUILD_ABI@_la_LDFLAGS = $(AM_LDFLAGS) -no-undefined
lib_libCsMultimedia@BUILD_ABI@_la_LIBADD = lib/libCsCore@BUILD_ABI@.la lib/libCsGui@BUILD_ABI@.la

MULTIMEDIA_INCLUDES =
MULTIMEDIA_PUBLIC_INCLUDES =
MULTIMEDIA_PRIVATE_INCLUDES =
MULTIMEDIA_DIRS =

EXTRA_DIST += \
	src/multimedia/CMakeLists.txt \
	src/multimedia/audio/audio.cmake \
	src/multimedia/video/video.cmake

include src/multimedia/audio/Makefile.am
include src/multimedia/video/Makefile.am

MULTIMEDIA_HEADERS_INSTALLED= \
	$(patsubst %,include/QtMultimedia/%,$(MULTIMEDIA_INCLUDES)) \
	$(patsubst %,include/QtMultimedia/%,$(MULTIMEDIA_PUBLIC_INCLUDES)) \
	$(patsubst %,privateinclude/QtMultimedia/private/%,$(MULTIMEDIA_PRIVATE_INCLUDES)) \
	include/QtMultimedia \
	privateinclude/QtMultimedia/private

BUILT_SOURCES+=.multimedia-header-deps.mk

$(lib_libCsMultimedia@BUILD_ABI@_la_SOURCES) : $(CORE_HEADERS_INSTALLED) $(GUI_HEADERS_INSTALLED) $(MULTIMEDIA_HEADERS_INSTALLED) include/QtMultimedia privateinclude/QtMultimedia/private

-include .multimedia-header-deps.mk

.multimedia-header-deps.mk: Makefile
	@- rm -rf $@
	echo "include/QtMultimedia:" >> $@; \
	echo "	\$$(MKDIR_P) \$$@" >> $@ ; \
	echo "privateinclude/QtMultimedia/private:" >> $@; \
	echo "	\$$(MKDIR_P) \$$@" >> $@ ; \
	for i in $(MULTIMEDIA_PUBLIC_INCLUDES); do \
		echo "include/QtMultimedia/$$i: include/QtMultimedia/$$(echo $$i | tr A-Z a-z).h | include/QtMultimedia" >> $@; \
		echo "	\$$(INSTALL) \$$< \$$@" >> $@ ; \
	done; \
	for i in $(MULTIMEDIA_DIRS); do \
		echo "include/QtMultimedia/%: $$i/% | include/QtMultimedia" >> $@ ; \
		echo "	\$$(INSTALL) \$$< \$$@" >> $@ ; \
	done;\
	for i in $(MULTIMEDIA_DIRS); do \
		echo "privateinclude/QtMultimedia/private/%: $$i/% | privateinclude/QtMultimedia/private" >> $@ ; \
		echo "	\$$(INSTALL) \$$< \$$@" >> $@ ; \
	done
