/***********************************************************************
*
* Copyright (c) 2012-2014 Barbara Geller
* Copyright (c) 2012-2014 Ansel Sermersheim
* Copyright (c) 2012-2014 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
*
* This file is part of CopperSpice.
*
* CopperSpice is free software: you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with CopperSpice.  If not, see
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#ifndef QDECLARATIVEITEM_P_H
#define QDECLARATIVEITEM_P_H

#include <qdeclarativeitem.h>
#include <qdeclarativeanchors_p.h>
#include <qdeclarativeanchors_p_p.h>
#include <qdeclarativeitemchangelistener_p.h>
#include <qpodvector_p.h>
#include <qdeclarativestate_p.h>
#include <qdeclarativenullablevalue_p_p.h>
#include <qdeclarativenotifier_p.h>
#include <qdeclarativeglobal_p.h>
#include <qdeclarative.h>
#include <qdeclarativecontext.h>
#include <QtCore/qlist.h>
#include <QtCore/qdebug.h>
#include <qgraphicsitem_p.h>

QT_BEGIN_NAMESPACE

class QNetworkReply;
class QDeclarativeItemKeyFilter;
class QDeclarativeLayoutMirroringAttached;

//### merge into private?
class QDeclarativeContents : public QObject, public QDeclarativeItemChangeListener
{
   CS_OBJECT(QDeclarativeContents)
 public:
   QDeclarativeContents(QDeclarativeItem *item);
   ~QDeclarativeContents();

   QRectF rectF() const;

   void childRemoved(QDeclarativeItem *item);
   void childAdded(QDeclarativeItem *item);

   void calcGeometry() {
      calcWidth();
      calcHeight();
   }
   void complete();

 public:
   CS_SIGNAL_1(Public, void rectChanged(QRectF un_named_arg1))
   CS_SIGNAL_2(rectChanged, un_named_arg1)

 protected:
   void itemGeometryChanged(QDeclarativeItem *item, const QRectF &newGeometry, const QRectF &oldGeometry);
   void itemDestroyed(QDeclarativeItem *item);
   //void itemVisibilityChanged(QDeclarativeItem *item)

 private:
   void calcHeight(QDeclarativeItem *changed = 0);
   void calcWidth(QDeclarativeItem *changed = 0);

   QDeclarativeItem *m_item;
   qreal m_x;
   qreal m_y;
   qreal m_width;
   qreal m_height;
};

class Q_DECLARATIVE_EXPORT QDeclarativeItemPrivate : public QGraphicsItemPrivate
{
   Q_DECLARE_PUBLIC(QDeclarativeItem)

 public:
   QDeclarativeItemPrivate()
      : _anchors(0), _contents(0),
        baselineOffset(0),
        _anchorLines(0),
        _stateGroup(0), origin(QDeclarativeItem::Center),
        widthValid(false), heightValid(false),
        componentComplete(true), keepMouse(false),
        smooth(false), transformOriginDirty(true), doneEventPreHandler(false),
        inheritedLayoutMirror(false), effectiveLayoutMirror(false), isMirrorImplicit(true),
        inheritMirrorFromParent(false), inheritMirrorFromItem(false), hadFocus(false), hadActiveFocus(false), keyHandler(0),
        mWidth(0), mHeight(0), mImplicitWidth(0), mImplicitHeight(0), attachedLayoutDirection(0) {
      QGraphicsItemPrivate::acceptedMouseButtons = 0;
      isDeclarativeItem = 1;
      QGraphicsItemPrivate::flags = QGraphicsItem::GraphicsItemFlags(
                                       QGraphicsItem::ItemHasNoContents
                                       | QGraphicsItem::ItemIsFocusable
                                       | QGraphicsItem::ItemNegativeZStacksBehindParent);
   }

   void init(QDeclarativeItem *parent) {
      Q_Q(QDeclarativeItem);
      if (parent) {
         QDeclarative_setParent_noEvent(q, parent);
         q->setParentItem(parent);
         QDeclarativeItemPrivate *parentPrivate = QDeclarativeItemPrivate::get(parent);
         setImplicitLayoutMirror(parentPrivate->inheritedLayoutMirror, parentPrivate->inheritMirrorFromParent);
      }
      baselineOffset.invalidate();
      mouseSetsFocus = false;
   }

   bool isMirrored() const {
      return effectiveLayoutMirror;
   }

   // Private Properties
   qreal width() const;
   void setWidth(qreal);
   void resetWidth();

   qreal height() const;
   void setHeight(qreal);
   void resetHeight();

   virtual qreal implicitWidth() const;
   virtual qreal implicitHeight() const;
   virtual void implicitWidthChanged();
   virtual void implicitHeightChanged();

   void resolveLayoutMirror();
   void setImplicitLayoutMirror(bool mirror, bool inherit);
   void setLayoutMirror(bool mirror);

   QDeclarativeListProperty<QObject> data();
   QDeclarativeListProperty<QObject> resources();

   QDeclarativeListProperty<QDeclarativeState> states();
   QDeclarativeListProperty<QDeclarativeTransition> transitions();

   QString state() const;
   void setState(const QString &);

   QDeclarativeAnchorLine left() const;
   QDeclarativeAnchorLine right() const;
   QDeclarativeAnchorLine horizontalCenter() const;
   QDeclarativeAnchorLine top() const;
   QDeclarativeAnchorLine bottom() const;
   QDeclarativeAnchorLine verticalCenter() const;
   QDeclarativeAnchorLine baseline() const;

   // data property
   static void data_append(QDeclarativeListProperty<QObject> *, QObject *);
   static int data_count(QDeclarativeListProperty<QObject> *);
   static QObject *data_at(QDeclarativeListProperty<QObject> *, int);
   static void data_clear(QDeclarativeListProperty<QObject> *);

   // resources property
   static QObject *resources_at(QDeclarativeListProperty<QObject> *, int);
   static void resources_append(QDeclarativeListProperty<QObject> *, QObject *);
   static int resources_count(QDeclarativeListProperty<QObject> *);
   static void resources_clear(QDeclarativeListProperty<QObject> *);

   // transform property
   static int transform_count(QDeclarativeListProperty<QGraphicsTransform> *list);
   static void transform_append(QDeclarativeListProperty<QGraphicsTransform> *list, QGraphicsTransform *);
   static QGraphicsTransform *transform_at(QDeclarativeListProperty<QGraphicsTransform> *list, int);
   static void transform_clear(QDeclarativeListProperty<QGraphicsTransform> *list);

   static QDeclarativeItemPrivate *get(QDeclarativeItem *item) {
      return item->d_func();
   }

   // Accelerated property accessors
   QDeclarativeNotifier parentNotifier;
   static void parentProperty(QObject *o, void *rv, QDeclarativeNotifierEndpoint *e);

   QDeclarativeAnchors *anchors() {
      if (!_anchors) {
         Q_Q(QDeclarativeItem);
         _anchors = new QDeclarativeAnchors(q);
         if (!componentComplete) {
            _anchors->classBegin();
         }
      }
      return _anchors;
   }
   QDeclarativeAnchors *_anchors;
   QDeclarativeContents *_contents;

   QDeclarativeNullableValue<qreal> baselineOffset;

   struct AnchorLines {
      AnchorLines(QGraphicsObject *);
      QDeclarativeAnchorLine left;
      QDeclarativeAnchorLine right;
      QDeclarativeAnchorLine hCenter;
      QDeclarativeAnchorLine top;
      QDeclarativeAnchorLine bottom;
      QDeclarativeAnchorLine vCenter;
      QDeclarativeAnchorLine baseline;
   };
   mutable AnchorLines *_anchorLines;
   AnchorLines *anchorLines() const {
      Q_Q(const QDeclarativeItem);
      if (!_anchorLines) _anchorLines =
            new AnchorLines(const_cast<QDeclarativeItem *>(q));
      return _anchorLines;
   }

   enum ChangeType {
      Geometry = 0x01,
      SiblingOrder = 0x02,
      Visibility = 0x04,
      Opacity = 0x08,
      Destroyed = 0x10
   };

   using ChangeTypes = QFlags<ChangeType>;

   struct ChangeListener {
      ChangeListener(QDeclarativeItemChangeListener *l, QDeclarativeItemPrivate::ChangeTypes t) : listener(l), types(t) {}
      QDeclarativeItemChangeListener *listener;
      QDeclarativeItemPrivate::ChangeTypes types;
      bool operator==(const ChangeListener &other) const {
         return listener == other.listener && types == other.types;
      }
   };

   void addItemChangeListener(QDeclarativeItemChangeListener *listener, ChangeTypes types) {
      changeListeners.append(ChangeListener(listener, types));
   }
   void removeItemChangeListener(QDeclarativeItemChangeListener *, ChangeTypes types);
   QPODVector<ChangeListener, 4> changeListeners;

   QDeclarativeStateGroup *_states();
   QDeclarativeStateGroup *_stateGroup;

   QDeclarativeItem::TransformOrigin origin: 5;
   bool widthValid: 1;
   bool heightValid: 1;
   bool componentComplete: 1;
   bool keepMouse: 1;
   bool smooth: 1;
   bool transformOriginDirty : 1;
   bool doneEventPreHandler : 1;
   bool inheritedLayoutMirror: 1;
   bool effectiveLayoutMirror: 1;
   bool isMirrorImplicit: 1;
   bool inheritMirrorFromParent: 1;
   bool inheritMirrorFromItem: 1;
   bool hadFocus: 1;
   bool hadActiveFocus: 1;

   QDeclarativeItemKeyFilter *keyHandler;

   qreal mWidth;
   qreal mHeight;
   qreal mImplicitWidth;
   qreal mImplicitHeight;

   QDeclarativeLayoutMirroringAttached *attachedLayoutDirection;


   QPointF computeTransformOrigin() const;

   virtual void setPosHelper(const QPointF &pos) {
      Q_Q(QDeclarativeItem);
      QRectF oldGeometry(this->pos.x(), this->pos.y(), mWidth, mHeight);
      QGraphicsItemPrivate::setPosHelper(pos);
      q->geometryChanged(QRectF(this->pos.x(), this->pos.y(), mWidth, mHeight), oldGeometry);
   }

   // Reimplemented from QGraphicsItemPrivate
   virtual void focusScopeItemChange(bool isSubFocusItem) {
      if (hadFocus != isSubFocusItem) {
         hadFocus = isSubFocusItem;
         emit q_func()->focusChanged(isSubFocusItem);
      }
   }

   // Reimplemented from QGraphicsItemPrivate
   virtual void siblingOrderChange() {
      Q_Q(QDeclarativeItem);
      for (int ii = 0; ii < changeListeners.count(); ++ii) {
         const QDeclarativeItemPrivate::ChangeListener &change = changeListeners.at(ii);
         if (change.types & QDeclarativeItemPrivate::SiblingOrder) {
            change.listener->itemSiblingOrderChanged(q);
         }
      }
   }

   // Reimplemented from QGraphicsItemPrivate
   virtual void transformChanged();

   virtual void focusChanged(bool);

   virtual void mirrorChange() {};

   static qint64 consistentTime;
   static void setConsistentTime(qint64 t);
   static void start(QElapsedTimer &);
   static qint64 elapsed(QElapsedTimer &);
   static qint64 restart(QElapsedTimer &);
};

/*
    Key filters can be installed on a QDeclarativeItem, but not removed.  Currently they
    are only used by attached objects (which are only destroyed on Item
    destruction), so this isn't a problem.  If in future this becomes any form
    of public API, they will have to support removal too.
*/
class QDeclarativeItemKeyFilter
{
 public:
   QDeclarativeItemKeyFilter(QDeclarativeItem * = 0);
   virtual ~QDeclarativeItemKeyFilter();

