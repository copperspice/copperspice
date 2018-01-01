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

#ifndef QWIZARD_WIN_P_H
#define QWIZARD_WIN_P_H

#ifndef QT_NO_WIZARD
#ifndef QT_NO_STYLE_WINDOWSVISTA

#include <qt_windows.h>
#include <qobject.h>
#include <qwidget.h>
#include <qabstractbutton.h>
#include <qwidget_p.h>
#include <qstylehelper_p.h>

QT_BEGIN_NAMESPACE

class QVistaBackButton : public QAbstractButton
{
 public:
   QVistaBackButton(QWidget *widget);

   QSize sizeHint() const override;
   inline QSize minimumSizeHint() const override{
      return sizeHint();
   }

   void enterEvent(QEvent *event) override;
   void leaveEvent(QEvent *event) override;
   void paintEvent(QPaintEvent *event) override;
};

class QWizard;

class QVistaHelper : public QObject
{
 public:
   QVistaHelper(QWizard *wizard);
   ~QVistaHelper();
   enum TitleBarChangeType { NormalTitleBar, ExtendedTitleBar };
   bool setDWMTitleBar(TitleBarChangeType type);
   void setTitleBarIconAndCaptionVisible(bool visible);
   void mouseEvent(QEvent *event);
   bool handleWinEvent(MSG *message, long *result);
   void resizeEvent(QResizeEvent *event);
   void paintEvent(QPaintEvent *event);
   QVistaBackButton *backButton() const {
      return backButton_;
   }

   void disconnectBackButton();
   void hideBackButton() {
      if (backButton_) {
         backButton_->hide();
      }
   }

   void setWindowPosHack();
   QColor basicWindowFrameColor();
   enum VistaState { VistaAero, VistaBasic, Classic, Dirty };
   static VistaState vistaState();
   static int titleBarSize() {
      return frameSize() + captionSize();
   }

   static int topPadding() { // padding under text
      return int(QStyleHelper::dpiScaled(
                    QSysInfo::WindowsVersion >= QSysInfo::WV_WINDOWS7 ? 4 : 6));
   }

   static int topOffset();

 private:
   static HFONT getCaptionFont(HANDLE hTheme);
   bool drawTitleText(QPainter *painter, const QString &text, const QRect &rect, HDC hdc);
   static bool drawBlackRect(const QRect &rect, HDC hdc);

   static int frameSize() {
      return GetSystemMetrics(SM_CYSIZEFRAME);
   }

   static int captionSize() {
      return GetSystemMetrics(SM_CYCAPTION);
   }

   static int backButtonSize() {
      return int(QStyleHelper::dpiScaled(30));
   }
   static int iconSize() {
      return 16;   // Standard Aero
   }
   static int glowSize() {
      return 10;
   }
   int leftMargin() {
      return backButton_->isVisible() ? backButtonSize() + iconSpacing : 0;
   }

   int titleOffset();
   bool resolveSymbols();
   void drawTitleBar(QPainter *painter);
   void setMouseCursor(QPoint pos);
   void collapseTopFrameStrut();
   bool winEvent(MSG *message, long *result);
   void mouseMoveEvent(QMouseEvent *event);
   void mousePressEvent(QMouseEvent *event);
   void mouseReleaseEvent(QMouseEvent *event);
   bool eventFilter(QObject *obj, QEvent *event) override;

   static int instanceCount;
   static bool is_vista;
   static VistaState cachedVistaState;
   static bool isCompositionEnabled();
   static bool isThemeActive();
   enum Changes { resizeTop, movePosition, noChange } change;
   QPoint pressedPos;
   bool pressed;
   QRect rtTop;
   QRect rtTitle;
   QWizard *wizard;
   QVistaBackButton *backButton_;

   int titleBarOffset;  // Extra spacing above the text
   int iconSpacing;    // Space between button and icon
   int textSpacing;    // Space between icon and text
};


QT_END_NAMESPACE

#endif // QT_NO_STYLE_WINDOWSVISTA
#endif // QT_NO_WIZARD
#endif // QWIZARD_WIN_P_H
