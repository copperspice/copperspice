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

#ifndef QDECLARATIVEPATHVIEW_P_H
#define QDECLARATIVEPATHVIEW_P_H

#include <qdeclarativeitem.h>
#include <qdeclarativepath_p.h>

QT_BEGIN_NAMESPACE

class QDeclarativePathViewPrivate;
class QDeclarativePathViewAttached;

class QDeclarativePathView : public QDeclarativeItem
{
   DECL_CS_OBJECT(QDeclarativePathView)

   DECL_CS_PROPERTY_READ(model, model)
   DECL_CS_PROPERTY_WRITE(model, setModel)
   DECL_CS_PROPERTY_NOTIFY(model, modelChanged)
   DECL_CS_PROPERTY_READ(*path, path)
   DECL_CS_PROPERTY_WRITE(*path, setPath)
   DECL_CS_PROPERTY_NOTIFY(*path, pathChanged)
   DECL_CS_PROPERTY_READ(currentIndex, currentIndex)
   DECL_CS_PROPERTY_WRITE(currentIndex, setCurrentIndex)
   DECL_CS_PROPERTY_NOTIFY(currentIndex, currentIndexChanged)
   DECL_CS_PROPERTY_READ(offset, offset)
   DECL_CS_PROPERTY_WRITE(offset, setOffset)
   DECL_CS_PROPERTY_NOTIFY(offset, offsetChanged)

   DECL_CS_PROPERTY_READ(*highlight, highlight)
   DECL_CS_PROPERTY_WRITE(*highlight, setHighlight)
   DECL_CS_PROPERTY_NOTIFY(*highlight, highlightChanged)
   DECL_CS_PROPERTY_READ(*highlightItem, highlightItem)
   DECL_CS_PROPERTY_NOTIFY(*highlightItem, highlightItemChanged)

   DECL_CS_PROPERTY_READ(preferredHighlightBegin, preferredHighlightBegin)
   DECL_CS_PROPERTY_WRITE(preferredHighlightBegin, setPreferredHighlightBegin)
   DECL_CS_PROPERTY_NOTIFY(preferredHighlightBegin, preferredHighlightBeginChanged)
   DECL_CS_PROPERTY_READ(preferredHighlightEnd, preferredHighlightEnd)
   DECL_CS_PROPERTY_WRITE(preferredHighlightEnd, setPreferredHighlightEnd)
   DECL_CS_PROPERTY_NOTIFY(preferredHighlightEnd, preferredHighlightEndChanged)
   DECL_CS_PROPERTY_READ(highlightRangeMode, highlightRangeMode)
   DECL_CS_PROPERTY_WRITE(highlightRangeMode, setHighlightRangeMode)
   DECL_CS_PROPERTY_NOTIFY(highlightRangeMode, highlightRangeModeChanged)
   DECL_CS_PROPERTY_READ(highlightMoveDuration, highlightMoveDuration)
   DECL_CS_PROPERTY_WRITE(highlightMoveDuration, setHighlightMoveDuration)
   DECL_CS_PROPERTY_NOTIFY(highlightMoveDuration, highlightMoveDurationChanged)

   DECL_CS_PROPERTY_READ(dragMargin, dragMargin)
   DECL_CS_PROPERTY_WRITE(dragMargin, setDragMargin)
   DECL_CS_PROPERTY_NOTIFY(dragMargin, dragMarginChanged)
   DECL_CS_PROPERTY_READ(flickDeceleration, flickDeceleration)
   DECL_CS_PROPERTY_WRITE(flickDeceleration, setFlickDeceleration)
   DECL_CS_PROPERTY_NOTIFY(flickDeceleration, flickDecelerationChanged)
   DECL_CS_PROPERTY_READ(interactive, isInteractive)
   DECL_CS_PROPERTY_WRITE(interactive, setInteractive)
   DECL_CS_PROPERTY_NOTIFY(interactive, interactiveChanged)

   DECL_CS_PROPERTY_READ(moving, isMoving)
   DECL_CS_PROPERTY_NOTIFY(moving, movingChanged)
   DECL_CS_PROPERTY_READ(flicking, isFlicking)
   DECL_CS_PROPERTY_NOTIFY(flicking, flickingChanged)

   DECL_CS_PROPERTY_READ(count, count)
   DECL_CS_PROPERTY_NOTIFY(count, countChanged)
   DECL_CS_PROPERTY_READ(*delegate, delegate)
   DECL_CS_PROPERTY_WRITE(*delegate, setDelegate)
   DECL_CS_PROPERTY_NOTIFY(*delegate, delegateChanged)
   DECL_CS_PROPERTY_READ(pathItemCount, pathItemCount)
   DECL_CS_PROPERTY_WRITE(pathItemCount, setPathItemCount)
   DECL_CS_PROPERTY_NOTIFY(pathItemCount, pathItemCountChanged)

