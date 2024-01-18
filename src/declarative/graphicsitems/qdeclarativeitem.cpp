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

#include <qdeclarativeitem.h>
#include <qdeclarativeevents_p_p.h>
#include <qdeclarativeengine_p.h>
#include <qgraphicsitem_p.h>
#include <qdeclarativeitem_p.h>

#include <qdeclarativeengine.h>
#include <qdeclarativeopenmetaobject_p.h>
#include <qdeclarativestate_p.h>
#include <qdeclarativeview.h>
#include <qdeclarativestategroup_p.h>
#include <qdeclarativecomponent.h>
#include <qdeclarativeinfo.h>

#include <QDebug>
#include <QPen>
#include <QEvent>
#include <QGraphicsSceneMouseEvent>
#include <QtCore/qnumeric.h>
#include <QtScript/qscriptengine.h>
#include <QtGui/qgraphicstransform.h>
#include <qlistmodelinterface_p.h>

#include <float.h>

QT_BEGIN_NAMESPACE

/*!
    \qmlclass Transform QGraphicsTransform
    \ingroup qml-transform-elements
    \since 4.7
    \brief The Transform elements provide a way of building advanced transformations on Items.

    The Transform element is a base type which cannot be instantiated directly.
    The following concrete Transform types are available:

    \list
    \o \l Rotation
    \o \l Scale
    \o \l Translate
    \endlist

    The Transform elements let you create and control advanced transformations that can be configured
    independently using specialized properties.

    You can assign any number of Transform elements to an \l Item. Each Transform is applied in order,
    one at a time.
*/

/*!
    \qmlclass Translate QDeclarativeTranslate
    \ingroup qml-transform-elements
    \since 4.7
    \brief The Translate object provides a way to move an Item without changing its x or y properties.

    The Translate object provides independent control over position in addition to the Item's x and y properties.

    The following example moves the Y axis of the \l Rectangle elements while still allowing the \l Row element
    to lay the items out as if they had not been transformed:
    \qml
    import QtQuick 1.0

    Row {
        Rectangle {
            width: 100; height: 100
            color: "blue"
            transform: Translate { y: 20 }
        }
        Rectangle {
            width: 100; height: 100
            color: "red"
            transform: Translate { y: -20 }
        }
    }
    \endqml

    \image translate.png
*/

/*!
    \qmlproperty real Translate::x

    The translation along the X axis.
*/

/*!
    \qmlproperty real Translate::y

    The translation along the Y axis.
*/

/*!
    \qmlclass Scale QGraphicsScale
    \ingroup qml-transform-elements
    \since 4.7
    \brief The Scale element provides a way to scale an Item.

    The Scale element gives more control over scaling than using \l Item's \l{Item::scale}{scale} property. Specifically,
    it allows a different scale for the x and y axes, and allows the scale to be relative to an
    arbitrary point.

    The following example scales the X axis of the Rectangle, relative to its interior point 25, 25:
    \qml
    Rectangle {
        width: 100; height: 100
        color: "blue"
        transform: Scale { origin.x: 25; origin.y: 25; xScale: 3}
    }
    \endqml

    \sa Rotation, Translate
*/

/*!
    \qmlproperty real Scale::origin.x
    \qmlproperty real Scale::origin.y

    The point that the item is scaled from (i.e., the point that stays fixed relative to the parent as
    the rest of the item grows). By default the origin is 0, 0.
*/

/*!
    \qmlproperty real Scale::xScale

    The scaling factor for the X axis.
*/

/*!
    \qmlproperty real Scale::yScale

    The scaling factor for the Y axis.
*/

/*!
    \qmlclass Rotation QGraphicsRotation
    \ingroup qml-transform-elements
    \since 4.7
    \brief The Rotation object provides a way to rotate an Item.

    The Rotation object gives more control over rotation than using \l Item's \l{Item::rotation}{rotation} property.
    Specifically, it allows (z axis) rotation to be relative to an arbitrary point.

    The following example rotates a Rectangle around its interior point 25, 25:
    \qml
    Rectangle {
        width: 100; height: 100
        color: "blue"
        transform: Rotation { origin.x: 25; origin.y: 25; angle: 45}
    }
    \endqml

    Rotation also provides a way to specify 3D-like rotations for Items. For these types of
    rotations you must specify the axis to rotate around in addition to the origin point.

    The following example shows various 3D-like rotations applied to an \l Image.
    \snippet doc/src/snippets/declarative/rotation.qml 0

    \image axisrotation.png

    \sa {declarative/ui-components/dialcontrol}{Dial Control example}, {declarative/toys/clocks}{Clocks example}
*/

/*!
    \qmlproperty real Rotation::origin.x
    \qmlproperty real Rotation::origin.y

    The origin point of the rotation (i.e., the point that stays fixed relative to the parent as
    the rest of the item rotates). By default the origin is 0, 0.
*/

/*!
    \qmlproperty real Rotation::axis.x
    \qmlproperty real Rotation::axis.y
    \qmlproperty real Rotation::axis.z

    The axis to rotate around. For simple (2D) rotation around a point, you do not need to specify an axis,
    as the default axis is the z axis (\c{ axis { x: 0; y: 0; z: 1 } }).

    For a typical 3D-like rotation you will usually specify both the origin and the axis.

    \image 3d-rotation-axis.png
*/

/*!
    \qmlproperty real Rotation::angle

    The angle to rotate, in degrees clockwise.
*/

QDeclarativeContents::QDeclarativeContents(QDeclarativeItem *item) : m_item(item), m_x(0), m_y(0), m_width(0),
   m_height(0)
{
   //### optimize
   connect(this, SIGNAL(rectChanged(QRectF)), m_item, SIGNAL(childrenRectChanged(QRectF)));
}

QDeclarativeContents::~QDeclarativeContents()
{
   QList<QGraphicsItem *> children = m_item->childItems();
   for (int i = 0; i < children.count(); ++i) {
      QDeclarativeItem *child = qobject_cast<QDeclarativeItem *>(children.at(i));
      if (!child) { //### Should this be ignoring non-QDeclarativeItem graphicsobjects?
         continue;
      }
      QDeclarativeItemPrivate::get(child)->removeItemChangeListener(this,
            QDeclarativeItemPrivate::Geometry | QDeclarativeItemPrivate::Destroyed);
   }
}

QRectF QDeclarativeContents::rectF() const
{
   return QRectF(m_x, m_y, m_width, m_height);
}

void QDeclarativeContents::calcHeight(QDeclarativeItem *changed)
{
   qreal oldy = m_y;
   qreal oldheight = m_height;

   if (changed) {
      qreal top = oldy;
      qreal bottom = oldy + oldheight;
      qreal y = changed->y();
      if (y + changed->height() > bottom) {
         bottom = y + changed->height();
      }
      if (y < top) {
         top = y;
      }
      m_y = top;
      m_height = bottom - top;
   } else {
      qreal top = FLT_MAX;
      qreal bottom = 0;
      QList<QGraphicsItem *> children = m_item->childItems();
      for (int i = 0; i < children.count(); ++i) {
         QDeclarativeItem *child = qobject_cast<QDeclarativeItem *>(children.at(i));
         if (!child) { //### Should this be ignoring non-QDeclarativeItem graphicsobjects?
            continue;
         }
         qreal y = child->y();
         if (y + child->height() > bottom) {
            bottom = y + child->height();
         }
         if (y < top) {
            top = y;
         }
      }
      if (!children.isEmpty()) {
         m_y = top;
      }
      m_height = qMax(bottom - top, qreal(0.0));
   }

   if (m_height != oldheight || m_y != oldy) {
      emit rectChanged(rectF());
   }
}

void QDeclarativeContents::calcWidth(QDeclarativeItem *changed)
{
   qreal oldx = m_x;
   qreal oldwidth = m_width;

   if (changed) {
      qreal left = oldx;
      qreal right = oldx + oldwidth;
      qreal x = changed->x();
      if (x + changed->width() > right) {
         right = x + changed->width();
      }
      if (x < left) {
         left = x;
      }
      m_x = left;
      m_width = right - left;
   } else {
      qreal left = FLT_MAX;
      qreal right = 0;
      QList<QGraphicsItem *> children = m_item->childItems();
      for (int i = 0; i < children.count(); ++i) {
         QDeclarativeItem *child = qobject_cast<QDeclarativeItem *>(children.at(i));
         if (!child) { //### Should this be ignoring non-QDeclarativeItem graphicsobjects?
            continue;
         }
         qreal x = child->x();
         if (x + child->width() > right) {
            right = x + child->width();
         }
         if (x < left) {
            left = x;
         }
      }
      if (!children.isEmpty()) {
         m_x = left;
      }
      m_width = qMax(right - left, qreal(0.0));
   }

   if (m_width != oldwidth || m_x != oldx) {
      emit rectChanged(rectF());
   }
}

void QDeclarativeContents::complete()
{
   QList<QGraphicsItem *> children = m_item->childItems();
   for (int i = 0; i < children.count(); ++i) {
      QDeclarativeItem *child = qobject_cast<QDeclarativeItem *>(children.at(i));
      if (!child) { //### Should this be ignoring non-QDeclarativeItem graphicsobjects?
         continue;
      }
      QDeclarativeItemPrivate::get(child)->addItemChangeListener(this,
            QDeclarativeItemPrivate::Geometry | QDeclarativeItemPrivate::Destroyed);
      //###what about changes to visibility?
   }

   calcGeometry();
}

void QDeclarativeContents::itemGeometryChanged(QDeclarativeItem *changed, const QRectF &newGeometry,
      const QRectF &oldGeometry)
{
   Q_UNUSED(changed)
   //### we can only pass changed if the left edge has moved left, or the right edge has moved right
   if (newGeometry.width() != oldGeometry.width() || newGeometry.x() != oldGeometry.x()) {
      calcWidth(/*changed*/);
   }
   if (newGeometry.height() != oldGeometry.height() || newGeometry.y() != oldGeometry.y()) {
      calcHeight(/*changed*/);
   }
}

void QDeclarativeContents::itemDestroyed(QDeclarativeItem *item)
{
   if (item) {
      QDeclarativeItemPrivate::get(item)->removeItemChangeListener(this,
            QDeclarativeItemPrivate::Geometry | QDeclarativeItemPrivate::Destroyed);
   }
   calcGeometry();
}

void QDeclarativeContents::childRemoved(QDeclarativeItem *item)
{
   if (item) {
      QDeclarativeItemPrivate::get(item)->removeItemChangeListener(this,
            QDeclarativeItemPrivate::Geometry | QDeclarativeItemPrivate::Destroyed);
   }
   calcGeometry();
}

void QDeclarativeContents::childAdded(QDeclarativeItem *item)
{
   if (item) {
      QDeclarativeItemPrivate::get(item)->addItemChangeListener(this,
            QDeclarativeItemPrivate::Geometry | QDeclarativeItemPrivate::Destroyed);
   }
   calcWidth(item);
   calcHeight(item);
}

QDeclarativeItemKeyFilter::QDeclarativeItemKeyFilter(QDeclarativeItem *item)
   : m_processPost(false), m_next(0)
{
   QDeclarativeItemPrivate *p =
      item ? static_cast<QDeclarativeItemPrivate *>(QGraphicsItemPrivate::get(item)) : 0;
   if (p) {
      m_next = p->keyHandler;
      p->keyHandler = this;
   }
}

QDeclarativeItemKeyFilter::~QDeclarativeItemKeyFilter()
{
}

void QDeclarativeItemKeyFilter::keyPressed(QKeyEvent *event, bool post)
{
   if (m_next) {
      m_next->keyPressed(event, post);
   }
}

void QDeclarativeItemKeyFilter::keyReleased(QKeyEvent *event, bool post)
{
   if (m_next) {
      m_next->keyReleased(event, post);
   }
}

void QDeclarativeItemKeyFilter::inputMethodEvent(QInputMethodEvent *event, bool post)
{
   if (m_next) {
      m_next->inputMethodEvent(event, post);
   }
}

QVariant QDeclarativeItemKeyFilter::inputMethodQuery(Qt::InputMethodQuery query) const
{
   if (m_next) {
      return m_next->inputMethodQuery(query);
   }
   return QVariant();
}

void QDeclarativeItemKeyFilter::componentComplete()
{
   if (m_next) {
      m_next->componentComplete();
   }
}


/*!
    \qmlclass KeyNavigation QDeclarativeKeyNavigationAttached
    \ingroup qml-basic-interaction-elements
    \since 4.7
    \brief The KeyNavigation attached property supports key navigation by arrow keys.

    Key-based user interfaces commonly allow the use of arrow keys to navigate between
    focusable items.  The KeyNavigation attached property enables this behavior by providing a
    convenient way to specify the item that should gain focus when an arrow or tab key is pressed.

    The following example provides key navigation for a 2x2 grid of items:

    \snippet doc/src/snippets/declarative/keynavigation.qml 0

    The top-left item initially receives focus by setting \l {Item::}{focus} to
    \c true. When an arrow key is pressed, the focus will move to the
    appropriate item, as defined by the value that has been set for
    the KeyNavigation \l left, \l right, \l up or \l down properties.

    Note that if a KeyNavigation attached property receives the key press and release
    events for a requested arrow or tab key, the event is accepted and does not
    propagate any further.

    By default, KeyNavigation receives key events after the item to which it is attached.
    If the item accepts the key event, the KeyNavigation attached property will not
    receive an event for that key.  Setting the \l priority property to
    \c KeyNavigation.BeforeItem allows the event to be used for key navigation
    before the item, rather than after.

    If item to which the focus is switching is not enabled or visible, an attempt will
    be made to skip this item and focus on the next. This is possible if there are
    a chain of items with the same KeyNavigation handler. If multiple items in a row are not enabled
    or visible, they will also be skipped.

    \sa {Keys}{Keys attached property}
*/

