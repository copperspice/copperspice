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

#include <qaccessiblewidget_common_p.h>

#include <qabstracttextdocumentlayout.h>
#include <qapplication.h>
#include <qclipboard.h>
#include <qtextedit.h>
#include <qtextdocument.h>
#include <qtextobject.h>
#include <qplaintextedit.h>
#include <qtextboundaryfinder.h>
#include <qscrollbar.h>
#include <qapplication.h>
#include <qstackedwidget.h>
#include <qtoolbox.h>
#include <qmdiarea.h>
#include <qmdisubwindow.h>
#include <qdialogbuttonbox.h>
#include <qrubberband.h>
#include <qtextbrowser.h>
#include <qcalendarwidget.h>
#include <qabstractitemview.h>
#include <qdockwidget.h>
#include <qmainwindow.h>
#include <qabstractbutton.h>
#include <qfocusframe.h>

#include <qdockwidget_p.h>
#include <qtextedit_p.h>

#include <limits.h>

#ifndef QT_NO_ACCESSIBILITY

QString qt_accStripAmp(const QString &text);
QString qt_accHotKey(const QString &text);

QList<QWidget *> childWidgets(const QWidget *widget)
{
   if (widget == nullptr) {
      return QList<QWidget *>();
   }

   QList<QObject *> list = widget->children();
   QList<QWidget *> widgets;

   for (int i = 0; i < list.size(); ++i) {
      QWidget *w = qobject_cast<QWidget *>(list.at(i));
      if (!w) {
         continue;
      }
      QString objectName = w->objectName();
      if (!w->isWindow()
         && !qobject_cast<QFocusFrame *>(w)
         && !qobject_cast<QMenu *>(w)
         && objectName != QLatin1String("qt_rubberband")
         && objectName != QLatin1String("qt_qmainwindow_extended_splitter")) {
         widgets.append(w);
      }
   }
   return widgets;
}

#if !defined(QT_NO_TEXTEDIT) && !defined(QT_NO_CURSOR)

QAccessiblePlainTextEdit::QAccessiblePlainTextEdit(QWidget *o)
   : QAccessibleTextWidget(o)
{
   Q_ASSERT(widget()->inherits("QPlainTextEdit"));
}

QPlainTextEdit *QAccessiblePlainTextEdit::plainTextEdit() const
{
   return static_cast<QPlainTextEdit *>(widget());
}

QString QAccessiblePlainTextEdit::text(QAccessible::Text t) const
{
   if (t == QAccessible::Value) {
      return plainTextEdit()->toPlainText();
   }

   return QAccessibleWidget::text(t);
}

void QAccessiblePlainTextEdit::setText(QAccessible::Text t, const QString &text)
{
   if (t != QAccessible::Value) {
      QAccessibleWidget::setText(t, text);
      return;
   }
   if (plainTextEdit()->isReadOnly()) {
      return;
   }

   plainTextEdit()->setPlainText(text);
}

QAccessible::State QAccessiblePlainTextEdit::state() const
{
   QAccessible::State st = QAccessibleTextWidget::state();
   if (plainTextEdit()->isReadOnly()) {
      st.readOnly = true;
   } else {
      st.editable = true;
   }
   return st;
}

void *QAccessiblePlainTextEdit::interface_cast(QAccessible::InterfaceType t)
{
   if (t == QAccessible::TextInterface) {
      return static_cast<QAccessibleTextInterface *>(this);
   } else if (t == QAccessible::EditableTextInterface) {
      return static_cast<QAccessibleEditableTextInterface *>(this);
   }
   return QAccessibleWidget::interface_cast(t);
}

QPoint QAccessiblePlainTextEdit::scrollBarPosition() const
{
   QPoint result;
   result.setX(plainTextEdit()->horizontalScrollBar() ? plainTextEdit()->horizontalScrollBar()->sliderPosition() : 0);
   result.setY(plainTextEdit()->verticalScrollBar() ? plainTextEdit()->verticalScrollBar()->sliderPosition() : 0);
   return result;
}

QTextCursor QAccessiblePlainTextEdit::textCursor() const
{
   return plainTextEdit()->textCursor();
}

void QAccessiblePlainTextEdit::setTextCursor(const QTextCursor &textCursor)
{
   plainTextEdit()->setTextCursor(textCursor);
}

QTextDocument *QAccessiblePlainTextEdit::textDocument() const
{
   return plainTextEdit()->document();
}

