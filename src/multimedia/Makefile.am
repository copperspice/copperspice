if MULTIMEDIA
lib_LTLIBRARIES += lib/libCsMultimedia@BUILD_ABI@.la
endif

lib_libCsMultimedia@BUILD_ABI@_la_SOURCES =

lib_libCsMultimedia@BUILD_ABI@_la_CXXFLAGS = -Iinclude \
   -Iinclude/QtCore -Iprivateinclude/QtCore/private  \
   -Iinclude/QtGui \
   -Iinclude/QtNetwork \
   -Iinclude/QtOpenGL \
   -Iinclude/QtMultimedia -Iprivateinclude/QtMultimedia/private \
   $(AM_CXXFLAGS) \
   -DBUILD_DATE=\"`date +%Y-%m-%d`\" \
   -DQT_BUILD_MULTIMEDIA_LIB

lib_libCsMultimedia@BUILD_ABI@_la_LDFLAGS = $(AM_LDFLAGS) -no-undefined

lib_libCsMultimedia@BUILD_ABI@_la_LIBADD = lib/libCsCore@BUILD_ABI@.la  lib/libCsGui@BUILD_ABI@.la \
   lib/libCsNetwork@BUILD_ABI@.la lib/libCsOpenGL@BUILD_ABI@.la

MULTIMEDIA_INCLUDES =
MULTIMEDIA_PUBLIC_INCLUDES =
MULTIMEDIA_PRIVATE_INCLUDES =
MULTIMEDIA_DIRS =

EXTRA_DIST += \
	src/multimedia/CMakeLists.txt \
	src/multimedia/audio/audio.cmake \
	src/multimedia/controls/controls.cmake \
	src/multimedia/kernel/kernel.cmake \
	src/multimedia/playback/playback.cmake \
	src/multimedia/radio/radio.cmake \
	src/multimedia/recording/recording.cmake \
	src/multimedia/video/video.cmake \
	src/multimedia/widgets/widgets.cmake \
	src/plugins/multimedia/mediaservices/avfoundation/avfoundation.cmake \
	src/plugins/multimedia/mediaservices/directshow/directshow.cmake \
	src/plugins/multimedia/mediaservices/gstreamer/gstreamer.cmake \
	src/plugins/multimedia/playlistformats/playlistformats.cmake

include src/multimedia/audio/Makefile.am
include src/multimedia/controls/Makefile.am
include src/multimedia/kernel/Makefile.am
include src/multimedia/playback/Makefile.am
include src/multimedia/radio/Makefile.am
include src/multimedia/recording/Makefile.am
include src/multimedia/video/Makefile.am
include src/multimedia/widgets/Makefile.am

# plugins
include src/plugins/multimedia/mediaservices/avfoundation/Makefile.am
include src/plugins/multimedia/mediaservices/directshow/Makefile.am
include src/plugins/multimedia/mediaservices/gstreamer/Makefile.am
include src/plugins/multimedia/playlistformats/Makefile.am

MULTIMEDIA_HEADERS_INSTALLED= \
	$(patsubst %,include/QtMultimedia/%,$(MULTIMEDIA_INCLUDES)) \
	$(patsubst %,include/QtMultimedia/%,$(MULTIMEDIA_PUBLIC_INCLUDES)) \
	$(patsubst %,privateinclude/QtMultimedia/private/%,$(MULTIMEDIA_PRIVATE_INCLUDES)) \
	include/QtMultimedia \
	privateinclude/QtMultimedia/private

BUILT_SOURCES+=.multimedia-header-deps.mk

$(lib_libCsMultimedia@BUILD_ABI@_la_SOURCES) : $(CORE_HEADERS_INSTALLED) $(GUI_HEADERS_INSTALLED) \
   $(NETWORK_HEADERS_INSTALLED) $(MULTIMEDIA_HEADERS_INSTALLED) $(OPENGL_HEADERS_INSTALLED) \
   include/QtMultimedia privateinclude/QtMultimedia/private

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
