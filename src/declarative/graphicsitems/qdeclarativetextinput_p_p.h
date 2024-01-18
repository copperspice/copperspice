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

#ifndef QDECLARATIVETEXTINPUT_P_P_H
#define QDECLARATIVETEXTINPUT_P_P_H

#include <qdeclarativetextinput_p.h>
#include <qdeclarativeimplicitsizeitem_p_p.h>
#include <qdeclarative.h>
#include <QPointer>
#include <qlinecontrol_p.h>

#ifndef QT_NO_LINEEDIT

QT_BEGIN_NAMESPACE

class QDeclarativeTextInputPrivate : public QDeclarativeImplicitSizePaintedItemPrivate
{
   Q_DECLARE_PUBLIC(QDeclarativeTextInput)
 public:
   QDeclarativeTextInputPrivate() : control(new QLineControl),
      color((QRgb)0), style(QDeclarativeText::Normal),
      styleColor((QRgb)0), hAlign(QDeclarativeTextInput::AlignLeft),
      mouseSelectionMode(QDeclarativeTextInput::SelectCharacters), inputMethodHints(Qt::ImhNone),
      hscroll(0), oldScroll(0), oldValidity(false), focused(false), focusOnPress(true),
      showInputPanelOnFocus(true), clickCausedFocus(false), cursorVisible(false),
      autoScroll(true), selectByMouse(false), canPaste(false), hAlignImplicit(true),
      selectPressed(false) {
   }

   ~QDeclarativeTextInputPrivate() {
   }

   int xToPos(int x, QTextLine::CursorPosition betweenOrOn = QTextLine::CursorBetweenCharacters) const {
      Q_Q(const QDeclarativeTextInput);
      QRect cr = q->boundingRect().toRect();
      x -= cr.x() - hscroll;
      return control->xToPos(x, betweenOrOn);
   }

   void init();
   void startCreatingCursor();
   void focusChanged(bool hasFocus);
   void updateHorizontalScroll();
   bool determineHorizontalAlignment();
   bool setHAlign(QDeclarativeTextInput::HAlignment, bool forceAlign = false);
   void mirrorChange();
   int calculateTextWidth();
   bool sendMouseEventToInputContext(QGraphicsSceneMouseEvent *event, QEvent::Type eventType);
   void updateInputMethodHints();

   QLineControl *control;

   QFont font;
   QFont sourceFont;
   QColor  color;
   QColor  selectionColor;
   QColor  selectedTextColor;
   QDeclarativeText::TextStyle style;
   QColor  styleColor;
   QDeclarativeTextInput::HAlignment hAlign;
   QDeclarativeTextInput::SelectionMode mouseSelectionMode;
   Qt::InputMethodHints inputMethodHints;
   QPointer<QDeclarativeComponent> cursorComponent;
   QPointer<QDeclarativeItem> cursorItem;
   QPointF pressPos;

   int lastSelectionStart;
   int lastSelectionEnd;
   int oldHeight;
   int oldWidth;
   int hscroll;
   int oldScroll;
   bool oldValidity: 1;
   bool focused: 1;
   bool focusOnPress: 1;
   bool showInputPanelOnFocus: 1;
   bool clickCausedFocus: 1;
   bool cursorVisible: 1;
   bool autoScroll: 1;
   bool selectByMouse: 1;
   bool canPaste: 1;
   bool hAlignImplicit: 1;
   bool selectPressed: 1;

   static inline QDeclarativeTextInputPrivate *get(QDeclarativeTextInput *t) {
      return t->d_func();
   }
};

QT_END_NAMESPACE

#endif // QT_NO_LINEEDIT

#endif

