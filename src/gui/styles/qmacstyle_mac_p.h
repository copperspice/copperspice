/***********************************************************************
*
* Copyright (c) 2012-2018 Barbara Geller
* Copyright (c) 2012-2018 Ansel Sermersheim
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
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
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#ifndef QMACSTYLE_MAC_P_H
#define QMACSTYLE_MAC_P_H

#include <cs_carbon_wrapper_p.h>

#include <qmacstyle_mac.h>
#include <qapplication_p.h>
#include <qcombobox_p.h>
#include <qmacstylepixmaps_mac_p.h>
#include <qpaintengine_mac_p.h>
#include <qpainter_p.h>
#include <qprintengine_mac_p.h>
#include <qstylehelper_p.h>
#include <qapplication.h>
#include <qbitmap.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qdialogbuttonbox.h>
#include <qdockwidget.h>
#include <qevent.h>
#include <qfocusframe.h>
#include <qformlayout.h>
#include <qgroupbox.h>
#include <qhash.h>
#include <qheaderview.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qlistview.h>
#include <qmainwindow.h>
#include <qmap.h>
#include <qmenubar.h>
#include <qpaintdevice.h>
#include <qpainter.h>
#include <qpixmapcache.h>
#include <qpointer.h>
#include <qprogressbar.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qrubberband.h>
#include <qsizegrip.h>
#include <qspinbox.h>
#include <qsplitter.h>
#include <qstyleoption.h>
#include <qtextedit.h>
#include <qtextstream.h>
#include <qtoolbar.h>
#include <qtoolbutton.h>
#include <qtreeview.h>
#include <qtableview.h>
#include <qwizard.h>
#include <qdebug.h>
#include <qlibrary.h>
#include <qdatetimeedit.h>
#include <qmath.h>
#include <QtGui/qgraphicsproxywidget.h>
#include <QtGui/qgraphicsview.h>
#include <qt_cocoa_helpers_mac_p.h>

QT_BEGIN_NAMESPACE

/*
    AHIG:
        Apple Human Interface Guidelines
        http://developer.apple.com/documentation/UserExperience/Conceptual/OSXHIGuidelines/

    Builder:
        Apple Interface Builder v. 3.1.1
*/

// this works as long as we have at most 16 different control types
#define CT1(c) CT2(c, c)
#define CT2(c1, c2) ((uint(c1) << 16) | uint(c2))

enum QAquaWidgetSize { QAquaSizeLarge = 0, QAquaSizeSmall = 1, QAquaSizeMini = 2,
                       QAquaSizeUnknown = -1
                     };

#define SIZE(large, small, mini) \
    (controlSize == QAquaSizeLarge ? (large) : controlSize == QAquaSizeSmall ? (small) : (mini))

// same as return SIZE(...) but optimized
#define return_SIZE(large, small, mini) \
    do { \
        static const int sizes[] = { (large), (small), (mini) }; \
        return sizes[controlSize]; \
    } while (0)

bool qt_mac_buttonIsRenderedFlat(const QPushButton *pushButton, const QStyleOptionButton *option);

class QMacStylePrivate : public QObject
{
   GUI_CS_OBJECT(QMacStylePrivate)

 public:
   QMacStylePrivate(QMacStyle *style);

   // Ideally these wouldn't exist, but since they already exist we need some accessors.
   static const int PushButtonLeftOffset;
   static const int PushButtonTopOffset;
   static const int PushButtonRightOffset;
   static const int PushButtonBottomOffset;
   static const int MiniButtonH;
   static const int SmallButtonH;
   static const int BevelButtonW;
   static const int BevelButtonH;
   static const int PushButtonContentPadding;


   // Stuff from QAquaAnimate:
   bool addWidget(QWidget *);
   void removeWidget(QWidget *);

   enum Animates { AquaPushButton, AquaProgressBar, AquaListViewItemOpen };
   bool animatable(Animates, const QWidget *) const;
   void stopAnimate(Animates, QWidget *);
   void startAnimate(Animates, QWidget *);
   static ThemeDrawState getDrawState(QStyle::State flags);

   QAquaWidgetSize aquaSizeConstrain(const QStyleOption *option, const QWidget *widg, 
         QStyle::ContentsType ct = QStyle::CT_CustomBase, QSize szHint = QSize(-1, -1), QSize *insz = 0) const;

   void getSliderInfo(QStyle::ComplexControl cc, const QStyleOptionSlider *slider,
         HIThemeTrackDrawInfo *tdi, const QWidget *needToRemoveMe);

   bool doAnimate(Animates);
   inline int animateSpeed(Animates) const {
      return 33;
   }

   // Utility functions
   void drawColorlessButton(const HIRect &macRect, HIThemeButtonDrawInfo *bdi, QPainter *p, const QStyleOption *opt) const;

   QSize pushButtonSizeFromContents(const QStyleOptionButton *btn) const;

   HIRect pushButtonContentBounds(const QStyleOptionButton *btn, const HIThemeButtonDrawInfo *bdi) const;

   void initComboboxBdi(const QStyleOptionComboBox *combo, HIThemeButtonDrawInfo *bdi,
                  const QWidget *widget, const ThemeDrawState &tds);

   static HIRect comboboxInnerBounds(const HIRect &outerBounds, int buttonKind);

   static QRect comboboxEditBounds(const QRect &outerBounds, const HIThemeButtonDrawInfo &bdi);

   static void drawCombobox(const HIRect &outerBounds, const HIThemeButtonDrawInfo &bdi, QPainter *p);
   static void drawTableHeader(const HIRect &outerBounds, bool drawTopBorder, bool drawLeftBorder,
                  const HIThemeButtonDrawInfo &bdi, QPainter *p);

   bool contentFitsInPushButton(const QStyleOptionButton *btn, HIThemeButtonDrawInfo *bdi,
                  ThemeButtonKind buttonKindToCheck) const;

   void initHIThemePushButton(const QStyleOptionButton *btn, const QWidget *widget,
                  const ThemeDrawState tds,HIThemeButtonDrawInfo *bdi) const;

   QPixmap generateBackgroundPattern() const;

   QPointer<QPushButton> defaultButton; //default push buttons
   int timerID;
   QList<QPointer<QWidget> > progressBars; //existing progress bars that need animation

   struct ButtonState {
      int frame;
      enum { ButtonDark, ButtonLight } dir;
   } buttonState;

   UInt8 progressFrame;
   QPointer<QFocusFrame> focusWidget;
   CFAbsoluteTime defaultButtonStart;
   QMacStyle *q;
   bool mouseDown;

 protected:
   bool eventFilter(QObject *, QEvent *) override;
   void timerEvent(QTimerEvent *) override;

 private :
   GUI_CS_SLOT_1(Private, void startAnimationTimer())
   GUI_CS_SLOT_2(startAnimationTimer)
  
};

QT_END_NAMESPACE

#endif // QMACSTYLE_MAC_P_H