   CS_ENUM(HighlightRangeMode)

 public:
   QDeclarativePathView(QDeclarativeItem *parent = 0);
   virtual ~QDeclarativePathView();

   QVariant model() const;
   void setModel(const QVariant &);

   QDeclarativePath *path() const;
   void setPath(QDeclarativePath *);

   int currentIndex() const;
   void setCurrentIndex(int idx);

   qreal offset() const;
   void setOffset(qreal offset);

   QDeclarativeComponent *highlight() const;
   void setHighlight(QDeclarativeComponent *highlight);
   QDeclarativeItem *highlightItem();

   enum HighlightRangeMode { NoHighlightRange, ApplyRange, StrictlyEnforceRange };
   HighlightRangeMode highlightRangeMode() const;
   void setHighlightRangeMode(HighlightRangeMode mode);

   qreal preferredHighlightBegin() const;
   void setPreferredHighlightBegin(qreal);

   qreal preferredHighlightEnd() const;
   void setPreferredHighlightEnd(qreal);

   int highlightMoveDuration() const;
   void setHighlightMoveDuration(int);

   qreal dragMargin() const;
   void setDragMargin(qreal margin);

   qreal flickDeceleration() const;
   void setFlickDeceleration(qreal dec);

   bool isInteractive() const;
   void setInteractive(bool);

   bool isMoving() const;
   bool isFlicking() const;

   int count() const;

   QDeclarativeComponent *delegate() const;
   void setDelegate(QDeclarativeComponent *);

   int pathItemCount() const;
   void setPathItemCount(int);

   static QDeclarativePathViewAttached *qmlAttachedProperties(QObject *);

   DECL_CS_SLOT_1(Public, void incrementCurrentIndex())
   DECL_CS_SLOT_2(incrementCurrentIndex)
   DECL_CS_SLOT_1(Public, void decrementCurrentIndex())
   DECL_CS_SLOT_2(decrementCurrentIndex)

   DECL_CS_SIGNAL_1(Public, void currentIndexChanged())
   DECL_CS_SIGNAL_2(currentIndexChanged)
   DECL_CS_SIGNAL_1(Public, void offsetChanged())
   DECL_CS_SIGNAL_2(offsetChanged)
   DECL_CS_SIGNAL_1(Public, void modelChanged())
   DECL_CS_SIGNAL_2(modelChanged)
   DECL_CS_SIGNAL_1(Public, void countChanged())
   DECL_CS_SIGNAL_2(countChanged)
   DECL_CS_SIGNAL_1(Public, void pathChanged())
   DECL_CS_SIGNAL_2(pathChanged)
   DECL_CS_SIGNAL_1(Public, void preferredHighlightBeginChanged())
   DECL_CS_SIGNAL_2(preferredHighlightBeginChanged)
   DECL_CS_SIGNAL_1(Public, void preferredHighlightEndChanged())
   DECL_CS_SIGNAL_2(preferredHighlightEndChanged)
   DECL_CS_SIGNAL_1(Public, void highlightRangeModeChanged())
   DECL_CS_SIGNAL_2(highlightRangeModeChanged)
   DECL_CS_SIGNAL_1(Public, void dragMarginChanged())
   DECL_CS_SIGNAL_2(dragMarginChanged)
   DECL_CS_SIGNAL_1(Public, void snapPositionChanged())
   DECL_CS_SIGNAL_2(snapPositionChanged)
   DECL_CS_SIGNAL_1(Public, void delegateChanged())
   DECL_CS_SIGNAL_2(delegateChanged)
   DECL_CS_SIGNAL_1(Public, void pathItemCountChanged())
   DECL_CS_SIGNAL_2(pathItemCountChanged)
   DECL_CS_SIGNAL_1(Public, void flickDecelerationChanged())
   DECL_CS_SIGNAL_2(flickDecelerationChanged)
   DECL_CS_SIGNAL_1(Public, void interactiveChanged())
   DECL_CS_SIGNAL_2(interactiveChanged)
   DECL_CS_SIGNAL_1(Public, void movingChanged())
   DECL_CS_SIGNAL_2(movingChanged)
   DECL_CS_SIGNAL_1(Public, void flickingChanged())
   DECL_CS_SIGNAL_2(flickingChanged)
   DECL_CS_SIGNAL_1(Public, void highlightChanged())
   DECL_CS_SIGNAL_2(highlightChanged)
   DECL_CS_SIGNAL_1(Public, void highlightItemChanged())
   DECL_CS_SIGNAL_2(highlightItemChanged)
   DECL_CS_SIGNAL_1(Public, void highlightMoveDurationChanged())
   DECL_CS_SIGNAL_2(highlightMoveDurationChanged)
   DECL_CS_SIGNAL_1(Public, void movementStarted())
   DECL_CS_SIGNAL_2(movementStarted)
   DECL_CS_SIGNAL_1(Public, void movementEnded())
   DECL_CS_SIGNAL_2(movementEnded)
   DECL_CS_SIGNAL_1(Public, void flickStarted())
   DECL_CS_SIGNAL_2(flickStarted)
   DECL_CS_SIGNAL_1(Public, void flickEnded())
   DECL_CS_SIGNAL_2(flickEnded)

