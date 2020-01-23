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

#include <qaccessiblewidget.h>

#ifndef QT_NO_ACCESSIBILITY

#include <qaction.h>
#include <qapplication.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qwidget.h>
#include <qdebug.h>
#include <qmath.h>
#include <QRubberBand>
#include <QFocusFrame>
#include <QMenu>

#include <qwidget_p.h>

static QList<QWidget *> childWidgets(const QWidget *widget)
{
   QList<QObject *> list = widget->children();
   QList<QWidget *> widgets;

   for (int i = 0; i < list.size(); ++i) {
      QWidget *tmpWidget = qobject_cast<QWidget *>(list.at(i));

      if (tmpWidget && ! tmpWidget->isWindow() && ! qobject_cast<QFocusFrame *>(tmpWidget)

#if !defined(QT_NO_MENU)
         && ! qobject_cast<QMenu *>(tmpWidget)
#endif

         && tmpWidget->objectName() != "qt_rubberband" && tmpWidget->objectName() != "qt_spinbox_lineedit") {

         widgets.append(tmpWidget);
      }
   }

   return widgets;
}

static QString buddyString(const QWidget *widget)
{
   if (! widget) {
      return QString();
   }

   QWidget *parent = widget->parentWidget();
   if (!parent) {
      return QString();
   }

#ifndef QT_NO_SHORTCUT
   QObjectList ol = parent->children();

   for (int i = 0; i < ol.size(); ++i) {
      QLabel *label = qobject_cast<QLabel *>(ol.at(i));
      if (label && label->buddy() == widget) {
         return label->text();
      }
   }
#endif

#ifndef QT_NO_GROUPBOX
   QGroupBox *groupbox = qobject_cast<QGroupBox *>(parent);
   if (groupbox) {
      return groupbox->title();
   }
#endif

   return QString();
}

/* This function will return the offset of the '&' in the text that would be
   preceding the accelerator character.
   If this text does not have an accelerator, -1 will be returned. */
static int qt_accAmpIndex(const QString &text)
{
#ifndef QT_NO_SHORTCUT
   if (text.isEmpty()) {
      return -1;
   }

   int fa = 0;

   while ((fa = text.indexOf('&', fa)) != -1) {
      ++fa;

      if (fa < text.length()) {
         // ignore "&&"
         if (text.at(fa) == '&') {
            ++fa;
            continue;
         } else {
            return fa - 1;
            break;
         }
      }
   }
#endif

   return -1;
}

QString qt_accStripAmp(const QString &text)
{
   QString newText(text);
   int ampIndex = qt_accAmpIndex(newText);
   if (ampIndex != -1) {
      newText.remove(ampIndex, 1);
   }

   return newText.replace(QLatin1String("&&"), QLatin1String("&"));
}

QString qt_accHotKey(const QString &text)
{
   int ampIndex = qt_accAmpIndex(text);

   if (ampIndex != -1) {
      return QKeySequence(Qt::ALT).toString(QKeySequence::NativeText) + text.at(ampIndex + 1);
   }

   return QString();
}

class QAccessibleWidgetPrivate
{
 public:
   QAccessibleWidgetPrivate()
      : role(QAccessible::Client)
   {}

   QAccessible::Role role;
   QString name;
   QStringList primarySignals;
};

QAccessibleWidget::QAccessibleWidget(QWidget *w, QAccessible::Role role, const QString &name)
   : QAccessibleObject(w)
{
   Q_ASSERT(widget());
   d = new QAccessibleWidgetPrivate();
   d->role = role;
   d->name = name;
}

bool QAccessibleWidget::isValid() const
{
   if (!object() || static_cast<QWidget *>(object())->d_func()->data.in_destructor) {
      return false;
   }
   return QAccessibleObject::isValid();
}
QWindow *QAccessibleWidget::window() const
{
   const QWidget *w = widget();
   Q_ASSERT(w);
   QWindow *result = w->windowHandle();
   if (!result) {
      if (const QWidget *nativeParent = w->nativeParentWidget()) {
         result = nativeParent->windowHandle();
      }
   }
   return result;
}


QAccessibleWidget::~QAccessibleWidget()
{
   delete d;
}