   virtual void keyPressed(QKeyEvent *event, bool post);
   virtual void keyReleased(QKeyEvent *event, bool post);
   virtual void inputMethodEvent(QInputMethodEvent *event, bool post);
   virtual QVariant inputMethodQuery(Qt::InputMethodQuery query) const;
   virtual void componentComplete();

   bool m_processPost;

 private:
   QDeclarativeItemKeyFilter *m_next;
};

class QDeclarativeKeyNavigationAttachedPrivate
{
 public:
   QDeclarativeKeyNavigationAttachedPrivate()
      : left(0), right(0), up(0), down(0), tab(0), backtab(0) {}

   QDeclarativeItem *left;
   QDeclarativeItem *right;
   QDeclarativeItem *up;
   QDeclarativeItem *down;
   QDeclarativeItem *tab;
   QDeclarativeItem *backtab;
};

class QDeclarativeKeyNavigationAttached : public QObject, public QDeclarativeItemKeyFilter
{
   CS_OBJECT(QDeclarativeKeyNavigationAttached)
   Q_DECLARE_PRIVATE(QDeclarativeKeyNavigationAttached)

   CS_PROPERTY_READ(*left, left)
   CS_PROPERTY_WRITE(*left, setLeft)
   CS_PROPERTY_NOTIFY(*left, leftChanged)
   CS_PROPERTY_READ(*right, right)
   CS_PROPERTY_WRITE(*right, setRight)
   CS_PROPERTY_NOTIFY(*right, rightChanged)
   CS_PROPERTY_READ(*up, up)
   CS_PROPERTY_WRITE(*up, setUp)
   CS_PROPERTY_NOTIFY(*up, upChanged)
   CS_PROPERTY_READ(*down, down)
   CS_PROPERTY_WRITE(*down, setDown)
   CS_PROPERTY_NOTIFY(*down, downChanged)
   CS_PROPERTY_READ(*tab, tab)
   CS_PROPERTY_WRITE(*tab, setTab)
   CS_PROPERTY_NOTIFY(*tab, tabChanged)
   CS_PROPERTY_READ(*backtab, backtab)
   CS_PROPERTY_WRITE(*backtab, setBacktab)
   CS_PROPERTY_NOTIFY(*backtab, backtabChanged)
   CS_PROPERTY_READ(priority, priority)
   CS_PROPERTY_WRITE(priority, setPriority)
   CS_PROPERTY_NOTIFY(priority, priorityChanged)