/*!
    \qmlproperty Item KeyNavigation::left
    \qmlproperty Item KeyNavigation::right
    \qmlproperty Item KeyNavigation::up
    \qmlproperty Item KeyNavigation::down
    \qmlproperty Item KeyNavigation::tab
    \qmlproperty Item KeyNavigation::backtab

    These properties hold the item to assign focus to
    when the left, right, up or down cursor keys, or the
    tab key are pressed.
*/

/*!
    \qmlproperty Item KeyNavigation::tab
    \qmlproperty Item KeyNavigation::backtab

    These properties hold the item to assign focus to
    when the Tab key or Shift+Tab key combination (Backtab) are pressed.
*/

QDeclarativeKeyNavigationAttached::QDeclarativeKeyNavigationAttached(QObject *parent)
   : QObject(*(new QDeclarativeKeyNavigationAttachedPrivate), parent),
     QDeclarativeItemKeyFilter(qobject_cast<QDeclarativeItem *>(parent))
{
   m_processPost = true;
}

QDeclarativeKeyNavigationAttached *
QDeclarativeKeyNavigationAttached::qmlAttachedProperties(QObject *obj)
{
   return new QDeclarativeKeyNavigationAttached(obj);
}

QDeclarativeItem *QDeclarativeKeyNavigationAttached::left() const
{
   Q_D(const QDeclarativeKeyNavigationAttached);
   return d->left;
}

void QDeclarativeKeyNavigationAttached::setLeft(QDeclarativeItem *i)
{
   Q_D(QDeclarativeKeyNavigationAttached);
   if (d->left == i) {
      return;
   }
   d->left = i;
   emit leftChanged();
}

QDeclarativeItem *QDeclarativeKeyNavigationAttached::right() const
{
   Q_D(const QDeclarativeKeyNavigationAttached);
   return d->right;
}

void QDeclarativeKeyNavigationAttached::setRight(QDeclarativeItem *i)
{
   Q_D(QDeclarativeKeyNavigationAttached);
   if (d->right == i) {
      return;
   }
   d->right = i;
   emit rightChanged();
}

QDeclarativeItem *QDeclarativeKeyNavigationAttached::up() const
{
   Q_D(const QDeclarativeKeyNavigationAttached);
   return d->up;
}

void QDeclarativeKeyNavigationAttached::setUp(QDeclarativeItem *i)
{
   Q_D(QDeclarativeKeyNavigationAttached);
   if (d->up == i) {
      return;
   }
   d->up = i;
   emit upChanged();
}

QDeclarativeItem *QDeclarativeKeyNavigationAttached::down() const
{
   Q_D(const QDeclarativeKeyNavigationAttached);
   return d->down;
}

void QDeclarativeKeyNavigationAttached::setDown(QDeclarativeItem *i)
{
   Q_D(QDeclarativeKeyNavigationAttached);
   if (d->down == i) {
      return;
   }
   d->down = i;
   emit downChanged();
}

QDeclarativeItem *QDeclarativeKeyNavigationAttached::tab() const
{
   Q_D(const QDeclarativeKeyNavigationAttached);
   return d->tab;
}

void QDeclarativeKeyNavigationAttached::setTab(QDeclarativeItem *i)
{
   Q_D(QDeclarativeKeyNavigationAttached);
   if (d->tab == i) {
      return;
   }
   d->tab = i;
   emit tabChanged();
}

QDeclarativeItem *QDeclarativeKeyNavigationAttached::backtab() const
{
   Q_D(const QDeclarativeKeyNavigationAttached);
   return d->backtab;
}

void QDeclarativeKeyNavigationAttached::setBacktab(QDeclarativeItem *i)
{
   Q_D(QDeclarativeKeyNavigationAttached);
   if (d->backtab == i) {
      return;
   }
   d->backtab = i;
   emit backtabChanged();
}

/*!
    \qmlproperty enumeration KeyNavigation::priority

    This property determines whether the keys are processed before
    or after the attached item's own key handling.

    \list
    \o KeyNavigation.BeforeItem - process the key events before normal
    item key processing.  If the event is used for key navigation, it will be accepted and will not
    be passed on to the item.
    \o KeyNavigation.AfterItem (default) - process the key events after normal item key
    handling.  If the item accepts the key event it will not be
    handled by the KeyNavigation attached property handler.
    \endlist
*/
QDeclarativeKeyNavigationAttached::Priority QDeclarativeKeyNavigationAttached::priority() const
{
   return m_processPost ? AfterItem : BeforeItem;
}

void QDeclarativeKeyNavigationAttached::setPriority(Priority order)
{
   bool processPost = order == AfterItem;
   if (processPost != m_processPost) {
      m_processPost = processPost;
      emit priorityChanged();
   }
}

void QDeclarativeKeyNavigationAttached::keyPressed(QKeyEvent *event, bool post)
{
   Q_D(QDeclarativeKeyNavigationAttached);
   event->ignore();

   if (post != m_processPost) {
      QDeclarativeItemKeyFilter::keyPressed(event, post);
      return;
   }

   bool mirror = false;
   switch (event->key()) {
      case Qt::Key_Left: {
         if (QDeclarativeItem *parentItem = qobject_cast<QDeclarativeItem *>(parent())) {
            mirror = QDeclarativeItemPrivate::get(parentItem)->effectiveLayoutMirror;
         }
         QDeclarativeItem *leftItem = mirror ? d->right : d->left;
         if (leftItem) {
            setFocusNavigation(leftItem, mirror ? "right" : "left");
            event->accept();
         }
         break;
      }
      case Qt::Key_Right: {
         if (QDeclarativeItem *parentItem = qobject_cast<QDeclarativeItem *>(parent())) {
            mirror = QDeclarativeItemPrivate::get(parentItem)->effectiveLayoutMirror;
         }
         QDeclarativeItem *rightItem = mirror ? d->left : d->right;
         if (rightItem) {
            setFocusNavigation(rightItem, mirror ? "left" : "right");
            event->accept();
         }
         break;
      }
      case Qt::Key_Up:
         if (d->up) {
            setFocusNavigation(d->up, "up");
            event->accept();
         }
         break;
      case Qt::Key_Down:
         if (d->down) {
            setFocusNavigation(d->down, "down");
            event->accept();
         }
         break;
      case Qt::Key_Tab:
         if (d->tab) {
            setFocusNavigation(d->tab, "tab");
            event->accept();
         }
         break;
      case Qt::Key_Backtab:
         if (d->backtab) {
            setFocusNavigation(d->backtab, "backtab");
            event->accept();
         }
         break;
      default:
         break;
   }

   if (!event->isAccepted()) {
      QDeclarativeItemKeyFilter::keyPressed(event, post);
   }
}

void QDeclarativeKeyNavigationAttached::keyReleased(QKeyEvent *event, bool post)
{
   Q_D(QDeclarativeKeyNavigationAttached);
   event->ignore();

   if (post != m_processPost) {
      QDeclarativeItemKeyFilter::keyReleased(event, post);
      return;
   }

   bool mirror = false;
   switch (event->key()) {
      case Qt::Key_Left:
         if (QDeclarativeItem *parentItem = qobject_cast<QDeclarativeItem *>(parent())) {
            mirror = QDeclarativeItemPrivate::get(parentItem)->effectiveLayoutMirror;
         }
         if (mirror ? d->right : d->left) {
            event->accept();
         }
         break;
      case Qt::Key_Right:
         if (QDeclarativeItem *parentItem = qobject_cast<QDeclarativeItem *>(parent())) {
            mirror = QDeclarativeItemPrivate::get(parentItem)->effectiveLayoutMirror;
         }
         if (mirror ? d->left : d->right) {
            event->accept();
         }
         break;
      case Qt::Key_Up:
         if (d->up) {
            event->accept();
         }
         break;
      case Qt::Key_Down:
         if (d->down) {
            event->accept();
         }
         break;
      case Qt::Key_Tab:
         if (d->tab) {
            event->accept();
         }
         break;
      case Qt::Key_Backtab:
         if (d->backtab) {
            event->accept();
         }
         break;
      default:
         break;
   }

   if (!event->isAccepted()) {
      QDeclarativeItemKeyFilter::keyReleased(event, post);
   }
}

void QDeclarativeKeyNavigationAttached::setFocusNavigation(QDeclarativeItem *currentItem, const char *dir)
{
   QDeclarativeItem *initialItem = currentItem;
   bool isNextItem = false;
   do {
      isNextItem = false;
      if (currentItem->isVisible() && currentItem->isEnabled()) {
         currentItem->setFocus(true);
      } else {
         QObject *attached =
            qmlAttachedPropertiesObject<QDeclarativeKeyNavigationAttached>(currentItem, false);
         if (attached) {
            QDeclarativeItem *tempItem = qvariant_cast<QDeclarativeItem *>(attached->property(dir));
            if (tempItem) {
               currentItem = tempItem;
               isNextItem = true;
            }
         }
      }
   } while (currentItem != initialItem && isNextItem);
}

/*!
    \qmlclass LayoutMirroring QDeclarativeLayoutMirroringAttached
    \since QtQuick 1.1
    \ingroup qml-utility-elements
    \brief The LayoutMirroring attached property is used to mirror layout behavior.

    The LayoutMirroring attached property is used to horizontally mirror \l {anchor-layout}{Item anchors},
    \l{Using QML Positioner and Repeater Items}{positioner} elements (such as \l Row and \l Grid)
    and views (such as \l GridView and horizontal \l ListView). Mirroring is a visual change: left
    anchors become right anchors, and positioner elements like \l Grid and \l Row reverse the
    horizontal layout of child items.

    Mirroring is enabled for an item by setting the \l enabled property to true. By default, this
    only affects the item itself; setting the \l childrenInherit property to true propagates the mirroring
    behavior to all child elements as well. If the \c LayoutMirroring attached property has not been defined
    for an item, mirroring is not enabled.

    The following example shows mirroring in action. The \l Row below is specified as being anchored
    to the left of its parent. However, since mirroring has been enabled, the anchor is horizontally
    reversed and it is now anchored to the right. Also, since items in a \l Row are positioned
    from left to right by default, they are now positioned from right to left instead, as demonstrated
    by the numbering and opacity of the items:

    \snippet doc/src/snippets/declarative/layoutmirroring.qml 0

    \image layoutmirroring.png

    Layout mirroring is useful when it is necessary to support both left-to-right and right-to-left
    layout versions of an application to target different language areas. The \l childrenInherit
    property allows layout mirroring to be applied without manually setting layout configurations
    for every item in an application. Keep in mind, however, that mirroring does not affect any
    positioning that is defined by the \l Item \l {Item::}{x} coordinate value, so even with
    mirroring enabled, it will often be necessary to apply some layout fixes to support the
    desired layout direction. Also, it may be necessary to disable the mirroring of individual
    child items (by setting \l {enabled}{LayoutMirroring.enabled} to false for such items) if
    mirroring is not the desired behavior, or if the child item already implements mirroring in
    some custom way.

    See \l {QML Right-to-left User Interfaces} for further details on using \c LayoutMirroring and
    other related features to implement right-to-left support for an application.
*/

/*!
    \qmlproperty bool LayoutMirroring::enabled

    This property holds whether the item's layout is mirrored horizontally. Setting this to true
    horizontally reverses \l {anchor-layout}{anchor} settings such that left anchors become right,
    and right anchors become left. For \l{Using QML Positioner and Repeater Items}{positioner} elements
    (such as \l Row and \l Grid) and view elements (such as \l {GridView}{GridView} and \l {ListView}{ListView})
    this also mirrors the horizontal layout direction of the item.

    The default value is false.
*/

/*!
    \qmlproperty bool LayoutMirroring::childrenInherit

    This property holds whether the \l {enabled}{LayoutMirroring.enabled} value for this item
    is inherited by its children.

    The default value is false.
*/

QDeclarativeLayoutMirroringAttached::QDeclarativeLayoutMirroringAttached(QObject *parent) : QObject(parent),
   itemPrivate(0)
{
   if (QDeclarativeItem *item = qobject_cast<QDeclarativeItem *>(parent)) {
      itemPrivate = QDeclarativeItemPrivate::get(item);
      itemPrivate->attachedLayoutDirection = this;
   } else {
      qmlInfo(parent) << tr("LayoutDirection attached property only works with Items");
   }
}

QDeclarativeLayoutMirroringAttached *QDeclarativeLayoutMirroringAttached::qmlAttachedProperties(QObject *object)
{
   return new QDeclarativeLayoutMirroringAttached(object);
}

bool QDeclarativeLayoutMirroringAttached::enabled() const
{
   return itemPrivate ? itemPrivate->effectiveLayoutMirror : false;
}

void QDeclarativeLayoutMirroringAttached::setEnabled(bool enabled)
{
   if (!itemPrivate) {
      return;
   }

   itemPrivate->isMirrorImplicit = false;
   if (enabled != itemPrivate->effectiveLayoutMirror) {
      itemPrivate->setLayoutMirror(enabled);
      if (itemPrivate->inheritMirrorFromItem) {
         itemPrivate->resolveLayoutMirror();
      }
   }
}

void QDeclarativeLayoutMirroringAttached::resetEnabled()
{
   if (itemPrivate && !itemPrivate->isMirrorImplicit) {
      itemPrivate->isMirrorImplicit = true;
      itemPrivate->resolveLayoutMirror();
   }
}

bool QDeclarativeLayoutMirroringAttached::childrenInherit() const
{
   return itemPrivate ? itemPrivate->inheritMirrorFromItem : false;
}

void QDeclarativeLayoutMirroringAttached::setChildrenInherit(bool childrenInherit)
{
   if (itemPrivate && childrenInherit != itemPrivate->inheritMirrorFromItem) {
      itemPrivate->inheritMirrorFromItem = childrenInherit;
      itemPrivate->resolveLayoutMirror();
      childrenInheritChanged();
   }
}

