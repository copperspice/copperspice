list(APPEND GUI_PUBLIC_INCLUDES
   QCommonStyle
   QProxyStyle
   QStyle
   QStyleFactory
   QStyleFactoryInterface
   QStyleHintReturn
   QStyleHintReturnMask
   QStyleHintReturnVariant
   QStyleOption
   QStyleOptionButton
   QStyleOptionComboBox
   QStyleOptionComplex
   QStyleOptionDockWidget
   QStyleOptionFocusRect
   QStyleOptionFrame
   QStyleOptionGraphicsItem
   QStyleOptionGroupBox
   QStyleOptionHeader
   QStyleOptionMenuItem
   QStyleOptionProgressBar
   QStyleOptionRubberBand
   QStyleOptionSizeGrip
   QStyleOptionSlider
   QStyleOptionSpinBox
   QStyleOptionTab
   QStyleOptionTabBarBase
   QStyleOptionTabWidgetFrame
   QStyleOptionTitleBar
   QStyleOptionToolBar
   QStyleOptionToolBox
   QStyleOptionToolButton
   QStyleOptionViewItem
   QStylePlugin
)

list(APPEND GUI_INCLUDES
   ${CMAKE_CURRENT_SOURCE_DIR}/styles/qcommonstyle.h
   ${CMAKE_CURRENT_SOURCE_DIR}/styles/qmacstyle.h
   ${CMAKE_CURRENT_SOURCE_DIR}/styles/qproxystyle.h
   ${CMAKE_CURRENT_SOURCE_DIR}/styles/qstyle.h
   ${CMAKE_CURRENT_SOURCE_DIR}/styles/qstylefactory.h
   ${CMAKE_CURRENT_SOURCE_DIR}/styles/qstylefactoryinterface.h
   ${CMAKE_CURRENT_SOURCE_DIR}/styles/qstylehintreturn.h
   ${CMAKE_CURRENT_SOURCE_DIR}/styles/qstylehintreturnmask.h
   ${CMAKE_CURRENT_SOURCE_DIR}/styles/qstylehintreturnvariant.h
   ${CMAKE_CURRENT_SOURCE_DIR}/styles/qstyleoption.h
   ${CMAKE_CURRENT_SOURCE_DIR}/styles/qstyleoptionbutton.h
   ${CMAKE_CURRENT_SOURCE_DIR}/styles/qstyleoptioncombobox.h
   ${CMAKE_CURRENT_SOURCE_DIR}/styles/qstyleoptioncomplex.h
   ${CMAKE_CURRENT_SOURCE_DIR}/styles/qstyleoptiondockwidget.h
   ${CMAKE_CURRENT_SOURCE_DIR}/styles/qstyleoptionfocusrect.h
   ${CMAKE_CURRENT_SOURCE_DIR}/styles/qstyleoptionframe.h
   ${CMAKE_CURRENT_SOURCE_DIR}/styles/qstyleoptiongraphicsitem.h
   ${CMAKE_CURRENT_SOURCE_DIR}/styles/qstyleoptiongroupbox.h
   ${CMAKE_CURRENT_SOURCE_DIR}/styles/qstyleoptionheader.h
   ${CMAKE_CURRENT_SOURCE_DIR}/styles/qstyleoptionmenuitem.h
   ${CMAKE_CURRENT_SOURCE_DIR}/styles/qstyleoptionprogressbar.h
   ${CMAKE_CURRENT_SOURCE_DIR}/styles/qstyleoptionrubberband.h
   ${CMAKE_CURRENT_SOURCE_DIR}/styles/qstyleoptionsizegrip.h
   ${CMAKE_CURRENT_SOURCE_DIR}/styles/qstyleoptionslider.h
   ${CMAKE_CURRENT_SOURCE_DIR}/styles/qstyleoptionspinbox.h
   ${CMAKE_CURRENT_SOURCE_DIR}/styles/qstyleoptiontab.h
   ${CMAKE_CURRENT_SOURCE_DIR}/styles/qstyleoptiontabbarbase.h
   ${CMAKE_CURRENT_SOURCE_DIR}/styles/qstyleoptiontabwidgetframe.h
   ${CMAKE_CURRENT_SOURCE_DIR}/styles/qstyleoptiontitlebar.h
   ${CMAKE_CURRENT_SOURCE_DIR}/styles/qstyleoptiontoolbar.h
   ${CMAKE_CURRENT_SOURCE_DIR}/styles/qstyleoptiontoolbox.h
   ${CMAKE_CURRENT_SOURCE_DIR}/styles/qstyleoptiontoolbutton.h
   ${CMAKE_CURRENT_SOURCE_DIR}/styles/qstyleoptionviewitem.h
   ${CMAKE_CURRENT_SOURCE_DIR}/styles/qstyleplugin.h
)