   CS_ENUM(Priority)

 public:
   QDeclarativeKeyNavigationAttached(QObject * = 0);

   QDeclarativeItem *left() const;
   void setLeft(QDeclarativeItem *);
   QDeclarativeItem *right() const;
   void setRight(QDeclarativeItem *);
   QDeclarativeItem *up() const;
   void setUp(QDeclarativeItem *);
   QDeclarativeItem *down() const;
   void setDown(QDeclarativeItem *);
   QDeclarativeItem *tab() const;
   void setTab(QDeclarativeItem *);
   QDeclarativeItem *backtab() const;
   void setBacktab(QDeclarativeItem *);

   enum Priority { BeforeItem, AfterItem };
   Priority priority() const;
   void setPriority(Priority);

   static QDeclarativeKeyNavigationAttached *qmlAttachedProperties(QObject *);

 public:
   CS_SIGNAL_1(Public, void leftChanged())
   CS_SIGNAL_2(leftChanged)
   CS_SIGNAL_1(Public, void rightChanged())
   CS_SIGNAL_2(rightChanged)
   CS_SIGNAL_1(Public, void upChanged())
   CS_SIGNAL_2(upChanged)
   CS_SIGNAL_1(Public, void downChanged())
   CS_SIGNAL_2(downChanged)
   CS_SIGNAL_1(Public, void tabChanged())
   CS_SIGNAL_2(tabChanged)
   CS_SIGNAL_1(Public, void backtabChanged())
   CS_SIGNAL_2(backtabChanged)
   CS_SIGNAL_1(Public, void priorityChanged())
   CS_SIGNAL_2(priorityChanged)

