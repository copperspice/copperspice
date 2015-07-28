set(GUI_PUBLIC_INCLUDES
    ${GUI_PUBLIC_INCLUDES}
    QGraphicsEffect
    QGraphicsBlurEffect
    QGraphicsColorizeEffect
    QGraphicsOpacityEffect
    QGraphicsDropShadowEffect
)

set(GUI_INCLUDES
    ${GUI_INCLUDES}
    ${CMAKE_CURRENT_SOURCE_DIR}/effects/qgraphicsblureffect.h
    ${CMAKE_CURRENT_SOURCE_DIR}/effects/qgraphicscolorizeeffect.h
    ${CMAKE_CURRENT_SOURCE_DIR}/effects/qgraphicsdropshadoweffect.h
    ${CMAKE_CURRENT_SOURCE_DIR}/effects/qgraphicsopacityeffect.h
    ${CMAKE_CURRENT_SOURCE_DIR}/effects/qgraphicseffect.h
)

set(GUI_PRIVATE_INCLUDES
    ${GUI_PRIVATE_INCLUDES}
    ${CMAKE_CURRENT_SOURCE_DIR}/effects/qgraphicseffect_p.h
)

set(GUI_SOURCES
    ${GUI_SOURCES}
    ${CMAKE_CURRENT_SOURCE_DIR}/effects/qgraphicseffect.cpp
)