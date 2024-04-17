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


class QWizard;

class QVistaBackButton : public QAbstractButton
{
 public:
   QVistaBackButton(QWidget *widget);

   QSize sizeHint() const override;

   QSize minimumSizeHint() const override {
      return sizeHint();
   }

   void enterEvent(QEvent *event) override;
   void leaveEvent(QEvent *event) override;
   void paintEvent(QPaintEvent *event) override;
};

class QVistaHelper : public QObject
{
 public:
   enum TitleBarChangeType {
      NormalTitleBar,
      ExtendedTitleBar
   };

   enum VistaState {
      VistaAero,
      VistaBasic,
      Classic,
      Dirty
   };

   QVistaHelper(QWizard *wizard);
   ~QVistaHelper();

   void updateCustomMargins(bool vistaMargins);
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

   QColor basicWindowFrameColor();

   static VistaState vistaState();

   static int titleBarSize() {
      return QVistaHelper::titleBarSizeDp() / QVistaHelper::m_devicePixelRatio;
   }

   static int titleBarSizeDp() {
      return QVistaHelper::frameSizeDp() + QVistaHelper::captionSizeDp();
   }

   static int topPadding() {
      // padding under text
      return int(QStyleHelper::dpiScaled(QSysInfo::WindowsVersion >= QSysInfo::WV_WINDOWS7 ? 4 : 6));
   }

   static int topOffset();
   static HDC backingStoreDC(const QWidget *wizard, QPoint *offset);

 private:
   enum Changes {
      resizeTop,
      movePosition,
      noChange
   };

   HWND wizardHWND() const;
   bool drawTitleText(QPainter *painter, const QString &text, const QRect &rect, HDC hdc);
   static bool drawBlackRect(const QRect &rect, HDC hdc);

   static int frameSize() {
      return QVistaHelper::frameSizeDp() / QVistaHelper::m_devicePixelRatio;
   }

   static int frameSizeDp();
   static int captionSizeDp();

   static int captionSize() {
      return QVistaHelper::captionSizeDp() / QVistaHelper::m_devicePixelRatio;
   }

   static int backButtonSize() {
      return int(QStyleHelper::dpiScaled(30));
   }

   static int iconSize();
   static int glowSize();

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

   Changes change;
   QPoint pressedPos;
   bool pressed;
   QRect rtTop;
   QRect rtTitle;
   QWizard *wizard;
   QVistaBackButton *backButton_;

   int titleBarOffset;  // Extra spacing above the text
   int iconSpacing;    // Space between button and icon
   int textSpacing;    // Space between icon and text
   static int m_devicePixelRatio;
};

#endif // QT_NO_STYLE_WINDOWSVISTA
#endif // QT_NO_WIZARD

#endif