QWidget *QAccessiblePlainTextEdit::viewport() const
{
   return plainTextEdit()->viewport();
}

void QAccessiblePlainTextEdit::scrollToSubstring(int startIndex, int endIndex)
{
   //TODO: Not implemented
   (void) startIndex;
   (void) endIndex;
}

QAccessibleTextEdit::QAccessibleTextEdit(QWidget *o)
   : QAccessibleTextWidget(o, QAccessible::EditableText)
{
   Q_ASSERT(widget()->inherits("QTextEdit"));
}

QTextEdit *QAccessibleTextEdit::textEdit() const
{
   return static_cast<QTextEdit *>(widget());
}

QTextCursor QAccessibleTextEdit::textCursor() const
{
   return textEdit()->textCursor();
}

QTextDocument *QAccessibleTextEdit::textDocument() const
{
   return textEdit()->document();
}

void QAccessibleTextEdit::setTextCursor(const QTextCursor &textCursor)
{
   textEdit()->setTextCursor(textCursor);
}

QWidget *QAccessibleTextEdit::viewport() const
{
   return textEdit()->viewport();
}

QPoint QAccessibleTextEdit::scrollBarPosition() const
{
   QPoint result;
   result.setX(textEdit()->horizontalScrollBar() ? textEdit()->horizontalScrollBar()->sliderPosition() : 0);
   result.setY(textEdit()->verticalScrollBar() ? textEdit()->verticalScrollBar()->sliderPosition() : 0);
   return result;
}

QString QAccessibleTextEdit::text(QAccessible::Text t) const
{
   if (t == QAccessible::Value) {
      return textEdit()->toPlainText();
   }

   return QAccessibleWidget::text(t);
}

void QAccessibleTextEdit::setText(QAccessible::Text t, const QString &text)
{
   if (t != QAccessible::Value) {
      QAccessibleWidget::setText(t, text);
      return;
   }
   if (textEdit()->isReadOnly()) {
      return;
   }

   textEdit()->setText(text);
}

QAccessible::State QAccessibleTextEdit::state() const
{
   QAccessible::State st = QAccessibleTextWidget::state();
   if (textEdit()->isReadOnly()) {
      st.readOnly = true;
   } else {
      st.editable = true;
   }

   return st;
}

void *QAccessibleTextEdit::interface_cast(QAccessible::InterfaceType t)
{
   if (t == QAccessible::TextInterface) {
      return static_cast<QAccessibleTextInterface *>(this);
   } else if (t == QAccessible::EditableTextInterface) {
      return static_cast<QAccessibleEditableTextInterface *>(this);
   }

   return QAccessibleWidget::interface_cast(t);
}

void QAccessibleTextEdit::scrollToSubstring(int startIndex, int endIndex)
{
   QTextEdit *edit = textEdit();

   QTextCursor cursor = textCursor();
   cursor.setPosition(startIndex);
   QRect r = edit->cursorRect(cursor);

   cursor.setPosition(endIndex);
   r.setBottomRight(edit->cursorRect(cursor).bottomRight());
   r.moveTo(r.x() + edit->horizontalScrollBar()->value(), r.y() + edit->verticalScrollBar()->value());

   // ensureVisible is not public
   if (! QMetaObject::invokeMethod(edit, "_q_ensureVisible", Q_ARG(const QRectF &, r))) {
      qWarning("QAccessibleTextEdit::scrollToSubstring() Process failed");
   }
}

#endif // QT_NO_TEXTEDIT && QT_NO_CURSOR

#ifndef QT_NO_STACKEDWIDGET

QAccessibleStackedWidget::QAccessibleStackedWidget(QWidget *widget)
   : QAccessibleWidget(widget, QAccessible::LayeredPane)
{
   Q_ASSERT(qobject_cast<QStackedWidget *>(widget));
}

QAccessibleInterface *QAccessibleStackedWidget::childAt(int x, int y) const
{
   if (! stackedWidget()->isVisible()) {
      return nullptr;
   }

   QWidget *currentWidget = stackedWidget()->currentWidget();
   if (! currentWidget) {
      return nullptr;
   }

   QPoint position = currentWidget->mapFromGlobal(QPoint(x, y));
   if (currentWidget->rect().contains(position)) {
      return child(stackedWidget()->currentIndex());
   }

   return nullptr;
}

