list(APPEND GUI_PUBLIC_INCLUDES
   QAbstractButton
   QAbstractScrollArea
   QAbstractSlider
   QAbstractSpinBox
   QWidget
   QWidgetAction
   QWidgetData
   QWidgetItem
   QWidgetItemV2
   QWidgetList
   QWidgetMapper
   QWidgetSet
)

list(APPEND GUI_INCLUDES
   ${CMAKE_CURRENT_SOURCE_DIR}/widgetbase/qabstractbutton.h
   ${CMAKE_CURRENT_SOURCE_DIR}/widgetbase/qabstractscrollarea.h
   ${CMAKE_CURRENT_SOURCE_DIR}/widgetbase/qabstractslider.h
   ${CMAKE_CURRENT_SOURCE_DIR}/widgetbase/qabstractspinbox.h
   ${CMAKE_CURRENT_SOURCE_DIR}/widgetbase/qwidget.h
   ${CMAKE_CURRENT_SOURCE_DIR}/widgetbase/qwidgetaction.h
   ${CMAKE_CURRENT_SOURCE_DIR}/widgetbase/qwidgetdata.h
   ${CMAKE_CURRENT_SOURCE_DIR}/widgetbase/qwidgetitem.h
   ${CMAKE_CURRENT_SOURCE_DIR}/widgetbase/qwidgetitemv2.h
   ${CMAKE_CURRENT_SOURCE_DIR}/widgetbase/qwidgetlist.h
   ${CMAKE_CURRENT_SOURCE_DIR}/widgetbase/qwidgetmapper.h
   ${CMAKE_CURRENT_SOURCE_DIR}/widgetbase/qwidgetset.h
)

list(APPEND GUI_PRIVATE_INCLUDES
   ${CMAKE_CURRENT_SOURCE_DIR}/widgetbase/qabstractbutton_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/widgetbase/qabstractscrollarea_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/widgetbase/qabstractslider_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/widgetbase/qabstractspinbox_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/widgetbase/qwidget_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/widgetbase/qwidgetaction_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/widgetbase/qwidgetanimator_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/widgetbase/qwidgetresizehandler_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/widgetbase/qwidgetwindow_p.h
)

target_sources(CsGui
   PRIVATE
   ${CMAKE_CURRENT_SOURCE_DIR}/widgetbase/qabstractbutton.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/widgetbase/qabstractslider.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/widgetbase/qabstractspinbox.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/widgetbase/qabstractscrollarea.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/widgetbase/qwidget.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/widgetbase/qwidgetaction.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/widgetbase/qwidgetresizehandler.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/widgetbase/qwidgetanimator.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/widgetbase/qwidgetwindow.cpp
)