 private:
   virtual void keyPressed(QKeyEvent *event, bool post);
   virtual void keyReleased(QKeyEvent *event, bool post);
   void setFocusNavigation(QDeclarativeItem *currentItem, const char *dir);
};

class QDeclarativeLayoutMirroringAttached : public QObject
{
   CS_OBJECT(QDeclarativeLayoutMirroringAttached)
   CS_PROPERTY_READ(enabled, enabled)
   CS_PROPERTY_WRITE(enabled, setEnabled)
   CS_PROPERTY_RESET(enabled, resetEnabled)
   CS_PROPERTY_NOTIFY(enabled, enabledChanged)
   CS_PROPERTY_READ(childrenInherit, childrenInherit)
   CS_PROPERTY_WRITE(childrenInherit, setChildrenInherit)
   CS_PROPERTY_NOTIFY(childrenInherit, childrenInheritChanged)

 public:
   explicit QDeclarativeLayoutMirroringAttached(QObject *parent = 0);

   bool enabled() const;
   void setEnabled(bool);
   void resetEnabled();

   bool childrenInherit() const;
   void setChildrenInherit(bool);

   static QDeclarativeLayoutMirroringAttached *qmlAttachedProperties(QObject *);
 public:
   CS_SIGNAL_1(Public, void enabledChanged())
   CS_SIGNAL_2(enabledChanged)
   CS_SIGNAL_1(Public, void childrenInheritChanged())
   CS_SIGNAL_2(childrenInheritChanged)