int QAccessibleStackedWidget::childCount() const
{
   return stackedWidget()->count();
}

int QAccessibleStackedWidget::indexOfChild(const QAccessibleInterface *child) const
{
   if (!child) {
      return -1;
   }

   QWidget *widget = qobject_cast<QWidget *>(child->object());
   return stackedWidget()->indexOf(widget);
}

QAccessibleInterface *QAccessibleStackedWidget::child(int index) const
{
   if (index < 0 || index >= stackedWidget()->count()) {
      return nullptr;
   }
   return QAccessible::queryAccessibleInterface(stackedWidget()->widget(index));
}

QStackedWidget *QAccessibleStackedWidget::stackedWidget() const
{
   return static_cast<QStackedWidget *>(object());
}
#endif

#ifndef QT_NO_TOOLBOX
QAccessibleToolBox::QAccessibleToolBox(QWidget *widget)
   : QAccessibleWidget(widget, QAccessible::LayeredPane)
{
   Q_ASSERT(qobject_cast<QToolBox *>(widget));
}

QToolBox *QAccessibleToolBox::toolBox() const
{
   return static_cast<QToolBox *>(object());
}
#endif

#ifndef QT_NO_MDIAREA
QAccessibleMdiArea::QAccessibleMdiArea(QWidget *widget)
   : QAccessibleWidget(widget, QAccessible::LayeredPane)
{
   Q_ASSERT(qobject_cast<QMdiArea *>(widget));
}

int QAccessibleMdiArea::childCount() const
{
   return mdiArea()->subWindowList().count();
}

QAccessibleInterface *QAccessibleMdiArea::child(int index) const
{
   QList<QMdiSubWindow *> subWindows = mdiArea()->subWindowList();

   QWidget *targetObject = subWindows.value(index);
   if (!targetObject) {
      return nullptr;
   }

   return QAccessible::queryAccessibleInterface(targetObject);
}

int QAccessibleMdiArea::indexOfChild(const QAccessibleInterface *child) const
{
   if (!child || !child->object() || mdiArea()->subWindowList().isEmpty()) {
      return -1;
   }
   if (QMdiSubWindow *window = qobject_cast<QMdiSubWindow *>(child->object())) {
      return mdiArea()->subWindowList().indexOf(window);
   }
   return -1;
}

QMdiArea *QAccessibleMdiArea::mdiArea() const
{
   return static_cast<QMdiArea *>(object());
}

QAccessibleMdiSubWindow::QAccessibleMdiSubWindow(QWidget *widget)
   : QAccessibleWidget(widget, QAccessible::Window)
{
   Q_ASSERT(qobject_cast<QMdiSubWindow *>(widget));
}

QString QAccessibleMdiSubWindow::text(QAccessible::Text textType) const
{
   if (textType == QAccessible::Name) {
      QString title = mdiSubWindow()->windowTitle();
      title.replace(QLatin1String("[*]"), QLatin1String(""));
      return title;
   }
   return QAccessibleWidget::text(textType);
}

void QAccessibleMdiSubWindow::setText(QAccessible::Text textType, const QString &text)
{
   if (textType == QAccessible::Name) {
      mdiSubWindow()->setWindowTitle(text);
   } else {
      QAccessibleWidget::setText(textType, text);
   }
}

QAccessible::State QAccessibleMdiSubWindow::state() const
{
   QAccessible::State state;
   state.focusable = true;
   if (!mdiSubWindow()->isMaximized()) {
      state.movable = true;
      state.sizeable = true;
   }
   if (mdiSubWindow()->isAncestorOf(QApplication::focusWidget())
      || QApplication::focusWidget() == mdiSubWindow()) {
      state.focused = true;
   }
   if (!mdiSubWindow()->isVisible()) {
      state.invisible = true;
   }
   if (const QWidget *parent = mdiSubWindow()->parentWidget())
      if (!parent->contentsRect().contains(mdiSubWindow()->geometry())) {
         state.offscreen = true;
      }
   if (!mdiSubWindow()->isEnabled()) {
      state.disabled = true;
   }
   return state;
}

int QAccessibleMdiSubWindow::childCount() const
{
   if (mdiSubWindow()->widget()) {
      return 1;
   }
   return 0;
}

