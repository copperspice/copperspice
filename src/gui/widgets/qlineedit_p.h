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

#ifndef QLINEEDIT_P_H
#define QLINEEDIT_P_H

#include <qglobal.h>

#ifndef QT_NO_LINEEDIT

#include <qlineedit.h>
#include <qtoolbutton.h>
#include <qtextlayout.h>
#include <qicon.h>
#include <qstyleoption.h>
#include <qbasictimer.h>
#include <qcompleter.h>
#include <qpointer.h>
#include <qmimedata.h>

#include <qlinecontrol_p.h>
#include <qlinecontrol_p.h>
#include <qwidget_p.h>

#include <algorithm>

class QLineEditPrivate;

class QLineEditIconButton : public QToolButton
{
   GUI_CS_OBJECT(QLineEditIconButton)

   GUI_CS_PROPERTY_READ(opacity, opacity)
   GUI_CS_PROPERTY_WRITE(opacity, setOpacity)

 public:
   explicit QLineEditIconButton(QWidget *parent = nullptr);

   qreal opacity() const {
      return m_opacity;
   }
   void setOpacity(qreal value);

#ifndef QT_NO_ANIMATION
   void animateShow(bool visible) {
      startOpacityAnimation(visible ? 1.0 : 0.0);
   }
#endif

 protected:
   void actionEvent(QActionEvent *e) override;
   void paintEvent(QPaintEvent *event) override;

 private :
   GUI_CS_SLOT_1(Private, void updateCursor())
   GUI_CS_SLOT_2(updateCursor)

#ifndef QT_NO_ANIMATION
   void startOpacityAnimation(qreal endValue);
#endif

   QLineEditPrivate *lineEditPrivate() const;

   qreal m_opacity;
};

class QLineEditPrivate : public QWidgetPrivate
{
   Q_DECLARE_PUBLIC(QLineEdit)

 public:
   enum SideWidgetFlag {
      SideWidgetFadeInWithText = 0x1,
      SideWidgetCreatedByWidgetAction = 0x2,
      SideWidgetClearButton = 0x4
   };

   struct SideWidgetEntry {
      SideWidgetEntry(QWidget *w = nullptr, QAction *a = nullptr, int _flags = 0)
         : widget(w), action(a), flags(_flags)
      {
      }

      QWidget *widget;
      QAction *action;
      int flags;
   };

   typedef QVector<SideWidgetEntry> SideWidgetEntryList;

   struct SideWidgetParameters {
      int iconSize;
      int widgetWidth;
      int widgetHeight;
      int margin;
   };

   QLineEditPrivate()
      : control(nullptr), frame(1), contextMenuEnabled(1), cursorVisible(0),
        dragEnabled(0), clickCausedFocus(0), hscroll(0), vscroll(0),
        alignment(Qt::AlignLeading | Qt::AlignVCenter),
        leftTextMargin(0), topTextMargin(0), rightTextMargin(0), bottomTextMargin(0),
        lastTextSize(0)
   { }

   ~QLineEditPrivate()
   { }

   QLineControl *control;

#ifndef QT_NO_CONTEXTMENU
   QPointer<QAction> selectAllAction;
#endif

   void init(const QString &);

   QRect adjustedControlRect(const QRect &) const;

   int xToPos(int x, QTextLine::CursorPosition = QTextLine::CursorBetweenCharacters) const;
   bool inSelection(int x) const;
   QRect cursorRect() const;
   void setCursorVisible(bool visible);

   void updatePasswordEchoEditing(bool);

   void resetInputMethod();
   inline bool shouldEnableInputMethod() const {
      return !control->isReadOnly();
   }

   inline bool shouldShowPlaceholderText() const {
      return control->text().isEmpty() && control->preeditAreaText().isEmpty()
         && !((alignment & Qt::AlignHCenter) && q_func()->hasFocus());
   }

   static inline QLineEditPrivate *get(QLineEdit *lineEdit) {
      return lineEdit->d_func();
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
   void _q_editFocusChange(bool isFocusChanged);
#endif

   void _q_selectionChanged();
   void _q_updateNeeded(const QRect &);

#ifndef QT_NO_COMPLETER
   void _q_completionHighlighted(const QString &);
#endif

   QPoint mousePressPos;

#ifndef QT_NO_DRAGANDDROP
   QBasicTimer dndTimer;
   void drag();
#endif

   void _q_textChanged(const QString &);
   void _q_clearButtonClicked();

   int leftTextMargin;
   int topTextMargin;
   int rightTextMargin;
   int bottomTextMargin;

   QString placeholderText;
   QWidget *addAction(QAction *newAction, QAction *before, QLineEdit::ActionPosition, int flags = 0);
   void removeAction(QAction *action);
   SideWidgetParameters sideWidgetParameters() const;
   QIcon clearButtonIcon() const;
   void setClearButtonEnabled(bool enabled);
   void positionSideWidgets();

   inline bool hasSideWidgets() const {
      return !leadingSideWidgets.isEmpty() || !trailingSideWidgets.isEmpty();
   }

   inline const SideWidgetEntryList &leftSideWidgetList() const {
      return q_func()->layoutDirection() == Qt::LeftToRight ? leadingSideWidgets : trailingSideWidgets;
   }

   inline const SideWidgetEntryList &rightSideWidgetList() const {
      return q_func()->layoutDirection() == Qt::LeftToRight ? trailingSideWidgets : leadingSideWidgets;
   }

   int effectiveLeftTextMargin() const;
   int effectiveRightTextMargin() const;

 private:
   typedef QPair<QLineEdit::ActionPosition, int> PositionIndexPair;

   PositionIndexPair findSideWidget(const QAction *a) const;

   SideWidgetEntryList leadingSideWidgets;
   SideWidgetEntryList trailingSideWidgets;
   int lastTextSize;
};

static bool isSideWidgetVisible(const QLineEditPrivate::SideWidgetEntry &e)
{
   return e.widget->isVisible();
}

inline int QLineEditPrivate::effectiveLeftTextMargin() const
{
   int result = leftTextMargin;
   if (!leftSideWidgetList().isEmpty()) {
      const SideWidgetParameters p = sideWidgetParameters();
      result += (p.margin + p.widgetWidth)
         * int(std::count_if(leftSideWidgetList().constBegin(), leftSideWidgetList().constEnd(),
               isSideWidgetVisible));
   }
   return result;
}

inline int QLineEditPrivate::effectiveRightTextMargin() const
{
   int result = rightTextMargin;
   if (!rightSideWidgetList().isEmpty()) {
      const SideWidgetParameters p = sideWidgetParameters();
      result += (p.margin + p.widgetWidth)
         * int(std::count_if(rightSideWidgetList().constBegin(), rightSideWidgetList().constEnd(),
               isSideWidgetVisible));
   }
   return result;
}

#endif // QT_NO_LINEEDIT

#endif // QLINEEDIT_P_H
