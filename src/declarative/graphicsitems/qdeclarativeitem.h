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

#ifndef QDECLARATIVEITEM_H
#define QDECLARATIVEITEM_H

#include <QtDeclarative/qdeclarative.h>
#include <QtDeclarative/qdeclarativecomponent.h>
#include <QtCore/QObject>
#include <QtCore/QList>
#include <QtGui/qgraphicsitem.h>
#include <QtGui/qgraphicstransform.h>
#include <QtGui/qfont.h>
#include <QtGui/qaction.h>

QT_BEGIN_NAMESPACE

class QDeclarativeState;
class QDeclarativeAnchorLine;
class QDeclarativeTransition;
class QDeclarativeKeyEvent;
class QDeclarativeAnchors;
class QDeclarativeItemPrivate;

class Q_DECLARATIVE_EXPORT QDeclarativeItem : public QGraphicsObject, public QDeclarativeParserStatus
{
   DECL_CS_OBJECT(QDeclarativeItem)

   CS_INTERFACES(QDeclarativeParserStatus)

   DECL_CS_PROPERTY_READ(*, parentItem)
   DECL_CS_PROPERTY_WRITE(*, setParentItem)
   DECL_CS_PROPERTY_NOTIFY(*, parentChanged)
   DECL_CS_PROPERTY_DESIGNABLE(*, false)
   DECL_CS_PROPERTY_FINAL(*)

   /* BROOM (decalartive)
       Q_PRIVATE_PROPERTY(QDeclarativeItem::d_func(), QDeclarativeListProperty<QObject> data READ data DESIGNABLE false)
       Q_PRIVATE_PROPERTY(QDeclarativeItem::d_func(), QDeclarativeListProperty<QObject> resources READ resources DESIGNABLE false)
       Q_PRIVATE_PROPERTY(QDeclarativeItem::d_func(), QDeclarativeListProperty<QDeclarativeState> states READ states DESIGNABLE false)
       Q_PRIVATE_PROPERTY(QDeclarativeItem::d_func(), QDeclarativeListProperty<QDeclarativeTransition> transitions READ transitions DESIGNABLE false)
       Q_PRIVATE_PROPERTY(QDeclarativeItem::d_func(), QString state READ state WRITE setState NOTIFY stateChanged)
   */

   DECL_CS_PROPERTY_READ(data, data)
   DECL_CS_PROPERTY_DESIGNABLE(data, false)

   DECL_CS_PROPERTY_READ(resources, resources)
   DECL_CS_PROPERTY_DESIGNABLE(resources, false)

   DECL_CS_PROPERTY_READ(states, states)
   DECL_CS_PROPERTY_DESIGNABLE(states, false)

   DECL_CS_PROPERTY_READ(transitions, transitions)
   DECL_CS_PROPERTY_DESIGNABLE(transitions, false)

   DECL_CS_PROPERTY_READ(state, state)
   DECL_CS_PROPERTY_WRITE(state, setState)
   DECL_CS_PROPERTY_NOTIFY(state, stateChanged)

   DECL_CS_PROPERTY_READ(childrenRect, childrenRect)
   DECL_CS_PROPERTY_NOTIFY(childrenRect, childrenRectChanged)
   DECL_CS_PROPERTY_DESIGNABLE(childrenRect, false)
   DECL_CS_PROPERTY_FINAL(childrenRect)