void QDeclarativeItemPrivate::resolveLayoutMirror()
{
   Q_Q(QDeclarativeItem);
   if (QDeclarativeItem *parentItem = q->parentItem()) {
      QDeclarativeItemPrivate *parentPrivate = QDeclarativeItemPrivate::get(parentItem);
      setImplicitLayoutMirror(parentPrivate->inheritedLayoutMirror, parentPrivate->inheritMirrorFromParent);
   } else {
      setImplicitLayoutMirror(isMirrorImplicit ? false : effectiveLayoutMirror, inheritMirrorFromItem);
   }
}

void QDeclarativeItemPrivate::setImplicitLayoutMirror(bool mirror, bool inherit)
{
   inherit = inherit || inheritMirrorFromItem;
   if (!isMirrorImplicit && inheritMirrorFromItem) {
      mirror = effectiveLayoutMirror;
   }
   if (mirror == inheritedLayoutMirror && inherit == inheritMirrorFromParent) {
      return;
   }

   inheritMirrorFromParent = inherit;
   inheritedLayoutMirror = inheritMirrorFromParent ? mirror : false;

   if (isMirrorImplicit) {
      setLayoutMirror(inherit ? inheritedLayoutMirror : false);
   }
   for (int i = 0; i < children.count(); ++i) {
      if (QDeclarativeItem *child = qobject_cast<QDeclarativeItem *>(children.at(i))) {
         QDeclarativeItemPrivate *childPrivate = QDeclarativeItemPrivate::get(child);
         childPrivate->setImplicitLayoutMirror(inheritedLayoutMirror, inheritMirrorFromParent);
      }
   }
}

void QDeclarativeItemPrivate::setLayoutMirror(bool mirror)
{
   if (mirror != effectiveLayoutMirror) {
      effectiveLayoutMirror = mirror;
      if (_anchors) {
         _anchors->d_func()->fillChanged();
         _anchors->d_func()->centerInChanged();
         _anchors->d_func()->updateHorizontalAnchors();
      }
      mirrorChange();
      if (attachedLayoutDirection) {
         emit attachedLayoutDirection->enabledChanged();
      }
   }
}

/*!
    \qmlclass Keys QDeclarativeKeysAttached
    \ingroup qml-basic-interaction-elements
    \since 4.7
    \brief The Keys attached property provides key handling to Items.

    All visual primitives support key handling via the Keys
    attached property.  Keys can be handled via the onPressed
    and onReleased signal properties.

    The signal properties have a \l KeyEvent parameter, named
    \e event which contains details of the event.  If a key is
    handled \e event.accepted should be set to true to prevent the
    event from propagating up the item hierarchy.

    \section1 Example Usage

    The following example shows how the general onPressed handler can
    be used to test for a certain key; in this case, the left cursor
    key:

    \snippet doc/src/snippets/declarative/keys/keys-pressed.qml key item

    Some keys may alternatively be handled via specific signal properties,
    for example \e onSelectPressed.  These handlers automatically set
    \e event.accepted to true.

    \snippet doc/src/snippets/declarative/keys/keys-handler.qml key item

    See \l{Qt::Key}{Qt.Key} for the list of keyboard codes.

    \section1 Key Handling Priorities

    The Keys attached property can be configured to handle key events
    before or after the item it is attached to. This makes it possible
    to intercept events in order to override an item's default behavior,
    or act as a fallback for keys not handled by the item.

    If \l priority is Keys.BeforeItem (default) the order of key event processing is:

    \list 1
    \o Items specified in \c forwardTo
    \o specific key handlers, e.g. onReturnPressed
    \o onKeyPress, onKeyRelease handlers
    \o Item specific key handling, e.g. TextInput key handling
    \o parent item
    \endlist

    If priority is Keys.AfterItem the order of key event processing is:

    \list 1
    \o Item specific key handling, e.g. TextInput key handling
    \o Items specified in \c forwardTo
    \o specific key handlers, e.g. onReturnPressed
    \o onKeyPress, onKeyRelease handlers
    \o parent item
    \endlist

    If the event is accepted during any of the above steps, key
    propagation stops.

    \sa KeyEvent, {KeyNavigation}{KeyNavigation attached property}
*/

/*!
    \qmlproperty bool Keys::enabled

    This flags enables key handling if true (default); otherwise
    no key handlers will be called.
*/

/*!
    \qmlproperty enumeration Keys::priority

    This property determines whether the keys are processed before
    or after the attached item's own key handling.

    \list
    \o Keys.BeforeItem (default) - process the key events before normal
    item key processing.  If the event is accepted it will not
    be passed on to the item.
    \o Keys.AfterItem - process the key events after normal item key
    handling.  If the item accepts the key event it will not be
    handled by the Keys attached property handler.
    \endlist
*/

/*!
    \qmlproperty list<Object> Keys::forwardTo

    This property provides a way to forward key presses, key releases, and keyboard input
    coming from input methods to other items. This can be useful when you want
    one item to handle some keys (e.g. the up and down arrow keys), and another item to
    handle other keys (e.g. the left and right arrow keys).  Once an item that has been
    forwarded keys accepts the event it is no longer forwarded to items later in the
    list.

    This example forwards key events to two lists:
    \qml
    Item {
        ListView {
            id: list1
            // ...
        }
        ListView {
            id: list2
            // ...
        }
        Keys.forwardTo: [list1, list2]
        focus: true
    }
    \endqml
*/

/*!
    \qmlsignal Keys::onPressed(KeyEvent event)

    This handler is called when a key has been pressed. The \a event
    parameter provides information about the event.
*/

/*!
    \qmlsignal Keys::onReleased(KeyEvent event)

    This handler is called when a key has been released. The \a event
    parameter provides information about the event.
*/

/*!
    \qmlsignal Keys::onDigit0Pressed(KeyEvent event)

    This handler is called when the digit '0' has been pressed. The \a event
    parameter provides information about the event.
*/

/*!
    \qmlsignal Keys::onDigit1Pressed(KeyEvent event)

    This handler is called when the digit '1' has been pressed. The \a event
    parameter provides information about the event.
*/

/*!
    \qmlsignal Keys::onDigit2Pressed(KeyEvent event)

    This handler is called when the digit '2' has been pressed. The \a event
    parameter provides information about the event.
*/

/*!
    \qmlsignal Keys::onDigit3Pressed(KeyEvent event)

    This handler is called when the digit '3' has been pressed. The \a event
    parameter provides information about the event.
*/

/*!
    \qmlsignal Keys::onDigit4Pressed(KeyEvent event)

    This handler is called when the digit '4' has been pressed. The \a event
    parameter provides information about the event.
*/

/*!
    \qmlsignal Keys::onDigit5Pressed(KeyEvent event)

    This handler is called when the digit '5' has been pressed. The \a event
    parameter provides information about the event.
*/

/*!
    \qmlsignal Keys::onDigit6Pressed(KeyEvent event)

    This handler is called when the digit '6' has been pressed. The \a event
    parameter provides information about the event.
*/

/*!
    \qmlsignal Keys::onDigit7Pressed(KeyEvent event)

    This handler is called when the digit '7' has been pressed. The \a event
    parameter provides information about the event.
*/

/*!
    \qmlsignal Keys::onDigit8Pressed(KeyEvent event)

    This handler is called when the digit '8' has been pressed. The \a event
    parameter provides information about the event.
*/

/*!
    \qmlsignal Keys::onDigit9Pressed(KeyEvent event)

    This handler is called when the digit '9' has been pressed. The \a event
    parameter provides information about the event.
*/

/*!
    \qmlsignal Keys::onLeftPressed(KeyEvent event)

    This handler is called when the Left arrow has been pressed. The \a event
    parameter provides information about the event.
*/

/*!
    \qmlsignal Keys::onRightPressed(KeyEvent event)

    This handler is called when the Right arrow has been pressed. The \a event
    parameter provides information about the event.
*/

/*!
    \qmlsignal Keys::onUpPressed(KeyEvent event)

    This handler is called when the Up arrow has been pressed. The \a event
    parameter provides information about the event.
*/

/*!
    \qmlsignal Keys::onDownPressed(KeyEvent event)

    This handler is called when the Down arrow has been pressed. The \a event
    parameter provides information about the event.
*/

/*!
    \qmlsignal Keys::onTabPressed(KeyEvent event)

    This handler is called when the Tab key has been pressed. The \a event
    parameter provides information about the event.
*/

/*!
    \qmlsignal Keys::onBacktabPressed(KeyEvent event)

    This handler is called when the Shift+Tab key combination (Backtab) has
    been pressed. The \a event parameter provides information about the event.
*/

/*!
    \qmlsignal Keys::onAsteriskPressed(KeyEvent event)

    This handler is called when the Asterisk '*' has been pressed. The \a event
    parameter provides information about the event.
*/

/*!
    \qmlsignal Keys::onEscapePressed(KeyEvent event)

    This handler is called when the Escape key has been pressed. The \a event
    parameter provides information about the event.
*/

/*!
    \qmlsignal Keys::onReturnPressed(KeyEvent event)

    This handler is called when the Return key has been pressed. The \a event
    parameter provides information about the event.
*/

/*!
    \qmlsignal Keys::onEnterPressed(KeyEvent event)

    This handler is called when the Enter key has been pressed. The \a event
    parameter provides information about the event.
*/

/*!
    \qmlsignal Keys::onDeletePressed(KeyEvent event)

    This handler is called when the Delete key has been pressed. The \a event
    parameter provides information about the event.
*/

/*!
    \qmlsignal Keys::onSpacePressed(KeyEvent event)

    This handler is called when the Space key has been pressed. The \a event
    parameter provides information about the event.
*/

/*!
    \qmlsignal Keys::onBackPressed(KeyEvent event)

    This handler is called when the Back key has been pressed. The \a event
    parameter provides information about the event.
*/

/*!
    \qmlsignal Keys::onCancelPressed(KeyEvent event)

    This handler is called when the Cancel key has been pressed. The \a event
    parameter provides information about the event.
*/

/*!
    \qmlsignal Keys::onSelectPressed(KeyEvent event)

    This handler is called when the Select key has been pressed. The \a event
    parameter provides information about the event.
*/

/*!
    \qmlsignal Keys::onYesPressed(KeyEvent event)

    This handler is called when the Yes key has been pressed. The \a event
    parameter provides information about the event.
*/

/*!
    \qmlsignal Keys::onNoPressed(KeyEvent event)

    This handler is called when the No key has been pressed. The \a event
    parameter provides information about the event.
*/

/*!
    \qmlsignal Keys::onContext1Pressed(KeyEvent event)

    This handler is called when the Context1 key has been pressed. The \a event
    parameter provides information about the event.
*/

/*!
    \qmlsignal Keys::onContext2Pressed(KeyEvent event)

    This handler is called when the Context2 key has been pressed. The \a event
    parameter provides information about the event.
*/

/*!
    \qmlsignal Keys::onContext3Pressed(KeyEvent event)

    This handler is called when the Context3 key has been pressed. The \a event
    parameter provides information about the event.
*/

/*!
    \qmlsignal Keys::onContext4Pressed(KeyEvent event)

    This handler is called when the Context4 key has been pressed. The \a event
    parameter provides information about the event.
*/

/*!
    \qmlsignal Keys::onCallPressed(KeyEvent event)

    This handler is called when the Call key has been pressed. The \a event
    parameter provides information about the event.
*/

/*!
    \qmlsignal Keys::onHangupPressed(KeyEvent event)

    This handler is called when the Hangup key has been pressed. The \a event
    parameter provides information about the event.
*/

/*!
    \qmlsignal Keys::onFlipPressed(KeyEvent event)

    This handler is called when the Flip key has been pressed. The \a event
    parameter provides information about the event.
*/

/*!
    \qmlsignal Keys::onMenuPressed(KeyEvent event)

    This handler is called when the Menu key has been pressed. The \a event
    parameter provides information about the event.
*/

/*!
    \qmlsignal Keys::onVolumeUpPressed(KeyEvent event)

    This handler is called when the VolumeUp key has been pressed. The \a event
    parameter provides information about the event.
*/

/*!
    \qmlsignal Keys::onVolumeDownPressed(KeyEvent event)

    This handler is called when the VolumeDown key has been pressed. The \a event
    parameter provides information about the event.
*/

const QDeclarativeKeysAttached::SigMap QDeclarativeKeysAttached::sigMap[] = {
   { Qt::Key_Left, "leftPressed" },
   { Qt::Key_Right, "rightPressed" },
   { Qt::Key_Up, "upPressed" },
   { Qt::Key_Down, "downPressed" },
   { Qt::Key_Tab, "tabPressed" },
   { Qt::Key_Backtab, "backtabPressed" },
   { Qt::Key_Asterisk, "asteriskPressed" },
   { Qt::Key_NumberSign, "numberSignPressed" },
   { Qt::Key_Escape, "escapePressed" },
   { Qt::Key_Return, "returnPressed" },
   { Qt::Key_Enter, "enterPressed" },
   { Qt::Key_Delete, "deletePressed" },
   { Qt::Key_Space, "spacePressed" },
   { Qt::Key_Back, "backPressed" },
   { Qt::Key_Cancel, "cancelPressed" },
   { Qt::Key_Select, "selectPressed" },
   { Qt::Key_Yes, "yesPressed" },
   { Qt::Key_No, "noPressed" },
   { Qt::Key_Context1, "context1Pressed" },
   { Qt::Key_Context2, "context2Pressed" },
   { Qt::Key_Context3, "context3Pressed" },
   { Qt::Key_Context4, "context4Pressed" },
   { Qt::Key_Call, "callPressed" },
   { Qt::Key_Hangup, "hangupPressed" },
   { Qt::Key_Flip, "flipPressed" },
   { Qt::Key_Menu, "menuPressed" },
   { Qt::Key_VolumeUp, "volumeUpPressed" },
   { Qt::Key_VolumeDown, "volumeDownPressed" },
   { 0, 0 }
};

