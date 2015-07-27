add_definitions(-DQT_BUILD_GUI_LIB
    -DQT_BUILTIN_GIF_READER
    -DMNG_BUILD_SO
    -DMNG_NO_INCLUDE_JNG
    -DPNG_NO_CONFIG_H
    -DMNG_BUILD_SO
    -DMNG_NO_INCLUDE_JNG
)

set(EXTRA_GUI_LIBS CsCore${BUILD_MAJOR})
set(EXTRA_GUI_CXXFLAGS "-Wno-undefined")
set(EXTRA_GUI_LDFLAGS " ") # whitespace is required!

add_subdirectory(accessible)
add_subdirectory(animation)
add_subdirectory(dialogs)
add_subdirectory(effects)
# add_subdirectory(embedded) # currently unused
add_subdirectory(graphicsview)
add_subdirectory(image)

# TODO: process
# add_subdirectory(inputmethod)
# add_subdirectory(itemviews)
# add_subdirectory(kernel)
# add_subdirectory(math3d)
add_subdirectory(painting)
# add_subdirectory(statemachine)
# add_subdirectory(styles)
# add_subdirectory(text)
# add_subdirectory(util)
# add_subdirectory(widgets)

# TODO: make them macros to be shared between components
foreach(pubheader ${GUI_PUBLIC_INCLUDES})
    string(TOLOWER ${pubheader} pubname)
    get_filename_component(pubname ${pubname} NAME)
    set(pubout ${CMAKE_BINARY_DIR}/include/QtGui/${pubheader})
    message(STATUS "Writing public: ${pubout}")
    file(WRITE ${pubout} "#include <${pubname}.h>")
endforeach(pubheader)

foreach(privheader ${GUI_PRIVATE_INCLUDES})
    get_filename_component(privname ${privheader} NAME)
    set(privout ${CMAKE_BINARY_DIR}/privateinclude/QtGui/${privname})
    message(STATUS "Writing private: ${privout}")
    file(COPY ${privheader} DESTINATION ${CMAKE_BINARY_DIR}/privateinclude/QtGui/)
endforeach(privheader)

foreach(header ${GUI_INCLUDES})
    get_filename_component(headername ${header} NAME)
    set(headout ${CMAKE_BINARY_DIR}/include/QtGui/${headername})
    message(STATUS "Writing: ${headout}")
    file(COPY ${header} DESTINATION ${CMAKE_BINARY_DIR}/include/QtGui/)
endforeach(header)

set(UI_FOUND)
foreach(src ${GUI_SOURCES})
    string(FIND ${src} ".ui" UI_FOUND)
    if(UI_FOUND)
        get_filename_component(uiname ${src} NAME_WE)
        set(uiout ${CMAKE_BINARY_DIR}/include/ui_${uiname}.h)
        message(STATUS "Writing UI: ${uiout}")
        execute_process(COMMAND "uic" "${src}" "-o" "${uiout}")
    endif()
endforeach()

include_directories(
    ${CMAKE_BINARY_DIR}/include
    ${CMAKE_BINARY_DIR}/privateinclude
    ${CMAKE_BINARY_DIR}/include/QtCore
    ${CMAKE_BINARY_DIR}/privateinclude/QtCore
    ${CMAKE_BINARY_DIR}/include/QtGui
    ${CMAKE_BINARY_DIR}/privateinclude/QtGui
    ${CMAKE_CURRENT_SOURCE_DIR}/access
    ${CMAKE_CURRENT_SOURCE_DIR}/bearer
    ${CMAKE_CURRENT_SOURCE_DIR}/kernel
    ${CMAKE_CURRENT_SOURCE_DIR}/socket
    ${CMAKE_CURRENT_SOURCE_DIR}/ssl
    ${CMAKE_SOURCE_DIR}/src/3rdparty/harfbuzz/src
    ${CMAKE_SOURCE_DIR}/src/3rdparty/wintab
    ${CMAKE_SOURCE_DIR}/src/3rdparty/zlib
    ${CMAKE_SOURCE_DIR}/src/3rdparty/libtiff/libtiff
    ${CMAKE_SOURCE_DIR}/src/3rdparty/libmng
    ${CMAKE_SOURCE_DIR}/src/3rdparty/libjpeg
    ${CMAKE_SOURCE_DIR}/src/3rdparty/libpng
)

set(EXTRA_DIST
    src/gui/mac/macresources.qrc
    src/gui/mac/images/copyarrowcursor.png
    src/gui/mac/images/forbiddencursor.png
    src/gui/mac/images/leopard-unified-toolbar-on.png
    src/gui/mac/images/pluscursor.png
    src/gui/mac/images/spincursor.png
    src/gui/mac/images/waitcursor.png
)

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(EXTRA_GUI_LIBS
        ${EXTRA_GUI_LIBS}
        ${FONTCONFIG_LIBRARIES}
        X11
        lXrender
        lXrandr
        lXcursor
        lXinerama
        SM
        ICE
        audio
        pthread
        rt
    )
    set(EXTRA_GUI_LDFLAGS
        ${EXTRA_GUI_LDFLAGS}
         -Wl,-luuid --export-all-symbols
    )
    add_definitions(
        -DQT_NO_XINPUT
        -DQT_NO_STYLE_MAC
        -DQT_NO_STYLE_WINDOWSXP
        -DQT_NO_STYLE_WINDOWSVISTA
        -DQT_NO_TABLET
    )
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Win32")
    set(EXTRA_GUI_LIBS
        ${EXTRA_GUI_LIBS}
        gdi32
        winmm
        ole32
        winspool
        oleaut32
        comdlg32
        imm32
    )
    set(EXTRA_GUI_LDFLAGS
        ${EXTRA_GUI_LDFLAGS
         -Wl,-luuid --export-all-symbols
    )
    add_definitions(
        -DQT_NO_STYLE_MAC
        -DQT_NO_LPR
        -DQT_NO_DIRECTWRITE
        -DQT_MAKEDLL
   )
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(GUI_SOURCES
        ${GUI_SOURCES}
        ${CMAKE_CURRENT_SOURCE_DIR}/mac/qrc_macresources.cpp
    )
    set(EXTRA_GUI_LIBS
        ${EXTRA_GUI_LIBS}
        ObjC
    )
    set(EXTRA_GUI_LDFLAGS
        ${EXTRA_GUI_LDFLAGS
        -framework Carbon
    )
    add_definitions(-DQT_NO_STYLE_WINDOWSVISTA)

    # FIXME: what the heck?
#     macnibdir=$(prefix)/mac/qt_menu.nib
#     dist_macnib_DATA +=
#         src/gui/mac/qt_menu.nib/classes.nib
#         src/gui/mac/qt_menu.nib/info.nib
#         src/gui/mac/qt_menu.nib/keyedobjects.nib
endif()


add_library(CsGui${BUILD_MAJOR} SHARED ${GUI_SOURCES} ${GUI_INCLUDES})
target_link_libraries(CsGui${BUILD_MAJOR} ${EXTRA_GUI_LIBS})
set_target_properties(CsGui${BUILD_MAJOR} PROPERTIES
    SOVERSION "0.0.0" # for compat
    COMPILE_FLAGS ${EXTRA_GUI_CXXFLAGS}
    LINK_FLAGS ${EXTRA_GUI_LDFLAGS}
)

if(UI_FOUND)
    add_dependencies(CsGui${BUILD_MAJOR} uic)
endif()

install(TARGETS CsGui${BUILD_MAJOR} LIBRARY DESTINATION lib)
install(DIRECTORY ${CMAKE_BINARY_DIR}/include/QtGui DESTINATION include)