 private:
   friend class QDeclarativeItemPrivate;
   QDeclarativeItemPrivate *itemPrivate;
};

class QDeclarativeKeysAttachedPrivate
{
 public:
   QDeclarativeKeysAttachedPrivate()
      : inPress(false), inRelease(false)
      , inIM(false), enabled(true), imeItem(0), item(0) {
   }

   bool isConnected(const char *signalName);

   QGraphicsItem *finalFocusProxy(QGraphicsItem *item) const {
      QGraphicsItem *fp;
      while ((fp = item->focusProxy())) {
         item = fp;
      }
      return item;
   }

   //loop detection
   bool inPress: 1;
   bool inRelease: 1;
   bool inIM: 1;

   bool enabled : 1;

   QGraphicsItem *imeItem;
   QList<QDeclarativeItem *> targets;
   QDeclarativeItem *item;
};

class QDeclarativeKeysAttached : public QObject, public QDeclarativeItemKeyFilter
{
   CS_OBJECT(QDeclarativeKeysAttached)
   Q_DECLARE_PRIVATE(QDeclarativeKeysAttached)

   CS_PROPERTY_READ(enabled, enabled)
   CS_PROPERTY_WRITE(enabled, setEnabled)
   CS_PROPERTY_NOTIFY(enabled, enabledChanged)
   CS_PROPERTY_READ(forwardTo, forwardTo)
   CS_PROPERTY_READ(priority, priority)
   CS_PROPERTY_WRITE(priority, setPriority)
   CS_PROPERTY_NOTIFY(priority, priorityChanged)

   CS_ENUM(Priority)

 public:
   QDeclarativeKeysAttached(QObject *parent = 0);
   ~QDeclarativeKeysAttached();

   bool enabled() const {
      Q_D(const QDeclarativeKeysAttached);
      return d->enabled;
   }
   void setEnabled(bool enabled) {
      Q_D(QDeclarativeKeysAttached);
      if (enabled != d->enabled) {
         d->enabled = enabled;
         emit enabledChanged();
      }
   }

   enum Priority { BeforeItem, AfterItem};
   Priority priority() const;
   void setPriority(Priority);

   QDeclarativeListProperty<QDeclarativeItem> forwardTo() {
      Q_D(QDeclarativeKeysAttached);
      return QDeclarativeListProperty<QDeclarativeItem>(this, d->targets);
   }

   virtual void componentComplete();

   static QDeclarativeKeysAttached *qmlAttachedProperties(QObject *);