bool QDeclarativeKeysAttachedPrivate::isConnected(const char *signalName)
{
   return isSignalConnected(signalIndex(signalName));
}

QDeclarativeKeysAttached::QDeclarativeKeysAttached(QObject *parent)
   : QObject(*(new QDeclarativeKeysAttachedPrivate), parent),
     QDeclarativeItemKeyFilter(qobject_cast<QDeclarativeItem *>(parent))
{
   Q_D(QDeclarativeKeysAttached);
   m_processPost = false;
   d->item = qobject_cast<QDeclarativeItem *>(parent);
}

QDeclarativeKeysAttached::~QDeclarativeKeysAttached()
{
}

QDeclarativeKeysAttached::Priority QDeclarativeKeysAttached::priority() const
{
   return m_processPost ? AfterItem : BeforeItem;
}

void QDeclarativeKeysAttached::setPriority(Priority order)
{
   bool processPost = order == AfterItem;
   if (processPost != m_processPost) {
      m_processPost = processPost;
      emit priorityChanged();
   }
}

void QDeclarativeKeysAttached::componentComplete()
{
   Q_D(QDeclarativeKeysAttached);
   if (d->item) {
      for (int ii = 0; ii < d->targets.count(); ++ii) {
         QGraphicsItem *targetItem = d->finalFocusProxy(d->targets.at(ii));
         if (targetItem && (targetItem->flags() & QGraphicsItem::ItemAcceptsInputMethod)) {
            d->item->setFlag(QGraphicsItem::ItemAcceptsInputMethod);
            break;
         }
      }
   }
}

void QDeclarativeKeysAttached::keyPressed(QKeyEvent *event, bool post)
{
   Q_D(QDeclarativeKeysAttached);
   if (post != m_processPost || !d->enabled || d->inPress) {
      event->ignore();
      QDeclarativeItemKeyFilter::keyPressed(event, post);
      return;
   }

   // first process forwards
   if (d->item && d->item->scene()) {
      d->inPress = true;
      for (int ii = 0; ii < d->targets.count(); ++ii) {
         QGraphicsItem *i = d->finalFocusProxy(d->targets.at(ii));
         if (i && i->isVisible()) {
            d->item->scene()->sendEvent(i, event);
            if (event->isAccepted()) {
               d->inPress = false;
               return;
            }
         }
      }
      d->inPress = false;
   }

   QDeclarativeKeyEvent ke(*event);
   QByteArray keySignal = keyToSignal(event->key());
   if (!keySignal.isEmpty()) {
      keySignal += "(QDeclarativeKeyEvent*)";
      if (d->isConnected(keySignal)) {
         // If we specifically handle a key then default to accepted
         ke.setAccepted(true);
         int idx = QDeclarativeKeysAttached::staticMetaObject.indexOfSignal(keySignal);
         metaObject()->method(idx).invoke(this, Qt::DirectConnection, Q_ARG(QDeclarativeKeyEvent *, &ke));
      }
   }
   if (!ke.isAccepted()) {
      emit pressed(&ke);
   }
   event->setAccepted(ke.isAccepted());

   if (!event->isAccepted()) {
      QDeclarativeItemKeyFilter::keyPressed(event, post);
   }
}

void QDeclarativeKeysAttached::keyReleased(QKeyEvent *event, bool post)
{
   Q_D(QDeclarativeKeysAttached);
   if (post != m_processPost || !d->enabled || d->inRelease) {
      event->ignore();
      QDeclarativeItemKeyFilter::keyReleased(event, post);
      return;
   }

   if (d->item && d->item->scene()) {
      d->inRelease = true;
      for (int ii = 0; ii < d->targets.count(); ++ii) {
         QGraphicsItem *i = d->finalFocusProxy(d->targets.at(ii));
         if (i && i->isVisible()) {
            d->item->scene()->sendEvent(i, event);
            if (event->isAccepted()) {
               d->inRelease = false;
               return;
            }
         }
      }
      d->inRelease = false;
   }

   QDeclarativeKeyEvent ke(*event);
   emit released(&ke);
   event->setAccepted(ke.isAccepted());

   if (!event->isAccepted()) {
      QDeclarativeItemKeyFilter::keyReleased(event, post);
   }
}

void QDeclarativeKeysAttached::inputMethodEvent(QInputMethodEvent *event, bool post)
{
   Q_D(QDeclarativeKeysAttached);
   if (post == m_processPost && d->item && !d->inIM && d->item->scene()) {
      d->inIM = true;
      for (int ii = 0; ii < d->targets.count(); ++ii) {
         QGraphicsItem *i = d->finalFocusProxy(d->targets.at(ii));
         if (i && i->isVisible() && (i->flags() & QGraphicsItem::ItemAcceptsInputMethod)) {
            d->item->scene()->sendEvent(i, event);
            if (event->isAccepted()) {
               d->imeItem = i;
               d->inIM = false;
               return;
            }
         }
      }
      d->inIM = false;
   }
   if (!event->isAccepted()) {
      QDeclarativeItemKeyFilter::inputMethodEvent(event, post);
   }
}

class QDeclarativeItemAccessor : public QGraphicsItem
{
 public:
   QVariant doInputMethodQuery(Qt::InputMethodQuery query) const {
      return QGraphicsItem::inputMethodQuery(query);
   }
};

QVariant QDeclarativeKeysAttached::inputMethodQuery(Qt::InputMethodQuery query) const
{
   Q_D(const QDeclarativeKeysAttached);
   if (d->item) {
      for (int ii = 0; ii < d->targets.count(); ++ii) {
         QGraphicsItem *i = d->finalFocusProxy(d->targets.at(ii));
         if (i && i->isVisible() && (i->flags() & QGraphicsItem::ItemAcceptsInputMethod) &&
               i == d->imeItem) { //### how robust is i == d->imeItem check?
            QVariant v = static_cast<QDeclarativeItemAccessor *>(i)->doInputMethodQuery(query);
            if (v.userType() == QVariant::RectF) {
               v = d->item->mapRectFromItem(i, v.toRectF());   //### cost?
            }
            return v;
         }
      }
   }
   return QDeclarativeItemKeyFilter::inputMethodQuery(query);
}

QDeclarativeKeysAttached *QDeclarativeKeysAttached::qmlAttachedProperties(QObject *obj)
{
   return new QDeclarativeKeysAttached(obj);
}

/*!
    \class QDeclarativeItem
    \since 4.7
    \brief The QDeclarativeItem class provides the most basic of all visual items in QML.

    All visual items in Qt Declarative inherit from QDeclarativeItem.  Although QDeclarativeItem
    has no visual appearance, it defines all the properties that are
    common across visual items - such as the x and y position, the
    width and height, \l {anchor-layout}{anchoring} and key handling.

    You can subclass QDeclarativeItem to provide your own custom visual item that inherits
    these features. Note that, because it does not draw anything, QDeclarativeItem sets the
    QGraphicsItem::ItemHasNoContents flag. If you subclass QDeclarativeItem to create a visual
    item, you will need to unset this flag.

*/

/*!
    \qmlclass Item QDeclarativeItem
    \ingroup qml-basic-visual-elements
    \since 4.7
    \brief The Item is the most basic of all visual items in QML.

    All visual items in Qt Declarative inherit from Item.  Although Item
    has no visual appearance, it defines all the properties that are
    common across visual items - such as the x and y position, the
    width and height, \l {anchor-layout}{anchoring} and key handling.

    Item is also useful for grouping items together.

    \qml
    Item {
        Image {
            source: "tile.png"
        }
        Image {
            x: 80
            width: 100
            height: 100
            source: "tile.png"
        }
        Image {
            x: 190
            width: 100
            height: 100
            fillMode: Image.Tile
            source: "tile.png"
        }
    }
    \endqml


    \section1 Key Handling

    Key handling is available to all Item-based visual elements via the \l {Keys}{Keys}
    attached property.  The \e Keys attached property provides basic handlers such
    as \l {Keys::onPressed}{onPressed} and \l {Keys::onReleased}{onReleased},
    as well as handlers for specific keys, such as
    \l {Keys::onCancelPressed}{onCancelPressed}.  The example below
    assigns \l {qmlfocus}{focus} to the item and handles
    the Left key via the general \e onPressed handler and the Select key via the
    onSelectPressed handler:

    \qml
    Item {
        focus: true
        Keys.onPressed: {
            if (event.key == Qt.Key_Left) {
                console.log("move left");
                event.accepted = true;
            }
        }
        Keys.onSelectPressed: console.log("Selected");
    }
    \endqml

    See the \l {Keys}{Keys} attached property for detailed documentation.

    \section1 Layout Mirroring

    Item layouts can be mirrored using the \l {LayoutMirroring}{LayoutMirroring} attached property.

*/

/*!
    \fn void QDeclarativeItem::childrenRectChanged(const QRectF &)
    \internal
*/

/*!
    \fn void QDeclarativeItem::baselineOffsetChanged(qreal)
    \internal
*/

/*!
    \fn void QDeclarativeItem::stateChanged(const QString &state)
    \internal
*/

/*!
    \fn void QDeclarativeItem::parentChanged(QDeclarativeItem *)
    \internal
*/

/*!
    \fn void QDeclarativeItem::smoothChanged(bool)
    \internal
*/

/*!
    \fn void QDeclarativeItem::clipChanged(bool)
    \internal
*/

/*! \fn void QDeclarativeItem::transformOriginChanged(TransformOrigin)
  \internal
*/

/*!
    \fn void QDeclarativeItem::focusChanged(bool)
    \internal
*/

/*!
    \fn void QDeclarativeItem::activeFocusChanged(bool)
    \internal
*/

// ### Must fix
struct RegisterAnchorLineAtStartup {
   RegisterAnchorLineAtStartup() {
      qRegisterMetaType<QDeclarativeAnchorLine>("QDeclarativeAnchorLine");
   }
};
static RegisterAnchorLineAtStartup registerAnchorLineAtStartup;


/*!
    \fn QDeclarativeItem::QDeclarativeItem(QDeclarativeItem *parent)

    Constructs a QDeclarativeItem with the given \a parent.
*/
QDeclarativeItem::QDeclarativeItem(QDeclarativeItem *parent)
   : QGraphicsObject(*(new QDeclarativeItemPrivate), parent, 0)
{
   Q_D(QDeclarativeItem);
   d->init(parent);
}

/*! \internal
*/
QDeclarativeItem::QDeclarativeItem(QDeclarativeItemPrivate &dd, QDeclarativeItem *parent)
   : QGraphicsObject(dd, parent, 0)
{
   Q_D(QDeclarativeItem);
   d->init(parent);
}

/*!
    Destroys the QDeclarativeItem.
*/
QDeclarativeItem::~QDeclarativeItem()
{
   Q_D(QDeclarativeItem);
   for (int ii = 0; ii < d->changeListeners.count(); ++ii) {
      QDeclarativeAnchorsPrivate *anchor = d->changeListeners.at(ii).listener->anchorPrivate();
      if (anchor) {
         anchor->clearItem(this);
      }
   }
   if (!d->parent || (parentItem() && !parentItem()->QGraphicsItem::d_ptr->inDestructor)) {
      for (int ii = 0; ii < d->changeListeners.count(); ++ii) {
         QDeclarativeAnchorsPrivate *anchor = d->changeListeners.at(ii).listener->anchorPrivate();
         if (anchor && anchor->item && anchor->item->parentItem() != this) { //child will be deleted anyway
            anchor->updateOnComplete();
         }
      }
   }
   for (int ii = 0; ii < d->changeListeners.count(); ++ii) {
      const QDeclarativeItemPrivate::ChangeListener &change = d->changeListeners.at(ii);
      if (change.types & QDeclarativeItemPrivate::Destroyed) {
         change.listener->itemDestroyed(this);
      }
   }
   d->changeListeners.clear();
   delete d->_anchorLines;
   d->_anchorLines = 0;
   delete d->_anchors;
   d->_anchors = 0;
   delete d->_stateGroup;
   d->_stateGroup = 0;
   delete d->_contents;
   d->_contents = 0;
}

/*!
    \qmlproperty enumeration Item::transformOrigin
    This property holds the origin point around which scale and rotation transform.

    Nine transform origins are available, as shown in the image below.

    \image declarative-transformorigin.png

    This example rotates an image around its bottom-right corner.
    \qml
    Image {
        source: "myimage.png"
        transformOrigin: Item.BottomRight
        rotation: 45
    }
    \endqml

    The default transform origin is \c Item.Center.

    To set an arbitrary transform origin point use the \l Scale or \l Rotation
    transform elements.
*/

/*!
    \qmlproperty Item Item::parent
    This property holds the parent of the item.
*/

/*!
    \property QDeclarativeItem::parent
    This property holds the parent of the item.
*/
void QDeclarativeItem::setParentItem(QDeclarativeItem *parent)
{
   QGraphicsObject::setParentItem(parent);
}

/*!
    Returns the QDeclarativeItem parent of this item.
*/
QDeclarativeItem *QDeclarativeItem::parentItem() const
{
   return qobject_cast<QDeclarativeItem *>(QGraphicsObject::parentItem());
}

/*!
    \qmlproperty real Item::childrenRect.x
    \qmlproperty real Item::childrenRect.y
    \qmlproperty real Item::childrenRect.width
    \qmlproperty real Item::childrenRect.height

    The childrenRect properties allow an item access to the geometry of its
    children. This property is useful if you have an item that needs to be
    sized to fit its children.
*/


/*!
    \qmlproperty list<Item> Item::children
    \qmlproperty list<Object> Item::resources

    The children property contains the list of visual children of this item.
    The resources property contains non-visual resources that you want to
    reference by name.

    Generally you can rely on Item's default property to handle all this for
    you, but it can come in handy in some cases.

    \qml
    Item {
        children: [
            Text {},
            Rectangle {}
        ]
        resources: [
            Component {
                id: myComponent
                Text {}
            }
        ]
    }
    \endqml
*/

