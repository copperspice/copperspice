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

#ifndef QACCESSIBLEWIDGETS_H
#define QACCESSIBLEWIDGETS_H

#include <QtGui/qaccessible2.h>
#include <QtGui/qaccessiblewidget.h>

#ifndef QT_NO_ACCESSIBILITY

#include <QtCore/QPointer>
#include <QtCore/QPair>

QT_BEGIN_NAMESPACE

class QTextEdit;
class QStackedWidget;
class QToolBox;
class QMdiArea;
class QMdiSubWindow;
class QWorkspace;
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

class QAccessibleTextWidget : public QAccessibleWidgetEx,
                              public QAccessibleTextInterface,
                              public QAccessibleEditableTextInterface
{
public:
    QAccessibleTextWidget(QWidget *o, Role r = EditableText, const QString& name = QString());
    void addSelection(int startOffset, int endOffset);
    void setSelection(int selectionIndex, int startOffset, int endOffset);
    void removeSelection(int selectionIndex);
    void selection(int selectionIndex, int *startOffset, int *endOffset);
    int selectionCount();

    int characterCount();

    void setCursorPosition(int position);
    int cursorPosition();

    QString text(int startOffset, int endOffset);

    QString textAtOffset(int offset, QAccessible2::BoundaryType boundaryType,
                         int *startOffset, int *endOffset);
    QString textBeforeOffset (int offset, QAccessible2::BoundaryType boundaryType,
                              int *startOffset, int *endOffset);
    QString textAfterOffset(int offset, QAccessible2::BoundaryType boundaryType,
                            int *startOffset, int *endOffset);

    QString attributes(int offset, int *startOffset, int *endOffset);

    void deleteText(int startOffset, int endOffset);
    void insertText(int offset, const QString &text);

    void copyText(int startOffset, int endOffset);
    void cutText(int startOffset, int endOffset);
    void pasteText(int offset);
    void setAttributes(int startOffset, int endOffset, const QString &attributes);

    void replaceText(int startOffset, int endOffset, const QString &text);

    QRect characterRect(int offset, QAccessible2::CoordinateType coordType);
    int offsetAtPoint(const QPoint &point, QAccessible2::CoordinateType coordType);
protected:
    QTextCursor textCursorForRange(int startOffset, int endOffset) const;
    QPair<int, int> getBoundaries(int offset, QAccessible2::BoundaryType boundaryType);
    virtual QPoint scrollBarPosition() const;
    virtual QTextCursor textCursor() const = 0;
    virtual void setTextCursor(const QTextCursor &) = 0;
    virtual QTextDocument *textDocument() const = 0;
    virtual QWidget *viewport() const = 0;
};

class QAccessiblePlainTextEdit : public QAccessibleTextWidget
{
    Q_ACCESSIBLE_OBJECT
public:
    explicit QAccessiblePlainTextEdit(QWidget *o);

    int childCount() const;

    // QAccessibleTextInterface
    void scrollToSubstring(int startIndex, int endIndex);
protected:
    QPlainTextEdit *plainTextEdit() const;

    QPoint scrollBarPosition() const;
    QTextCursor textCursor() const;
    void setTextCursor(const QTextCursor &textCursor);
    QTextDocument *textDocument() const;
    QWidget *viewport() const;
};

#ifndef QT_NO_TEXTEDIT
class QAccessibleTextEdit : public QAccessibleTextWidget
{
    Q_ACCESSIBLE_OBJECT
public:
    explicit QAccessibleTextEdit(QWidget *o);

    QString text(Text t, int child) const;
    void setText(Text t, int control, const QString &text);
    Role role(int child) const;

    QVariant invokeMethodEx(QAccessible::Method method, int child, const QVariantList &params);

    QRect rect(int child) const;
    int childAt(int x, int y) const;

    int childCount() const;

    // QAccessibleTextInterface
    void scrollToSubstring(int startIndex, int endIndex);

    // QAccessibleEditableTextInterface
    void copyText(int startOffset, int endOffset);
    void cutText(int startOffset, int endOffset);
    void pasteText(int offset);
    void setAttributes(int startOffset, int endOffset, const QString &attributes);

protected:
    QTextEdit *textEdit() const;

    QPoint scrollBarPosition() const;
    QTextCursor textCursor() const;
    void setTextCursor(const QTextCursor &textCursor);
    QTextDocument *textDocument() const;
    QWidget *viewport() const;
private:
    int childOffset;
};
#endif // QT_NO_TEXTEDIT

class QAccessibleStackedWidget : public QAccessibleWidgetEx
{
    Q_ACCESSIBLE_OBJECT
public:
    explicit QAccessibleStackedWidget(QWidget *widget);

    QVariant invokeMethodEx(QAccessible::Method method, int child, const QVariantList &params);
    int childAt(int x, int y) const;
    int childCount() const;
    int indexOfChild(const QAccessibleInterface *child) const;
    int navigate(RelationFlag relation, int entry, QAccessibleInterface **target) const;

protected:
    QStackedWidget *stackedWidget() const;
};

class QAccessibleToolBox : public QAccessibleWidgetEx
{
    Q_ACCESSIBLE_OBJECT
public:
    explicit QAccessibleToolBox(QWidget *widget);

    QString text(Text textType, int child) const;
    void setText(Text textType, int child, const QString &text);
    State state(int child) const;
    QVariant invokeMethodEx(QAccessible::Method method, int child, const QVariantList &params);
    int childCount() const;
    int indexOfChild(const QAccessibleInterface *child) const;
    int navigate(RelationFlag relation, int entry, QAccessibleInterface **target) const;

protected:
    QToolBox *toolBox() const;
};

