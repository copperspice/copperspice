/***********************************************************************
*
* Copyright (c) 2012-2024 Barbara Geller
* Copyright (c) 2012-2024 Ansel Sermersheim
*
* Copyright (c) 2015 The Qt Company Ltd.
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
*
* This file is part of CopperSpice.
*
* CopperSpice is free software. You can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*
* https://www.gnu.org/licenses/
*
***********************************************************************/

#ifndef QWINDOWSVISTASTYLE_P_H
#define QWINDOWSVISTASTYLE_P_H

#include <qwindows_xpstyle_p.h>

#if ! defined(QT_NO_STYLE_WINDOWSVISTA)

#include <qlibrary.h>
#include <qpaintengine.h>
#include <qwidget.h>
#include <qapplication.h>
#include <qpixmapcache.h>
#include <qstyleoption.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qcheckbox.h>
#include <qlineedit.h>
#include <qgroupbox.h>
#include <qtoolbutton.h>
#include <qspinbox.h>
#include <qtoolbar.h>
#include <qcombobox.h>
#include <qscrollbar.h>
#include <qprogressbar.h>
#include <qdockwidget.h>
#include <qlistview.h>
#include <qtreeview.h>
#include <qtextedit.h>
#include <qmessagebox.h>
#include <qdialogbuttonbox.h>
#include <qinputdialog.h>
#include <qtableview.h>
#include <qdatetime.h>
#include <qcommandlinkbutton.h>

#include <qstyleanimation_p.h>
#include <qpaintengine_raster_p.h>

class QWindowsVistaStylePrivate;

#if !defined(SCHEMA_VERIFY_VSSYM32)
#define TMT_ANIMATIONDURATION       5006
#define TMT_TRANSITIONDURATIONS     6000
#define EP_EDITBORDER_NOSCROLL      6
#define EP_EDITBORDER_HVSCROLL      9
#define EP_BACKGROUND               3
#define EBS_NORMAL                  1
#define EBS_HOT                     2
#define EBS_DISABLED                3
#define EBS_READONLY                5
#define PBS_DEFAULTED_ANIMATING     6
#define MBI_NORMAL                  1
#define MBI_HOT                     2
#define MBI_PUSHED                  3
#define MBI_DISABLED                4
#define MB_ACTIVE                   1
#define MB_INACTIVE                 2
#define PP_FILL                     5
#define PP_FILLVERT                 6
#define PP_MOVEOVERLAY              8
#define PP_MOVEOVERLAYVERT          10
#define MENU_BARBACKGROUND          7
#define MENU_BARITEM                8
#define MENU_POPUPCHECK             11
#define MENU_POPUPCHECKBACKGROUND   12
#define MENU_POPUPGUTTER            13
#define MENU_POPUPITEM              14
#define MENU_POPUPBORDERS           10
#define MENU_POPUPSEPARATOR         15
#define MC_CHECKMARKNORMAL          1
#define MC_CHECKMARKDISABLED        2
#define MC_BULLETNORMAL             3
#define MC_BULLETDISABLED           4
#define ABS_UPHOVER                 17
#define ABS_DOWNHOVER               18
#define ABS_LEFTHOVER               19
#define ABS_RIGHTHOVER              20
#define CP_DROPDOWNBUTTONRIGHT      6
#define CP_DROPDOWNBUTTONLEFT       7
#define SCRBS_HOVER                 5
#define TVP_HOTGLYPH                4
#define SPI_GETCLIENTAREAANIMATION  0x1042
#define TDLG_PRIMARYPANEL           1
#define TDLG_SECONDARYPANEL         8
#endif

class QWindowsVistaStyle : public QWindowsXPStyle
{
   GUI_CS_OBJECT(QWindowsVistaStyle)

 public:
   QWindowsVistaStyle();

   QWindowsVistaStyle(const QWindowsVistaStyle &) = delete;
   QWindowsVistaStyle &operator=(const QWindowsVistaStyle &) = delete;

   ~QWindowsVistaStyle();

   void drawPrimitive(PrimitiveElement element, const QStyleOption *option, QPainter *painter,
      const QWidget *widget = nullptr) const override;

   void drawControl(ControlElement element, const QStyleOption *option, QPainter *painter,
      const QWidget *widget) const override;

   void drawComplexControl(ComplexControl control, const QStyleOptionComplex *option, QPainter *painter,
      const QWidget *widget) const override;

   QSize sizeFromContents(ContentsType type, const QStyleOption *option, const QSize &size,
      const QWidget *widget) const override;

   QRect subElementRect(SubElement element, const QStyleOption *option, const QWidget *widget) const override;

   QRect subControlRect(ComplexControl control, const QStyleOptionComplex *option, SubControl sc,
      const QWidget *widget) const override;

   SubControl hitTestComplexControl(ComplexControl control, const QStyleOptionComplex *option, const QPoint &pos,
      const QWidget *widget = nullptr) const override;

   QIcon standardIcon(StandardPixmap standardIcon, const QStyleOption *option = nullptr,
      const QWidget *widget = nullptr) const override;

   QPixmap standardPixmap(StandardPixmap standardPixmap, const QStyleOption *option,
      const QWidget *widget = nullptr) const override;

   int pixelMetric(PixelMetric metric, const QStyleOption *option = nullptr, const QWidget *widget = nullptr) const override;

   int styleHint(StyleHint hint, const QStyleOption *opt = nullptr, const QWidget *widget = nullptr,
      QStyleHintReturn *styleHintReturn = nullptr) const override;

   void polish(QWidget *widget) override;
   void unpolish(QWidget *widget) override;
   void polish(QPalette &pal) override;
   void polish(QApplication *app) override;
   void unpolish(QApplication *app) override;

   QPalette standardPalette() const override;

 private:
   Q_DECLARE_PRIVATE(QWindowsVistaStyle)

   friend class QStyleFactory;
};

class QWindowsVistaAnimation : public QBlendStyleAnimation
{
   GUI_CS_OBJECT(QWindowsVistaAnimation)

 public:
   QWindowsVistaAnimation(Type type, QObject *target)
      : QBlendStyleAnimation(type, target)
   { }

   bool isUpdateNeeded() const override;
   void paint(QPainter *painter, const QStyleOption *option);
};

// Handles state transition animations
class QWindowsVistaTransition : public QWindowsVistaAnimation
{
   GUI_CS_OBJECT(QWindowsVistaTransition)

 public :
   QWindowsVistaTransition(QObject *target)
      : QWindowsVistaAnimation(Transition, target)
   { }
};

// Handles pulse animations (default buttons)
class QWindowsVistaPulse: public QWindowsVistaAnimation
{
   GUI_CS_OBJECT(QWindowsVistaPulse)

 public :
   QWindowsVistaPulse(QObject *target)
      : QWindowsVistaAnimation(Pulse, target)
   { }
};

class QWindowsVistaStylePrivate :  public QWindowsXPStylePrivate
{
   Q_DECLARE_PUBLIC(QWindowsVistaStyle)

 public:
   QWindowsVistaStylePrivate();

   static int fixedPixelMetric(QStyle::PixelMetric pm);
   static inline bool useVista();
   bool transitionsEnabled() const;
};

#endif // QT_NO_STYLE_WINDOWSVISTA
#endif
