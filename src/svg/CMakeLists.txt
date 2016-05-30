add_definitions(
    -DQT_BUILD_SVG_LIB
    -DQT_SHARED
    -DQT_MAKEDLL
)
set(EXTRA_SVG_LIBS CsCore${BUILD_ABI} CsGui${BUILD_ABI})
set(EXTRA_SVG_CXXFLAGS)
set(EXTRA_SVG_LDFLAGS)

include_directories(
    ${CMAKE_BINARY_DIR}/include
    ${CMAKE_BINARY_DIR}/privateinclude
    ${CMAKE_BINARY_DIR}/include/QtCore
    ${CMAKE_BINARY_DIR}/privateinclude/QtCore
    ${CMAKE_BINARY_DIR}/privateinclude/QtCore/private
    ${CMAKE_BINARY_DIR}/include/QtGui
    ${CMAKE_BINARY_DIR}/privateinclude/QtGui
    ${CMAKE_BINARY_DIR}/privateinclude/QtGui/private
    ${CMAKE_BINARY_DIR}/include/QtSvg
    ${CMAKE_BINARY_DIR}/privateinclude/QtSvg
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

set(SVG_INCLUDES
    ${CMAKE_CURRENT_SOURCE_DIR}/qsvgrenderer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qsvgwidget.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qgraphicssvgitem.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qsvggenerator.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qtsvg.h
)

set(SVG_PRIVATE_INCLUDES
    ${CMAKE_CURRENT_SOURCE_DIR}/qsvggraphics_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qsvghandler_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qsvgnode_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qsvgstructure_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qsvgstyle_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qsvgfont_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qsvgtinydocument_p.h
)

set(SVG_SOURCES
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

macro_generate_public("${SVG_PUBLIC_INCLUDES}" QtSvg)
macro_generate_private("${SVG_PRIVATE_INCLUDES}" QtSvg)
macro_generate_misc("${SVG_INCLUDES}" QtSvg)

if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    set(EXTRA_SVG_LIBS
        ${EXTRA_SVG_LIBS}
        ws2_32
    )
endif()

function_variable_fixup("${EXTRA_SVG_CXXFLAGS}" EXTRA_SVG_CXXFLAGS)
function_variable_fixup("${EXTRA_SVG_LDFLAGS}" EXTRA_SVG_LDFLAGS)

add_library(CsSvg${BUILD_ABI} SHARED ${SVG_SOURCES} ${SVG_INCLUDES})
target_link_libraries(CsSvg${BUILD_ABI} ${EXTRA_SVG_LIBS})
set_target_properties(CsSvg${BUILD_ABI} PROPERTIES
    VERSION "0" # for compat
    SOVERSION "0.0.0" # for compat
    COMPILE_FLAGS ${EXTRA_SVG_CXXFLAGS}
    LINK_FLAGS ${EXTRA_SVG_LDFLAGS}
)

macro_generate_resources("${SVG_SOURCES}")
macro_generate_package(
    CsSvg${BUILD_ABI}
    QtSvg
    "${EXTRA_SVG_CXXFLAGS}"
    "${EXTRA_SVG_LIBS}"
    "CsCore${BUILD_ABI} CsGui${BUILD_ABI}"
)

install(
    TARGETS CsSvg${BUILD_ABI}
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
