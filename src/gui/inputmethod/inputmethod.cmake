set(GUI_PUBLIC_INCLUDES
    ${GUI_PUBLIC_INCLUDES}
    QInputContext
    QInputContextFactory
    QInputContextFactoryInterface
    QInputContextPlugin
)

set(GUI_INCLUDES
    ${GUI_INCLUDES}
    ${CMAKE_CURRENT_SOURCE_DIR}/inputmethod/qinputcontext.h
    ${CMAKE_CURRENT_SOURCE_DIR}/inputmethod/qinputcontextfactory.h
    ${CMAKE_CURRENT_SOURCE_DIR}/inputmethod/qinputcontextfactoryinterface.h
    ${CMAKE_CURRENT_SOURCE_DIR}/inputmethod/qinputcontextplugin.h
)

set(GUI_PRIVATE_INCLUDES
    ${GUI_PRIVATE_INCLUDES}
    ${CMAKE_CURRENT_SOURCE_DIR}/inputmethod/qinputcontext_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/inputmethod/qmacinputcontext_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/inputmethod/qximinputcontext_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/inputmethod/qwininputcontext_p.h
)

set(GUI_SOURCES
    ${GUI_SOURCES}
    ${CMAKE_CURRENT_SOURCE_DIR}/inputmethod/qinputcontextfactory.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/inputmethod/qinputcontextplugin.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/inputmethod/qinputcontext.cpp
)

if(X11_FOUND)
    set(GUI_SOURCES
        ${GUI_SOURCES}
        ${CMAKE_CURRENT_SOURCE_DIR}/inputmethod/qximinputcontext_x11.cpp
    )
endif()

# FIXME: COCOA?
if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(GUI_SOURCES
        ${GUI_SOURCES}
        ${CMAKE_CURRENT_SOURCE_DIR}/inputmethod/qmacinputcontext_mac.cpp
    )
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    set(GUI_SOURCES
        ${GUI_SOURCES}
        ${CMAKE_CURRENT_SOURCE_DIR}/inputmethod/qwininputcontext_win.cpp
    )
endif()