 public:
   CS_SIGNAL_1(Public, void enabledChanged())
   CS_SIGNAL_2(enabledChanged)
   CS_SIGNAL_1(Public, void priorityChanged())
   CS_SIGNAL_2(priorityChanged)
   CS_SIGNAL_1(Public, void pressed(QDeclarativeKeyEvent *event))
   CS_SIGNAL_2(pressed, event)
   CS_SIGNAL_1(Public, void released(QDeclarativeKeyEvent *event))
   CS_SIGNAL_2(released, event)
   CS_SIGNAL_1(Public, void digit0Pressed(QDeclarativeKeyEvent *event))
   CS_SIGNAL_2(digit0Pressed, event)
   CS_SIGNAL_1(Public, void digit1Pressed(QDeclarativeKeyEvent *event))
   CS_SIGNAL_2(digit1Pressed, event)
   CS_SIGNAL_1(Public, void digit2Pressed(QDeclarativeKeyEvent *event))
   CS_SIGNAL_2(digit2Pressed, event)
   CS_SIGNAL_1(Public, void digit3Pressed(QDeclarativeKeyEvent *event))
   CS_SIGNAL_2(digit3Pressed, event)
   CS_SIGNAL_1(Public, void digit4Pressed(QDeclarativeKeyEvent *event))
   CS_SIGNAL_2(digit4Pressed, event)
   CS_SIGNAL_1(Public, void digit5Pressed(QDeclarativeKeyEvent *event))
   CS_SIGNAL_2(digit5Pressed, event)
   CS_SIGNAL_1(Public, void digit6Pressed(QDeclarativeKeyEvent *event))
   CS_SIGNAL_2(digit6Pressed, event)
   CS_SIGNAL_1(Public, void digit7Pressed(QDeclarativeKeyEvent *event))
   CS_SIGNAL_2(digit7Pressed, event)
   CS_SIGNAL_1(Public, void digit8Pressed(QDeclarativeKeyEvent *event))
   CS_SIGNAL_2(digit8Pressed, event)
   CS_SIGNAL_1(Public, void digit9Pressed(QDeclarativeKeyEvent *event))
   CS_SIGNAL_2(digit9Pressed, event)

   CS_SIGNAL_1(Public, void leftPressed(QDeclarativeKeyEvent *event))
   CS_SIGNAL_2(leftPressed, event)
   CS_SIGNAL_1(Public, void rightPressed(QDeclarativeKeyEvent *event))
   CS_SIGNAL_2(rightPressed, event)
   CS_SIGNAL_1(Public, void upPressed(QDeclarativeKeyEvent *event))
   CS_SIGNAL_2(upPressed, event)
   CS_SIGNAL_1(Public, void downPressed(QDeclarativeKeyEvent *event))
   CS_SIGNAL_2(downPressed, event)
   CS_SIGNAL_1(Public, void tabPressed(QDeclarativeKeyEvent *event))
   CS_SIGNAL_2(tabPressed, event)
   CS_SIGNAL_1(Public, void backtabPressed(QDeclarativeKeyEvent *event))
   CS_SIGNAL_2(backtabPressed, event)

