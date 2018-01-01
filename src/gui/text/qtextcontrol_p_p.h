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

#ifndef QTEXTCONTROL_P_P_H
#define QTEXTCONTROL_P_P_H

#include <QtGui/qtextdocumentfragment.h>
#include <QtGui/qscrollbar.h>
#include <QtGui/qtextcursor.h>
#include <QtGui/qtextformat.h>
#include <QtGui/qmenu.h>
#include <QtGui/qabstracttextdocumentlayout.h>
#include <QtCore/qbasictimer.h>
#include <QtCore/qpointer.h>

QT_BEGIN_NAMESPACE

class QMimeData;
class QAbstractScrollArea;
class QInputContext;

class QTextControlPrivate
{
   Q_DECLARE_PUBLIC(QTextControl)

 public:
   QTextControlPrivate();
   virtual ~QTextControlPrivate() {}

   bool cursorMoveKeyEvent(QKeyEvent *e);

   void updateCurrentCharFormat();

   void indent();
   void outdent();

   void gotoNextTableCell();
   void gotoPreviousTableCell();

   void createAutoBulletList();

   void init(Qt::TextFormat format = Qt::RichText, const QString &text = QString(), QTextDocument *document = 0);
   void setContent(Qt::TextFormat format = Qt::RichText, const QString &text = QString(), QTextDocument *document = 0);
   void startDrag();

   void paste(const QMimeData *source);

   void setCursorPosition(const QPointF &pos);
   void setCursorPosition(int pos, QTextCursor::MoveMode mode = QTextCursor::MoveAnchor);

   void repaintCursor();
   inline void repaintSelection() {
      repaintOldAndNewSelection(QTextCursor());
   }
   void repaintOldAndNewSelection(const QTextCursor &oldSelection);

   void selectionChanged(bool forceEmitSelectionChanged = false);

   void _q_updateCurrentCharFormatAndSelection();

#ifndef QT_NO_CLIPBOARD
   void setClipboardSelection();
#endif

   void _q_emitCursorPosChanged(const QTextCursor &someCursor);

   void setBlinkingCursorEnabled(bool enable);

   void extendWordwiseSelection(int suggestedNewPosition, qreal mouseXPosition);
   void extendBlockwiseSelection(int suggestedNewPosition);

   void _q_deleteSelected();

   void _q_setCursorAfterUndoRedo(int undoPosition, int charsAdded, int charsRemoved);

   QRectF cursorRectPlusUnicodeDirectionMarkers(const QTextCursor &cursor) const;
   QRectF rectForPosition(int position) const;
   QRectF selectionRect(const QTextCursor &cursor) const;
   inline QRectF selectionRect() const {
      return selectionRect(this->cursor);
   }

   QString anchorForCursor(const QTextCursor &anchor) const;

   void keyPressEvent(QKeyEvent *e);
   void mousePressEvent(QEvent *e, Qt::MouseButton button, const QPointF &pos, Qt::KeyboardModifiers modifiers,
         Qt::MouseButtons buttons, const QPoint &globalPos);

   void mouseMoveEvent(QEvent *e, Qt::MouseButton button, const QPointF &pos, Qt::KeyboardModifiers modifiers,
         Qt::MouseButtons buttons, const QPoint &globalPos);

   void mouseReleaseEvent(QEvent *e, Qt::MouseButton button, const QPointF &pos, Qt::KeyboardModifiers modifiers,
         Qt::MouseButtons buttons, const QPoint &globalPos);

   void mouseDoubleClickEvent(QEvent *e, Qt::MouseButton button, const QPointF &pos, Qt::KeyboardModifiers modifiers,
         Qt::MouseButtons buttons, const QPoint &globalPos);

   bool sendMouseEventToInputContext(QEvent *e,  QEvent::Type eventType, Qt::MouseButton button, const QPointF &pos,
         Qt::KeyboardModifiers modifiers, Qt::MouseButtons buttons, const QPoint &globalPos);

   void contextMenuEvent(const QPoint &screenPos, const QPointF &docPos, QWidget *contextWidget);
   void focusEvent(QFocusEvent *e);

#ifdef QT_KEYPAD_NAVIGATION
   void editFocusEvent(QEvent *e);
#endif
   bool dragEnterEvent(QEvent *e, const QMimeData *mimeData);
   void dragLeaveEvent();
   bool dragMoveEvent(QEvent *e, const QMimeData *mimeData, const QPointF &pos);
   bool dropEvent(const QMimeData *mimeData, const QPointF &pos, Qt::DropAction dropAction, QWidget *source);

   void inputMethodEvent(QInputMethodEvent *);

   void activateLinkUnderCursor(QString href = QString());

#ifndef QT_NO_TOOLTIP
   void showToolTip(const QPoint &globalPos, const QPointF &pos, QWidget *contextWidget);
#endif

   void append(const QString &text, Qt::TextFormat format = Qt::AutoText);

   QInputContext *inputContext();

   QTextDocument *doc;
   bool cursorOn;
   QTextCursor cursor;
   bool cursorIsFocusIndicator;
   QTextCharFormat lastCharFormat;

   QTextCursor dndFeedbackCursor;

   Qt::TextInteractionFlags interactionFlags;

   QBasicTimer cursorBlinkTimer;
   QBasicTimer trippleClickTimer;
   QPointF trippleClickPoint;

   bool dragEnabled;

   bool mousePressed;

   bool mightStartDrag;
   QPoint dragStartPos;
   QPointer<QWidget> contextWidget;

   bool lastSelectionState;

   bool ignoreAutomaticScrollbarAdjustement;

   QTextCursor selectedWordOnDoubleClick;
   QTextCursor selectedBlockOnTrippleClick;

   bool overwriteMode;
   bool acceptRichText;

   int preeditCursor;
   bool hideCursor; // used to hide the cursor in the preedit area

   QVector<QAbstractTextDocumentLayout::Selection> extraSelections;

   QPalette palette;
   bool hasFocus;

#ifdef QT_KEYPAD_NAVIGATION
   bool hasEditFocus;
#endif

   bool isEnabled;

   QString highlightedAnchor; // Anchor below cursor
   QString anchorOnMousePress;
   bool hadSelectionOnMousePress;

   bool ignoreUnusedNavigationEvents;
   bool openExternalLinks;

   bool wordSelectionEnabled;

   QString linkToCopy;
   void _q_copyLink();
   void _q_updateBlock(const QTextBlock &);
   void _q_documentLayoutChanged();

 protected:
   QTextControl *q_ptr;
};

QT_END_NAMESPACE

#endif // QTEXTCONTROL_P_H
