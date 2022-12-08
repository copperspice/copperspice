add_library(CsSvg SHARED "")
add_library(CopperSpice::CsSvg ALIAS CsSvg)

set_target_properties(CsSvg PROPERTIES OUTPUT_NAME CsSvg${BUILD_ABI})

add_definitions(
   -DQT_BUILD_SVG_LIB
)

set(EXTRA_SVG_CXXFLAGS)
set(EXTRA_SVG_LDFLAGS)

include_directories(
   ${CMAKE_BINARY_DIR}/privateinclude/QtCore/private

   ${CMAKE_BINARY_DIR}/include/QtGui
   ${CMAKE_BINARY_DIR}/privateinclude/QtGui/private

   ${CMAKE_BINARY_DIR}/include/QtSvg
   ${CMAKE_BINARY_DIR}/privateinclude/QtSvg/private

   ${CMAKE_CURRENT_SOURCE_DIR}
   ${CMAKE_SOURCE_DIR}/src/3rdparty/harfbuzz/src
)

set(SVG_PUBLIC_INCLUDES
   QtSvg
   QGraphicsSvgItem
   QSvgGenerator
   QSvgRenderer
   QSvgWidget
)

list(APPEND SVG_INCLUDES
   ${CMAKE_CURRENT_SOURCE_DIR}/qsvgrenderer.h
   ${CMAKE_CURRENT_SOURCE_DIR}/qsvgwidget.h
   ${CMAKE_CURRENT_SOURCE_DIR}/qgraphicssvgitem.h
   ${CMAKE_CURRENT_SOURCE_DIR}/qsvggenerator.h
   ${CMAKE_CURRENT_SOURCE_DIR}/qtsvg.h
)

list(APPEND SVG_PRIVATE_INCLUDES
   ${CMAKE_CURRENT_SOURCE_DIR}/qsvggraphics_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/qsvghandler_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/qsvgnode_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/qsvgstructure_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/qsvgstyle_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/qsvgfont_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/qsvgtinydocument_p.h
)

target_sources(CsSvg
   PRIVATE
   ${CMAKE_CURRENT_SOURCE_DIR}/qsvggraphics.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/qsvghandler.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/qsvgnode.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/qsvgstructure.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/qsvgstyle.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/qsvgfont.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/qsvgtinydocument.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/qsvgrenderer.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/qsvgwidget.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/qgraphicssvgitem.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/qsvggenerator.cpp
)

macro_generate_public("${SVG_PUBLIC_INCLUDES}"   QtSvg)
macro_generate_private("${SVG_PRIVATE_INCLUDES}" QtSvg)
macro_generate_misc("${SVG_INCLUDES}" QtSvg)

if(CMAKE_SYSTEM_NAME MATCHES "Windows")
   target_link_libraries(CsSvg
      PRIVATE
      ws2_32
   )
endif()

function_variable_fixup("${EXTRA_SVG_CXXFLAGS}" EXTRA_SVG_CXXFLAGS)
function_variable_fixup("${EXTRA_SVG_LDFLAGS}"  EXTRA_SVG_LDFLAGS)

target_sources(CsSvg
   PRIVATE
   ${SVG_INCLUDES}
)

target_include_directories(CsSvg
   PUBLIC
   $<INSTALL_INTERFACE:include/QtSvg>
)

target_link_libraries(CsSvg
   PRIVATE
   CsCore
   CsGui
)

set_target_properties(CsSvg
   PROPERTIES
   COMPILE_FLAGS ${EXTRA_SVG_CXXFLAGS}
   LINK_FLAGS ${EXTRA_SVG_LDFLAGS}
)

function_generate_resources(CsSvg)

macro_generate_package(
   CsSvg
   QtSvg
  "${EXTRA_SVG_CXXFLAGS}"
   "CsCore CsGui"
)

if(CMAKE_SYSTEM_NAME MATCHES "(Linux|OpenBSD|FreeBSD|NetBSD|DragonFly)")
   add_definitions(
      -DHAVE_POSIX_MEMALIGN
   )
endif()

install(
   TARGETS CsSvg
   EXPORT CopperSpiceLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS}
   RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
   LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
   ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(
   DIRECTORY ${CMAKE_BINARY_DIR}/include/QtSvg
   DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
   COMPONENT Devel
)
