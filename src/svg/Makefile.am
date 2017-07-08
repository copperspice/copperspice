lib_LTLIBRARIES += lib/libCsSvg@BUILD_ABI@.la
lib_libCsSvg@BUILD_ABI@_la_SOURCES =

lib_libCsSvg@BUILD_ABI@_la_CXXFLAGS = $(AM_CXXFLAGS) -Iinclude -Iinclude/QtCore  -Iinclude/QtGui -Iinclude/QtSvg \
   -Iprivateinclude/QtCore/private  -Iprivateinclude/QtGui/private  -Iprivateinclude/QtSvg/private   \
   -Isrc/svg  -I$(top_srcdir)/src/3rdparty/harfbuzz/src   \
   -DBUILD_DATE=\"`date +%Y-%m-%d`\"    \
	-DQT_BUILD_SVG_LIB

lib_libCsSvg@BUILD_ABI@_la_LDFLAGS = $(AM_LDFLAGS) -no-undefined
lib_libCsSvg@BUILD_ABI@_la_LIBADD = lib/libCsCore@BUILD_ABI@.la lib/libCsGui@BUILD_ABI@.la

if OSTYPE_WIN
lib_libCsSvg@BUILD_ABI@_la_LDFLAGS += -lws2_32
endif


SVG_PUBLIC_INCLUDES = \
	QtSvg \
	QGraphicsSvgItem \
	QSvgGenerator \
	QSvgRenderer \
	QSvgWidget

SVG_INCLUDES = \
	qsvgrenderer.h          \
	qsvgwidget.h            \
	qgraphicssvgitem.h      \
	qsvggenerator.h         \
	qtsvg.h

SVG_PRIVATE_INCLUDES = \
	qsvggraphics_p.h        \
	qsvghandler_p.h         \
	qsvgnode_p.h            \
	qsvgstructure_p.h       \
	qsvgstyle_p.h           \
	qsvgfont_p.h            \
	qsvgtinydocument_p.h

SVG_DIRS = src/svg
lib_libCsSvg@BUILD_ABI@_la_SOURCES+= \
	src/svg/qsvggraphics.cpp        \
	src/svg/qsvghandler.cpp         \
	src/svg/qsvgnode.cpp            \
	src/svg/qsvgstructure.cpp       \
	src/svg/qsvgstyle.cpp           \
	src/svg/qsvgfont.cpp            \
	src/svg/qsvgtinydocument.cpp    \
	src/svg/qsvgrenderer.cpp        \
	src/svg/qsvgwidget.cpp          \
	src/svg/qgraphicssvgitem.cpp    \
	src/svg/qsvggenerator.cpp

noinst_HEADERS+=$(patsubst %,src/svg/%,$(SVG_INCLUDES)) \
		$(patsubst %,src/svg/%,$(SVG_PRIVATE_INCLUDES))

SVG_HEADERS_INSTALLED= \
	$(patsubst %,include/QtSvg/%,$(SVG_INCLUDES)) \
	$(patsubst %,include/QtSvg/%,$(SVG_PUBLIC_INCLUDES)) \
	$(patsubst %,privateinclude/QtSvg/private/%,$(SVG_PRIVATE_INCLUDES)) \
	include/QtSvg privateinclude/QtSvg/private

BUILT_SOURCES+=.svg-header-deps.mk

$(lib_libCsSvg@BUILD_ABI@_la_SOURCES) : $(CORE_HEADERS_INSTALLED) $(GUI_HEADERS_INSTALLED) $(SVG_HEADERS_INSTALLED)

EXTRA_DIST += \
	src/svg/CMakeLists.txt


-include .svg-header-deps.mk

.svg-header-deps.mk: Makefile
	@- rm -rf $@
	echo "include/QtSvg:" >> $@; \
	echo "	\$$(MKDIR_P) \$$@" >> $@ ; \
	echo "privateinclude/QtSvg/private:" >> $@; \
	echo "	\$$(MKDIR_P) \$$@" >> $@ ; \
	for i in $(SVG_PUBLIC_INCLUDES); do \
		echo "include/QtSvg/$$i: include/QtSvg/$$(echo $$i | tr A-Z a-z).h | include/QtSvg" >> $@; \
		echo "	\$$(INSTALL) \$$< \$$@" >> $@ ; \
	done; \
	for i in $(SVG_DIRS); do \
		echo "include/QtSvg/%: $$i/% | include/QtSvg" >> $@ ; \
		echo "	\$$(INSTALL) \$$< \$$@" >> $@ ; \
	done;\
	for i in $(SVG_DIRS); do \
		echo "privateinclude/QtSvg/private/%: $$i/% | privateinclude/QtSvg/private" >> $@ ; \
		echo "	\$$(INSTALL) \$$< \$$@" >> $@ ; \
	done
