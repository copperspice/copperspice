/***********************************************************************
*
* Copyright (c) 2012-2019 Barbara Geller
* Copyright (c) 2012-2019 Ansel Sermersheim
*
* Copyright (C) 2015 The Qt Company Ltd.
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

QT_BEGIN_NAMESPACE

static QList<QWidget *> childWidgets(const QWidget *widget)
{
   QList<QObject *> list = widget->children();
   QList<QWidget *> widgets;
   for (int i = 0; i < list.size(); ++i) {
      QWidget *w = qobject_cast<QWidget *>(list.at(i));
      if (w && !w->isWindow()
            && !qobject_cast<QFocusFrame *>(w)
#if !defined(QT_NO_MENU)
            && !qobject_cast<QMenu *>(w)
#endif
            && w->objectName() != QLatin1String("qt_rubberband")) {
         widgets.append(w);
      }
   }
   return widgets;
}

static QString buddyString(const QWidget *widget)
{
   if (!widget) {
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
   QChar ac;

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

   return -1;
#else
   Q_UNUSED(text);
   return -1;
#endif
}

QString Q_GUI_EXPORT qt_accStripAmp(const QString &text)
{
   QString newText(text);
   int ampIndex = qt_accAmpIndex(newText);
   if (ampIndex != -1) {
      newText.remove(ampIndex, 1);
   }

   return newText.replace(QLatin1String("&&"), QLatin1String("&"));
}

QString Q_GUI_EXPORT qt_accHotKey(const QString &text)
{
   int ampIndex = qt_accAmpIndex(text);

   if (ampIndex != -1) {
      return QKeySequence(Qt::ALT).toString() + text.at(ampIndex + 1);
   }

   return QString();
}

class QAccessibleWidgetPrivate : public QAccessible
{
 public:
   QAccessibleWidgetPrivate()
      : role(Client) {
   }

   Role role;
   QString name;
   QString description;
   QString value;
   QString help;
   QString accelerator;
   QStringList primarySignals;
   const QAccessibleInterface *asking;
};

QAccessibleWidget::QAccessibleWidget(QWidget *w, Role role, const QString &name)
   : QAccessibleObject(w)
{
   Q_ASSERT(widget());
   d = new QAccessibleWidgetPrivate();
   d->role = role;
   d->name = name;
   d->asking = 0;
}

/*!
    Destroys this object.
*/
QAccessibleWidget::~QAccessibleWidget()
{
   delete d;
}

/*!
    Returns the associated widget.
*/
QWidget *QAccessibleWidget::widget() const
{
   return qobject_cast<QWidget *>(object());
}

/*!
    Returns the associated widget's parent object, which is either the
    parent widget, or qApp for top-level widgets.
*/
QObject *QAccessibleWidget::parentObject() const
{
   QObject *parent = object()->parent();
   if (!parent) {
      parent = qApp;
   }
   return parent;
}

int QAccessibleWidget::childAt(int x, int y) const
{
   QWidget *w = widget();
   if (!w->isVisible()) {
      return -1;
   }
   QPoint gp = w->mapToGlobal(QPoint(0, 0));
   if (!QRect(gp.x(), gp.y(), w->width(), w->height()).contains(x, y)) {
      return -1;
   }

   QWidgetList list = childWidgets(w);
   int ccount = childCount();

   // a complex child
   if (list.size() < ccount) {
      for (int i = 1; i <= ccount; ++i) {
         if (rect(i).contains(x, y)) {
            return i;
         }
      }
      return 0;
   }

   QPoint rp = w->mapFromGlobal(QPoint(x, y));
   for (int i = 0; i < list.size(); ++i) {
      QWidget *child = list.at(i);
      if (!child->isWindow() && !child->isHidden() && child->geometry().contains(rp)) {
         return i + 1;
      }
   }
   return 0;
}

QRect QAccessibleWidget::rect(int child) const
{
   if (child) {
      qWarning("QAccessibleWidget::rect: This implementation does not support subelements! "
               "(ID %d unknown for %s)", child, csPrintable(widget()->metaObject()->className()));
   }

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
      qWarning("Signal %s unknown in %s", csPrintable(s), csPrintable(object()->metaObject()->className()));
   }

   d->primarySignals << s;
}

void QAccessibleWidget::setValue(const QString &value)
{
   d->value = value;
}

void QAccessibleWidget::setDescription(const QString &desc)
{
   d->description = desc;
}

