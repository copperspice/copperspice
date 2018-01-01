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

#ifndef QLINEEDIT_P_H
#define QLINEEDIT_P_H

#include <QtCore/qglobal.h>

#ifndef QT_NO_LINEEDIT
#include <qwidget_p.h>
#include <QtGui/qlineedit.h>
#include <QtGui/qtextlayout.h>
#include <QtGui/qstyleoption.h>
#include <QtCore/qbasictimer.h>
#include <QtGui/qcompleter.h>
#include <QtCore/qpointer.h>
#include <QtGui/qlineedit.h>
#include <qlinecontrol_p.h>

QT_BEGIN_NAMESPACE

class QLineEditPrivate : public QWidgetPrivate
{
   Q_DECLARE_PUBLIC(QLineEdit)

 public:

   QLineEditPrivate()
      : control(0), frame(1), contextMenuEnabled(1), cursorVisible(0),
        dragEnabled(0), clickCausedFocus(0), hscroll(0), vscroll(0),
        alignment(Qt::AlignLeading | Qt::AlignVCenter),
        leftTextMargin(0), topTextMargin(0), rightTextMargin(0), bottomTextMargin(0) {
   }

   ~QLineEditPrivate() {
   }

   QLineControl *control;

#ifndef QT_NO_CONTEXTMENU
   QPointer<QAction> selectAllAction;
#endif
   void init(const QString &);

   QRect adjustedControlRect(const QRect &) const;

   int xToPos(int x, QTextLine::CursorPosition = QTextLine::CursorBetweenCharacters) const;
   QRect cursorRect() const;
   void setCursorVisible(bool visible);

   void updatePasswordEchoEditing(bool);

   inline bool shouldEnableInputMethod() const {
      return !control->isReadOnly();
   }

   QPoint tripleClick;
   QBasicTimer tripleClickTimer;
   uint frame : 1;
   uint contextMenuEnabled : 1;
   uint cursorVisible : 1;
   uint dragEnabled : 1;
   uint clickCausedFocus : 1;
   int hscroll;
   int vscroll;
   uint alignment;
   static const int verticalMargin;
   static const int horizontalMargin;

   bool sendMouseEventToInputContext(QMouseEvent *e);

   QRect adjustedContentsRect() const;

   void _q_handleWindowActivate();
   void _q_textEdited(const QString &);
   void _q_cursorPositionChanged(int, int);

#ifdef QT_KEYPAD_NAVIGATION
   void _q_editFocusChange(bool);
#endif

   void _q_selectionChanged();
   void _q_updateNeeded(const QRect &);

#ifndef QT_NO_COMPLETER
   void _q_completionHighlighted(QString);
#endif

#ifndef QT_NO_DRAGANDDROP
   QPoint dndPos;
   QBasicTimer dndTimer;
   void drag();
#endif

   int leftTextMargin;
   int topTextMargin;
   int rightTextMargin;
   int bottomTextMargin;

   QString placeholderText;
};

#endif // QT_NO_LINEEDIT

QT_END_NAMESPACE

#endif // QLINEEDIT_P_H