/*!
    Returns true if construction of the QML component is complete; otherwise
    returns false.

    It is often desirable to delay some processing until the component is
    completed.

    \sa componentComplete()
*/
bool QDeclarativeItem::isComponentComplete() const
{
   Q_D(const QDeclarativeItem);
   return d->componentComplete;
}

void QDeclarativeItemPrivate::data_append(QDeclarativeListProperty<QObject> *prop, QObject *o)
{
   if (!o) {
      return;
   }

   QDeclarativeItem *that = static_cast<QDeclarativeItem *>(prop->object);

   // This test is measurably (albeit only slightly) faster than qobject_cast<>()
   const QMetaObject *mo = o->metaObject();
   while (mo && mo != &QGraphicsObject::staticMetaObject) {
      mo = mo->d.superdata;
   }

   if (mo) {
      QGraphicsObject *graphicsObject = static_cast<QGraphicsObject *>(o);
      QDeclarativeItemPrivate *contentItemPrivate = static_cast<QDeclarativeItemPrivate *>(QGraphicsItemPrivate::get(
               graphicsObject));
      if (contentItemPrivate->componentComplete) {
         graphicsObject->setParentItem(that);
      } else {
         contentItemPrivate->setParentItemHelper(that, /*newParentVariant=*/0, /*thisPointerVariant=*/0);
      }
   } else {
      o->setParent(that);
   }
}

static inline int children_count_helper(QDeclarativeListProperty<QObject> *prop)
{
   QGraphicsItemPrivate *d = QGraphicsItemPrivate::get(static_cast<QGraphicsObject *>(prop->object));
   return d->children.count();
}

static inline QObject *children_at_helper(QDeclarativeListProperty<QObject> *prop, int index)
{
   QGraphicsItemPrivate *d = QGraphicsItemPrivate::get(static_cast<QGraphicsObject *>(prop->object));
   if (index >= 0 && index < d->children.count()) {
      return d->children.at(index)->toGraphicsObject();
   } else {
      return 0;
   }
}

static inline void children_clear_helper(QDeclarativeListProperty<QObject> *prop)
{
   QDeclarativeItemPrivate *d = static_cast<QDeclarativeItemPrivate *>(QGraphicsItemPrivate::get(
                                   static_cast<QGraphicsObject *>(prop->object)));
   int childCount = d->children.count();
   if (d->componentComplete) {
      for (int index = 0 ; index < childCount; index++) {
         d->children.at(0)->setParentItem(0);
      }
   } else {
      for (int index = 0 ; index < childCount; index++) {
         QGraphicsItemPrivate::get(d->children.at(0))->setParentItemHelper(0, /*newParentVariant=*/0, /*thisPointerVariant=*/0);
      }
   }
}

int QDeclarativeItemPrivate::data_count(QDeclarativeListProperty<QObject> *prop)
{
   return resources_count(prop) + children_count_helper(prop);
}

QObject *QDeclarativeItemPrivate::data_at(QDeclarativeListProperty<QObject> *prop, int i)
{
   int resourcesCount = resources_count(prop);
   if (i < resourcesCount) {
      return resources_at(prop, i);
   }
   const int j = i - resourcesCount;
   if (j < children_count_helper(prop)) {
      return children_at_helper(prop, j);
   }
   return 0;
}

void QDeclarativeItemPrivate::data_clear(QDeclarativeListProperty<QObject> *prop)
{
   resources_clear(prop);
   children_clear_helper(prop);
}

QObject *QDeclarativeItemPrivate::resources_at(QDeclarativeListProperty<QObject> *prop, int index)
{
   const QObjectList children = prop->object->children();
   if (index < children.count()) {
      return children.at(index);
   } else {
      return 0;
   }
}

void QDeclarativeItemPrivate::resources_append(QDeclarativeListProperty<QObject> *prop, QObject *o)
{
   o->setParent(prop->object);
}

int QDeclarativeItemPrivate::resources_count(QDeclarativeListProperty<QObject> *prop)
{
   return prop->object->children().count();
}

void QDeclarativeItemPrivate::resources_clear(QDeclarativeListProperty<QObject> *prop)
{
   const QObjectList children = prop->object->children();
   for (int index = 0; index < children.count(); index++) {
      children.at(index)->setParent(0);
   }
}

int QDeclarativeItemPrivate::transform_count(QDeclarativeListProperty<QGraphicsTransform> *list)
{
   QGraphicsObject *object = qobject_cast<QGraphicsObject *>(list->object);
   if (object) {
      QGraphicsItemPrivate *d = QGraphicsItemPrivate::get(object);
      return d->transformData ? d->transformData->graphicsTransforms.size() : 0;
   } else {
      return 0;
   }
}

void QDeclarativeItemPrivate::transform_append(QDeclarativeListProperty<QGraphicsTransform> *list,
      QGraphicsTransform *item)
{
   QGraphicsObject *object = qobject_cast<QGraphicsObject *>(list->object);
   if (object && item) { // QGraphicsItem applies the list in the wrong order, so we prepend.
      QGraphicsItemPrivate::get(object)->prependGraphicsTransform(item);
   }
}

QGraphicsTransform *QDeclarativeItemPrivate::transform_at(QDeclarativeListProperty<QGraphicsTransform> *list, int idx)
{
   QGraphicsObject *object = qobject_cast<QGraphicsObject *>(list->object);
   if (object) {
      QGraphicsItemPrivate *d = QGraphicsItemPrivate::get(object);
      if (!d->transformData) {
         return 0;
      }
      return d->transformData->graphicsTransforms.at(idx);
   } else {
      return 0;
   }
}

void QDeclarativeItemPrivate::transform_clear(QDeclarativeListProperty<QGraphicsTransform> *list)
{
   QGraphicsObject *object = qobject_cast<QGraphicsObject *>(list->object);
   if (object) {
      QGraphicsItemPrivate *d = QGraphicsItemPrivate::get(object);
      if (!d->transformData) {
         return;
      }
      object->setTransformations(QList<QGraphicsTransform *>());
   }
}

void QDeclarativeItemPrivate::parentProperty(QObject *o, void *rv, QDeclarativeNotifierEndpoint *e)
{
   QDeclarativeItem *item = static_cast<QDeclarativeItem *>(o);
   if (e) {
      e->connect(&item->d_func()->parentNotifier);
   }
   *((QDeclarativeItem **)rv) = item->parentItem();
}

/*!
    \qmlproperty list<Object> Item::data
    \default

    The data property allows you to freely mix visual children and resources
    in an item.  If you assign a visual item to the data list it becomes
    a child and if you assign any other object type, it is added as a resource.

    So you can write:
    \qml
    Item {
        Text {}
        Rectangle {}
        Timer {}
    }
    \endqml

    instead of:
    \qml
    Item {
        children: [
            Text {},
            Rectangle {}
        ]
        resources: [
            Timer {}
        ]
    }
    \endqml

    data is a behind-the-scenes property: you should never need to explicitly
    specify it.
 */

QDeclarativeListProperty<QObject> QDeclarativeItemPrivate::data()
{
   return QDeclarativeListProperty<QObject>(q_func(), 0, QDeclarativeItemPrivate::data_append,
          QDeclarativeItemPrivate::data_count,
          QDeclarativeItemPrivate::data_at,
          QDeclarativeItemPrivate::data_clear
                                           );
}

/*!
    \property QDeclarativeItem::childrenRect
    \brief The geometry of an item's children.

    This property holds the (collective) position and size of the item's children.
*/
QRectF QDeclarativeItem::childrenRect()
{
   Q_D(QDeclarativeItem);
   if (!d->_contents) {
      d->_contents = new QDeclarativeContents(this);
      if (d->componentComplete) {
         d->_contents->complete();
      }
   }
   return d->_contents->rectF();
}

bool QDeclarativeItem::clip() const
{
   return flags() & ItemClipsChildrenToShape;
}

void QDeclarativeItem::setClip(bool c)
{
   if (clip() == c) {
      return;
   }
   setFlag(ItemClipsChildrenToShape, c);
   emit clipChanged(c);
}

/*!
  \qmlproperty real Item::x
  \qmlproperty real Item::y
  \qmlproperty real Item::width
  \qmlproperty real Item::height

  Defines the item's position and size relative to its parent.

  \qml
  Item { x: 100; y: 100; width: 100; height: 100 }
  \endqml
 */

/*!
  \qmlproperty real Item::z

  Sets the stacking order of sibling items.  By default the stacking order is 0.

  Items with a higher stacking value are drawn on top of siblings with a
  lower stacking order.  Items with the same stacking value are drawn
  bottom up in the order they appear.  Items with a negative stacking
  value are drawn under their parent's content.

  The following example shows the various effects of stacking order.

  \table
  \row
  \o \image declarative-item_stacking1.png
  \o Same \c z - later children above earlier children:
  \qml
  Item {
      Rectangle {
          color: "red"
          width: 100; height: 100
      }
      Rectangle {
          color: "blue"
          x: 50; y: 50; width: 100; height: 100
      }
  }
  \endqml
  \row
  \o \image declarative-item_stacking2.png
  \o Higher \c z on top:
  \qml
  Item {
      Rectangle {
          z: 1
          color: "red"
          width: 100; height: 100
      }
      Rectangle {
          color: "blue"
          x: 50; y: 50; width: 100; height: 100
      }
  }
  \endqml
  \row
  \o \image declarative-item_stacking3.png
  \o Same \c z - children above parents:
  \qml
  Item {
      Rectangle {
          color: "red"
          width: 100; height: 100
          Rectangle {
              color: "blue"
              x: 50; y: 50; width: 100; height: 100
          }
      }
  }
  \endqml
  \row
  \o \image declarative-item_stacking4.png
  \o Lower \c z below:
  \qml
  Item {
      Rectangle {
          color: "red"
          width: 100; height: 100
          Rectangle {
              z: -1
              color: "blue"
              x: 50; y: 50; width: 100; height: 100
          }
      }
  }
  \endqml
  \endtable
 */

/*!
    \qmlproperty bool Item::visible

    This property holds whether the item is visible. By default this is true.

    Setting this property directly affects the \c visible value of child
    items. When set to \c false, the \c visible values of all child items also
    become \c false. When set to \c true, the \c visible values of child items
    are returned to \c true, unless they have explicitly been set to \c false.

    (Because of this flow-on behavior, using the \c visible property may not
    have the intended effect if a property binding should only respond to
    explicit property changes. In such cases it may be better to use the
    \l opacity property instead.)

    Setting this property to \c false automatically causes \l focus to be set
    to \c false, and this item will longer receive mouse and keyboard events.
    (In contrast, setting the \l opacity to 0 does not affect the \l focus
    property and the receiving of key events.)

    \note This property's value is only affected by changes to this property or
    the parent's \c visible property. It does not change, for example, if this
    item moves off-screen, or if the \l opacity changes to 0.
*/


/*!
  This function is called to handle this item's changes in
  geometry from \a oldGeometry to \a newGeometry. If the two
  geometries are the same, it doesn't do anything.
 */
void QDeclarativeItem::geometryChanged(const QRectF &newGeometry,
                                       const QRectF &oldGeometry)
{
   Q_D(QDeclarativeItem);

   if (d->_anchors) {
      d->_anchors->d_func()->updateMe();
   }

   if (transformOrigin() != QDeclarativeItem::TopLeft
         && (newGeometry.width() != oldGeometry.width() || newGeometry.height() != oldGeometry.height())) {
      if (d->transformData) {
         QPointF origin = d->computeTransformOrigin();
         if (transformOriginPoint() != origin) {
            setTransformOriginPoint(origin);
         }
      } else {
         d->transformOriginDirty = true;
      }
   }

   for (int ii = 0; ii < d->changeListeners.count(); ++ii) {
      const QDeclarativeItemPrivate::ChangeListener &change = d->changeListeners.at(ii);
      if (change.types & QDeclarativeItemPrivate::Geometry) {
         change.listener->itemGeometryChanged(this, newGeometry, oldGeometry);
      }
   }

   if (newGeometry.width() != oldGeometry.width()) {
      emit widthChanged();
   }
   if (newGeometry.height() != oldGeometry.height()) {
      emit heightChanged();
   }
}

void QDeclarativeItemPrivate::removeItemChangeListener(QDeclarativeItemChangeListener *listener, ChangeTypes types)
{
   ChangeListener change(listener, types);
   changeListeners.removeOne(change);
}

/*! \internal */
void QDeclarativeItem::keyPressEvent(QKeyEvent *event)
{
   Q_D(QDeclarativeItem);
   keyPressPreHandler(event);
   if (event->isAccepted()) {
      return;
   }
   if (d->keyHandler) {
      d->keyHandler->keyPressed(event, true);
   } else {
      event->ignore();
   }
}

/*! \internal */
void QDeclarativeItem::keyReleaseEvent(QKeyEvent *event)
{
   Q_D(QDeclarativeItem);
   keyReleasePreHandler(event);
   if (event->isAccepted()) {
      return;
   }
   if (d->keyHandler) {
      d->keyHandler->keyReleased(event, true);
   } else {
      event->ignore();
   }
}

/*! \internal */
void QDeclarativeItem::inputMethodEvent(QInputMethodEvent *event)
{
   Q_D(QDeclarativeItem);
   inputMethodPreHandler(event);
   if (event->isAccepted()) {
      return;
   }
   if (d->keyHandler) {
      d->keyHandler->inputMethodEvent(event, true);
   } else {
      event->ignore();
   }
}

/*! \internal */
QVariant QDeclarativeItem::inputMethodQuery(Qt::InputMethodQuery query) const
{
   Q_D(const QDeclarativeItem);
   QVariant v;
   if (d->keyHandler) {
      v = d->keyHandler->inputMethodQuery(query);
   }

   if (!v.isValid()) {
      v = QGraphicsObject::inputMethodQuery(query);
   }

   return v;
}

