add_library(CsOpenGL SHARED "")
add_library(CopperSpice::CsOpenGL ALIAS CsOpenGL)

set_target_properties(CsOpenGL PROPERTIES OUTPUT_NAME CsOpenGL${BUILD_ABI})

target_compile_definitions(CsOpenGL
   PRIVATE
   -DQT_BUILD_OPENGL_LIB
   -DQT_NO_EGL
)

set(EXTRA_OPENGL_CXXFLAGS)
set(EXTRA_OPENGL_LDFLAGS)

include_directories(
   ${CMAKE_BINARY_DIR}/privateinclude/QtCore/private

   ${CMAKE_BINARY_DIR}/include/QtGui
   ${CMAKE_BINARY_DIR}/privateinclude/QtGui/private

   ${CMAKE_BINARY_DIR}/include/QtOpenGL
   ${CMAKE_BINARY_DIR}/privateinclude/QtOpenGL/private

   ${CMAKE_CURRENT_SOURCE_DIR}
   ${CMAKE_CURRENT_SOURCE_DIR}/gl2paintengineex
   ${CMAKE_SOURCE_DIR}/src/3rdparty/harfbuzz/src
   ${OPENGL_INCLUDE_DIR}
)

list(APPEND OPENGL_PUBLIC_INCLUDES
   QGLBuffer
   QGLColormap
   QGLContext
   QGLFormat
   QGLFramebufferObject
   QGLFunctions
   QGLPixelBuffer
   QGLShader
   QGLShaderProgram
   QGLWidget
)

list(APPEND OPENGL_INCLUDES
   ${CMAKE_CURRENT_SOURCE_DIR}/qgl.h
   ${CMAKE_CURRENT_SOURCE_DIR}/qglbuffer.h
   ${CMAKE_CURRENT_SOURCE_DIR}/qglcolormap.h
   ${CMAKE_CURRENT_SOURCE_DIR}/qglcontext.h
   ${CMAKE_CURRENT_SOURCE_DIR}/qglformat.h
   ${CMAKE_CURRENT_SOURCE_DIR}/qglframebufferobject.h
   ${CMAKE_CURRENT_SOURCE_DIR}/qglfunctions.h
   ${CMAKE_CURRENT_SOURCE_DIR}/qglpixelbuffer.h
   ${CMAKE_CURRENT_SOURCE_DIR}/qglshader.h
   ${CMAKE_CURRENT_SOURCE_DIR}/qglshaderprogram.h
   ${CMAKE_CURRENT_SOURCE_DIR}/qglwidget.h
)

list(APPEND OPENGL_PRIVATE_INCLUDES
   ${CMAKE_CURRENT_SOURCE_DIR}/qgl_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/qglframebufferobject_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/qglpaintdevice_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/qglpixelbuffer_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/qgraphicsshadereffect_p.h

   ${CMAKE_CURRENT_SOURCE_DIR}/gl2paintengineex/qgl2pexvertexarray_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/gl2paintengineex/qglcustomshaderstage_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/gl2paintengineex/qglengineshadermanager_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/gl2paintengineex/qglengineshadersource_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/gl2paintengineex/qglgradientcache_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/gl2paintengineex/qglshadercache_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/gl2paintengineex/qpaintengineex_opengl2_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/gl2paintengineex/qtextureglyphcache_gl_p.h
)

target_sources(CsOpenGL
   PRIVATE
   ${CMAKE_CURRENT_SOURCE_DIR}/gl2paintengineex/qgl2pexvertexarray.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/gl2paintengineex/qglcustomshaderstage.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/gl2paintengineex/qglengineshadermanager.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/gl2paintengineex/qglgradientcache.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/gl2paintengineex/qpaintengineex_opengl2.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/gl2paintengineex/qtextureglyphcache_gl.cpp

   ${CMAKE_CURRENT_SOURCE_DIR}/qgl.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/qglbuffer.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/qglcolormap.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/qglframebufferobject.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/qglfunctions.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/qglpaintdevice.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/qglpixelbuffer.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/qglshaderprogram.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/qgraphicsshadereffect.cpp

   ${OPENGL_INCLUDES}
)

macro_generate_public("${OPENGL_PUBLIC_INCLUDES}" QtOpenGL)
macro_generate_private("${OPENGL_PRIVATE_INCLUDES}" QtOpenGL)
macro_generate_misc("${OPENGL_INCLUDES}" QtOpenGL)

if(CMAKE_SYSTEM_NAME MATCHES "Darwin")
   list(APPEND EXTRA_OPENGL_LDFLAGS
      -framework AppKit
      -framework Carbon
      -framework OpenGL
   )

elseif(CMAKE_SYSTEM_NAME MATCHES "Windows")
   target_link_libraries(CsOpenGL
      PUBLIC
      opengl32
      gdi32
      kernel32
      user32
   )

   target_compile_definitions(CsOpenGL
      PRIVATE
      -DQT_NO_STYLE_GTK
      -DQT_NO_STYLE_WINDOWSVISTA
   )

endif()

function_variable_fixup("${EXTRA_OPENGL_CXXFLAGS}" EXTRA_OPENGL_CXXFLAGS)
function_variable_fixup("${EXTRA_OPENGL_LDFLAGS}"  EXTRA_OPENGL_LDFLAGS)

target_include_directories(CsOpenGL
   PUBLIC
   $<INSTALL_INTERFACE:include/QtOpenGL>
)

target_link_libraries(CsOpenGL
   PUBLIC
   CsCore
   CsGui
   ${OPENGL_LIBRARIES}
)

set_target_properties(CsOpenGL
   PROPERTIES
   COMPILE_FLAGS ${EXTRA_OPENGL_CXXFLAGS}
   LINK_FLAGS ${EXTRA_OPENGL_LDFLAGS}
)

function_generate_resources(CsOpenGL)

macro_generate_package(
   CsOpenGL
   QtOpenGL
   ""
   "CsCore CsGui"
)

install(
   TARGETS CsOpenGL
   EXPORT CopperSpiceLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS}
   RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
   LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
   ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(
   DIRECTORY ${CMAKE_BINARY_DIR}/include/QtOpenGL
   DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
   COMPONENT Devel
)

