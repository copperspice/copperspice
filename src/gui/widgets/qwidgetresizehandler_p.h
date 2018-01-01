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

#ifndef QWIDGETRESIZEHANDLER_P_H
#define QWIDGETRESIZEHANDLER_P_H

#include <QtCore/qobject.h>
#include <QtCore/qpoint.h>

#ifndef QT_NO_RESIZEHANDLER

QT_BEGIN_NAMESPACE

class QMouseEvent;
class QKeyEvent;

class Q_GUI_EXPORT QWidgetResizeHandler : public QObject
{
   GUI_CS_OBJECT(QWidgetResizeHandler)

 public:
   enum Action {
      Move     = 0x01,
      Resize   = 0x02,
      Any      = Move | Resize
   };

   explicit QWidgetResizeHandler(QWidget *parent, QWidget *cw = 0);

   inline void setActive(bool b) {
      setActive(Any, b);
   }
   void setActive(Action ac, bool b);
   inline bool isActive() const {
      return isActive(Any);
   }
   bool isActive(Action ac) const;
   inline void setMovingEnabled(bool b) {
      movingEnabled = b;
   }
   inline bool isMovingEnabled() const {
      return movingEnabled;
   }

   inline bool isButtonDown() const {
      return buttonDown;
   }

   inline void setExtraHeight(int h) {
      extrahei = h;
   }
   inline void setSizeProtection(bool b) {
      sizeprotect = b;
   }

   inline void setFrameWidth(int w) {
      fw = w;
   }

   void doResize();
   void doMove();
 
   GUI_CS_SIGNAL_1(Public, void activate())
   GUI_CS_SIGNAL_2(activate)

 protected:
   bool eventFilter(QObject *o, QEvent *e) override;
   void mouseMoveEvent(QMouseEvent *e);
   void keyPressEvent(QKeyEvent *e);

 private:
   Q_DISABLE_COPY(QWidgetResizeHandler)

   enum MousePosition {
      Nowhere,
      TopLeft, BottomRight, BottomLeft, TopRight,
      Top, Bottom, Left, Right,
      Center
   };

   QWidget *widget;
   QWidget *childWidget;
   QPoint moveOffset;
   QPoint invertedMoveOffset;
   MousePosition mode;
   int fw;
   int extrahei;
   int range;
   uint buttonDown         : 1;
   uint moveResizeMode     : 1;
   uint activeForResize    : 1;
   uint sizeprotect        : 1;
   uint movingEnabled      : 1;
   uint activeForMove      : 1;

   void setMouseCursor(MousePosition m);

   inline bool isMove() const {
      return moveResizeMode && mode == Center;
   }

   inline bool isResize() const {
      return moveResizeMode && !isMove();
   }
};

QT_END_NAMESPACE

#endif // QT_NO_RESIZEHANDLER

#endif // QWIDGETRESIZEHANDLER_P_H