   /* BROOM (decalartive)
       Q_PRIVATE_PROPERTY(QDeclarativeItem::d_func(), QDeclarativeAnchors * anchors READ anchors DESIGNABLE false CONSTANT FINAL)
       Q_PRIVATE_PROPERTY(QDeclarativeItem::d_func(), QDeclarativeAnchorLine left READ left CONSTANT FINAL)
       Q_PRIVATE_PROPERTY(QDeclarativeItem::d_func(), QDeclarativeAnchorLine right READ right CONSTANT FINAL)
       Q_PRIVATE_PROPERTY(QDeclarativeItem::d_func(), QDeclarativeAnchorLine horizontalCenter READ horizontalCenter CONSTANT FINAL)
       Q_PRIVATE_PROPERTY(QDeclarativeItem::d_func(), QDeclarativeAnchorLine top READ top CONSTANT FINAL)
       Q_PRIVATE_PROPERTY(QDeclarativeItem::d_func(), QDeclarativeAnchorLine bottom READ bottom CONSTANT FINAL)
       Q_PRIVATE_PROPERTY(QDeclarativeItem::d_func(), QDeclarativeAnchorLine verticalCenter READ verticalCenter CONSTANT FINAL)
       Q_PRIVATE_PROPERTY(QDeclarativeItem::d_func(), QDeclarativeAnchorLine baseline READ baseline CONSTANT FINAL)
   */

   DECL_CS_PROPERTY_READ(anchors, anchors)
   DECL_CS_PROPERTY_DESIGNABLE(anchors, false)
   DECL_CS_PROPERTY_CONSTANT(anchors)
   DECL_CS_PROPERTY_FINAL(anchors)

   DECL_CS_PROPERTY_READ(left, left)
   DECL_CS_PROPERTY_CONSTANT(left)
   DECL_CS_PROPERTY_FINAL(left)

   DECL_CS_PROPERTY_READ(right, right)
   DECL_CS_PROPERTY_CONSTANT(right)
   DECL_CS_PROPERTY_FINAL(right)

   DECL_CS_PROPERTY_READ(horizontalCenter, horizontalCenter)
   DECL_CS_PROPERTY_CONSTANT(horizontalCenter)
   DECL_CS_PROPERTY_FINAL(horizontalCenter)

   DECL_CS_PROPERTY_READ(top, top)
   DECL_CS_PROPERTY_CONSTANT(top)
   DECL_CS_PROPERTY_FINAL(top)

   DECL_CS_PROPERTY_READ(bottom, bottom)
   DECL_CS_PROPERTY_CONSTANT(bottom)
   DECL_CS_PROPERTY_FINAL(bottom)

   DECL_CS_PROPERTY_READ(verticalCenter, verticalCenter)
   DECL_CS_PROPERTY_CONSTANT(verticalCenter)
   DECL_CS_PROPERTY_FINAL(verticalCenter)

   DECL_CS_PROPERTY_READ(baseline, baseline)
   DECL_CS_PROPERTY_CONSTANT(baseline)
   DECL_CS_PROPERTY_FINAL(baseline)

   DECL_CS_PROPERTY_READ(baselineOffset, baselineOffset)
   DECL_CS_PROPERTY_WRITE(baselineOffset, setBaselineOffset)
   DECL_CS_PROPERTY_NOTIFY(baselineOffset, baselineOffsetChanged)
   DECL_CS_PROPERTY_READ(clip, clip)
   DECL_CS_PROPERTY_WRITE(clip, setClip)
   DECL_CS_PROPERTY_NOTIFY(clip, clipChanged)// ### move to QGI/QGO, NOTIFY
   DECL_CS_PROPERTY_READ(focus, hasFocus)
   DECL_CS_PROPERTY_WRITE(focus, setFocus)
   DECL_CS_PROPERTY_NOTIFY(focus, focusChanged)
   DECL_CS_PROPERTY_FINAL(focus)
   DECL_CS_PROPERTY_READ(activeFocus, hasActiveFocus)
   DECL_CS_PROPERTY_NOTIFY(activeFocus, activeFocusChanged)
   DECL_CS_PROPERTY_READ(transform, transform)
   DECL_CS_PROPERTY_DESIGNABLE(transform, false)
   DECL_CS_PROPERTY_FINAL(transform)
   DECL_CS_PROPERTY_READ(transformOrigin, transformOrigin)
   DECL_CS_PROPERTY_WRITE(transformOrigin, setTransformOrigin)
   DECL_CS_PROPERTY_NOTIFY(transformOrigin, transformOriginChanged)
   DECL_CS_PROPERTY_READ(transformOriginPoint, transformOriginPoint)// transformOriginPoint is read-only for Item
   DECL_CS_PROPERTY_READ(smooth, smooth)
   DECL_CS_PROPERTY_WRITE(smooth, setSmooth)
   DECL_CS_PROPERTY_NOTIFY(smooth, smoothChanged)
   DECL_CS_PROPERTY_READ(implicitWidth, implicitWidth)
   DECL_CS_PROPERTY_WRITE(implicitWidth, setImplicitWidth)
   DECL_CS_PROPERTY_NOTIFY(implicitWidth, implicitWidthChanged)
   DECL_CS_PROPERTY_REVISION(implicitWidth, 1)
   DECL_CS_PROPERTY_READ(implicitHeight, implicitHeight)
   DECL_CS_PROPERTY_WRITE(implicitHeight, setImplicitHeight)
   DECL_CS_PROPERTY_NOTIFY(implicitHeight, implicitHeightChanged)
   DECL_CS_PROPERTY_REVISION(implicitHeight, 1)

