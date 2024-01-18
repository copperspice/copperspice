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

#ifndef QACCESSIBLEWIDGETS_H
#define QACCESSIBLEWIDGETS_H

#include <qaccessiblewidget.h>

#ifndef QT_NO_ACCESSIBILITY

#include <qpointer.h>
#include <qpair.h>

class QTextEdit;
class QStackedWidget;
class QToolBox;
class QMdiArea;
class QMdiSubWindow;
class QRubberBand;
class QTextBrowser;
class QCalendarWidget;
class QAbstractItemView;
class QDockWidget;
class QDockWidgetLayout;
class QMainWindow;
class QPlainTextEdit;
class QTextCursor;
class QTextDocument;

#ifndef QT_NO_CURSOR
class QAccessibleTextWidget : public QAccessibleWidget,
   public QAccessibleTextInterface,
   public QAccessibleEditableTextInterface
{
 public:
   QAccessibleTextWidget(QWidget *o, QAccessible::Role r = QAccessible::EditableText, const QString &name = QString());

   QAccessible::State state() const override;

   // QAccessibleTextInterface
   //  selection
   void selection(int selectionIndex, int *startOffset, int *endOffset) const override;
   int selectionCount() const override;
   void addSelection(int startOffset, int endOffset) override;
   void removeSelection(int selectionIndex) override;
   void setSelection(int selectionIndex, int startOffset, int endOffset) override;

   // cursor
   int cursorPosition() const override;
   void setCursorPosition(int position) override;

   // text
   QString text(int startOffset, int endOffset) const override;
   QString textBeforeOffset(int offset, QAccessible::TextBoundaryType boundaryType,
      int *startOffset, int *endOffset) const override;
   QString textAfterOffset(int offset, QAccessible::TextBoundaryType boundaryType,
      int *startOffset, int *endOffset) const override;
   QString textAtOffset(int offset, QAccessible::TextBoundaryType boundaryType,
      int *startOffset, int *endOffset) const override;
   int characterCount() const override;

   // character <-> geometry
   QRect characterRect(int offset) const override;
   int offsetAtPoint(const QPoint &point) const override;

   QString attributes(int offset, int *startOffset, int *endOffset) const override;

   // QAccessibleEditableTextInterface
   void deleteText(int startOffset, int endOffset) override;
   void insertText(int offset, const QString &text) override;
   void replaceText(int startOffset, int endOffset, const QString &text) override;

   using QAccessibleWidget::text;

 protected:
   QTextCursor textCursorForRange(int startOffset, int endOffset) const;
   virtual QPoint scrollBarPosition() const;
   // return the current text cursor at the caret position including a potential selection
   virtual QTextCursor textCursor() const = 0;
   virtual void setTextCursor(const QTextCursor &) = 0;
   virtual QTextDocument *textDocument() const = 0;
   virtual QWidget *viewport() const = 0;
};

#ifndef QT_NO_TEXTEDIT
class QAccessiblePlainTextEdit : public QAccessibleTextWidget
{
 public:
   explicit QAccessiblePlainTextEdit(QWidget *o);

   QString text(QAccessible::Text t) const override;
   void setText(QAccessible::Text t, const QString &text) override;
   QAccessible::State state() const override;

   void *interface_cast(QAccessible::InterfaceType t) override;

   // QAccessibleTextInterface
   void scrollToSubstring(int startIndex, int endIndex) override;

   using QAccessibleTextWidget::text;

 protected:
   QPlainTextEdit *plainTextEdit() const;

   QPoint scrollBarPosition() const override;
   QTextCursor textCursor() const override;
   void setTextCursor(const QTextCursor &textCursor) override;
   QTextDocument *textDocument() const override;
   QWidget *viewport() const override;
};

class QAccessibleTextEdit : public QAccessibleTextWidget
{
 public:
   explicit QAccessibleTextEdit(QWidget *o);

   QString text(QAccessible::Text t) const override;
   void setText(QAccessible::Text t, const QString &text) override;
   QAccessible::State state() const override;

   void *interface_cast(QAccessible::InterfaceType t) override;

   // QAccessibleTextInterface
   void scrollToSubstring(int startIndex, int endIndex) override;

   using QAccessibleTextWidget::text;

 protected:
   QTextEdit *textEdit() const;

