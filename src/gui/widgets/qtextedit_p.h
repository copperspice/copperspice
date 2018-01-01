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

#ifndef QTEXTEDIT_P_H
#define QTEXTEDIT_P_H

#include <qabstractscrollarea_p.h>
#include <QtGui/qtextdocumentfragment.h>
#include <QtGui/qscrollbar.h>
#include <QtGui/qtextcursor.h>
#include <QtGui/qtextformat.h>
#include <QtGui/qmenu.h>
#include <QtGui/qabstracttextdocumentlayout.h>
#include <QtCore/qbasictimer.h>
#include <QtCore/qurl.h>
#include <qtextcontrol_p.h>
#include <qtextedit.h>

QT_BEGIN_NAMESPACE

#ifndef QT_NO_TEXTEDIT

class QMimeData;

class QTextEditPrivate : public QAbstractScrollAreaPrivate
{
   Q_DECLARE_PUBLIC(QTextEdit)

 public:
   QTextEditPrivate();

   void init(const QString &html = QString());
   void paint(QPainter *p, QPaintEvent *e);
   void _q_repaintContents(const QRectF &contentsRect);

   inline QPoint mapToContents(const QPoint &point) const {
      return QPoint(point.x() + horizontalOffset(), point.y() + verticalOffset());
   }

   void _q_adjustScrollbars();
   void _q_ensureVisible(const QRectF &rect);
   void relayoutDocument();

   void createAutoBulletList();
   void pageUpDown(QTextCursor::MoveOperation op, QTextCursor::MoveMode moveMode);

   inline int horizontalOffset() const {
      return q_func()->isRightToLeft() ? (hbar->maximum() - hbar->value()) : hbar->value();
   }

   inline int verticalOffset() const {
      return vbar->value();
   }

   inline void sendControlEvent(QEvent *e) {
      control->processEvent(e, QPointF(horizontalOffset(), verticalOffset()), viewport);
   }

   void _q_currentCharFormatChanged(const QTextCharFormat &format);

   void updateDefaultTextOption();

   // re-implemented by QTextBrowser, called by QTextDocument::loadResource
   virtual QUrl resolveUrl(const QUrl &url) const {
      return url;
   }

   QTextControl *control;

   QTextEdit::AutoFormatting autoFormatting;
   bool tabChangesFocus;

   QBasicTimer autoScrollTimer;
   QPoint autoScrollDragPos;

   QTextEdit::LineWrapMode lineWrap;
   int lineWrapColumnOrWidth;
   QTextOption::WrapMode wordWrap;

   uint ignoreAutomaticScrollbarAdjustment : 1;
   uint preferRichText : 1;
   uint showCursorOnInitialShow : 1;
   uint inDrag : 1;
   uint clickCausedFocus : 1;

   QString anchorToScrollToWhenVisible;

#ifdef QT_KEYPAD_NAVIGATION
   QBasicTimer deleteAllTimer;
#endif

};

#endif // QT_NO_TEXTEDIT

QT_END_NAMESPACE

#endif // QTEXTEDIT_P_H