QAccessibleInterface *QAccessibleMdiSubWindow::child(int index) const
{
   QMdiSubWindow *source = mdiSubWindow();
   if (index != 0 || !source->widget()) {
      return nullptr;
   }

   return QAccessible::queryAccessibleInterface(source->widget());
}

int QAccessibleMdiSubWindow::indexOfChild(const QAccessibleInterface *child) const
{
   if (child && child->object() && child->object() == mdiSubWindow()->widget()) {
      return 0;
   }
   return -1;
}

QRect QAccessibleMdiSubWindow::rect() const
{
   if (mdiSubWindow()->isHidden()) {
      return QRect();
   }

   if (!mdiSubWindow()->parent()) {
      return QAccessibleWidget::rect();
   }

   const QPoint pos = mdiSubWindow()->mapToGlobal(QPoint(0, 0));

   return QRect(pos, mdiSubWindow()->size());
}

QMdiSubWindow *QAccessibleMdiSubWindow::mdiSubWindow() const
{
   return static_cast<QMdiSubWindow *>(object());
}
#endif // QT_NO_MDIAREA

#ifndef QT_NO_DIALOGBUTTONBOX
// ======================= QAccessibleDialogButtonBox ======================
QAccessibleDialogButtonBox::QAccessibleDialogButtonBox(QWidget *widget)
   : QAccessibleWidget(widget, QAccessible::Grouping)
{
   Q_ASSERT(qobject_cast<QDialogButtonBox *>(widget));
}

#endif

#if ! defined(QT_NO_TEXTBROWSER) && ! defined(QT_NO_CURSOR)
QAccessibleTextBrowser::QAccessibleTextBrowser(QWidget *widget)
   : QAccessibleTextEdit(widget)
{
   Q_ASSERT(qobject_cast<QTextBrowser *>(widget));
}

QAccessible::Role QAccessibleTextBrowser::role() const
{
   return QAccessible::StaticText;
}
#endif

#ifndef QT_NO_CALENDARWIDGET

QAccessibleCalendarWidget::QAccessibleCalendarWidget(QWidget *widget)
   : QAccessibleWidget(widget, QAccessible::Table)
{
   Q_ASSERT(qobject_cast<QCalendarWidget *>(widget));
}

int QAccessibleCalendarWidget::childCount() const
{
   return calendarWidget()->isNavigationBarVisible() ? 2 : 1;
}

int QAccessibleCalendarWidget::indexOfChild(const QAccessibleInterface *child) const
{
   if (!child || !child->object() || childCount() <= 0) {
      return -1;
   }

   if (qobject_cast<QAbstractItemView *>(child->object())) {
      return childCount() - 1;   // FIXME
   }

   return 0;
}

QAccessibleInterface *QAccessibleCalendarWidget::child(int index) const
{
   if (index < 0 || index >= childCount()) {
      return nullptr;
   }

   if (childCount() > 1 && index == 0) {
      return QAccessible::queryAccessibleInterface(navigationBar());
   }

   return QAccessible::queryAccessibleInterface(calendarView());
}

QCalendarWidget *QAccessibleCalendarWidget::calendarWidget() const
{
   return static_cast<QCalendarWidget *>(object());
}

QAbstractItemView *QAccessibleCalendarWidget::calendarView() const
{
   for (QObject *child : calendarWidget()->children()) {
      if (child->objectName() == QLatin1String("qt_calendar_calendarview")) {
         return static_cast<QAbstractItemView *>(child);
      }
   }

   return nullptr;
}

QWidget *QAccessibleCalendarWidget::navigationBar() const
{
   for (QObject *child : calendarWidget()->children()) {
      if (child->objectName() == QLatin1String("qt_calendar_navigationbar")) {
         return static_cast<QWidget *>(child);
      }
   }

   return nullptr;
}
#endif

#ifndef QT_NO_DOCKWIDGET

// Dock Widget - order of children:
// - Content widget
// - Float button
// - Close button
// If there is a custom title bar widget, that one becomes child 1, after the content 0
// (in that case the buttons are ignored)
QAccessibleDockWidget::QAccessibleDockWidget(QWidget *widget)
   : QAccessibleWidget(widget, QAccessible::Window)
{
}

QDockWidgetLayout *QAccessibleDockWidget::dockWidgetLayout() const
{
   return qobject_cast<QDockWidgetLayout *>(dockWidget()->layout());
}

int QAccessibleDockWidget::childCount() const
{
   if (dockWidget()->titleBarWidget()) {
      return dockWidget()->widget() ? 2 : 1;
   }

   return dockWidgetLayout()->count();
}

