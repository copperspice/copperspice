/***********************************************************************
*
* Copyright (c) 2012-2020 Barbara Geller
* Copyright (c) 2012-2020 Ansel Sermersheim
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

#include <qaccessiblewidget_complex_p.h>

#include <qaccessible.h>
#include <qapplication.h>
#include <qabstractbutton.h>
#include <qevent.h>
#include <qheaderview.h>
#include <qtabbar.h>
#include <qcombobox.h>
#include <qlistview.h>
#include <qtableview.h>
#include <qlineedit.h>
#include <qstyle.h>
#include <qstyleoption.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qtreeview.h>
#include <qtabbar_p.h>
#include <QAbstractScrollArea>
#include <QScrollArea>
#include <QScrollBar>
#include <QDebug>

#ifndef QT_NO_ACCESSIBILITY

QT_BEGIN_NAMESPACE

QString qt_accStripAmp(const QString &text);
QString qt_accHotKey(const QString &text);

#ifndef QT_NO_TABBAR
/*!
  \class QAccessibleTabBar
  \brief The QAccessibleTabBar class implements the QAccessibleInterface for tab bars.
  \internal

  \ingroup accessibility
*/

class QAccessibleTabButton: public QAccessibleInterface, public QAccessibleActionInterface
{
 public:
   QAccessibleTabButton(QTabBar *parent, int index)
      : m_parent(parent), m_index(index)
   {}

   void *interface_cast(QAccessible::InterfaceType t) override {
      if (t == QAccessible::ActionInterface) {
         return static_cast<QAccessibleActionInterface *>(this);
      }
      return 0;
   }

   QObject *object() const override {
      return 0;
   }
   QAccessible::Role role() const override {
      return QAccessible::PageTab;
   }
   QAccessible::State state() const override {
      if (!isValid()) {
         QAccessible::State s;
         s.invalid = true;
         return s;
      }
      return parent()->state();
   }
   QRect rect() const override {
      if (!isValid()) {
         return QRect();
      }

      QPoint tp = m_parent->mapToGlobal(QPoint(0, 0));
      QRect rec = m_parent->tabRect(m_index);
      rec = QRect(tp.x() + rec.x(), tp.y() + rec.y(), rec.width(), rec.height());
      return rec;
   }

   bool isValid() const override {
      return m_parent.data() && m_parent->count() > m_index;
   }

   QAccessibleInterface *childAt(int, int) const override {
      return 0;
   }
   int childCount() const override {
      return 0;
   }
   int indexOfChild(const QAccessibleInterface *) const override  {
      return -1;
   }

   QString text(QAccessible::Text t) const override {
      if (!isValid()) {
         return QString();
      }
      switch (t) {
         case QAccessible::Name:
            return qt_accStripAmp(m_parent->tabText(m_index));
         case QAccessible::Accelerator:
            return qt_accHotKey(m_parent->tabText(m_index));
         case QAccessible::Description:
            return m_parent->tabToolTip(m_index);
         case QAccessible::Help:
            return m_parent->tabWhatsThis(m_index);
         default:
            break;
      }
      return QString();
   }

   void setText(QAccessible::Text, const QString &) override {}

   QAccessibleInterface *parent() const override {
      return QAccessible::queryAccessibleInterface(m_parent.data());
   }
   QAccessibleInterface *child(int) const override {
      return 0;
   }

   // action interface
   QStringList actionNames() const override {
      return QStringList(pressAction());
   }

   void doAction(const QString &actionName) override {
      if (isValid() && actionName == pressAction()) {
         m_parent->setCurrentIndex(m_index);
      }
   }

   QStringList keyBindingsForAction(const QString &) const override {
      return QStringList();
   }

   int index() const {
      return m_index;
   }

 private:
   QPointer<QTabBar> m_parent;
   int m_index;

};

/*!
  Constructs a QAccessibleTabBar object for \a w.
*/
QAccessibleTabBar::QAccessibleTabBar(QWidget *w)
   : QAccessibleWidget(w, QAccessible::PageTabList)
{
   Q_ASSERT(tabBar());
}

QAccessibleTabBar::~QAccessibleTabBar()
{
   for (QAccessible::Id id : m_childInterfaces) {
      QAccessible::deleteAccessibleInterface(id);
   }
}

/*! Returns the QTabBar. */
QTabBar *QAccessibleTabBar::tabBar() const
{
   return qobject_cast<QTabBar *>(object());
}