   CS_SIGNAL_1(Public, void asteriskPressed(QDeclarativeKeyEvent *event))
   CS_SIGNAL_2(asteriskPressed, event)
   CS_SIGNAL_1(Public, void numberSignPressed(QDeclarativeKeyEvent *event))
   CS_SIGNAL_2(numberSignPressed, event)
   CS_SIGNAL_1(Public, void escapePressed(QDeclarativeKeyEvent *event))
   CS_SIGNAL_2(escapePressed, event)
   CS_SIGNAL_1(Public, void returnPressed(QDeclarativeKeyEvent *event))
   CS_SIGNAL_2(returnPressed, event)
   CS_SIGNAL_1(Public, void enterPressed(QDeclarativeKeyEvent *event))
   CS_SIGNAL_2(enterPressed, event)
   CS_SIGNAL_1(Public, void deletePressed(QDeclarativeKeyEvent *event))
   CS_SIGNAL_2(deletePressed, event)
   CS_SIGNAL_1(Public, void spacePressed(QDeclarativeKeyEvent *event))
   CS_SIGNAL_2(spacePressed, event)
   CS_SIGNAL_1(Public, void backPressed(QDeclarativeKeyEvent *event))
   CS_SIGNAL_2(backPressed, event)
   CS_SIGNAL_1(Public, void cancelPressed(QDeclarativeKeyEvent *event))
   CS_SIGNAL_2(cancelPressed, event)
   CS_SIGNAL_1(Public, void selectPressed(QDeclarativeKeyEvent *event))
   CS_SIGNAL_2(selectPressed, event)
   CS_SIGNAL_1(Public, void yesPressed(QDeclarativeKeyEvent *event))
   CS_SIGNAL_2(yesPressed, event)
   CS_SIGNAL_1(Public, void noPressed(QDeclarativeKeyEvent *event))
   CS_SIGNAL_2(noPressed, event)
   CS_SIGNAL_1(Public, void context1Pressed(QDeclarativeKeyEvent *event))
   CS_SIGNAL_2(context1Pressed, event)
   CS_SIGNAL_1(Public, void context2Pressed(QDeclarativeKeyEvent *event))
   CS_SIGNAL_2(context2Pressed, event)
   CS_SIGNAL_1(Public, void context3Pressed(QDeclarativeKeyEvent *event))
   CS_SIGNAL_2(context3Pressed, event)
   CS_SIGNAL_1(Public, void context4Pressed(QDeclarativeKeyEvent *event))
   CS_SIGNAL_2(context4Pressed, event)
   CS_SIGNAL_1(Public, void callPressed(QDeclarativeKeyEvent *event))
   CS_SIGNAL_2(callPressed, event)
   CS_SIGNAL_1(Public, void hangupPressed(QDeclarativeKeyEvent *event))
   CS_SIGNAL_2(hangupPressed, event)
   CS_SIGNAL_1(Public, void flipPressed(QDeclarativeKeyEvent *event))
   CS_SIGNAL_2(flipPressed, event)
   CS_SIGNAL_1(Public, void menuPressed(QDeclarativeKeyEvent *event))
   CS_SIGNAL_2(menuPressed, event)
   CS_SIGNAL_1(Public, void volumeUpPressed(QDeclarativeKeyEvent *event))
   CS_SIGNAL_2(volumeUpPressed, event)
   CS_SIGNAL_1(Public, void volumeDownPressed(QDeclarativeKeyEvent *event))
   CS_SIGNAL_2(volumeDownPressed, event)

 private:
   virtual void keyPressed(QKeyEvent *event, bool post);
   virtual void keyReleased(QKeyEvent *event, bool post);
   virtual void inputMethodEvent(QInputMethodEvent *, bool post);
   virtual QVariant inputMethodQuery(Qt::InputMethodQuery query) const;

   const QByteArray keyToSignal(int key) {
      QByteArray keySignal;
      if (key >= Qt::Key_0 && key <= Qt::Key_9) {
         keySignal = "digit0Pressed";
         keySignal[5] = '0' + (key - Qt::Key_0);
      } else {
         int i = 0;
         while (sigMap[i].key && sigMap[i].key != key) {
            ++i;
         }
         keySignal = sigMap[i].sig;
      }
      return keySignal;
   }

   struct SigMap {
      int key;
      const char *sig;
   };

   static const SigMap sigMap[];
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QDeclarativeItemPrivate::ChangeTypes);

QT_END_NAMESPACE

QML_DECLARE_TYPE(QDeclarativeKeysAttached)
QML_DECLARE_TYPEINFO(QDeclarativeKeysAttached, QML_HAS_ATTACHED_PROPERTIES)
QML_DECLARE_TYPE(QDeclarativeKeyNavigationAttached)
QML_DECLARE_TYPEINFO(QDeclarativeKeyNavigationAttached, QML_HAS_ATTACHED_PROPERTIES)
QML_DECLARE_TYPE(QDeclarativeLayoutMirroringAttached)
QML_DECLARE_TYPEINFO(QDeclarativeLayoutMirroringAttached, QML_HAS_ATTACHED_PROPERTIES)

#endif // QDECLARATIVEITEM_P_H
