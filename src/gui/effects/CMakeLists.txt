set(GUI_PUBLIC_INCLUDES
    ${GUI_PUBLIC_INCLUDES}
    QGraphicsEffect
    QGraphicsBlurEffect
    QGraphicsColorizeEffect
    QGraphicsOpacityEffect
    QGraphicsDropShadowEffect
    PARENT_SCOPE
)

set(GUI_INCLUDES
    ${GUI_INCLUDES}
    ${CMAKE_CURRENT_SOURCE_DIR}/qgraphicsblureffect.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qgraphicscolorizeeffect.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qgraphicsdropshadoweffect.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qgraphicsopacityeffect.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qgraphicseffect.h
    PARENT_SCOPE
)

set(GUI_PRIVATE_INCLUDES
    ${GUI_PRIVATE_INCLUDES}
    ${CMAKE_CURRENT_SOURCE_DIR}/qgraphicseffect_p.h
    PARENT_SCOPE
)

set(GUI_SOURCES
    ${GUI_SOURCES}
    ${CMAKE_CURRENT_SOURCE_DIR}/qgraphicseffect.cpp
    PARENT_SCOPE
)