QAccessibleInterface *QAccessibleTabBar::child(int index) const
{
   if (QAccessible::Id id = m_childInterfaces.value(index)) {
      return QAccessible::accessibleInterface(id);
   }

   // first the tabs, then 2 buttons
   if (index < tabBar()->count()) {
      QAccessibleTabButton *button = new QAccessibleTabButton(tabBar(), index);
      QAccessible::registerAccessibleInterface(button);
      m_childInterfaces.insert(index, QAccessible::uniqueId(button));
      return button;
   } else if (index >= tabBar()->count()) {
      // left button
      if (index - tabBar()->count() == 0) {
         return QAccessible::queryAccessibleInterface(tabBar()->d_func()->leftB);
      }
      // right button
      if (index - tabBar()->count() == 1) {
         return QAccessible::queryAccessibleInterface(tabBar()->d_func()->rightB);
      }
   }
   return 0;
}

int QAccessibleTabBar::indexOfChild(const QAccessibleInterface *child) const
{
   if (child->object() && child->object() == tabBar()->d_func()->leftB) {
      return tabBar()->count();
   }
   if (child->object() && child->object() == tabBar()->d_func()->rightB) {
      return tabBar()->count() + 1;
   }
   if (child->role() == QAccessible::PageTab) {
      QAccessibleInterface *parent = child->parent();
      if (parent == this) {
         const QAccessibleTabButton *tabButton = static_cast<const QAccessibleTabButton *>(child);
         return tabButton->index();
      }
   }
   return -1;
}

int QAccessibleTabBar::childCount() const
{
   // tabs + scroll buttons
   return tabBar()->count() + 2;
}

QString QAccessibleTabBar::text(QAccessible::Text t) const
{
   if (t == QAccessible::Name) {
      return qt_accStripAmp(tabBar()->tabText(tabBar()->currentIndex()));
   } else if (t == QAccessible::Accelerator) {
      return qt_accHotKey(tabBar()->tabText(tabBar()->currentIndex()));
   }
   return QString();
}

#endif // QT_NO_TABBAR

#ifndef QT_NO_COMBOBOX
/*!
  \class QAccessibleComboBox
  \brief The QAccessibleComboBox class implements the QAccessibleInterface for editable and read-only combo boxes.
  \internal

  \ingroup accessibility
*/

/*!
  Constructs a QAccessibleComboBox object for \a w.
*/
QAccessibleComboBox::QAccessibleComboBox(QWidget *w)
   : QAccessibleWidget(w, QAccessible::ComboBox)
{
   Q_ASSERT(comboBox());
}

/*!
  Returns the combobox.
*/
QComboBox *QAccessibleComboBox::comboBox() const
{
   return qobject_cast<QComboBox *>(object());
}

QAccessibleInterface *QAccessibleComboBox::child(int index) const
{
   if (index == 0) {
      QAbstractItemView *view = comboBox()->view();
      //QWidget *parent = view ? view->parentWidget() : 0;
      return QAccessible::queryAccessibleInterface(view);
   } else if (index == 1 && comboBox()->isEditable()) {
      return QAccessible::queryAccessibleInterface(comboBox()->lineEdit());
   }
   return 0;
}

int QAccessibleComboBox::childCount() const
{
   // list and text edit
   return comboBox()->isEditable() ? 2 : 1;
}

QAccessibleInterface *QAccessibleComboBox::childAt(int x, int y) const
{
   if (comboBox()->isEditable() && comboBox()->lineEdit()->rect().contains(x, y)) {
      return child(1);
   }
   return 0;
}

int QAccessibleComboBox::indexOfChild(const QAccessibleInterface *child) const
{
   if (comboBox()->view() == child->object()) {
      return 0;
   }
   if (comboBox()->isEditable() && comboBox()->lineEdit() == child->object()) {
      return 1;
   }
   return -1;
}

/*! \reimp */
QString QAccessibleComboBox::text(QAccessible::Text t) const
{
   QString str;

   switch (t) {
      case QAccessible::Name:
#ifndef Q_OS_UNIX // on Linux we use relations for this, name is text (fall through to Value)
         str = QAccessibleWidget::text(t);
         break;
#endif
      case QAccessible::Value:
         if (comboBox()->isEditable()) {
            str = comboBox()->lineEdit()->text();
         } else {
            str = comboBox()->currentText();
         }
         break;
#ifndef QT_NO_SHORTCUT
      case QAccessible::Accelerator:
         str = QKeySequence(Qt::Key_Down).toString(QKeySequence::NativeText);
         break;
#endif
      default:
         break;
   }
   if (str.isEmpty()) {
      str = QAccessibleWidget::text(t);
   }
   return str;
}

QStringList QAccessibleComboBox::actionNames() const
{
   return QStringList() << showMenuAction() << pressAction();
}