#ifndef QT_NO_MDIAREA
class QAccessibleMdiArea : public QAccessibleWidgetEx
{
public:
    explicit QAccessibleMdiArea(QWidget *widget);

    State state(int child) const;
    QVariant invokeMethodEx(QAccessible::Method method, int child, const QVariantList &params);
    int childCount() const;
    int indexOfChild(const QAccessibleInterface *child) const;
    int navigate(RelationFlag relation, int entry, QAccessibleInterface **target) const;

protected:
    QMdiArea *mdiArea() const;
};

class QAccessibleMdiSubWindow : public QAccessibleWidgetEx
{
public:
    explicit QAccessibleMdiSubWindow(QWidget *widget);

    QString text(Text textType, int child) const;
    void setText(Text textType, int child, const QString &text);
    State state(int child) const;
    QVariant invokeMethodEx(QAccessible::Method method, int child, const QVariantList &params);
    int childCount() const;
    int indexOfChild(const QAccessibleInterface *child) const;
    int navigate(RelationFlag relation, int entry, QAccessibleInterface **target) const;
    QRect rect(int child) const;
    int childAt(int x, int y) const;

protected:
    QMdiSubWindow *mdiSubWindow() const;
};
#endif // QT_NO_MDIAREA

#ifndef QT_NO_WORKSPACE
class QAccessibleWorkspace : public QAccessibleWidgetEx
{
public:
    explicit QAccessibleWorkspace(QWidget *widget);

    State state(int child) const;
    QVariant invokeMethodEx(QAccessible::Method method, int child, const QVariantList &params);
    int childCount() const;
    int indexOfChild(const QAccessibleInterface *child) const;
    int navigate(RelationFlag relation, int entry, QAccessibleInterface **target) const;

protected:
    QWorkspace *workspace() const;
};
#endif

class QAccessibleDialogButtonBox : public QAccessibleWidgetEx
{
public:
    explicit QAccessibleDialogButtonBox(QWidget *widget);

    QVariant invokeMethodEx(QAccessible::Method method, int child, const QVariantList &params);
};

#ifndef QT_NO_TEXTBROWSER
class QAccessibleTextBrowser : public QAccessibleTextEdit
{
public:
    explicit QAccessibleTextBrowser(QWidget *widget);

    Role role(int child) const;
};
#endif // QT_NO_TEXTBROWSER

#ifndef QT_NO_CALENDARWIDGET
class QAccessibleCalendarWidget : public QAccessibleWidgetEx
{
public:
    explicit QAccessibleCalendarWidget(QWidget *widget);

    QVariant invokeMethodEx(QAccessible::Method method, int child, const QVariantList &params);
    int childCount() const;
    int indexOfChild(const QAccessibleInterface *child) const;
    int navigate(RelationFlag relation, int entry, QAccessibleInterface **target) const;
    QRect rect(int child) const;
    int childAt(int x, int y) const;

protected:
    QCalendarWidget *calendarWidget() const;

private:
    QAbstractItemView *calendarView() const;
    QWidget *navigationBar() const;
};
#endif // QT_NO_CALENDARWIDGET

#ifndef QT_NO_DOCKWIDGET
class QAccessibleDockWidget: public QAccessibleWidgetEx
{
public:
    explicit QAccessibleDockWidget(QWidget *widget);
    int navigate(RelationFlag relation, int entry, QAccessibleInterface **iface) const;
    int indexOfChild(const QAccessibleInterface *child) const;
    int childCount() const;
    QRect rect (int child ) const;
    Role role(int child) const;
    State state(int child) const;
    int childAt(int x, int y) const;

    QVariant invokeMethodEx(QAccessible::Method method, int child, const QVariantList &params);
    QDockWidget *dockWidget() const;
};

class QAccessibleTitleBar : public QAccessibleInterface
{
public:
    explicit QAccessibleTitleBar(QDockWidget *widget);
    QString actionText(int action, Text t, int child) const;
    bool doAction(int action, int child, const QVariantList& params = QVariantList());
    int userActionCount ( int child) const;
    int navigate(RelationFlag relation, int entry, QAccessibleInterface **iface) const;
    int indexOfChild(const QAccessibleInterface *child) const;
    int childCount() const;
    Relation relationTo(int child,  const QAccessibleInterface *other, int otherChild) const;
    void setText(Text t, int child, const QString &text);
    QString text(Text t, int child) const;
    Role role(int child) const;
    QRect rect (int child) const;
    State state(int child) const;
    int childAt(int x, int y) const;
    QObject *object() const;
    bool isValid() const;


    QPointer<QDockWidget> m_dockWidget;

    QDockWidget *dockWidget() const;
    QDockWidgetLayout *dockWidgetLayout() const;
};

#endif // QT_NO_DOCKWIDGET

#ifndef QT_NO_MAINWINDOW
class QAccessibleMainWindow : public QAccessibleWidgetEx
{
public:
    explicit QAccessibleMainWindow(QWidget *widget);

    QVariant invokeMethodEx(QAccessible::Method method, int child, const QVariantList &params);
    int childCount() const;
    int navigate(RelationFlag relation, int entry, QAccessibleInterface **iface) const;
    int indexOfChild(const QAccessibleInterface *iface) const;
    int childAt(int x, int y) const;
    QMainWindow *mainWindow() const;

};
#endif //QT_NO_MAINWINDOW

#endif // QT_NO_ACCESSIBILITY

QT_END_NAMESPACE

#endif // QACESSIBLEWIDGETS_H
