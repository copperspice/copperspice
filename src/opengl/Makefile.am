lib_LTLIBRARIES += lib/libCsOpenGL@BUILD_ABI@.la
lib_libCsOpenGL@BUILD_ABI@_la_SOURCES =

lib_libCsOpenGL@BUILD_ABI@_la_CXXFLAGS = $(AM_CXXFLAGS) -Iinclude -Iinclude/QtCore -Iinclude/QtGui  -Iinclude/QtOpenGL  \
   -Iprivateinclude/QtCore/private -Iprivateinclude/QtGui/private  -Iprivateinclude/QtOpenGL/private \
   -I$(top_srcdir)/src/3rdparty/harfbuzz/src -Isrc/opengl   \
   -DQT_NO_EGL   \
	-DQT_BUILD_OPENGL_LIB

lib_libCsOpenGL@BUILD_ABI@_la_OBJCXXFLAGS = $(lib_libCsOpenGL@BUILD_ABI@_la_CXXFLAGS)
lib_libCsOpenGL@BUILD_ABI@_la_LDFLAGS = $(AM_LDFLAGS) -no-undefined
lib_libCsOpenGL@BUILD_ABI@_la_LIBADD = lib/libCsCore@BUILD_ABI@.la lib/libCsGui@BUILD_ABI@.la

OPENGL_INCLUDES =
OPENGL_PUBLIC_INCLUDES =
OPENGL_PRIVATE_INCLUDES =
OPENGL_DIRS = src/opengl src/opengl/gl2paintengineex  src/opengl/util

EXTRA_DIST += \
	src/opengl/CMakeLists.txt

OPENGL_PUBLIC_INCLUDES += \
	QGLBuffer \
	QGLColormap \
	QGLContext \
	QGLFormat \
	QGLFramebufferObject \
	QGLFramebufferObjectFormat \
	QGLFunctions \
	QGLFunctionsPrivate \
	QGLPixelBuffer \
	QGLShader \
	QGLShaderProgram \
	QGLWidget \
	QMacCompatGLenum \
	QMacCompatGLint \
	QMacCompatGLuint \
	QMacGLCompatTypes \
	QtOpenGL

OPENGL_INCLUDES += \
	qgl.h \
	qglbuffer.h \
	qglcolormap.h \
	qglcontext.h \
	qglformat.h \
	qglframebufferobject.h  \
	qglframebufferobjectformat.h \
	qglfunctions.h \
	qglfunctionsprivate.h \
	qglpixelbuffer.h \
	qglshader.h \
	qglshaderprogram.h \
	qglwidget.h \
	qmaccompatglenum.h \
	qmaccompatglint.h \
	qmaccompatgluint.h \
	qmacglcompattypes.h \
	qtopengl.h

OPENGL_ROOT_PRIVATE_INCLUDES = \
	qgl_p.h \
	qglextensions_p.h \
	qglframebufferobject_p.h  \
	qglpaintdevice_p.h \
	qglpixelbuffer_p.h \
	qglpixmapfilter_p.h  \
	qgraphicsshadereffect_p.h \
	qgraphicssystem_gl_p.h \
	qpaintengine_opengl_p.h \
	qpixmapdata_gl_p.h \
	qpixmapdata_x11gl_p.h \
	qwindowsurface_gl_p.h

OPENGL_GL2_PRIVATE_INCLUDES = \
	qgl2pexvertexarray_p.h \
	qglcustomshaderstage_p.h \
	qglengineshadermanager_p.h \
	qglengineshadersource_p.h \
	qglgradientcache_p.h \
	qglshadercache_p.h \
	qpaintengineex_opengl2_p.h \
	qtextureglyphcache_gl_p.h \
	qtriangulatingstroker_p.h \
	qtriangulator_p.h

OPENGL_UTIL_PRIVATE_INCLUDES = fragmentprograms_p.h


OPENGL_PRIVATE_INCLUDES += $(OPENGL_ROOT_PRIVATE_INCLUDES)