list(APPEND GUI_PRIVATE_INCLUDES
   ${CMAKE_CURRENT_SOURCE_DIR}/styles/qcommonstylepixmaps_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/styles/qcommonstyle_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/styles/qfusionstyle_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/styles/qfusionstyle_p_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/styles/qgtkstyle_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/styles/qgtkglobal_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/styles/qgtkpainter_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/styles/qgtk2painter_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/styles/qmacstyle_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/styles/qproxystyle_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/styles/qrenderrule_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/styles/qstyle_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/styles/qstylehelper_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/styles/qstylesheetstyle_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/styles/qstyleanimation_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/styles/qwindows_style_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/styles/qwindows_xpstyle_p.h
   ${CMAKE_CURRENT_SOURCE_DIR}/styles/qwindows_vistastyle_p.h
)

target_sources(CsGui
   PRIVATE
   ${CMAKE_CURRENT_SOURCE_DIR}/styles/qstyleanimation.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/styles/qcommonstyle.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/styles/qstyle.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/styles/qfusionstyle.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/styles/qstylefactory.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/styles/qstyleoption.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/styles/qstyleplugin.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/styles/qstylehelper.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/styles/qproxystyle.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/styles/qstylesheetstyle.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/styles/qstylesheetstyle_default.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/styles/qstyle.qrc
   ${CMAKE_CURRENT_BINARY_DIR}/qrc_qstyle.cpp
)

if(X11_FOUND)
   target_sources(CsGui
      PRIVATE
      ${CMAKE_CURRENT_SOURCE_DIR}/styles/qwindows_style.cpp
   )
endif()

if(GTK2_FOUND)
   target_sources(CsGui
      PRIVATE
      ${CMAKE_CURRENT_SOURCE_DIR}/styles/qgtkstyle.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/styles/qgtkpainter.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/styles/qgtk2painter.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/styles/qgtkstyle_p.cpp
   )

   target_link_libraries(CsGui
      PRIVATE
      ${GTK2_LIBRARIES}
   )

   include_directories(${GTK2_INCLUDE_DIRS})
   add_definitions(${GTK2_DEFINITIONS})

else()
   add_definitions(-DQT_NO_STYLE_GTK)

endif()


if(CMAKE_SYSTEM_NAME MATCHES "Darwin")
   target_sources(CsGui
      PRIVATE
      ${CMAKE_CURRENT_SOURCE_DIR}/styles/qwindows_style.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/styles/qmacstyle.mm
   )

   add_definitions(-DQT_NO_STYLE_WINDOWSXP -DQT_NO_STYLE_GTK)

elseif(CMAKE_SYSTEM_NAME MATCHES "(OpenBSD|FreeBSD|NetBSD)")
   add_definitions(-DQT_NO_STYLE_MAC -DQT_NO_STYLE_WINDOWSXP)

elseif(CMAKE_SYSTEM_NAME MATCHES "Windows")
   target_sources(CsGui
      PRIVATE
      ${CMAKE_CURRENT_SOURCE_DIR}/styles/qwindows_style.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/styles/qwindows_xpstyle.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/styles/qwindows_vistastyle.cpp
   )

   target_link_libraries(CsGui
      PRIVATE
      uxtheme
   )

endif()