QWidget *QAccessibleWidget::widget() const
{
   return qobject_cast<QWidget *>(object());
}


QObject *QAccessibleWidget::parentObject() const
{
   QWidget *w = widget();
   if (!w || w->isWindow() || !w->parentWidget()) {
      return qApp;
   }

   return w->parent();
}


QRect QAccessibleWidget::rect() const
{
   QWidget *w = widget();
   if (!w->isVisible()) {
      return QRect();
   }
   QPoint wpos = w->mapToGlobal(QPoint(0, 0));

   return QRect(wpos.x(), wpos.y(), w->width(), w->height());
}


class QACConnectionObject : public QObject
{
 public:
   bool isSender(const QObject *receiver, const QString &signal) const {
      return CSInternalSender::isSender(this, receiver, signal);
   }

   QObjectList receiverList(const QString &signal) const {
      return CSInternalSender::receiverList(this, signal);
   }

   QObjectList senderList() const {
      return CSInternalSender::senderList(this);
   }
};

void QAccessibleWidget::addControllingSignal(const QString &signal)
{
   QString s = QMetaObject::normalizedSignature(signal);

   if (object()->metaObject()->indexOfSignal(s) < 0) {
      qWarning("QAccessibleWidget::addControllingSignal(): Signal %s unknown in %s", csPrintable(s),
                  csPrintable(object()->metaObject()->className()));
   }

   d->primarySignals << s;
}

static inline bool isAncestor(const QObject *obj, const QObject *child)
{
   while (child) {
      if (child == obj) {
         return true;
      }

      child = child->parent();
   }

   return false;
}

QVector<QPair<QAccessibleInterface *, QAccessible::Relation>> QAccessibleWidget::relations(QAccessible::Relation
      match /*= QAccessible::AllRelations*/) const
{
   QVector<QPair<QAccessibleInterface *, QAccessible::Relation>> rels;
   if (match & QAccessible::Label) {
      const QAccessible::Relation rel = QAccessible::Label;
      if (QWidget *parent = widget()->parentWidget()) {
#ifndef QT_NO_SHORTCUT
         // first check for all siblings that are labels to us
         // ideally we would go through all objects and check, but that
         // will be too expensive
         const QList<QWidget *> kids = childWidgets(parent);
         for (int i = 0; i < kids.count(); ++i) {
            if (QLabel *labelSibling = qobject_cast<QLabel *>(kids.at(i))) {
               if (labelSibling->buddy() == widget()) {
                  QAccessibleInterface *iface = QAccessible::queryAccessibleInterface(labelSibling);
                  rels.append(qMakePair(iface, rel));
               }
            }
         }
#endif
#ifndef QT_NO_GROUPBOX
         QGroupBox *groupbox = qobject_cast<QGroupBox *>(parent);
         if (groupbox && !groupbox->title().isEmpty()) {
            QAccessibleInterface *iface = QAccessible::queryAccessibleInterface(groupbox);
            rels.append(qMakePair(iface, rel));
         }
#endif
      }
   }

   if (match & QAccessible::Controlled) {
      QObjectList allReceivers;
      QACConnectionObject *connectionObject = (QACConnectionObject *)object();
      for (int sig = 0; sig < d->primarySignals.count(); ++sig) {
         const QObjectList receivers = connectionObject->receiverList(d->primarySignals.at(sig).toLatin1());
         allReceivers += receivers;
      }

      allReceivers.removeAll(object());  //### The object might connect to itself internally

      for (int i = 0; i < allReceivers.count(); ++i) {
         const QAccessible::Relation rel = QAccessible::Controlled;
         QAccessibleInterface *iface = QAccessible::queryAccessibleInterface(allReceivers.at(i));
         if (iface) {
            rels.append(qMakePair(iface, rel));
         }
      }
   }

   return rels;
}

QAccessibleInterface *QAccessibleWidget::parent() const
{
   return QAccessible::queryAccessibleInterface(parentObject());
}

/*! \reimp */
QAccessibleInterface *QAccessibleWidget::child(int index) const
{
   Q_ASSERT(widget());
   QWidgetList childList = childWidgets(widget());
   if (index >= 0 && index < childList.size()) {
      return QAccessible::queryAccessibleInterface(childList.at(index));
   }
   return 0;
}