QAccessibleInterface *QAccessibleDockWidget::child(int index) const
{
   if (dockWidget()->titleBarWidget()) {
      if ((!dockWidget()->widget() && index == 0) || (index == 1)) {
         return QAccessible::queryAccessibleInterface(dockWidget()->titleBarWidget());
      }
      if (index == 0) {
         return QAccessible::queryAccessibleInterface(dockWidget()->widget());
      }
   } else {
      QLayoutItem *item = dockWidgetLayout()->itemAt(index);
      if (item) {
         return QAccessible::queryAccessibleInterface(item->widget());
      }
   }
   return nullptr;
}

int QAccessibleDockWidget::indexOfChild(const QAccessibleInterface *child) const
{
   if (!child || !child->object() || child->object()->parent() != object()) {
      return -1;
   }

   if (dockWidget()->titleBarWidget() == child->object()) {
      return dockWidget()->widget() ? 1 : 0;
   }

   return dockWidgetLayout()->indexOf(qobject_cast<QWidget *>(child->object()));
}

QRect QAccessibleDockWidget::rect() const
{
   QRect rect;

   if (dockWidget()->isFloating()) {
      rect = dockWidget()->frameGeometry();
   } else {
      rect = dockWidget()->rect();
      rect.moveTopLeft(dockWidget()->mapToGlobal(rect.topLeft()));
   }

   return rect;
}

QDockWidget *QAccessibleDockWidget::dockWidget() const
{
   return static_cast<QDockWidget *>(object());
}

QString QAccessibleDockWidget::text(QAccessible::Text t) const
{
   if (t == QAccessible::Name) {
      return qt_accStripAmp(dockWidget()->windowTitle());
   } else if (t == QAccessible::Accelerator) {
      return qt_accHotKey(dockWidget()->windowTitle());
   }
   return QString();
}
#endif

#ifndef QT_NO_CURSOR
QAccessibleTextWidget::QAccessibleTextWidget(QWidget *o, QAccessible::Role r, const QString &name):
   QAccessibleWidget(o, r, name)
{
}

QAccessible::State QAccessibleTextWidget::state() const
{
   QAccessible::State s = QAccessibleWidget::state();
   s.selectableText = true;
   s.multiLine = true;
   return s;
}

QRect QAccessibleTextWidget::characterRect(int offset) const
{
   QTextBlock block = textDocument()->findBlock(offset);
   if (!block.isValid()) {
      return QRect();
   }

   QTextLayout *layout = block.layout();
   QPointF layoutPosition = layout->position();
   int relativeOffset = offset - block.position();
   QTextLine line = layout->lineForTextPosition(relativeOffset);

   QRect r;

   if (line.isValid()) {
      qreal x = line.cursorToX(relativeOffset);

      QTextCharFormat format;
      QTextBlock::iterator iter = block.begin();

      if (iter.atEnd()) {
         format = block.charFormat();

      } else {
         while (!iter.atEnd() && !iter.fragment().contains(offset)) {
            ++iter;
         }

         if (iter.atEnd()) { // newline should have same format as preceding character
            --iter;
         }
         format = iter.fragment().charFormat();
      }

      QFontMetrics fm(format.font());
      const QString ch = text(offset, offset + 1);

      if (!ch.isEmpty()) {
         int w = fm.width(ch);
         int h = fm.height();
         r = QRect(layoutPosition.x() + x, layoutPosition.y() + line.y() + line.ascent() + fm.descent() - h, w, h);
         r.moveTo(viewport()->mapToGlobal(r.topLeft()));
      }

      r.translate(-scrollBarPosition());
   }

   return r;
}

int QAccessibleTextWidget::offsetAtPoint(const QPoint &point) const
{
   QPoint p = viewport()->mapFromGlobal(point);
   // convert to document coordinates
   p += scrollBarPosition();

   return textDocument()->documentLayout()->hitTest(p, Qt::ExactHit);
}

int QAccessibleTextWidget::selectionCount() const
{
   return textCursor().hasSelection() ? 1 : 0;
}

