list(APPEND GUI_PUBLIC_INCLUDES
   QGraphicsEffect
   QGraphicsBlurEffect
   QGraphicsColorizeEffect
   QGraphicsOpacityEffect
   QGraphicsDropShadowEffect
)

list(APPEND GUI_INCLUDES
   ${CMAKE_CURRENT_SOURCE_DIR}/effects/qgraphicsblureffect.h
   ${CMAKE_CURRENT_SOURCE_DIR}/effects/qgraphicscolorizeeffect.h
   ${CMAKE_CURRENT_SOURCE_DIR}/effects/qgraphicsdropshadoweffect.h
   ${CMAKE_CURRENT_SOURCE_DIR}/effects/qgraphicsopacityeffect.h
   ${CMAKE_CURRENT_SOURCE_DIR}/effects/qgraphicseffect.h
)

list(APPEND GUI_PRIVATE_INCLUDES
   ${CMAKE_CURRENT_SOURCE_DIR}/effects/qgraphicseffect_p.h
)

target_sources(CsGui
   PRIVATE
   ${CMAKE_CURRENT_SOURCE_DIR}/effects/qgraphicseffect.cpp
)