   DECL_CS_ENUM(TransformOrigin)

   DECL_CS_CLASSINFO("DefaultProperty", "data")

 public:
   enum TransformOrigin {
      TopLeft, Top, TopRight,
      Left, Center, Right,
      BottomLeft, Bottom, BottomRight
   };

   QDeclarativeItem(QDeclarativeItem *parent = 0);
   virtual ~QDeclarativeItem();

   QDeclarativeItem *parentItem() const;
   void setParentItem(QDeclarativeItem *parent);

   QRectF childrenRect();

   bool clip() const;
   void setClip(bool);

   qreal baselineOffset() const;
   void setBaselineOffset(qreal);

   QDeclarativeListProperty<QGraphicsTransform> transform();

   qreal width() const;
   void setWidth(qreal);
   void resetWidth();
   qreal implicitWidth() const;

   qreal height() const;
   void setHeight(qreal);
   void resetHeight();
   qreal implicitHeight() const;

   void setSize(const QSizeF &size);

   TransformOrigin transformOrigin() const;
   void setTransformOrigin(TransformOrigin);

   bool smooth() const;
   void setSmooth(bool);

   QRectF boundingRect() const;
   virtual void paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *);

   bool hasActiveFocus() const;
   bool hasFocus() const;
   void setFocus(bool);

   bool keepMouseGrab() const;
   void setKeepMouseGrab(bool);

   DECL_CS_INVOKABLE_METHOD_1(Public, QScriptValue mapFromItem(const QScriptValue &item, qreal x, qreal y) const)
   DECL_CS_INVOKABLE_METHOD_2(mapFromItem)

   DECL_CS_INVOKABLE_METHOD_1(Public, QScriptValue mapToItem(const QScriptValue &item, qreal x, qreal y) const)
   DECL_CS_INVOKABLE_METHOD_2(maptToItem)

   DECL_CS_INVOKABLE_METHOD_1(Public, void forceActiveFocus())
   DECL_CS_INVOKABLE_METHOD_2(forceActiveFocus)

   DECL_CS_INVOKABLE_METHOD_1(Public, QDeclarativeItem *childAt(qreal x, qreal y) const)
   DECL_CS_INVOKABLE_METHOD_2(childAt)

   DECL_CS_SIGNAL_1(Public, void childrenRectChanged(const QRectF &un_named_arg1))
   DECL_CS_SIGNAL_2(childrenRectChanged, un_named_arg1)
   DECL_CS_SIGNAL_1(Public, void baselineOffsetChanged(qreal un_named_arg1))
   DECL_CS_SIGNAL_2(baselineOffsetChanged, un_named_arg1)
   DECL_CS_SIGNAL_1(Public, void stateChanged(const QString &un_named_arg1))
   DECL_CS_SIGNAL_2(stateChanged, un_named_arg1)
   DECL_CS_SIGNAL_1(Public, void focusChanged(bool un_named_arg1))
   DECL_CS_SIGNAL_2(focusChanged, un_named_arg1)
   DECL_CS_SIGNAL_1(Public, void activeFocusChanged(bool un_named_arg1))
   DECL_CS_SIGNAL_2(activeFocusChanged, un_named_arg1)
   DECL_CS_SIGNAL_1(Public, void parentChanged(QDeclarativeItem *un_named_arg1))
   DECL_CS_SIGNAL_2(parentChanged, un_named_arg1)
   DECL_CS_SIGNAL_1(Public, void transformOriginChanged(TransformOrigin un_named_arg1))
   DECL_CS_SIGNAL_2(transformOriginChanged, un_named_arg1)
   DECL_CS_SIGNAL_1(Public, void smoothChanged(bool un_named_arg1))
   DECL_CS_SIGNAL_2(smoothChanged, un_named_arg1)
   DECL_CS_SIGNAL_1(Public, void clipChanged(bool un_named_arg1))
   DECL_CS_SIGNAL_2(clipChanged, un_named_arg1)

   DECL_CS_SIGNAL_1(Public, void implicitWidthChanged())
   DECL_CS_SIGNAL_2(implicitWidthChanged)
   DECL_CS_REVISION(implicitWidthChanged, 1)

   DECL_CS_SIGNAL_1(Public, void implicitHeightChanged())
   DECL_CS_SIGNAL_2(implicitHeightChanged)
   DECL_CS_REVISION(implicitHeightChanged, 1)

 protected:
   bool isComponentComplete() const;
   virtual bool sceneEvent(QEvent *);
   virtual bool event(QEvent *);
   virtual QVariant itemChange(GraphicsItemChange, const QVariant &);

   void setImplicitWidth(qreal);
   bool widthValid() const; // ### better name?

   void setImplicitHeight(qreal);
   bool heightValid() const; // ### better name?

   virtual void classBegin();
   virtual void componentComplete();
   virtual void keyPressEvent(QKeyEvent *event);
   virtual void keyReleaseEvent(QKeyEvent *event);
   virtual void inputMethodEvent(QInputMethodEvent *);
   virtual QVariant inputMethodQuery(Qt::InputMethodQuery query) const;
   void keyPressPreHandler(QKeyEvent *);
   void keyReleasePreHandler(QKeyEvent *);
   void inputMethodPreHandler(QInputMethodEvent *);

   virtual void geometryChanged(const QRectF &newGeometry,
                                const QRectF &oldGeometry);

 protected:
   QDeclarativeItem(QDeclarativeItemPrivate &dd, QDeclarativeItem *parent = 0);

 private:
   Q_DISABLE_COPY(QDeclarativeItem)
   Q_DECLARE_PRIVATE_D(QGraphicsItem::d_ptr.data(), QDeclarativeItem)
};

template<typename T>
T qobject_cast(QGraphicsObject *o)
{
   QObject *obj = o;
   return qobject_cast<T>(obj);
}

// ### move to QGO
template<typename T>
T qobject_cast(QGraphicsItem *item)
{
   if (!item) {
      return 0;
   }
   QObject *o = item->toGraphicsObject();
   return qobject_cast<T>(o);
}

QDebug Q_DECLARATIVE_EXPORT operator<<(QDebug debug, QDeclarativeItem *item);

QT_END_NAMESPACE

QML_DECLARE_TYPE(QDeclarativeItem)
QML_DECLARE_TYPE(QGraphicsObject)
QML_DECLARE_TYPE(QGraphicsTransform)
QML_DECLARE_TYPE(QGraphicsScale)
QML_DECLARE_TYPE(QGraphicsRotation)
QML_DECLARE_TYPE(QGraphicsWidget)
QML_DECLARE_TYPE(QAction)


#endif // QDECLARATIVEITEM_H