QString QAccessibleTextWidget::attributes(int offset, int *startOffset, int *endOffset) const
{
   /* The list of attributes can be found at:
    http://linuxfoundation.org/collaborate/workgroups/accessibility/iaccessible2/textattributes
   */

   // IAccessible2 defines -1 as length and -2 as cursor position
   if (offset == -2) {
      offset = cursorPosition();
   }

   const int charCount = characterCount();

   // -1 doesn't make much sense here, but it's better to return something
   // screen readers may ask for text attributes at the cursor pos which may be equal to length
   if (offset == -1 || offset == charCount) {
      offset = charCount - 1;
   }

   if (offset < 0 || offset > charCount) {
      *startOffset = -1;
      *endOffset = -1;
      return QString();
   }


   QTextCursor cursor = textCursor();
   cursor.setPosition(offset);
   QTextBlock block = cursor.block();

   int blockStart = block.position();
   int blockEnd = blockStart + block.length();

   QTextBlock::iterator iter = block.begin();
   int lastFragmentIndex = blockStart;

   while (!iter.atEnd()) {
      QTextFragment f = iter.fragment();
      if (f.contains(offset)) {
         break;
      }
      lastFragmentIndex = f.position() + f.length();
      ++iter;
   }

   QTextCharFormat charFormat;

   if (!iter.atEnd()) {
      QTextFragment fragment = iter.fragment();
      charFormat = fragment.charFormat();
      int pos = fragment.position();
      // text block and fragment may overlap, use the smallest common range
      *startOffset = qMax(pos, blockStart);
      *endOffset = qMin(pos + fragment.length(), blockEnd);
   } else {
      charFormat = block.charFormat();
      *startOffset = lastFragmentIndex;
      *endOffset = blockEnd;
   }

   Q_ASSERT(*startOffset <= offset);
   Q_ASSERT(*endOffset >= offset);

   QTextBlockFormat blockFormat = cursor.blockFormat();

   QMap<QString, QString> attrs;
   QString family = charFormat.font().family();

   if (! family.isEmpty()) {
      family = family.replace('\\',  "\\\\");
      family = family.replace(':',   "\\:");
      family = family.replace(',',   "\\,");
      family = family.replace('=',   "\\=");
      family = family.replace(';',   "\\;");
      family = family.replace('\"',  "\\\"");

      attrs["font-family"] = QString("\"%1\"").formatArg(family);
   }

   int fontSize = int(charFormat.font().pointSize());
   if (fontSize) {
      attrs["font-size"] = QString("%1pt").formatArg(fontSize);
   }

   // Different weight values are not handled
   attrs["font-weight"] = charFormat.font().weight() > QFont::Normal ? QString("bold") : QString("normal");

   QFont::Style style = charFormat.font().style();
   attrs["font-style"] = (style == QFont::StyleItalic) ? QString("italic") : ((style == QFont::StyleOblique) ? QString("oblique") :
         QString("normal"));

   QTextCharFormat::UnderlineStyle underlineStyle = charFormat.underlineStyle();

   // underline could still be set in the default font
   if (underlineStyle == QTextCharFormat::NoUnderline && charFormat.font().underline()) {
      underlineStyle = QTextCharFormat::SingleUnderline;
   }

   QString underlineStyleValue;

   switch (underlineStyle) {
      case QTextCharFormat::NoUnderline:
         break;

      case QTextCharFormat::SingleUnderline:
         underlineStyleValue = "solid";
         break;

      case QTextCharFormat::DashUnderline:
         underlineStyleValue = "dash";
         break;

      case QTextCharFormat::DotLine:
         underlineStyleValue = "dash";
         break;

      case QTextCharFormat::DashDotLine:
         underlineStyleValue = "dot-dash";
         break;

      case QTextCharFormat::DashDotDotLine:
         underlineStyleValue = "dot-dot-dash";
         break;

      case QTextCharFormat::WaveUnderline:
         underlineStyleValue = "wave";
         break;

      case QTextCharFormat::SpellCheckUnderline:
         // this is not correct, but provides good approximation at least
         underlineStyleValue = "wave";
         break;

      default:
         qWarning() << "QAccessibleTextWidget::attributes() Unknown QTextCharFormat::UnderlineStyle value "
                    << underlineStyle << " could not be translated to IAccessible2 value";
         break;
   }

   if (! underlineStyleValue.isEmpty()) {
      // if underlineStyleValue is set, there is an underline, and Qt does not support other than single ones

      attrs["text-underline-style"] = underlineStyleValue;
      attrs["text-underline-type"]  = "single";
   }

   if (block.textDirection() == Qt::RightToLeft) {
      attrs["writing-mode"] = "rl";
   }

   QTextCharFormat::VerticalAlignment alignment = charFormat.verticalAlignment();

   if (alignment == QTextCharFormat::AlignSubScript) {
      attrs["text-position"] =  "sub";

   } else if (alignment == QTextCharFormat::AlignSuperScript) {
      attrs["text-position"] = "super";

   } else {
      attrs["text-position"] = "baseline";
   }

   QBrush background = charFormat.background();
   if (background.style() == Qt::SolidPattern) {
      attrs["background-color"] = QString("rgb(%1,%2,%3)").formatArg(background.color().red())
                  .formatArg(background.color().green()).formatArg(background.color().blue());
   }

   QBrush foreground = charFormat.foreground();
   if (foreground.style() == Qt::SolidPattern) {
      attrs["color"] = QString("rgb(%1,%2,%3)").formatArg(foreground.color().red())
                  .formatArg(foreground.color().green()).formatArg(foreground.color().blue());
   }

   switch (blockFormat.alignment() & (Qt::AlignLeft | Qt::AlignRight | Qt::AlignHCenter | Qt::AlignJustify)) {

      case Qt::AlignLeft:
         attrs["text-align"] = "left";
         break;

      case Qt::AlignRight:
         attrs["text-align"] = "right";
         break;

      case Qt::AlignHCenter:
         attrs["text-align"] = "center";
         break;

      case Qt::AlignJustify:
         attrs["text-align"] = "justify";
         break;
   }

   QString result;

   for (const QString &attributeName : attrs.keys()) {
      result += attributeName + ':' + attrs[attributeName] + ';';
   }

   return result;
}

