lib_LTLIBRARIES += lib/libCsGui1.la
lib_libCsGui1_la_SOURCES =
nodist_lib_libCsGui1_la_SOURCES = 

lib_libCsGui1_la_CXXFLAGS = $(AM_CXXFLAGS) -Iinclude  -Iinclude/QtGui -Iinclude/QtCore     \
    -Iprivateinclude/QtCore -Iprivateinclude/QtCore/private -Iprivateinclude -Iprivateinclude/QtGui/private  -Iprivateinclude/QtGui  \
    -I$(top_srcdir)/src/3rdparty/harfbuzz/src -I$(top_srcdir)/src/3rdparty/wintab \
    -I$(top_srcdir)/src/3rdparty/zlib         \
    -I$(top_srcdir)/src/3rdparty/libtiff/libtiff  \
    -I$(top_srcdir)/src/3rdparty/libmng  -I$(top_srcdir)/src/3rdparty/libjpeg  \
    -I$(top_srcdir)/src/gui/kernel \
    -Isrc/gui/graphicsview  -Isrc/gui/itemviews  -Isrc/gui/dialogs \
    -Isrc/gui/image  -Isrc/gui/kernel    \
    -Isrc/gui/styles  -Isrc/gui/text  -Isrc/gui/util  -Isrc/gui/widgets  -Isrc/gui/painting  \
    -DBUILD_DATE=\"`date +%Y-%m-%d`\"    \
    -DQT_BUILD_GUI_LIB  \
    -DQT_SHARED         \
    -DQT_BUILTIN_GIF_READER      \
    -DMNG_BUILD_SO               \
    -DMNG_NO_INCLUDE_JNG         \
    -DPNG_NO_CONFIG_H

lib_libCsGui1_la_CFLAGS = $(lib_libCsGui1_la_CXXFLAGS)
lib_libCsGui1_la_OBJCXXFLAGS =
lib_libCsGui1_la_LDFLAGS = $(AM_LDFLAGS) -no-undefined
lib_libCsGui1_la_LIBADD = lib/libCsCore1.la

if OSTYPE_LINUX
lib_libCsGui1_la_CXXFLAGS += -DQT_NO_XINPUT -DQT_NO_STYLE_MAC -DQT_NO_STYLE_WINDOWSXP -DQT_NO_STYLE_WINDOWSVISTA \
    -DQT_NO_TABLET $(FONTCONFIG_CFLAGS)

lib_libCsGui1_la_LDFLAGS += $(FONTCONFIG_LIBS) -lX11 -lXrender -lXrandr -lXcursor -lXinerama -lSM -lICE -laudio -lpthread -lrt
endif

if OSTYPE_BSD
lib_libCsGui1_la_CXXFLAGS += -DQT_NO_XINPUT -DQT_NO_STYLE_MAC -DQT_NO_STYLE_WINDOWSXP -DQT_NO_STYLE_WINDOWSVISTA \
    -DQT_NO_TABLET $(FONTCONFIG_CFLAGS)

lib_libCsGui1_la_LDFLAGS += $(FONTCONFIG_LIBS) -lX11 -lXrender -lXrandr -lXcursor -lXinerama -lSM -lICE -laudio -lpthread -lrt
endif

if OSTYPE_WIN
lib_libCsGui1_la_CXXFLAGS += -DQT_NO_STYLE_MAC -DQT_NO_LPR -DQT_NO_DIRECTWRITE -DQT_MAKEDLL
lib_libCsGui1_la_LDFLAGS += -lgdi32 -lwinmm -lole32 -lwinspool -loleaut32 -lcomdlg32 -limm32 -Wl,-luuid --export-all-symbols
endif

if OSTYPE_DARWIN
nodist_lib_libCsGui1_la_SOURCES += src/gui/mac/qrc_macresources.cpp
lib_libCsGui1_la_OBJCXXFLAGS += $(lib_libCsGui1_la_CXXFLAGS)
lib_libCsGui1_la_CXXFLAGS += -DQT_NO_STYLE_WINDOWSVISTA
lib_libCsGui1_la_LDFLAGS += -lobjc -framework Carbon
endif