/*!
  \internal
 */
void QDeclarativeItem::keyPressPreHandler(QKeyEvent *event)
{
   Q_D(QDeclarativeItem);
   if (d->keyHandler && !d->doneEventPreHandler) {
      d->keyHandler->keyPressed(event, false);
   } else {
      event->ignore();
   }
   d->doneEventPreHandler = true;
}

/*!
  \internal
 */
void QDeclarativeItem::keyReleasePreHandler(QKeyEvent *event)
{
   Q_D(QDeclarativeItem);
   if (d->keyHandler && !d->doneEventPreHandler) {
      d->keyHandler->keyReleased(event, false);
   } else {
      event->ignore();
   }
   d->doneEventPreHandler = true;
}

/*!
  \internal
 */
void QDeclarativeItem::inputMethodPreHandler(QInputMethodEvent *event)
{
   Q_D(QDeclarativeItem);
   if (d->keyHandler && !d->doneEventPreHandler) {
      d->keyHandler->inputMethodEvent(event, false);
   } else {
      event->ignore();
   }
   d->doneEventPreHandler = true;
}

/*!
    \internal
*/
QDeclarativeAnchorLine QDeclarativeItemPrivate::left() const
{
   return anchorLines()->left;
}

/*!
    \internal
*/
QDeclarativeAnchorLine QDeclarativeItemPrivate::right() const
{
   return anchorLines()->right;
}

/*!
    \internal
*/
QDeclarativeAnchorLine QDeclarativeItemPrivate::horizontalCenter() const
{
   return anchorLines()->hCenter;
}

/*!
    \internal
*/
QDeclarativeAnchorLine QDeclarativeItemPrivate::top() const
{
   return anchorLines()->top;
}

/*!
    \internal
*/
QDeclarativeAnchorLine QDeclarativeItemPrivate::bottom() const
{
   return anchorLines()->bottom;
}

/*!
    \internal
*/
QDeclarativeAnchorLine QDeclarativeItemPrivate::verticalCenter() const
{
   return anchorLines()->vCenter;
}


/*!
    \internal
*/
QDeclarativeAnchorLine QDeclarativeItemPrivate::baseline() const
{
   return anchorLines()->baseline;
}

/*!
  \qmlproperty AnchorLine Item::anchors.top
  \qmlproperty AnchorLine Item::anchors.bottom
  \qmlproperty AnchorLine Item::anchors.left
  \qmlproperty AnchorLine Item::anchors.right
  \qmlproperty AnchorLine Item::anchors.horizontalCenter
  \qmlproperty AnchorLine Item::anchors.verticalCenter
  \qmlproperty AnchorLine Item::anchors.baseline

  \qmlproperty Item Item::anchors.fill
  \qmlproperty Item Item::anchors.centerIn

  \qmlproperty real Item::anchors.margins
  \qmlproperty real Item::anchors.topMargin
  \qmlproperty real Item::anchors.bottomMargin
  \qmlproperty real Item::anchors.leftMargin
  \qmlproperty real Item::anchors.rightMargin
  \qmlproperty real Item::anchors.horizontalCenterOffset
  \qmlproperty real Item::anchors.verticalCenterOffset
  \qmlproperty real Item::anchors.baselineOffset

  \qmlproperty bool Item::anchors.mirrored

  Anchors provide a way to position an item by specifying its
  relationship with other items.

  Margins apply to top, bottom, left, right, and fill anchors.
  The \c anchors.margins property can be used to set all of the various margins at once, to the same value.
  Note that margins are anchor-specific and are not applied if an item does not
  use anchors.

  Offsets apply for horizontal center, vertical center, and baseline anchors.

  \table
  \row
  \o \image declarative-anchors_example.png
  \o Text anchored to Image, horizontally centered and vertically below, with a margin.
  \qml
  Item {
      Image {
          id: pic
          // ...
      }
      Text {
          id: label
          anchors.horizontalCenter: pic.horizontalCenter
          anchors.top: pic.bottom
          anchors.topMargin: 5
          // ...
      }
  }
  \endqml
  \row
  \o \image declarative-anchors_example2.png
  \o
  Left of Text anchored to right of Image, with a margin. The y
  property of both defaults to 0.

  \qml
  Item {
      Image {
          id: pic
          // ...
      }
      Text {
          id: label
          anchors.left: pic.right
          anchors.leftMargin: 5
          // ...
      }
  }
  \endqml
  \endtable

  \c anchors.fill provides a convenient way for one item to have the
  same geometry as another item, and is equivalent to connecting all
  four directional anchors.

  To clear an anchor value, set it to \c undefined.

  \c anchors.mirrored returns true it the layout has been \l {LayoutMirroring}{mirrored}.

  \note You can only anchor an item to siblings or a parent.

  For more information see \l {anchor-layout}{Anchor Layouts}.
*/

/*!
  \property QDeclarativeItem::baselineOffset
  \brief The position of the item's baseline in local coordinates.

  The baseline of a \l Text item is the imaginary line on which the text
  sits. Controls containing text usually set their baseline to the
  baseline of their text.

  For non-text items, a default baseline offset of 0 is used.
*/
qreal QDeclarativeItem::baselineOffset() const
{
   Q_D(const QDeclarativeItem);
   if (!d->baselineOffset.isValid()) {
      return 0.0;
   } else {
      return d->baselineOffset;
   }
}

void QDeclarativeItem::setBaselineOffset(qreal offset)
{
   Q_D(QDeclarativeItem);
   if (offset == d->baselineOffset) {
      return;
   }

   d->baselineOffset = offset;

   for (int ii = 0; ii < d->changeListeners.count(); ++ii) {
      const QDeclarativeItemPrivate::ChangeListener &change = d->changeListeners.at(ii);
      if (change.types & QDeclarativeItemPrivate::Geometry) {
         QDeclarativeAnchorsPrivate *anchor = change.listener->anchorPrivate();
         if (anchor) {
            anchor->updateVerticalAnchors();
         }
      }
   }
   emit baselineOffsetChanged(offset);
}

/*!
  \qmlproperty real Item::rotation
  This property holds the rotation of the item in degrees clockwise.

  This specifies how many degrees to rotate the item around its transformOrigin.
  The default rotation is 0 degrees (i.e. not rotated at all).

  \table
  \row
  \o \image declarative-rotation.png
  \o
  \qml
  Rectangle {
      color: "blue"
      width: 100; height: 100
      Rectangle {
          color: "red"
          x: 25; y: 25; width: 50; height: 50
          rotation: 30
      }
  }
  \endqml
  \endtable

  \sa transform, Rotation
*/

/*!
  \qmlproperty real Item::scale
  This property holds the scale of the item.

  A scale of less than 1 means the item will be displayed smaller than
  normal, and a scale of greater than 1 means the item will be
  displayed larger than normal.  A negative scale means the item will
  be mirrored.

  By default, items are displayed at a scale of 1 (i.e. at their
  normal size).

  Scaling is from the item's transformOrigin.

  \table
  \row
  \o \image declarative-scale.png
  \o
  \qml
  Rectangle {
      color: "blue"
      width: 100; height: 100
      Rectangle {
          color: "green"
          width: 25; height: 25
      }
      Rectangle {
          color: "red"
          x: 25; y: 25; width: 50; height: 50
          scale: 1.4
      }
  }
  \endqml
  \endtable

  \sa transform, Scale
*/

/*!
  \qmlproperty real Item::opacity

  This property holds the opacity of the item.  Opacity is specified as a
  number between 0 (fully transparent) and 1 (fully opaque).  The default is 1.

  When this property is set, the specified opacity is also applied
  individually to child items.  In almost all cases this is what you want,
  but in some cases it may produce undesired results. For example in the
  second set of rectangles below, the red rectangle has specified an opacity
  of 0.5, which affects the opacity of its blue child rectangle even though
  the child has not specified an opacity.

  \table
  \row
  \o \image declarative-item_opacity1.png
  \o
  \qml
    Item {
        Rectangle {
            color: "red"
            width: 100; height: 100
            Rectangle {
                color: "blue"
                x: 50; y: 50; width: 100; height: 100
            }
        }
    }
  \endqml
  \row
  \o \image declarative-item_opacity2.png
  \o
  \qml
    Item {
        Rectangle {
            opacity: 0.5
            color: "red"
            width: 100; height: 100
            Rectangle {
                color: "blue"
                x: 50; y: 50; width: 100; height: 100
            }
        }
    }
  \endqml
  \endtable

  If an item's opacity is set to 0, the item will no longer receive mouse
  events, but will continue to receive key events and will retain the keyboard
  \l focus if it has been set. (In contrast, setting the \l visible property
  to \c false stops both mouse and keyboard events, and also removes focus
  from the item.)
*/

/*!
  Returns a value indicating whether mouse input should
  remain with this item exclusively.

  \sa setKeepMouseGrab()
 */
bool QDeclarativeItem::keepMouseGrab() const
{
   Q_D(const QDeclarativeItem);
   return d->keepMouse;
}

/*!
  The flag indicating whether the mouse should remain
  with this item is set to \a keep.

  This is useful for items that wish to grab and keep mouse
  interaction following a predefined gesture.  For example,
  an item that is interested in horizontal mouse movement
  may set keepMouseGrab to true once a threshold has been
  exceeded.  Once keepMouseGrab has been set to true, filtering
  items will not react to mouse events.

  If the item does not indicate that it wishes to retain mouse grab,
  a filtering item may steal the grab. For example, Flickable may attempt
  to steal a mouse grab if it detects that the user has begun to
  move the viewport.

  \sa keepMouseGrab()
 */
void QDeclarativeItem::setKeepMouseGrab(bool keep)
{
   Q_D(QDeclarativeItem);
   d->keepMouse = keep;
}

/*!
    \qmlmethod object Item::mapFromItem(Item item, real x, real y)

    Maps the point (\a x, \a y), which is in \a item's coordinate system, to
    this item's coordinate system, and returns an object with \c x and \c y
    properties matching the mapped cooordinate.

    If \a item is a \c null value, this maps the point from the coordinate
    system of the root QML view.
*/

/*!
    Maps the point (\a x, \a y), which is in \a item's coordinate system, to
    this item's coordinate system, and returns a script value with \c x and \c y
    properties matching the mapped cooordinate.

    If \a item is a \c null value, this maps the point from the coordinate
    system of the root QML view.

    \sa Item::mapFromItem()
*/
QScriptValue QDeclarativeItem::mapFromItem(const QScriptValue &item, qreal x, qreal y) const
{
   QDeclarativeItem *itemObj = qobject_cast<QDeclarativeItem *>(item.toQObject());
   if (!itemObj && !item.isNull()) {
      qmlInfo(this) << "mapFromItem() given argument \"" << item.toString() << "\" which is neither null nor an Item";
      return 0;
   }

   // If QGraphicsItem::mapFromItem() is called with 0, behaves the same as mapFromScene()
   QPointF p = qobject_cast<QGraphicsItem *>(this)->mapFromItem(itemObj, x, y);

   // Use the script engine from the passed item, if available. Use this item's one otherwise.
   QScriptEngine *const se = itemObj ? item.engine() : QDeclarativeEnginePrivate::getScriptEngine(qmlEngine(this));

   // Engine-less items are unlikely, but nevertheless possible. Handle them.
   if (0 == se) {
      return QScriptValue(QScriptValue::UndefinedValue);
   }

   QScriptValue sv = se->newObject();
   sv.setProperty(QLatin1String("x"), p.x());
   sv.setProperty(QLatin1String("y"), p.y());
   return sv;
}

/*!
    \qmlmethod object Item::mapToItem(Item item, real x, real y)

    Maps the point (\a x, \a y), which is in this item's coordinate system, to
    \a item's coordinate system, and returns an object with \c x and \c y
    properties matching the mapped cooordinate.

    If \a item is a \c null value, this maps \a x and \a y to the coordinate
    system of the root QML view.
*/

/*!
    Maps the point (\a x, \a y), which is in this item's coordinate system, to
    \a item's coordinate system, and returns a script value with \c x and \c y
    properties matching the mapped cooordinate.

    If \a item is a \c null value, this maps \a x and \a y to the coordinate
    system of the root QML view.

    \sa Item::mapToItem()
*/
QScriptValue QDeclarativeItem::mapToItem(const QScriptValue &item, qreal x, qreal y) const
{
   QDeclarativeItem *itemObj = qobject_cast<QDeclarativeItem *>(item.toQObject());
   if (!itemObj && !item.isNull()) {
      qmlInfo(this) << "mapToItem() given argument \"" << item.toString() << "\" which is neither null nor an Item";
      return 0;
   }

   // If QGraphicsItem::mapToItem() is called with 0, behaves the same as mapToScene()
   QPointF p = qobject_cast<QGraphicsItem *>(this)->mapToItem(itemObj, x, y);

   // Use the script engine from the passed item, if available. Use this item's one otherwise.
   QScriptEngine *const se = itemObj ? item.engine() : QDeclarativeEnginePrivate::getScriptEngine(qmlEngine(this));

   // Engine-less items are unlikely, but nevertheless possible. Handle them.
   if (0 == se) {
      return QScriptValue(QScriptValue::UndefinedValue);
   }

   QScriptValue sv = se->newObject();
   sv.setProperty(QLatin1String("x"), p.x());
   sv.setProperty(QLatin1String("y"), p.y());
   return sv;
}

/*!
    \qmlmethod Item::forceActiveFocus()

    Forces active focus on the item.

    This method sets focus on the item and makes sure that all the focus scopes
    higher in the object hierarchy are also given the focus.
*/