int QAccessibleTextWidget::cursorPosition() const
{
   return textCursor().position();
}

void QAccessibleTextWidget::selection(int selectionIndex, int *startOffset, int *endOffset) const
{
   *startOffset = *endOffset = 0;
   QTextCursor cursor = textCursor();

   if (selectionIndex != 0 || !cursor.hasSelection()) {
      return;
   }

   *startOffset = cursor.selectionStart();
   *endOffset   = cursor.selectionEnd();
}

QString QAccessibleTextWidget::text(int startOffset, int endOffset) const
{
   QTextCursor cursor(textCursor());

   cursor.setPosition(startOffset, QTextCursor::MoveAnchor);
   cursor.setPosition(endOffset, QTextCursor::KeepAnchor);

   return cursor.selectedText().replace(QChar(QChar::ParagraphSeparator), QLatin1Char('\n'));
}

QPoint QAccessibleTextWidget::scrollBarPosition() const
{
   return QPoint(0, 0);
}

QString QAccessibleTextWidget::textBeforeOffset(int offset, QAccessible::TextBoundaryType boundaryType,
   int *startOffset, int *endOffset) const
{
   Q_ASSERT(startOffset);
   Q_ASSERT(endOffset);

   QTextCursor cursor = textCursor();
   cursor.setPosition(offset);
   QPair<int, int> boundaries = QAccessible::qAccessibleTextBoundaryHelper(cursor, boundaryType);
   cursor.setPosition(boundaries.first - 1);
   boundaries = QAccessible::qAccessibleTextBoundaryHelper(cursor, boundaryType);

   *startOffset = boundaries.first;
   *endOffset = boundaries.second;

   return text(boundaries.first, boundaries.second);
}

QString QAccessibleTextWidget::textAfterOffset(int offset, QAccessible::TextBoundaryType boundaryType,
   int *startOffset, int *endOffset) const
{
   Q_ASSERT(startOffset);
   Q_ASSERT(endOffset);

   QTextCursor cursor = textCursor();
   cursor.setPosition(offset);
   QPair<int, int> boundaries = QAccessible::qAccessibleTextBoundaryHelper(cursor, boundaryType);
   cursor.setPosition(boundaries.second);
   boundaries = QAccessible::qAccessibleTextBoundaryHelper(cursor, boundaryType);

   *startOffset = boundaries.first;
   *endOffset = boundaries.second;

   return text(boundaries.first, boundaries.second);
}