EXTRA_DIST += src/gui/mac/macresources.qrc \
	src/gui/mac/images/copyarrowcursor.png \
	src/gui/mac/images/forbiddencursor.png \
	src/gui/mac/images/leopard-unified-toolbar-on.png \
	src/gui/mac/images/pluscursor.png \
	src/gui/mac/images/spincursor.png \
	src/gui/mac/images/waitcursor.png \
	src/gui/CMakeLists.txt \
	src/gui/accessible/accessible.cmake \
	src/gui/animation/animation.cmake \
	src/gui/dialogs/dialogs.cmake \
	src/gui/effects/effects.cmake \
	src/gui/graphicsview/graphicsview.cmake \
	src/gui/image/image.cmake \
	src/gui/inputmethod/inputmethod.cmake \
	src/gui/itemviews/itemviews.cmake \
	src/gui/kernel/kernel.cmake \
	src/gui/math3d/math3d.cmake \
	src/gui/painting/painting.cmake \
	src/gui/statemachine/statemachine.cmake \
	src/gui/styles/styles.cmake \
	src/gui/text/text.cmake \
	src/gui/util/util.cmake \
	src/gui/widgets/widgets.cmake

GUI_INCLUDES =
GUI_PUBLIC_INCLUDES =
GUI_PRIVATE_INCLUDES =
GUI_DIRS =

include src/gui/accessible/Makefile.am
include src/gui/animation/Makefile.am
include src/gui/dialogs/Makefile.am
include src/gui/effects/Makefile.am
include src/gui/graphicsview/Makefile.am
include src/gui/image/Makefile.am
include src/gui/inputmethod/Makefile.am
include src/gui/itemviews/Makefile.am
include src/gui/kernel/Makefile.am
include src/gui/math3d/Makefile.am
include src/gui/painting/Makefile.am
include src/gui/statemachine/Makefile.am
include src/gui/styles/Makefile.am
include src/gui/text/Makefile.am
include src/gui/util/Makefile.am
include src/gui/widgets/Makefile.am

# currently unused
# include src/gui/embedded/Makefile.am

GUI_HEADERS_INSTALLED= \
	$(patsubst %,include/QtGui/%,$(GUI_INCLUDES)) \
	$(patsubst %,include/QtGui/%,$(GUI_PUBLIC_INCLUDES)) \
	$(patsubst %,privateinclude/QtGui/private/%,$(GUI_PRIVATE_INCLUDES)) \
	include/QtGui privateinclude/QtGui/private

BUILT_SOURCES+=.gui-header-deps.mk

$(lib_libCsGui1_la_SOURCES) : $(CORE_HEADERS_INSTALLED) $(GUI_HEADERS_INSTALLED)

-include .gui-header-deps.mk

.gui-header-deps.mk: Makefile
	@- rm -rf $@
	echo "include/QtGui:" >> $@; \
	echo "	\$$(MKDIR_P) \$$@" >> $@ ; \
	echo "privateinclude/QtGui/private:" >> $@; \
	echo "	\$$(MKDIR_P) \$$@" >> $@ ; \
	for i in $(GUI_PUBLIC_INCLUDES); do \
		echo "include/QtGui/$$i: include/QtGui/$$(echo $$i | tr A-Z a-z).h | include/QtGui" >> $@; \
		echo "	\$$(INSTALL) \$$< \$$@" >> $@ ; \
	done; \
	for i in $(GUI_DIRS); do \
		echo "include/QtGui/%: $$i/% | include/QtGui" >> $@ ; \
		echo "	\$$(INSTALL) \$$< \$$@" >> $@ ; \
	done;\
	for i in $(GUI_DIRS); do \
		echo "privateinclude/QtGui/private/%: $$i/% | privateinclude/QtGui/private" >> $@ ; \
		echo "	\$$(INSTALL) \$$< \$$@" >> $@ ; \
	done


macnibdir=$(prefix)/mac/qt_menu.nib
dist_macnib_DATA =

if OSTYPE_DARWIN
dist_macnib_DATA += \
		src/gui/mac/qt_menu.nib/classes.nib \
		src/gui/mac/qt_menu.nib/info.nib \
		src/gui/mac/qt_menu.nib/keyedobjects.nib
endif