/*! \reimp */
QAccessibleInterface *QAccessibleWidget::focusChild() const
{
   if (widget()->hasFocus()) {
      return QAccessible::queryAccessibleInterface(object());
   }

   QWidget *fw = widget()->focusWidget();
   if (!fw) {
      return 0;
   }

   if (isAncestor(widget(), fw) || fw == widget()) {
      return QAccessible::queryAccessibleInterface(fw);
   }
   return 0;
}

int QAccessibleWidget::childCount() const
{
   QWidgetList cl = childWidgets(widget());
   return cl.size();
}

int QAccessibleWidget::indexOfChild(const QAccessibleInterface *child) const
{
   if (!child) {
      return -1;
   }
   QWidgetList cl = childWidgets(widget());

   return cl.indexOf(qobject_cast<QWidget *>(child->object()));
}

// from qwidget.cpp
extern QString qt_setWindowTitle_helperHelper(const QString &, const QWidget *);

QString QAccessibleWidget::text(QAccessible::Text t) const
{
   QString str;

   switch (t) {
      case QAccessible::Name:
         if (!d->name.isEmpty()) {
            str = d->name;
         } else if (!widget()->accessibleName().isEmpty()) {
            str = widget()->accessibleName();
         } else if (widget()->isWindow()) {
            if (widget()->isMinimized()) {
               str = qt_setWindowTitle_helperHelper(widget()->windowIconText(), widget());
            } else {
               str = qt_setWindowTitle_helperHelper(widget()->windowTitle(), widget());
            }
         } else {
            str = qt_accStripAmp(buddyString(widget()));
         }
         break;
      case QAccessible::Description:
         str = widget()->accessibleDescription();
#ifndef QT_NO_TOOLTIP
         if (str.isEmpty()) {
            str = widget()->toolTip();
         }
#endif
         break;
      case QAccessible::Help:
#ifndef QT_NO_WHATSTHIS
         str = widget()->whatsThis();
#endif
         break;
      case QAccessible::Accelerator:
         str = qt_accHotKey(buddyString(widget()));
         break;
      case QAccessible::Value:
         break;
      default:
         break;
   }
   return str;
}

QStringList QAccessibleWidget::actionNames() const
{
   QStringList names;
   if (widget()->isEnabled()) {
      if (widget()->focusPolicy() != Qt::NoFocus) {
         names << setFocusAction();
      }
   }
   return names;
}

void QAccessibleWidget::doAction(const QString &actionName)
{
   if (!widget()->isEnabled()) {
      return;
   }

   if (actionName == setFocusAction()) {
      if (widget()->isWindow()) {
         widget()->activateWindow();
      }
      widget()->setFocus();
   }
}
QStringList QAccessibleWidget::keyBindingsForAction(const QString & /* actionName */) const
{
   return QStringList();
}

QAccessible::Role QAccessibleWidget::role() const
{
   return d->role;
}

QAccessible::State QAccessibleWidget::state() const
{
   QAccessible::State state;

   QWidget *w = widget();

   if (w->testAttribute(Qt::WA_WState_Visible) == false) {
      state.invisible = true;
   }
   if (w->focusPolicy() != Qt::NoFocus) {
      state.focusable = true;
   }
   if (w->hasFocus()) {
      state.focused = true;
   }
   if (!w->isEnabled()) {
      state.disabled = true;
   }
   if (w->isWindow()) {
      if (w->windowFlags() & Qt::WindowSystemMenuHint) {
         state.movable = true;
      }
      if (w->minimumSize() != w->maximumSize()) {
         state.sizeable = true;
      }
      if (w->isActiveWindow()) {
         state.active = true;
      }
   }

   return state;
}

QColor QAccessibleWidget::foregroundColor() const
{
   return widget()->palette().color(widget()->foregroundRole());
}
QColor QAccessibleWidget::backgroundColor() const
{

   return widget()->palette().color(widget()->backgroundRole());
}
void *QAccessibleWidget::interface_cast(QAccessible::InterfaceType t)
{

   if (t == QAccessible::ActionInterface) {
      return static_cast<QAccessibleActionInterface *>(this);
   }

   return 0;
}

#endif //QT_NO_ACCESSIBILITY