 protected:
   void mousePressEvent(QGraphicsSceneMouseEvent *event);
   void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
   void mouseReleaseEvent(QGraphicsSceneMouseEvent *);
   bool sendMouseEvent(QGraphicsSceneMouseEvent *event);
   bool sceneEventFilter(QGraphicsItem *, QEvent *);
   bool event(QEvent *event);
   void componentComplete();

 private :
   DECL_CS_SLOT_1(Private, void refill())
   DECL_CS_SLOT_2(refill)
   DECL_CS_SLOT_1(Private, void ticked())
   DECL_CS_SLOT_2(ticked)
   DECL_CS_SLOT_1(Private, void movementEnding())
   DECL_CS_SLOT_2(movementEnding)
   DECL_CS_SLOT_1(Private, void itemsInserted(int index, int count))
   DECL_CS_SLOT_2(itemsInserted)
   DECL_CS_SLOT_1(Private, void itemsRemoved(int index, int count))
   DECL_CS_SLOT_2(itemsRemoved)
   DECL_CS_SLOT_1(Private, void itemsMoved(int un_named_arg1, int un_named_arg2, int un_named_arg3))
   DECL_CS_SLOT_2(itemsMoved)
   DECL_CS_SLOT_1(Private, void modelReset())
   DECL_CS_SLOT_2(modelReset)
   DECL_CS_SLOT_1(Private, void createdItem(int index, QDeclarativeItem *item))
   DECL_CS_SLOT_2(createdItem)
   DECL_CS_SLOT_1(Private, void destroyingItem(QDeclarativeItem *item))
   DECL_CS_SLOT_2(destroyingItem)
   DECL_CS_SLOT_1(Private, void pathUpdated())
   DECL_CS_SLOT_2(pathUpdated)

   friend class QDeclarativePathViewAttached;
   Q_DISABLE_COPY(QDeclarativePathView)
   Q_DECLARE_PRIVATE_D(QGraphicsItem::d_ptr.data(), QDeclarativePathView)
};

class QDeclarativeOpenMetaObject;
class QDeclarativePathViewAttached : public QObject
{
   DECL_CS_OBJECT(QDeclarativePathViewAttached)

   DECL_CS_PROPERTY_READ(*view, view)
   DECL_CS_PROPERTY_CONSTANT(*view)
   DECL_CS_PROPERTY_READ(isCurrentItem, isCurrentItem)
   DECL_CS_PROPERTY_NOTIFY(isCurrentItem, currentItemChanged)
   DECL_CS_PROPERTY_READ(onPath, isOnPath)
   DECL_CS_PROPERTY_NOTIFY(onPath, pathChanged)

 public:
   QDeclarativePathViewAttached(QObject *parent);
   ~QDeclarativePathViewAttached();

   QDeclarativePathView *view() {
      return m_view;
   }

   bool isCurrentItem() const {
      return m_isCurrent;
   }
   void setIsCurrentItem(bool c) {
      if (m_isCurrent != c) {
         m_isCurrent = c;
         emit currentItemChanged();
      }
   }

   QVariant value(const QByteArray &name) const;
   void setValue(const QByteArray &name, const QVariant &val);

   bool isOnPath() const {
      return m_onPath;
   }
   void setOnPath(bool on) {
      if (on != m_onPath) {
         m_onPath = on;
         emit pathChanged();
      }
   }
   qreal m_percent;

   DECL_CS_SIGNAL_1(Public, void currentItemChanged())
   DECL_CS_SIGNAL_2(currentItemChanged)
   DECL_CS_SIGNAL_1(Public, void pathChanged())
   DECL_CS_SIGNAL_2(pathChanged)

 private:
   friend class QDeclarativePathViewPrivate;
   friend class QDeclarativePathView;
   QDeclarativePathView *m_view;
   QDeclarativeOpenMetaObject *m_metaobject;
   bool m_onPath : 1;
   bool m_isCurrent : 1;
};


QT_END_NAMESPACE

QML_DECLARE_TYPE(QDeclarativePathView)
QML_DECLARE_TYPEINFO(QDeclarativePathView, QML_HAS_ATTACHED_PROPERTIES)

#endif // QDECLARATIVEPATHVIEW_H