/*!
    Forces active focus on the item.

    This method sets focus on the item and makes sure that all the focus scopes
    higher in the object hierarchy are also given the focus.
*/
void QDeclarativeItem::forceActiveFocus()
{
   setFocus(true);
   QGraphicsItem *parent = parentItem();
   while (parent) {
      if (parent->flags() & QGraphicsItem::ItemIsFocusScope) {
         parent->setFocus(Qt::OtherFocusReason);
      }
      parent = parent->parentItem();
   }
}


/*!
  \qmlmethod Item::childAt(real x, real y)

  Returns the visible child item at point (\a x, \a y), which is in this
  item's coordinate system, or \c null if there is no such item.
*/

/*!
  Returns the visible child item at point (\a x, \a y), which is in this
  item's coordinate system, or 0 if there is no such item.
*/
QDeclarativeItem *QDeclarativeItem::childAt(qreal x, qreal y) const
{
   const QList<QGraphicsItem *> children = childItems();
   for (int i = children.count() - 1; i >= 0; --i) {
      if (QDeclarativeItem *child = qobject_cast<QDeclarativeItem *>(children.at(i))) {
         if (child->isVisible() && child->x() <= x
               && child->x() + child->width() >= x
               && child->y() <= y
               && child->y() + child->height() >= y) {
            return child;
         }
      }
   }
   return 0;
}

void QDeclarativeItemPrivate::focusChanged(bool flag)
{
   Q_Q(QDeclarativeItem);

   if (hadActiveFocus != flag) {
      hadActiveFocus = flag;
      emit q->activeFocusChanged(flag);
   }

   QDeclarativeItem *focusItem = q;
   for (QDeclarativeItem *p = q->parentItem(); p; p = p->parentItem()) {
      if (p->flags() & QGraphicsItem::ItemIsFocusScope) {
         if (!flag && QGraphicsItemPrivate::get(p)->focusScopeItem != focusItem) {
            break;
         }
         if (p->d_func()->hadActiveFocus != flag) {
            p->d_func()->hadActiveFocus = flag;
            emit p->activeFocusChanged(flag);
         }
         focusItem = p;
      }
   }

   // For all but the top most focus scope/item this will be called for us by QGraphicsItem.
   focusItem->d_func()->focusScopeItemChange(flag);
}

QDeclarativeListProperty<QObject> QDeclarativeItemPrivate::resources()
{
   return QDeclarativeListProperty<QObject>(q_func(), 0, QDeclarativeItemPrivate::resources_append,
          QDeclarativeItemPrivate::resources_count,
          QDeclarativeItemPrivate::resources_at,
          QDeclarativeItemPrivate::resources_clear
                                           );
}

/*!
  \qmlproperty list<State> Item::states
  This property holds a list of states defined by the item.

  \qml
  Item {
      states: [
          State {
              // ...
          },
          State {
              // ...
          }
          // ...
      ]
  }
  \endqml

  \sa {qmlstate}{States}
*/

QDeclarativeListProperty<QDeclarativeState> QDeclarativeItemPrivate::states()
{
   return _states()->statesProperty();
}

/*!
  \qmlproperty list<Transition> Item::transitions
  This property holds a list of transitions defined by the item.

  \qml
  Item {
      transitions: [
          Transition {
              // ...
          },
          Transition {
              // ...
          }
          // ...
      ]
  }
  \endqml

  \sa {QML Animation and Transitions}{Transitions}
*/


QDeclarativeListProperty<QDeclarativeTransition> QDeclarativeItemPrivate::transitions()
{
   return _states()->transitionsProperty();
}

/*
  \qmlproperty list<Filter> Item::filter
  This property holds a list of graphical filters to be applied to the item.

  \l {Filter}{Filters} include things like \l {Blur}{blurring}
  the item, or giving it a \l Reflection.  Some
  filters may not be available on all canvases; if a filter is not
  available on a certain canvas, it will simply not be applied for
  that canvas (but the QML will still be considered valid).

  \qml
  Item {
      filter: [
          Blur {
              // ...
          },
          Reflection {
              // ...
          }
          // ...
      ]
  }
  \endqml
*/

/*!
  \qmlproperty bool Item::clip
  This property holds whether clipping is enabled. The default clip value is \c false.

  If clipping is enabled, an item will clip its own painting, as well
  as the painting of its children, to its bounding rectangle.

  Non-rectangular clipping regions are not supported for performance reasons.
*/

/*!
  \property QDeclarativeItem::clip
  This property holds whether clipping is enabled. The default clip value is \c false.

  If clipping is enabled, an item will clip its own painting, as well
  as the painting of its children, to its bounding rectangle. If you set
  clipping during an item's paint operation, remember to re-set it to
  prevent clipping the rest of your scene.

  Non-rectangular clipping regions are not supported for performance reasons.
*/

/*!
  \qmlproperty string Item::state

  This property holds the name of the current state of the item.

  This property is often used in scripts to change between states. For
  example:

  \js
  function toggle() {
      if (button.state == 'On')
          button.state = 'Off';
      else
          button.state = 'On';
  }
  \endjs

  If the item is in its base state (i.e. no explicit state has been
  set), \c state will be a blank string. Likewise, you can return an
  item to its base state by setting its current state to \c ''.

  \sa {qmlstates}{States}
*/

QString QDeclarativeItemPrivate::state() const
{
   if (!_stateGroup) {
      return QString();
   } else {
      return _stateGroup->state();
   }
}

void QDeclarativeItemPrivate::setState(const QString &state)
{
   _states()->setState(state);
}

/*!
  \qmlproperty list<Transform> Item::transform
  This property holds the list of transformations to apply.

  For more information see \l Transform.
*/

/*! \internal */
QDeclarativeListProperty<QGraphicsTransform> QDeclarativeItem::transform()
{
   Q_D(QDeclarativeItem);
   return QDeclarativeListProperty<QGraphicsTransform>(this, 0, d->transform_append, d->transform_count,
          d->transform_at, d->transform_clear);
}

/*!
  \internal

  classBegin() is called when the item is constructed, but its
  properties have not yet been set.

  \sa componentComplete(), isComponentComplete()
*/
void QDeclarativeItem::classBegin()
{
   Q_D(QDeclarativeItem);
   d->componentComplete = false;
   if (d->_stateGroup) {
      d->_stateGroup->classBegin();
   }
   if (d->_anchors) {
      d->_anchors->classBegin();
   }
}

/*!
  \internal

  componentComplete() is called when all items in the component
  have been constructed.  It is often desirable to delay some
  processing until the component is complete an all bindings in the
  component have been resolved.
*/
void QDeclarativeItem::componentComplete()
{
   Q_D(QDeclarativeItem);
   d->componentComplete = true;
   if (d->_stateGroup) {
      d->_stateGroup->componentComplete();
   }
   if (d->_anchors) {
      d->_anchors->componentComplete();
      d->_anchors->d_func()->updateOnComplete();
   }
   if (d->keyHandler) {
      d->keyHandler->componentComplete();
   }
   if (d->_contents) {
      d->_contents->complete();
   }
}

QDeclarativeStateGroup *QDeclarativeItemPrivate::_states()
{
   Q_Q(QDeclarativeItem);
   if (!_stateGroup) {
      _stateGroup = new QDeclarativeStateGroup;
      if (!componentComplete) {
         _stateGroup->classBegin();
      }
      QObject::connect(_stateGroup, SIGNAL(stateChanged(QString)),
                       q, SIGNAL(stateChanged(QString)));
   }

   return _stateGroup;
}

QDeclarativeItemPrivate::AnchorLines::AnchorLines(QGraphicsObject *q)
{
   left.item = q;
   left.anchorLine = QDeclarativeAnchorLine::Left;
   right.item = q;
   right.anchorLine = QDeclarativeAnchorLine::Right;
   hCenter.item = q;
   hCenter.anchorLine = QDeclarativeAnchorLine::HCenter;
   top.item = q;
   top.anchorLine = QDeclarativeAnchorLine::Top;
   bottom.item = q;
   bottom.anchorLine = QDeclarativeAnchorLine::Bottom;
   vCenter.item = q;
   vCenter.anchorLine = QDeclarativeAnchorLine::VCenter;
   baseline.item = q;
   baseline.anchorLine = QDeclarativeAnchorLine::Baseline;
}

QPointF QDeclarativeItemPrivate::computeTransformOrigin() const
{
   Q_Q(const QDeclarativeItem);

   QRectF br = q->boundingRect();

   switch (origin) {
      default:
      case QDeclarativeItem::TopLeft:
         return QPointF(0, 0);
      case QDeclarativeItem::Top:
         return QPointF(br.width() / 2., 0);
      case QDeclarativeItem::TopRight:
         return QPointF(br.width(), 0);
      case QDeclarativeItem::Left:
         return QPointF(0, br.height() / 2.);
      case QDeclarativeItem::Center:
         return QPointF(br.width() / 2., br.height() / 2.);
      case QDeclarativeItem::Right:
         return QPointF(br.width(), br.height() / 2.);
      case QDeclarativeItem::BottomLeft:
         return QPointF(0, br.height());
      case QDeclarativeItem::Bottom:
         return QPointF(br.width() / 2., br.height());
      case QDeclarativeItem::BottomRight:
         return QPointF(br.width(), br.height());
   }
}

/*! \internal */
bool QDeclarativeItem::sceneEvent(QEvent *event)
{
   Q_D(QDeclarativeItem);
   if (event->type() == QEvent::KeyPress) {
      QKeyEvent *k = static_cast<QKeyEvent *>(event);
      if ((k->key() == Qt::Key_Tab || k->key() == Qt::Key_Backtab) &&
            !(k->modifiers() & (Qt::ControlModifier | Qt::AltModifier))) {
         keyPressEvent(static_cast<QKeyEvent *>(event));
         if (!event->isAccepted()) {
            return QGraphicsItem::sceneEvent(event);
         } else {
            return true;
         }
      } else {
         return QGraphicsItem::sceneEvent(event);
      }
   } else {
      bool rv = QGraphicsItem::sceneEvent(event);

      if (event->type() == QEvent::FocusIn ||
            event->type() == QEvent::FocusOut) {
         d->focusChanged(hasActiveFocus());
      }
      return rv;
   }
}

/*!
    \internal

    Note that unlike QGraphicsItems, QDeclarativeItem::itemChange() is \e not called
    during initial widget polishing. Items wishing to optimize start-up construction
    should instead consider using componentComplete().
*/
QVariant QDeclarativeItem::itemChange(GraphicsItemChange change,
                                      const QVariant &value)
{
   Q_D(QDeclarativeItem);
   switch (change) {
      case ItemParentHasChanged:
         d->resolveLayoutMirror();
         emit parentChanged(parentItem());
         d->parentNotifier.notify();
         break;
      case ItemVisibleHasChanged: {
         for (int ii = 0; ii < d->changeListeners.count(); ++ii) {
            const QDeclarativeItemPrivate::ChangeListener &change = d->changeListeners.at(ii);
            if (change.types & QDeclarativeItemPrivate::Visibility) {
               change.listener->itemVisibilityChanged(this);
            }
         }
      }
      break;
      case ItemOpacityHasChanged: {
         for (int ii = 0; ii < d->changeListeners.count(); ++ii) {
            const QDeclarativeItemPrivate::ChangeListener &change = d->changeListeners.at(ii);
            if (change.types & QDeclarativeItemPrivate::Opacity) {
               change.listener->itemOpacityChanged(this);
            }
         }
      }
      break;
      case ItemChildAddedChange:
         if (d->_contents && d->componentComplete)
            d->_contents->childAdded(qobject_cast<QDeclarativeItem *>(
                                        value.value<QGraphicsItem *>()));
         break;
      case ItemChildRemovedChange:
         if (d->_contents && d->componentComplete)
            d->_contents->childRemoved(qobject_cast<QDeclarativeItem *>(
                                          value.value<QGraphicsItem *>()));
         break;
      default:
         break;
   }

   return QGraphicsItem::itemChange(change, value);
}

/*! \internal */
QRectF QDeclarativeItem::boundingRect() const
{
   Q_D(const QDeclarativeItem);
   return QRectF(0, 0, d->mWidth, d->mHeight);
}

/*!
    \enum QDeclarativeItem::TransformOrigin

    Controls the point about which simple transforms like scale apply.

    \value TopLeft The top-left corner of the item.
    \value Top The center point of the top of the item.
    \value TopRight The top-right corner of the item.
    \value Left The left most point of the vertical middle.
    \value Center The center of the item.
    \value Right The right most point of the vertical middle.
    \value BottomLeft The bottom-left corner of the item.
    \value Bottom The center point of the bottom of the item.
    \value BottomRight The bottom-right corner of the item.
*/

/*!
    Returns the current transform origin.
*/
QDeclarativeItem::TransformOrigin QDeclarativeItem::transformOrigin() const
{
   Q_D(const QDeclarativeItem);
   return d->origin;
}

/*!
    Set the transform \a origin.
*/
void QDeclarativeItem::setTransformOrigin(TransformOrigin origin)
{
   Q_D(QDeclarativeItem);
   if (origin != d->origin) {
      d->origin = origin;
      if (d->transformData) {
         QGraphicsItem::setTransformOriginPoint(d->computeTransformOrigin());
      } else {
         d->transformOriginDirty = true;
      }
      emit transformOriginChanged(d->origin);
   }
}

void QDeclarativeItemPrivate::transformChanged()
{
   Q_Q(QDeclarativeItem);
   if (transformOriginDirty) {
      q->QGraphicsItem::setTransformOriginPoint(computeTransformOrigin());
      transformOriginDirty = false;
   }
}

/*!
    \property QDeclarativeItem::smooth
    \brief whether the item is smoothly transformed.

    This property is provided purely for the purpose of optimization. Turning
    smooth transforms off is faster, but looks worse; turning smooth
    transformations on is slower, but looks better.

    By default smooth transformations are off.
*/

