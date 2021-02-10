list(APPEND GUI_PUBLIC_INCLUDES
   QBoxLayout
   QFormLayout
   QGridLayout
   QHBoxLayout
   QLayout
   QLayoutItem
   QLayoutIterator
   QStackedLayout
   QVBoxLayout
)

list(APPEND GUI_INCLUDES
   ${CMAKE_CURRENT_SOURCE_DIR}/layout/qboxlayout.h
   ${CMAKE_CURRENT_SOURCE_DIR}/layout/qformlayout.h
   ${CMAKE_CURRENT_SOURCE_DIR}/layout/qgridlayout.h
   ${CMAKE_CURRENT_SOURCE_DIR}/layout/qhboxlayout.h
   ${CMAKE_CURRENT_SOURCE_DIR}/layout/qlayout.h
   ${CMAKE_CURRENT_SOURCE_DIR}/layout/qlayoutitem.h
   ${CMAKE_CURRENT_SOURCE_DIR}/layout/qlayoutiterator.h
   ${CMAKE_CURRENT_SOURCE_DIR}/layout/qstackedlayout.h
   ${CMAKE_CURRENT_SOURCE_DIR}/layout/qvboxlayout.h
)

list(APPEND GUI_PRIVATE_INCLUDES
   ${CMAKE_CURRENT_SOURCE_DIR}/layout/qabstractlayoutstyleinfo_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/layout/qdockarealayout_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/layout/qlayout_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/layout/qlayoutengine_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/layout/qlayoutpolicy_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/layout/qmainwindowlayout_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/layout/qtoolbarlayout_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/layout/qtoolbararealayout_p.h
)

target_sources(CsGui
   PRIVATE
   ${CMAKE_CURRENT_SOURCE_DIR}/layout/qabstractlayoutstyleinfo.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/layout/qboxlayout.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/layout/qdockarealayout.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/layout/qformlayout.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/layout/qgridlayout.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/layout/qlayout.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/layout/qlayoutengine.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/layout/qlayoutitem.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/layout/qlayoutpolicy.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/layout/qmainwindowlayout.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/layout/qstackedlayout.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/layout/qtoolbarlayout.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/layout/qtoolbararealayout.cpp
)