void QAccessibleWidget::setHelp(const QString &help)
{
   d->help = help;
}

void QAccessibleWidget::setAccelerator(const QString &accel)
{
   d->accelerator = accel;
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

QAccessible::Relation QAccessibleWidget::relationTo(int child,
                  const QAccessibleInterface *other, int otherChild) const
{
   Relation relation = Unrelated;

   if (d->asking == this) {
      // recursive call
      return relation;
   }

   QObject *obj = other ? other->object() : nullptr;
   if (! obj) {
      return relation;
   }

   QWidget *focus = widget()->focusWidget();
   if (object() == focus && isAncestor(obj, focus)) {
      relation |= FocusChild;
   }

   QACConnectionObject *connectionObject = (QACConnectionObject *)object();

   for (int sig = 0; sig < d->primarySignals.count(); ++sig) {
      if (connectionObject->isSender(obj, d->primarySignals.at(sig))) {
         relation |= Controller;
         break;
      }
   }

   // test for passive relationships
   // d->asking protects from endless recursion

   d->asking = this;
   int inverse = other->relationTo(otherChild, this, child);
   d->asking = nullptr;

   if (inverse & Controller) {
      relation |= Controlled;
   }
   if (inverse & Label) {
      relation |= Labelled;
   }

   if (obj == object()) {
      if (child && ! otherChild) {
         return relation | Child;
      }

      if (! child && otherChild) {
         return relation | Ancestor;
      }

      if (! child && ! otherChild) {
         return relation | Self;
      }
   }

   QObject *parent = object()->parent();
   if (obj == parent) {
      return relation | Child;
   }

   if (obj->parent() == parent) {
      relation |= Sibling;
      QAccessibleInterface *sibIface = QAccessible::queryAccessibleInterface(obj);
      Q_ASSERT(sibIface);

      QRect wg = rect(0);
      QRect sg = sibIface->rect(0);

      if (wg.intersects(sg)) {
         QAccessibleInterface *pIface = nullptr;
         sibIface->navigate(Ancestor, 1, &pIface);

         if (pIface && !((sibIface->state(0) | state(0)) & Invisible)) {
            int wi = pIface->indexOfChild(this);
            int si = pIface->indexOfChild(sibIface);

            if (wi > si) {
               relation |= QAccessible::Covers;

            } else {
               relation |= QAccessible::Covered;
            }
         }
         delete pIface;

      } else {
         QPoint wc = wg.center();
         QPoint sc = sg.center();

         if (wc.x() < sc.x()) {
            relation |= QAccessible::Left;

         } else if (wc.x() > sc.x()) {
            relation |= QAccessible::Right;
         }

         if (wc.y() < sc.y()) {
            relation |= QAccessible::Up;

         } else if (wc.y() > sc.y()) {
            relation |= QAccessible::Down;
         }
      }

      delete sibIface;

      return relation;
   }

   if (isAncestor(obj, object())) {
      return relation | Descendent;
   }

   if (isAncestor(object(), obj)) {
      return relation | Ancestor;
   }

   return relation;
}

int QAccessibleWidget::navigate(RelationFlag relation, int entry, QAccessibleInterface **target) const
{
   if (!target) {
      return -1;
   }

   *target = 0;
   QObject *targetObject = 0;

   QWidgetList childList = childWidgets(widget());
   bool complexWidget = childList.size() < childCount();

   switch (relation) {
      // Hierarchical
      case Self:
         targetObject = object();
         break;
      case Child:
         if (complexWidget) {
            if (entry > 0 && entry <= childCount()) {
               return entry;
            }
            return -1;
         } else {
            if (entry > 0 && childList.size() >= entry) {
               targetObject = childList.at(entry - 1);
            }
         }
         break;
      case Ancestor: {
         if (entry <= 0) {
            return -1;
         }
         targetObject = widget()->parentWidget();
         int i;
         for (i = entry; i > 1 && targetObject; --i) {
            targetObject = targetObject->parent();
         }
         if (!targetObject && i == 1) {
            targetObject = qApp;
         }
      }
      break;
      case Sibling: {
         QAccessibleInterface *iface = QAccessible::queryAccessibleInterface(parentObject());
         if (!iface) {
            return -1;
         }

         iface->navigate(Child, entry, target);
         delete iface;
         if (*target) {
            return 0;
         }
      }
      break;

      // Geometrical
      case QAccessible::Left:
         if (complexWidget && entry) {
            if (entry < 2 || widget()->height() > widget()->width() + 20) { // looks vertical
               return -1;
            }
            return entry - 1;
         }
      // fall through
      case QAccessible::Right:
         if (complexWidget && entry) {
            if (entry >= childCount() || widget()->height() > widget()->width() + 20) { // looks vertical
               return -1;
            }
            return entry + 1;
         }
      // fall through
      case QAccessible::Up:
         if (complexWidget && entry) {
            if (entry < 2 || widget()->width() > widget()->height() + 20) { // looks horizontal
               return - 1;
            }
            return entry - 1;
         }
      // fall through
      case QAccessible::Down:
         if (complexWidget && entry) {
            if (entry >= childCount() || widget()->width() > widget()->height()  + 20) { // looks horizontal
               return - 1;
            }
            return entry + 1;
         } else {
            QAccessibleInterface *pIface = QAccessible::queryAccessibleInterface(parentObject());
            if (!pIface) {
               return -1;
            }

            QRect startg = rect(0);
            QPoint startc = startg.center();
            QAccessibleInterface *candidate = 0;
            int mindist = 100000;
            int sibCount = pIface->childCount();
            for (int i = 0; i < sibCount; ++i) {
               QAccessibleInterface *sibling = 0;
               pIface->navigate(Child, i + 1, &sibling);
               Q_ASSERT(sibling);
               if ((relationTo(0, sibling, 0) & Self) || (sibling->state(0) & QAccessible::Invisible)) {
                  //ignore ourself and invisible siblings
                  delete sibling;
                  continue;
               }

               QRect sibg = sibling->rect(0);
               QPoint sibc = sibg.center();
               QPoint sibp;
               QPoint startp;
               QPoint distp;
               switch (relation) {
                  case QAccessible::Left:
                     startp = QPoint(startg.left(), startg.top() + startg.height() / 2);
                     sibp = QPoint(sibg.right(), sibg.top() + sibg.height() / 2);
                     if (QPoint(sibc - startc).x() >= 0) {
                        delete sibling;
                        continue;
                     }
                     distp = sibp - startp;
                     break;
                  case QAccessible::Right:
                     startp = QPoint(startg.right(), startg.top() + startg.height() / 2);
                     sibp = QPoint(sibg.left(), sibg.top() + sibg.height() / 2);
                     if (QPoint(sibc - startc).x() <= 0) {
                        delete sibling;
                        continue;
                     }
                     distp = sibp - startp;
                     break;
                  case QAccessible::Up:
                     startp = QPoint(startg.left() + startg.width() / 2, startg.top());
                     sibp = QPoint(sibg.left() + sibg.width() / 2, sibg.bottom());
                     if (QPoint(sibc - startc).y() >= 0) {
                        delete sibling;
                        continue;
                     }
                     distp = sibp - startp;
                     break;
                  case QAccessible::Down:
                     startp = QPoint(startg.left() + startg.width() / 2, startg.bottom());
                     sibp = QPoint(sibg.left() + sibg.width() / 2, sibg.top());
                     if (QPoint(sibc - startc).y() <= 0) {
                        delete sibling;
                        continue;
                     }
                     distp = sibp - startp;
                     break;
                  default:
                     break;
               }

               int dist = (int)qSqrt((qreal)distp.x() * distp.x() + distp.y() * distp.y());
               if (dist < mindist) {
                  delete candidate;
                  candidate = sibling;
                  mindist = dist;
               } else {
                  delete sibling;
               }
            }
            delete pIface;
            *target = candidate;
            if (*target) {
               return 0;
            }
         }
         break;
      case Covers:
         if (entry > 0) {
            QAccessibleInterface *pIface = QAccessible::queryAccessibleInterface(parentObject());
            if (!pIface) {
               return -1;
            }

            QRect r = rect(0);
            int sibCount = pIface->childCount();
            QAccessibleInterface *sibling = 0;
            for (int i = pIface->indexOfChild(this) + 1; i <= sibCount && entry; ++i) {
               pIface->navigate(Child, i, &sibling);
               if (!sibling || (sibling->state(0) & Invisible)) {
                  delete sibling;
                  sibling = 0;
                  continue;
               }
               if (sibling->rect(0).intersects(r)) {
                  --entry;
               }
               if (!entry) {
                  break;
               }
               delete sibling;
               sibling = 0;
            }
            delete pIface;
            *target = sibling;
            if (*target) {
               return 0;
            }
         }
         break;
      case Covered:
         if (entry > 0) {
            QAccessibleInterface *pIface = QAccessible::queryAccessibleInterface(parentObject());
            if (!pIface) {
               return -1;
            }

            QRect r = rect(0);
            int index = pIface->indexOfChild(this);
            QAccessibleInterface *sibling = 0;
            for (int i = 1; i < index && entry; ++i) {
               pIface->navigate(Child, i, &sibling);
               Q_ASSERT(sibling);
               if (!sibling || (sibling->state(0) & Invisible)) {
                  delete sibling;
                  sibling = 0;
                  continue;
               }
               if (sibling->rect(0).intersects(r)) {
                  --entry;
               }
               if (!entry) {
                  break;
               }
               delete sibling;
               sibling = 0;
            }
            delete pIface;
            *target = sibling;
            if (*target) {
               return 0;
            }
         }
         break;

      // Logical
      case FocusChild: {
         if (widget()->hasFocus()) {
            targetObject = object();
            break;
         }

         QWidget *fw = widget()->focusWidget();
         if (!fw) {
            return -1;
         }

         if (isAncestor(widget(), fw) || fw == widget()) {
            targetObject = fw;
         }
         /* ###
         QWidget *parent = fw;
         while (parent && !targetObject) {
             parent = parent->parentWidget();
             if (parent == widget())
                 targetObject = fw;
         }
         */
      }
      break;
      case Label:
         if (entry > 0) {
            QAccessibleInterface *pIface = QAccessible::queryAccessibleInterface(parentObject());
            if (!pIface) {
               return -1;
            }

            // first check for all siblings that are labels to us
            // ideally we would go through all objects and check, but that
            // will be too expensive
            int sibCount = pIface->childCount();
            QAccessibleInterface *candidate = 0;
            for (int i = 0; i < sibCount && entry; ++i) {
               const int childId = pIface->navigate(Child, i + 1, &candidate);
               Q_ASSERT(childId >= 0);
               if (childId > 0) {
                  candidate = pIface;
               }
               if (candidate->relationTo(childId, this, 0) & Label) {
                  --entry;
               }
               if (!entry) {
                  break;
               }
               if (candidate != pIface) {
                  delete candidate;
               }
               candidate = 0;
            }
            if (!candidate) {
               if (pIface->relationTo(0, this, 0) & Label) {
                  --entry;
               }
               if (!entry) {
                  candidate = pIface;
               }
            }
            if (pIface != candidate) {
               delete pIface;
            }

            *target = candidate;
            if (*target) {
               return 0;
            }
         }
         break;

      case Labelled: // only implemented in subclasses
         break;

      case Controller:
         if (entry > 0) {
            // check all senders we are connected to,
            // and figure out which one are controllers to us
            QACConnectionObject *connectionObject = (QACConnectionObject *)object();
            QObjectList allSenders = connectionObject->senderList();
            QObjectList senders;

            for (int s = 0; s < allSenders.size(); ++s) {
               QObject *sender = allSenders.at(s);
               QAccessibleInterface *candidate = QAccessible::queryAccessibleInterface(sender);

               if (!candidate) {
                  continue;
               }
               if (candidate->relationTo(0, this, 0)&Controller) {
                  senders << sender;
               }
               delete candidate;
            }
            if (entry <= senders.size()) {
               targetObject = senders.at(entry - 1);
            }
         }
         break;

      case Controlled:
         if (entry > 0) {
            QObjectList allReceivers;
            QACConnectionObject *connectionObject = (QACConnectionObject *)object();

            for (int sig = 0; sig < d->primarySignals.count(); ++sig) {
               QObjectList receivers = connectionObject->receiverList(d->primarySignals.at(sig));
               allReceivers += receivers;
            }

            if (entry <= allReceivers.size()) {
               targetObject = allReceivers.at(entry - 1);
            }
         }
         break;

      default:
         break;
   }

   *target = QAccessible::queryAccessibleInterface(targetObject);
   return *target ? 0 : -1;
}

int QAccessibleWidget::childCount() const
{
   QWidgetList cl = childWidgets(widget());
   return cl.size();
}

int QAccessibleWidget::indexOfChild(const QAccessibleInterface *child) const
{
   QWidgetList cl = childWidgets(widget());
   int index = cl.indexOf(qobject_cast<QWidget *>(child->object()));
   if (index != -1) {
      ++index;
   }
   return index;
}

// from qwidget.cpp
extern QString qt_setWindowTitle_helperHelper(const QString &, const QWidget *);

QString QAccessibleWidget::text(Text t, int child) const
{
   QString str;

   switch (t) {
      case Name:
         if (!d->name.isEmpty()) {
            str = d->name;
         } else if (!widget()->accessibleName().isEmpty()) {
            str = widget()->accessibleName();
         } else if (!child && widget()->isWindow()) {
            if (widget()->isMinimized()) {
               str = qt_setWindowTitle_helperHelper(widget()->windowIconText(), widget());
            } else {
               str = qt_setWindowTitle_helperHelper(widget()->windowTitle(), widget());
            }
         } else {
            str = qt_accStripAmp(buddyString(widget()));
         }
         break;
      case Description:
         if (!d->description.isEmpty()) {
            str = d->description;
         } else if (!widget()->accessibleDescription().isEmpty()) {
            str = widget()->accessibleDescription();
         }
#ifndef QT_NO_TOOLTIP
         else {
            str = widget()->toolTip();
         }
#endif
         break;
      case Help:
         if (!d->help.isEmpty()) {
            str = d->help;
         }
#ifndef QT_NO_WHATSTHIS
         else {
            str = widget()->whatsThis();
         }
#endif
         break;
      case Accelerator:
         if (!d->accelerator.isEmpty()) {
            str = d->accelerator;
         } else {
            str = qt_accHotKey(buddyString(widget()));
         }
         break;
      case Value:
         str = d->value;
         break;
      default:
         break;
   }
   return str;
}

#ifndef QT_NO_ACTION

int QAccessibleWidget::userActionCount(int child) const
{
   if (child) {
      return 0;
   }
   return widget()->actions().count();
}

QString QAccessibleWidget::actionText(int action, Text t, int child) const
{
   if (action == DefaultAction) {
      action = SetFocus;
   }

   if (action > 0 && !child) {
      QAction *act = widget()->actions().value(action - 1);
      if (act) {
         switch (t) {
            case Name:
               return act->text();
            case Description:
               return act->toolTip();
#ifndef QT_NO_SHORTCUT
            case Accelerator:
               return act->shortcut().toString();
#endif
            default:
               break;
         }
      }
   }

   return QAccessibleObject::actionText(action, t, child);
}

bool QAccessibleWidget::doAction(int action, int child, const QVariantList &params)
{
   if (action == SetFocus || action == DefaultAction) {
      if (child || !widget()->isEnabled()) {
         return false;
      }

      if ((widget()->focusPolicy() == Qt::NoFocus) && (!widget()->isWindow())) {
         return false;
      }

      if (!widget()->isWindow()) {
         widget()->setFocus();
      }

      widget()->activateWindow();

      return true;
   } else if (action > 0) {
      if (QAction *act = widget()->actions().value(action - 1)) {
         act->trigger();
         return true;
      }
   }
   return QAccessibleObject::doAction(action, child, params);
}

#endif // QT_NO_ACTION

QAccessible::Role QAccessibleWidget::role(int child) const
{
   if (!child) {
      return d->role;
   }

   QWidgetList childList = childWidgets(widget());
   if (childList.count() > 0 && child <= childList.count()) {
      QWidget *targetWidget = childList.at(child - 1);
      QAccessibleInterface *iface = QAccessible::queryAccessibleInterface(targetWidget);
      if (iface) {
         QAccessible::Role role = iface->role(0);
         delete iface;
         return role;
      }
   }

   return NoRole;
}

QAccessible::State QAccessibleWidget::state(int child) const
{
   if (child) {
      return Normal;
   }

   QAccessible::State state = Normal;

   QWidget *w = widget();
   if (w->testAttribute(Qt::WA_WState_Visible) == false) {
      state |= Invisible;
   }
   if (w->focusPolicy() != Qt::NoFocus) {
      state |= Focusable;
   }
   if (w->hasFocus()) {
      state |= Focused;
   }
   if (!w->isEnabled()) {
      state |= Unavailable;
   }
   if (w->isWindow()) {
      if (w->windowFlags() & Qt::WindowSystemMenuHint) {
         state |= Movable;
      }
      if (w->minimumSize() != w->maximumSize()) {
         state |= Sizeable;
      }
   }

   return state;
}

// ### Qt5: remove me - binary compatibility hack
QAccessibleWidgetEx::QAccessibleWidgetEx(QWidget *o, Role role, const QString &name)
   : QAccessibleObjectEx(o)
{
   Q_ASSERT(widget());
   d = new QAccessibleWidgetPrivate();
   d->role = role;
   d->name = name;
   d->asking = 0;
}

int QAccessibleWidgetEx::childCount() const
{
   return reinterpret_cast<const QAccessibleWidget *>(this)->QAccessibleWidget::childCount();
}
int QAccessibleWidgetEx::indexOfChild(const QAccessibleInterface *child) const
{
   return reinterpret_cast<const QAccessibleWidget *>(this)->QAccessibleWidget::indexOfChild(child);
}
QAccessible::Relation QAccessibleWidgetEx::relationTo(int child, const QAccessibleInterface *other,
      int otherChild) const
{
   return reinterpret_cast<const QAccessibleWidget *>(this)->QAccessibleWidget::relationTo(child, other, otherChild);
}

int QAccessibleWidgetEx::childAt(int x, int y) const
{
   return reinterpret_cast<const QAccessibleWidget *>(this)->QAccessibleWidget::childAt(x, y);
}
QRect QAccessibleWidgetEx::rect(int child) const
{
   return reinterpret_cast<const QAccessibleWidget *>(this)->QAccessibleWidget::rect(child);
}
int QAccessibleWidgetEx::navigate(RelationFlag rel, int entry, QAccessibleInterface **target) const
{
   return reinterpret_cast<const QAccessibleWidget *>(this)->QAccessibleWidget::navigate(rel, entry, target);
}

QString QAccessibleWidgetEx::text(Text t, int child) const
{
   return reinterpret_cast<const QAccessibleWidget *>(this)->QAccessibleWidget::text(t, child);
}
QAccessible::Role QAccessibleWidgetEx::role(int child) const
{
   return reinterpret_cast<const QAccessibleWidget *>(this)->QAccessibleWidget::role(child);
}
QAccessible::State QAccessibleWidgetEx::state(int child) const
{
   return (reinterpret_cast<const QAccessibleWidget *>(this)->QAccessibleWidget::state(child))
          | HasInvokeExtension;
}

QString QAccessibleWidgetEx::actionText(int action, Text t, int child) const
{
   return reinterpret_cast<const QAccessibleWidget *>(this)->QAccessibleWidget::actionText(action, t, child);
}
bool QAccessibleWidgetEx::doAction(int action, int child, const QVariantList &params)
{
   return reinterpret_cast<QAccessibleWidget *>(this)->QAccessibleWidget::doAction(action, child, params);
}

QAccessibleWidgetEx::~QAccessibleWidgetEx()
{
   delete d;
}
QWidget *QAccessibleWidgetEx::widget() const
{
   return reinterpret_cast<const QAccessibleWidget *>(this)->QAccessibleWidget::widget();
}
QObject *QAccessibleWidgetEx::parentObject() const
{
   return reinterpret_cast<const QAccessibleWidget *>(this)->QAccessibleWidget::parentObject();
}

void QAccessibleWidgetEx::addControllingSignal(const QString &signal)
{
   reinterpret_cast<QAccessibleWidget *>(this)->QAccessibleWidget::addControllingSignal(signal);
}
void QAccessibleWidgetEx::setValue(const QString &value)
{
   reinterpret_cast<QAccessibleWidget *>(this)->QAccessibleWidget::setValue(value);
}
void QAccessibleWidgetEx::setDescription(const QString &desc)
{
   reinterpret_cast<QAccessibleWidget *>(this)->QAccessibleWidget::setDescription(desc);
}
void QAccessibleWidgetEx::setHelp(const QString &help)
{
   reinterpret_cast<QAccessibleWidget *>(this)->QAccessibleWidget::setHelp(help);
}
void QAccessibleWidgetEx::setAccelerator(const QString &accel)
{
   reinterpret_cast<QAccessibleWidget *>(this)->QAccessibleWidget::setAccelerator(accel);
}

QVariant QAccessibleWidgetEx::invokeMethodEx(Method method, int child, const QVariantList & /*params*/)
{
   if (child) {
      return QVariant();
   }

   switch (method) {
      case ListSupportedMethods: {
         QSet<QAccessible::Method> set;
         set << ListSupportedMethods << ForegroundColor << BackgroundColor;
         return QVariant::fromValue(set);
      }
      case ForegroundColor:
         return widget()->palette().color(widget()->foregroundRole());
      case BackgroundColor:
         return widget()->palette().color(widget()->backgroundRole());
      default:
         return QVariant();
   }
}

QT_END_NAMESPACE

#endif //QT_NO_ACCESSIBILITY