QString QAccessibleComboBox::localizedActionDescription(const QString &actionName) const
{
   if (actionName == showMenuAction() || actionName == pressAction()) {
      return QComboBox::tr("Open the combo box selection popup");
   }
   return QString();
}

void QAccessibleComboBox::doAction(const QString &actionName)
{
   if (actionName == showMenuAction() || actionName == pressAction()) {
      if (comboBox()->view()->isVisible()) {
         comboBox()->hidePopup();
      } else {
         comboBox()->showPopup();
      }
   }
}

QStringList QAccessibleComboBox::keyBindingsForAction(const QString &/*actionName*/) const
{
   return QStringList();
}

#endif // QT_NO_COMBOBOX

#ifndef QT_NO_SCROLLAREA
// ======================= QAccessibleAbstractScrollArea =======================
QAccessibleAbstractScrollArea::QAccessibleAbstractScrollArea(QWidget *widget)
   : QAccessibleWidget(widget, QAccessible::Client)
{
   Q_ASSERT(qobject_cast<QAbstractScrollArea *>(widget));
}

QAccessibleInterface *QAccessibleAbstractScrollArea::child(int index) const
{
   return QAccessible::queryAccessibleInterface(accessibleChildren().at(index));
}

int QAccessibleAbstractScrollArea::childCount() const
{
   return accessibleChildren().count();
}

int QAccessibleAbstractScrollArea::indexOfChild(const QAccessibleInterface *child) const
{
   if (!child || !child->object()) {
      return -1;
   }
   return accessibleChildren().indexOf(qobject_cast<QWidget *>(child->object()));
}

bool QAccessibleAbstractScrollArea::isValid() const
{
   return (QAccessibleWidget::isValid() && abstractScrollArea() && abstractScrollArea()->viewport());
}

QAccessibleInterface *QAccessibleAbstractScrollArea::childAt(int x, int y) const
{
   if (!abstractScrollArea()->isVisible()) {
      return 0;
   }

   for (int i = 0; i < childCount(); ++i) {
      QPoint wpos = accessibleChildren().at(i)->mapToGlobal(QPoint(0, 0));
      QRect rect = QRect(wpos, accessibleChildren().at(i)->size());
      if (rect.contains(x, y)) {
         return child(i);
      }
   }
   return 0;
}

QAbstractScrollArea *QAccessibleAbstractScrollArea::abstractScrollArea() const
{
   return static_cast<QAbstractScrollArea *>(object());
}

QWidgetList QAccessibleAbstractScrollArea::accessibleChildren() const
{
   QWidgetList children;

   // Viewport.
   QWidget *viewport = abstractScrollArea()->viewport();
   if (viewport) {
      children.append(viewport);
   }

   // Horizontal scrollBar container.
   QScrollBar *horizontalScrollBar = abstractScrollArea()->horizontalScrollBar();
   if (horizontalScrollBar && horizontalScrollBar->isVisible()) {
      children.append(horizontalScrollBar->parentWidget());
   }

   // Vertical scrollBar container.
   QScrollBar *verticalScrollBar = abstractScrollArea()->verticalScrollBar();
   if (verticalScrollBar && verticalScrollBar->isVisible()) {
      children.append(verticalScrollBar->parentWidget());
   }

   // CornerWidget.
   QWidget *cornerWidget = abstractScrollArea()->cornerWidget();
   if (cornerWidget && cornerWidget->isVisible()) {
      children.append(cornerWidget);
   }

   return children;
}

QAccessibleAbstractScrollArea::AbstractScrollAreaElement QAccessibleAbstractScrollArea::elementType(QWidget *widget) const
{
   if (!widget) {
      return Undefined;
   }

   if (widget == abstractScrollArea()) {
      return Self;
   }
   if (widget == abstractScrollArea()->viewport()) {
      return Viewport;
   }
   if (widget->objectName() == QLatin1String("qt_scrollarea_hcontainer")) {
      return HorizontalContainer;
   }
   if (widget->objectName() == QLatin1String("qt_scrollarea_vcontainer")) {
      return VerticalContainer;
   }
   if (widget == abstractScrollArea()->cornerWidget()) {
      return CornerWidget;
   }

   return Undefined;
}

bool QAccessibleAbstractScrollArea::isLeftToRight() const
{
   return abstractScrollArea()->isLeftToRight();
}

// ======================= QAccessibleScrollArea ===========================
QAccessibleScrollArea::QAccessibleScrollArea(QWidget *widget)
   : QAccessibleAbstractScrollArea(widget)
{
   Q_ASSERT(qobject_cast<QScrollArea *>(widget));
}
#endif // QT_NO_SCROLLAREA

QT_END_NAMESPACE

#endif // QT_NO_ACCESSIBILITY