/*!
    Returns true if the item should be drawn with antialiasing and
    smooth pixmap filtering, false otherwise.

    The default is false.

    \sa setSmooth()
*/
bool QDeclarativeItem::smooth() const
{
   Q_D(const QDeclarativeItem);
   return d->smooth;
}

/*!
    Sets whether the item should be drawn with antialiasing and
    smooth pixmap filtering to \a smooth.

    \sa smooth()
*/
void QDeclarativeItem::setSmooth(bool smooth)
{
   Q_D(QDeclarativeItem);
   if (d->smooth == smooth) {
      return;
   }
   d->smooth = smooth;
   emit smoothChanged(smooth);
   update();
}

/*!
  \property QDeclarativeItem::anchors
  \internal
*/

/*!
  \property QDeclarativeItem::left
  \internal
*/

/*!
  \property QDeclarativeItem::right
  \internal
*/

/*!
  \property QDeclarativeItem::horizontalCenter
  \internal
*/

/*!
  \property QDeclarativeItem::top
  \internal
*/

/*!
  \property QDeclarativeItem::bottom
  \internal
*/

/*!
  \property QDeclarativeItem::verticalCenter
  \internal
*/

/*!
  \property QDeclarativeItem::focus
  \internal
*/

/*!
  \property QDeclarativeItem::transform
  \internal
*/

/*!
  \property QDeclarativeItem::transformOrigin
  \internal
*/

/*!
  \property QDeclarativeItem::activeFocus
  \internal
*/

/*!
  \property QDeclarativeItem::baseline
  \internal
*/

/*!
  \property QDeclarativeItem::data
  \internal
*/

/*!
  \property QDeclarativeItem::resources
  \internal
*/

/*!
  \property QDeclarativeItem::state
  \internal
*/

/*!
  \property QDeclarativeItem::states
  \internal
*/

/*!
  \property QDeclarativeItem::transformOriginPoint
  \internal
*/

/*!
  \property QDeclarativeItem::transitions
  \internal
*/

/*!
    \internal
    Return the width of the item
*/
qreal QDeclarativeItem::width() const
{
   Q_D(const QDeclarativeItem);
   return d->width();
}

/*!
    \internal
    Set the width of the item
*/
void QDeclarativeItem::setWidth(qreal w)
{
   Q_D(QDeclarativeItem);
   d->setWidth(w);
}

/*!
    \internal
    Reset the width of the item
*/
void QDeclarativeItem::resetWidth()
{
   Q_D(QDeclarativeItem);
   d->resetWidth();
}

/*!
    \internal
    Return the width of the item
*/
qreal QDeclarativeItemPrivate::width() const
{
   return mWidth;
}

/*!
    \internal
*/
void QDeclarativeItemPrivate::setWidth(qreal w)
{
   Q_Q(QDeclarativeItem);
   if (qIsNaN(w)) {
      return;
   }

   widthValid = true;
   if (mWidth == w) {
      return;
   }

   qreal oldWidth = mWidth;

   q->prepareGeometryChange();
   mWidth = w;

   q->geometryChanged(QRectF(q->x(), q->y(), width(), height()),
                      QRectF(q->x(), q->y(), oldWidth, height()));
}

/*!
    \internal
*/
void QDeclarativeItemPrivate::resetWidth()
{
   Q_Q(QDeclarativeItem);
   widthValid = false;
   q->setImplicitWidth(q->implicitWidth());
}

void QDeclarativeItemPrivate::implicitWidthChanged()
{
   Q_Q(QDeclarativeItem);
   emit q->implicitWidthChanged();
}

qreal QDeclarativeItemPrivate::implicitWidth() const
{
   return mImplicitWidth;
}

/*!
    Returns the width of the item that is implied by other properties that determine the content.
*/
qreal QDeclarativeItem::implicitWidth() const
{
   Q_D(const QDeclarativeItem);
   return d->implicitWidth();
}

/*!
    Sets the implied width of the item to \a w.
    This is the width implied by other properties that determine the content.
*/
void QDeclarativeItem::setImplicitWidth(qreal w)
{
   Q_D(QDeclarativeItem);
   bool changed = w != d->mImplicitWidth;
   d->mImplicitWidth = w;
   if (d->mWidth == w || widthValid()) {
      if (changed) {
         d->implicitWidthChanged();
      }
      return;
   }

   qreal oldWidth = d->mWidth;

   prepareGeometryChange();
   d->mWidth = w;

   geometryChanged(QRectF(x(), y(), width(), height()),
                   QRectF(x(), y(), oldWidth, height()));

   if (changed) {
      d->implicitWidthChanged();
   }
}

/*!
    Returns whether the width property has been set explicitly.
*/
bool QDeclarativeItem::widthValid() const
{
   Q_D(const QDeclarativeItem);
   return d->widthValid;
}

/*!
    \internal
    Return the height of the item
*/
qreal QDeclarativeItem::height() const
{
   Q_D(const QDeclarativeItem);
   return d->height();
}

/*!
    \internal
    Set the height of the item
*/
void QDeclarativeItem::setHeight(qreal h)
{
   Q_D(QDeclarativeItem);
   d->setHeight(h);
}

/*!
    \internal
    Reset the height of the item
*/
void QDeclarativeItem::resetHeight()
{
   Q_D(QDeclarativeItem);
   d->resetHeight();
}

/*!
    \internal
*/
qreal QDeclarativeItemPrivate::height() const
{
   return mHeight;
}

/*!
    \internal
*/
void QDeclarativeItemPrivate::setHeight(qreal h)
{
   Q_Q(QDeclarativeItem);
   if (qIsNaN(h)) {
      return;
   }

   heightValid = true;
   if (mHeight == h) {
      return;
   }

   qreal oldHeight = mHeight;

   q->prepareGeometryChange();
   mHeight = h;

   q->geometryChanged(QRectF(q->x(), q->y(), width(), height()),
                      QRectF(q->x(), q->y(), width(), oldHeight));
}

/*!
    \internal
*/
void QDeclarativeItemPrivate::resetHeight()
{
   Q_Q(QDeclarativeItem);
   heightValid = false;
   q->setImplicitHeight(q->implicitHeight());
}

void QDeclarativeItemPrivate::implicitHeightChanged()
{
   Q_Q(QDeclarativeItem);
   emit q->implicitHeightChanged();
}

qreal QDeclarativeItemPrivate::implicitHeight() const
{
   return mImplicitHeight;
}

/*!
    Returns the height of the item that is implied by other properties that determine the content.
*/
qreal QDeclarativeItem::implicitHeight() const
{
   Q_D(const QDeclarativeItem);
   return d->implicitHeight();
}

/*!
    \qmlproperty real Item::implicitWidth
    \qmlproperty real Item::implicitHeight
    \since QtQuick 1.1

    Defines the natural width or height of the Item if no \l width or \l height is specified.

    The default implicit size for most items is 0x0, however some elements have an inherent
    implicit size which cannot be overridden, e.g. Image, Text.

    Setting the implicit size is useful for defining components that have a preferred size
    based on their content, for example:

    \qml
    // Label.qml
    import QtQuick 1.1

    Item {
        property alias icon: image.source
        property alias label: text.text
        implicitWidth: text.implicitWidth + image.implicitWidth
        implicitHeight: Math.max(text.implicitHeight, image.implicitHeight)
        Image { id: image }
        Text {
            id: text
            wrapMode: Text.Wrap
            anchors.left: image.right; anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
        }
    }
    \endqml

    \bold Note: using implicitWidth of Text or TextEdit and setting the width explicitly
    incurs a performance penalty as the text must be laid out twice.
*/


/*!
    Sets the implied height of the item to \a h.
    This is the height implied by other properties that determine the content.
*/
void QDeclarativeItem::setImplicitHeight(qreal h)
{
   Q_D(QDeclarativeItem);
   bool changed = h != d->mImplicitHeight;
   d->mImplicitHeight = h;
   if (d->mHeight == h || heightValid()) {
      if (changed) {
         d->implicitHeightChanged();
      }
      return;
   }

   qreal oldHeight = d->mHeight;

   prepareGeometryChange();
   d->mHeight = h;

   geometryChanged(QRectF(x(), y(), width(), height()),
                   QRectF(x(), y(), width(), oldHeight));

   if (changed) {
      d->implicitHeightChanged();
   }
}

/*!
    Returns whether the height property has been set explicitly.
*/
bool QDeclarativeItem::heightValid() const
{
   Q_D(const QDeclarativeItem);
   return d->heightValid;
}

/*! \internal */
void QDeclarativeItem::setSize(const QSizeF &size)
{
   Q_D(QDeclarativeItem);
   d->heightValid = true;
   d->widthValid = true;

   if (d->height() == size.height() && d->width() == size.width()) {
      return;
   }

   qreal oldHeight = d->height();
   qreal oldWidth = d->width();

   prepareGeometryChange();
   d->setHeight(size.height());
   d->setWidth(size.width());

   geometryChanged(QRectF(x(), y(), width(), height()),
                   QRectF(x(), y(), oldWidth, oldHeight));
}

/*!
  \qmlproperty bool Item::activeFocus

  This property indicates whether the item has active focus.

  An item with active focus will receive keyboard input,
  or is a FocusScope ancestor of the item that will receive keyboard input.

  Usually, activeFocus is gained by setting focus on an item and its enclosing
  FocusScopes. In the following example \c input will have activeFocus.
  \qml
  Rectangle {
      FocusScope {
          focus: true
          TextInput {
              id: input
              focus: true
          }
      }
  }
  \endqml

  \sa focus, {qmlfocus}{Keyboard Focus}
*/

/*! \internal */
bool QDeclarativeItem::hasActiveFocus() const
{
   Q_D(const QDeclarativeItem);
   QGraphicsItem *fi = focusItem();
   QGraphicsScene *s = scene();
   bool hasOrWillGainFocus = fi && fi->isVisible() && (!s || s->focusItem() == fi);
   bool isOrIsScopeOfFocusItem = (fi == this || (d->flags & QGraphicsItem::ItemIsFocusScope));
   return hasOrWillGainFocus && isOrIsScopeOfFocusItem;
}

/*!
  \qmlproperty bool Item::focus
  This property indicates whether the item has focus within the enclosing focus scope. If true, this item
  will gain active focus when the enclosing focus scope gains active focus.
  In the following example, \c input will be given active focus when \c scope gains active focus.
  \qml
  Rectangle {
      FocusScope {
          id: scope
          TextInput {
              id: input
              focus: true
          }
      }
  }
  \endqml

  For the purposes of this property, the scene as a whole is assumed to act like a focus scope.
  On a practical level, that means the following QML will give active focus to \c input on startup.

  \qml
  Rectangle {
      TextInput {
          id: input
          focus: true
      }
  }
  \endqml

  \sa activeFocus, {qmlfocus}{Keyboard Focus}
*/

/*! \internal */
bool QDeclarativeItem::hasFocus() const
{
   Q_D(const QDeclarativeItem);
   QGraphicsItem *p = d->parent;
   while (p) {
      if (p->flags() & QGraphicsItem::ItemIsFocusScope) {
         return p->focusScopeItem() == this;
      }
      p = p->parentItem();
   }

   return hasActiveFocus();
}

/*! \internal */
void QDeclarativeItem::setFocus(bool focus)
{
   if (focus) {
      QGraphicsItem::setFocus(Qt::OtherFocusReason);
   } else {
      QGraphicsItem::clearFocus();
   }
}

/*!
    \internal
*/
void QDeclarativeItem::paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *)
{
}

/*!
    \internal
*/
bool QDeclarativeItem::event(QEvent *ev)
{
   Q_D(QDeclarativeItem);

   switch (ev->type()) {
      case QEvent::KeyPress:
      case QEvent::KeyRelease:
      case QEvent::InputMethod:
         d->doneEventPreHandler = false;
         break;
      default:
         break;
   }

   return QGraphicsObject::event(ev);
}

QDebug operator<<(QDebug debug, QDeclarativeItem *item)
{
   if (!item) {
      debug << "QDeclarativeItem(0)";
      return debug;
   }

   debug << item->metaObject()->className() << "(this =" << ((void *)item)
         << ", parent =" << ((void *)item->parentItem())
         << ", geometry =" << QRectF(item->pos(), QSizeF(item->width(), item->height()))
         << ", z =" << item->zValue() << ')';
   return debug;
}

qint64 QDeclarativeItemPrivate::consistentTime = -1;
void QDeclarativeItemPrivate::setConsistentTime(qint64 t)
{
   consistentTime = t;
}

class QElapsedTimerConsistentTimeHack
{
 public:
   void start() {
      t1 = QDeclarativeItemPrivate::consistentTime;
      t2 = 0;
   }
   qint64 elapsed() {
      return QDeclarativeItemPrivate::consistentTime - t1;
   }
   qint64 restart() {
      qint64 val = QDeclarativeItemPrivate::consistentTime - t1;
      t1 = QDeclarativeItemPrivate::consistentTime;
      t2 = 0;
      return val;
   }

 private:
   qint64 t1;
   qint64 t2;
};

void QDeclarativeItemPrivate::start(QElapsedTimer &t)
{
   if (QDeclarativeItemPrivate::consistentTime == -1) {
      t.start();
   } else {
      ((QElapsedTimerConsistentTimeHack *)&t)->start();
   }
}

qint64 QDeclarativeItemPrivate::elapsed(QElapsedTimer &t)
{
   if (QDeclarativeItemPrivate::consistentTime == -1) {
      return t.elapsed();
   } else {
      return ((QElapsedTimerConsistentTimeHack *)&t)->elapsed();
   }
}

qint64 QDeclarativeItemPrivate::restart(QElapsedTimer &t)
{
   if (QDeclarativeItemPrivate::consistentTime == -1) {
      return t.restart();
   } else {
      return ((QElapsedTimerConsistentTimeHack *)&t)->restart();
   }
}

QT_END_NAMESPACE