   QPoint scrollBarPosition() const override;
   QTextCursor textCursor() const override;
   void setTextCursor(const QTextCursor &textCursor) override;
   QTextDocument *textDocument() const override;
   QWidget *viewport() const override;
};
#endif // QT_NO_TEXTEDIT
#endif  //QT_NO_CURSOR

class QAccessibleStackedWidget : public QAccessibleWidget
{
 public:
   explicit QAccessibleStackedWidget(QWidget *widget);

   QAccessibleInterface *childAt(int x, int y) const override;
   int childCount() const override;
   int indexOfChild(const QAccessibleInterface *child) const override;
   QAccessibleInterface *child(int index) const override;

 protected:
   QStackedWidget *stackedWidget() const;
};

class QAccessibleToolBox : public QAccessibleWidget
{
 public:
   explicit QAccessibleToolBox(QWidget *widget);

   // FIXME we currently expose the toolbox but it is not keyboard navigatable
   // and the accessible hierarchy is not exactly beautiful.
   //    int childCount() const;
   //    QAccessibleInterface *child(int index) const;
   //    int indexOfChild(const QAccessibleInterface *child) const;

 protected:
   QToolBox *toolBox() const;
};

#ifndef QT_NO_MDIAREA
class QAccessibleMdiArea : public QAccessibleWidget
{
 public:
   explicit QAccessibleMdiArea(QWidget *widget);

   int childCount() const override;
   QAccessibleInterface *child(int index) const override;
   int indexOfChild(const QAccessibleInterface *child) const override;

 protected:
   QMdiArea *mdiArea() const;
};

class QAccessibleMdiSubWindow : public QAccessibleWidget
{
 public:
   explicit QAccessibleMdiSubWindow(QWidget *widget);

   QString text(QAccessible::Text textType) const override;
   void setText(QAccessible::Text textType, const QString &text) override;
   QAccessible::State state() const override;
   int childCount() const override;
   QAccessibleInterface *child(int index) const override;
   int indexOfChild(const QAccessibleInterface *child) const override;
   QRect rect() const override;

 protected:
   QMdiSubWindow *mdiSubWindow() const;
};
#endif // QT_NO_MDIAREA

class QAccessibleDialogButtonBox : public QAccessibleWidget
{
 public:
   explicit QAccessibleDialogButtonBox(QWidget *widget);
};

#if !defined(QT_NO_TEXTBROWSER) && !defined(QT_NO_CURSOR)
class QAccessibleTextBrowser : public QAccessibleTextEdit
{
 public:
   explicit QAccessibleTextBrowser(QWidget *widget);

   QAccessible::Role role() const override;
};
#endif // QT_NO_TEXTBROWSER && QT_NO_CURSOR

#ifndef QT_NO_CALENDARWIDGET
class QAccessibleCalendarWidget : public QAccessibleWidget
{
 public:
   explicit QAccessibleCalendarWidget(QWidget *widget);

   int childCount() const override;
   int indexOfChild(const QAccessibleInterface *child) const override;

   QAccessibleInterface *child(int index) const override;

 protected:
   QCalendarWidget *calendarWidget() const;

 private:
   QAbstractItemView *calendarView() const;
   QWidget *navigationBar() const;
};
#endif // QT_NO_CALENDARWIDGET

#ifndef QT_NO_DOCKWIDGET
class QAccessibleDockWidget: public QAccessibleWidget
{
 public:
   explicit QAccessibleDockWidget(QWidget *widget);
   QAccessibleInterface *child(int index) const override;
   int indexOfChild(const QAccessibleInterface *child) const override;
   int childCount() const override;
   QRect rect () const override;
   QString text(QAccessible::Text t) const override;

   QDockWidget *dockWidget() const;
 protected:
   QDockWidgetLayout *dockWidgetLayout() const;
};

#endif // QT_NO_DOCKWIDGET

#ifndef QT_NO_MAINWINDOW
class QAccessibleMainWindow : public QAccessibleWidget
{
 public:
   explicit QAccessibleMainWindow(QWidget *widget);

   QAccessibleInterface *child(int index) const override;
   int childCount() const override;
   int indexOfChild(const QAccessibleInterface *iface) const override;
   QAccessibleInterface *childAt(int x, int y) const override;
   QMainWindow *mainWindow() const;

};
#endif //QT_NO_MAINWINDOW

#endif // QT_NO_ACCESSIBILITY

#endif // QACESSIBLEWIDGETS_H
