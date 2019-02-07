GUI_DIRS += src/gui/opengl

GUI_PUBLIC_INCLUDES += \
	QOpenGL \
	QOpenGLBuffer \
	QOpenGLDebugMessage \
	QOpenGLFunctions \
	QOpenGLFunctions_1_0 \
	QOpenGLFunctions_2_0 \
	QOpenGLFunctions_3_0 \
	QOpenGLFunctions_3_2_Core \
	QOpenGLShader \
	QOpenGLShaderProgram \
	QOpenGLVertexArrayObject \
	QOpenGLVersionFunctions


GUI_OPENGL_INCLUDES = \
	qopengl.h \
	qopengles2ext.h \
	qopenglbuffer.h \
	qopengldebug.h \
	qopengldebugmessage.h \
	qopenglext.h \
	qopenglextrafunctions.h \
	qopenglframebufferobject.h \
	qopenglpaintdevice.h \
	qopenglpixeltransferoptions.h \
	qopenglshader.h \
	qopenglshaderprogram.h \
	qopengltexture.h \
	qopenglversionfunctions.h \
	qopenglvertexarrayobject.h \
	\
	qopenglfunctions.h \
	qopenglfunctions_1_0.h \
	qopenglfunctions_1_1.h \
	qopenglfunctions_1_2.h \
	qopenglfunctions_1_3.h \
	qopenglfunctions_1_4.h \
	qopenglfunctions_1_5.h \
	qopenglfunctions_2_0.h \
	qopenglfunctions_2_1.h \
	qopenglfunctions_3_0.h \
	qopenglfunctions_3_1.h \
	qopenglfunctions_3_2_compatibility.h \
	qopenglfunctions_3_2_core.h \
	qopenglfunctions_3_3_compatibility.h \
	qopenglfunctions_3_3_core.h \
	qopenglfunctions_4_0_compatibility.h \
	qopenglfunctions_4_0_core.h \
	qopenglfunctions_4_1_compatibility.h \
	qopenglfunctions_4_1_core.h \
	qopenglfunctions_4_2_compatibility.h \
	qopenglfunctions_4_2_core.h \
	qopenglfunctions_4_3_compatibility.h \
	qopenglfunctions_4_3_core.h \
	qopenglfunctions_4_4_compatibility.h \
	qopenglfunctions_4_4_core.h \
	qopenglfunctions_4_5_compatibility.h \
	qopenglfunctions_4_5_core.h \
	qopenglfunctions_es2.h


GUI_OPENGL_PRIVATE_INCLUDES = \
	qopengl_p.h \
	qopenglcustomshaderstage_p.h \
	qopenglframebufferobject_p.h  \
	qopenglextensions_p.h \
	qopenglgradientcache_p.h \
	qopengltexturecache_p.h \
	qopenglengineshadermanager_p.h \
	qopengl2pexvertexarray_p.h \
	qopenglengineshadersource_p.h \
	qopenglpaintengine_p.h \
	qopenglpaintdevice_p.h \
	qopengltextureglyphcache_p.h \
	qopengltextureblitter_p.h \
	qopengltexture_p.h \
	qopengltexturehelper_p.h \
	qopenglshadercache_p.h \
	qopenglversion_functions_p.h \
	qopenglvertexarrayobject_p.h \
	qtriangulator_p.h \
	qtriangulatingstroker_p.h \
	qrbtree_p.h

GUI_INCLUDES += $(GUI_OPENGL_INCLUDES)
GUI_PRIVATE_INCLUDES += $(GUI_OPENGL_PRIVATE_INCLUDES)

noinst_HEADERS+=$(patsubst %,src/gui/opengl/%,$(GUI_OPENGL_INCLUDES)) \
	$(patsubst %,src/gui/opengl/%,$(GUI_OPENGL_PRIVATE_INCLUDES))

lib_libCsGui@BUILD_ABI@_la_SOURCES += \
   src/gui/opengl/qopengl.cpp \
   src/gui/opengl/qopenglbuffer.cpp \
   src/gui/opengl/qopengldebug.cpp \
   src/gui/opengl/qopenglframebufferobject.cpp \
   src/gui/opengl/qopenglgradientcache.cpp \
   src/gui/opengl/qopenglpaintdevice.cpp \
   src/gui/opengl/qopenglshaderprogram.cpp \
   src/gui/opengl/qopengltexturecache.cpp \
   src/gui/opengl/qopenglengineshadermanager.cpp \
   src/gui/opengl/qopengl2pexvertexarray.cpp \
   src/gui/opengl/qopenglpaintengine.cpp \
   src/gui/opengl/qopenglcustomshaderstage.cpp \
   src/gui/opengl/qtriangulatingstroker.cpp \
   src/gui/opengl/qopengltextureglyphcache.cpp \
   src/gui/opengl/qtriangulator.cpp \
   src/gui/opengl/qopengltextureblitter.cpp \
   src/gui/opengl/qopengltexture.cpp \
   src/gui/opengl/qopengltexturehelper.cpp \
   src/gui/opengl/qopenglpixeltransferoptions.cpp \
   src/gui/opengl/qopenglversionfunctions.cpp \
   src/gui/opengl/qopenglversion_functions1.cpp \
   src/gui/opengl/qopenglversion_functions2.cpp \
   src/gui/opengl/qopenglvertexarrayobject.cpp \
   \
   src/gui/opengl/qopenglfunctions.cpp \
   src/gui/opengl/qopenglfunctions_1_0.cpp \
   src/gui/opengl/qopenglfunctions_1_1.cpp \
   src/gui/opengl/qopenglfunctions_1_2.cpp \
   src/gui/opengl/qopenglfunctions_1_3.cpp \
   src/gui/opengl/qopenglfunctions_1_4.cpp \
   src/gui/opengl/qopenglfunctions_1_5.cpp \
   src/gui/opengl/qopenglfunctions_2_0.cpp \
   src/gui/opengl/qopenglfunctions_2_1.cpp \
   src/gui/opengl/qopenglfunctions_3_0.cpp \
   src/gui/opengl/qopenglfunctions_3_1.cpp \
   src/gui/opengl/qopenglfunctions_3_2_compatibility.cpp \
   src/gui/opengl/qopenglfunctions_3_2_core.cpp \
   src/gui/opengl/qopenglfunctions_3_3_compatibility.cpp \
   src/gui/opengl/qopenglfunctions_3_3_core.cpp \
   src/gui/opengl/qopenglfunctions_4_0_compatibility.cpp \
   src/gui/opengl/qopenglfunctions_4_0_core.cpp \
   src/gui/opengl/qopenglfunctions_4_1_compatibility.cpp \
   src/gui/opengl/qopenglfunctions_4_1_core.cpp \
   src/gui/opengl/qopenglfunctions_4_2_compatibility.cpp \
   src/gui/opengl/qopenglfunctions_4_2_core.cpp \
   src/gui/opengl/qopenglfunctions_4_3_compatibility.cpp \
   src/gui/opengl/qopenglfunctions_4_3_core.cpp \
   src/gui/opengl/qopenglfunctions_4_4_compatibility.cpp \
   src/gui/opengl/qopenglfunctions_4_4_core.cpp \
   src/gui/opengl/qopenglfunctions_4_5_compatibility.cpp \
   src/gui/opengl/qopenglfunctions_4_5_core.cpp \
   src/gui/opengl/qopenglfunctions_es2.cpp

EXTRA_DIST+=