QString QAccessibleTextWidget::textAtOffset(int offset, QAccessible::TextBoundaryType boundaryType,
   int *startOffset, int *endOffset) const
{
   Q_ASSERT(startOffset);
   Q_ASSERT(endOffset);

   QTextCursor cursor = textCursor();
   cursor.setPosition(offset);
   QPair<int, int> boundaries = QAccessible::qAccessibleTextBoundaryHelper(cursor, boundaryType);

   *startOffset = boundaries.first;
   *endOffset = boundaries.second;

   return text(boundaries.first, boundaries.second);
}

void QAccessibleTextWidget::setCursorPosition(int position)
{
   QTextCursor cursor = textCursor();
   cursor.setPosition(position);
   setTextCursor(cursor);
}

void QAccessibleTextWidget::addSelection(int startOffset, int endOffset)
{
   setSelection(0, startOffset, endOffset);
}

void QAccessibleTextWidget::removeSelection(int selectionIndex)
{
   if (selectionIndex != 0) {
      return;
   }

   QTextCursor cursor = textCursor();
   cursor.clearSelection();
   setTextCursor(cursor);
}

void QAccessibleTextWidget::setSelection(int selectionIndex, int startOffset, int endOffset)
{
   if (selectionIndex != 0) {
      return;
   }

   QTextCursor cursor = textCursor();
   cursor.setPosition(startOffset, QTextCursor::MoveAnchor);
   cursor.setPosition(endOffset, QTextCursor::KeepAnchor);
   setTextCursor(cursor);
}

int QAccessibleTextWidget::characterCount() const
{
   QTextCursor cursor = textCursor();
   cursor.movePosition(QTextCursor::End);
   return cursor.position();
}

QTextCursor QAccessibleTextWidget::textCursorForRange(int startOffset, int endOffset) const
{
   QTextCursor cursor = textCursor();
   cursor.setPosition(startOffset, QTextCursor::MoveAnchor);
   cursor.setPosition(endOffset, QTextCursor::KeepAnchor);

   return cursor;
}

void QAccessibleTextWidget::deleteText(int startOffset, int endOffset)
{
   QTextCursor cursor = textCursorForRange(startOffset, endOffset);
   cursor.removeSelectedText();
}

void QAccessibleTextWidget::insertText(int offset, const QString &text)
{
   QTextCursor cursor = textCursor();
   cursor.setPosition(offset);
   cursor.insertText(text);
}

void QAccessibleTextWidget::replaceText(int startOffset, int endOffset, const QString &text)
{
   QTextCursor cursor = textCursorForRange(startOffset, endOffset);
   cursor.removeSelectedText();
   cursor.insertText(text);
}
#endif

#ifndef QT_NO_MAINWINDOW
QAccessibleMainWindow::QAccessibleMainWindow(QWidget *widget)
   : QAccessibleWidget(widget, QAccessible::Window) { }

QAccessibleInterface *QAccessibleMainWindow::child(int index) const
{
   QList<QWidget *> kids = childWidgets(mainWindow());
   if (index >= 0 && index < kids.count()) {
      return QAccessible::queryAccessibleInterface(kids.at(index));
   }

   return nullptr;
}

int QAccessibleMainWindow::childCount() const
{
   QList<QWidget *> kids = childWidgets(mainWindow());
   return kids.count();
}

int QAccessibleMainWindow::indexOfChild(const QAccessibleInterface *iface) const
{
   QList<QWidget *> kids = childWidgets(mainWindow());
   return kids.indexOf(static_cast<QWidget *>(iface->object()));
}

QAccessibleInterface *QAccessibleMainWindow::childAt(int x, int y) const
{
   QWidget *w = widget();
   if (! w->isVisible()) {
      return nullptr;
   }

   QPoint gp = w->mapToGlobal(QPoint(0, 0));
   if (!QRect(gp.x(), gp.y(), w->width(), w->height()).contains(x, y)) {
      return nullptr;
   }

   QWidgetList kids = childWidgets(mainWindow());
   QPoint rp = mainWindow()->mapFromGlobal(QPoint(x, y));

   for (int i = 0; i < kids.size(); ++i) {
      QWidget *child = kids.at(i);
      if (!child->isWindow() && !child->isHidden() && child->geometry().contains(rp)) {
         return QAccessible::queryAccessibleInterface(child);
      }
   }

   return nullptr;
}

QMainWindow *QAccessibleMainWindow::mainWindow() const
{
   return qobject_cast<QMainWindow *>(object());
}

#endif //QT_NO_MAINWINDOW

#endif // QT_NO_ACCESSIBILITY