lib_libCsOpenGL@BUILD_ABI@_la_SOURCES += \
	src/opengl/gl2paintengineex/qgl2pexvertexarray.cpp \
	src/opengl/gl2paintengineex/qglcustomshaderstage.cpp \
	src/opengl/gl2paintengineex/qglengineshadermanager.cpp \
	src/opengl/gl2paintengineex/qglgradientcache.cpp \
	src/opengl/gl2paintengineex/qpaintengineex_opengl2.cpp \
	src/opengl/gl2paintengineex/qtextureglyphcache_gl.cpp \
	src/opengl/gl2paintengineex/qtriangulatingstroker.cpp \
	src/opengl/gl2paintengineex/qtriangulator.cpp \
	src/opengl/qgl.cpp \
	src/opengl/qglbuffer.cpp \
	src/opengl/qglcolormap.cpp \
	src/opengl/qglextensions.cpp \
	src/opengl/qglframebufferobject.cpp \
	src/opengl/qglfunctions.cpp \
	src/opengl/qglpaintdevice.cpp \
	src/opengl/qglpixelbuffer.cpp \
	src/opengl/qglpixmapfilter.cpp \
	src/opengl/qglshaderprogram.cpp \
	src/opengl/qgraphicsshadereffect.cpp \
	src/opengl/qgraphicssystem_gl.cpp \
	src/opengl/qpaintengine_opengl.cpp \
	src/opengl/qpixmapdata_gl.cpp \
	src/opengl/qwindowsurface_gl.cpp


if X11
lib_libCsOpenGL@BUILD_ABI@_la_SOURCES += \
	src/opengl/qgl_x11.cpp \
	src/opengl/qglpixelbuffer_x11.cpp
endif

if COCOA
lib_libCsOpenGL@BUILD_ABI@_la_SOURCES += \
	src/opengl/qgl_mac.mm \
	src/opengl/qglpixelbuffer_mac.mm
lib_libCsOpenGL@BUILD_ABI@_la_LDFLAGS+= -framework AppKit -framework Carbon -framework OpenGL
endif

if OSTYPE_WIN
lib_libCsOpenGL@BUILD_ABI@_la_CXXFLAGS += -DQT_NO_STYLE_GTK -DQT_NO_STYLE_WINDOWSVISTA -DQT_BUILD_OPENGL_LIB
lib_libCsOpenGL@BUILD_ABI@_la_SOURCES += \
	src/opengl/qgl_win.cpp \
	src/opengl/qglpixelbuffer_win.cpp
lib_libCsOpenGL@BUILD_ABI@_la_LDFLAGS+= -lopengl32 -lgdi32 -lkernel32 -luser32
endif

noinst_HEADERS+=$(patsubst %,src/opengl/%,$(OPENGL_INCLUDES)) \
		$(patsubst %,src/opengl/%,$(OPENGL_ROOT_PRIVATE_INCLUDES)) \
		$(patsubst %,src/opengl/gl2paintengineex/%,$(OPENGL_GL2_PRIVATE_INCLUDES))  \
	$(patsubst %,src/opengl/util/%,$(OPENGL_UTIL_PRIVATE_INCLUDES))


OPENGL_HEADERS_INSTALLED= \
	$(patsubst %,include/QtOpenGL/%,$(OPENGL_INCLUDES)) \
	$(patsubst %,include/QtOpenGL/%,$(OPENGL_PUBLIC_INCLUDES)) \
	$(patsubst %,privateinclude/QtOpenGL/private/%,$(OPENGL_ROOT_PRIVATE_INCLUDES)) \
   $(patsubst %,privateinclude/QtOpenGL/private/%,$(OPENGL_GL2_PRIVATE_INCLUDES))  \
   $(patsubst %,privateinclude/QtOpenGL/private/%,$(OPENGL_UTIL_PRIVATE_INCLUDES)) \
	include/QtOpenGL privateinclude/QtOpenGL/private

BUILT_SOURCES+=.opengl-header-deps.mk

$(lib_libCsOpenGL@BUILD_ABI@_la_SOURCES) : $(CORE_HEADERS_INSTALLED) $(GUI_HEADERS_INSTALLED) $(OPENGL_HEADERS_INSTALLED)

-include .opengl-header-deps.mk

.opengl-header-deps.mk: Makefile
	@- rm -rf $@
	echo "include/QtOpenGL:" >> $@; \
	echo "	\$$(MKDIR_P) \$$@" >> $@ ; \
	echo "privateinclude/QtOpenGL/private:" >> $@; \
	echo "	\$$(MKDIR_P) \$$@" >> $@ ; \
	for i in $(OPENGL_PUBLIC_INCLUDES); do \
		echo "include/QtOpenGL/$$i: include/QtOpenGL/$$(echo $$i | tr A-Z a-z).h | include/QtOpenGL" >> $@; \
		echo "	\$$(INSTALL) \$$< \$$@" >> $@ ; \
	done; \
	for i in $(OPENGL_DIRS); do \
		echo "include/QtOpenGL/%: $$i/% | include/QtOpenGL" >> $@ ; \
		echo "	\$$(INSTALL) \$$< \$$@" >> $@ ; \
	done;\
	for i in $(OPENGL_DIRS); do \
		echo "privateinclude/QtOpenGL/private/%: $$i/% | privateinclude/QtOpenGL/private" >> $@ ; \
		echo "	\$$(INSTALL) \$$< \$$@" >> $@ ; \
	done
