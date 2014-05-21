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

#ifndef QDECLARATIVEPATHVIEW_H
#define QDECLARATIVEPATHVIEW_H

#include "qdeclarativeitem.h"
#include "private/qdeclarativepath_p.h"

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE


class QDeclarativePathViewPrivate;
class QDeclarativePathViewAttached;
class QDeclarativePathView : public QDeclarativeItem
{
    CS_OBJECT(QDeclarativePathView)

    CS_PROPERTY_READ(model, model)
    CS_PROPERTY_WRITE(model, setModel)
    CS_PROPERTY_NOTIFY(model, modelChanged)
    CS_PROPERTY_READ(*path, path)
    CS_PROPERTY_WRITE(*path, setPath)
    CS_PROPERTY_NOTIFY(*path, pathChanged)
    CS_PROPERTY_READ(currentIndex, currentIndex)
    CS_PROPERTY_WRITE(currentIndex, setCurrentIndex)
    CS_PROPERTY_NOTIFY(currentIndex, currentIndexChanged)
    CS_PROPERTY_READ(offset, offset)
    CS_PROPERTY_WRITE(offset, setOffset)
    CS_PROPERTY_NOTIFY(offset, offsetChanged)

    CS_PROPERTY_READ(*highlight, highlight)
    CS_PROPERTY_WRITE(*highlight, setHighlight)
    CS_PROPERTY_NOTIFY(*highlight, highlightChanged)
    CS_PROPERTY_READ(*highlightItem, highlightItem)
    CS_PROPERTY_NOTIFY(*highlightItem, highlightItemChanged)

    CS_PROPERTY_READ(preferredHighlightBegin, preferredHighlightBegin)
    CS_PROPERTY_WRITE(preferredHighlightBegin, setPreferredHighlightBegin)
    CS_PROPERTY_NOTIFY(preferredHighlightBegin, preferredHighlightBeginChanged)
    CS_PROPERTY_READ(preferredHighlightEnd, preferredHighlightEnd)
    CS_PROPERTY_WRITE(preferredHighlightEnd, setPreferredHighlightEnd)
    CS_PROPERTY_NOTIFY(preferredHighlightEnd, preferredHighlightEndChanged)
    CS_PROPERTY_READ(highlightRangeMode, highlightRangeMode)
    CS_PROPERTY_WRITE(highlightRangeMode, setHighlightRangeMode)
    CS_PROPERTY_NOTIFY(highlightRangeMode, highlightRangeModeChanged)
    CS_PROPERTY_READ(highlightMoveDuration, highlightMoveDuration)
    CS_PROPERTY_WRITE(highlightMoveDuration, setHighlightMoveDuration)
    CS_PROPERTY_NOTIFY(highlightMoveDuration, highlightMoveDurationChanged)

    CS_PROPERTY_READ(dragMargin, dragMargin)
    CS_PROPERTY_WRITE(dragMargin, setDragMargin)
    CS_PROPERTY_NOTIFY(dragMargin, dragMarginChanged)
    CS_PROPERTY_READ(flickDeceleration, flickDeceleration)
    CS_PROPERTY_WRITE(flickDeceleration, setFlickDeceleration)
    CS_PROPERTY_NOTIFY(flickDeceleration, flickDecelerationChanged)
    CS_PROPERTY_READ(interactive, isInteractive)
    CS_PROPERTY_WRITE(interactive, setInteractive)
    CS_PROPERTY_NOTIFY(interactive, interactiveChanged)

    CS_PROPERTY_READ(moving, isMoving)
    CS_PROPERTY_NOTIFY(moving, movingChanged)
    CS_PROPERTY_READ(flicking, isFlicking)
    CS_PROPERTY_NOTIFY(flicking, flickingChanged)

    CS_PROPERTY_READ(count, count)
    CS_PROPERTY_NOTIFY(count, countChanged)
    CS_PROPERTY_READ(*delegate, delegate)
    CS_PROPERTY_WRITE(*delegate, setDelegate)
    CS_PROPERTY_NOTIFY(*delegate, delegateChanged)
    CS_PROPERTY_READ(pathItemCount, pathItemCount)
    CS_PROPERTY_WRITE(pathItemCount, setPathItemCount)
    CS_PROPERTY_NOTIFY(pathItemCount, pathItemCountChanged)

    CS_ENUM(HighlightRangeMode)

public:
    QDeclarativePathView(QDeclarativeItem *parent=0);
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

public :
    CS_SLOT_1(Public, void incrementCurrentIndex())
    CS_SLOT_2(incrementCurrentIndex) 
    CS_SLOT_1(Public, void decrementCurrentIndex())
    CS_SLOT_2(decrementCurrentIndex) 

public:
    CS_SIGNAL_1(Public, void currentIndexChanged())
    CS_SIGNAL_2(currentIndexChanged) 
    CS_SIGNAL_1(Public, void offsetChanged())
    CS_SIGNAL_2(offsetChanged) 
    CS_SIGNAL_1(Public, void modelChanged())
    CS_SIGNAL_2(modelChanged) 
    CS_SIGNAL_1(Public, void countChanged())
    CS_SIGNAL_2(countChanged) 
    CS_SIGNAL_1(Public, void pathChanged())
    CS_SIGNAL_2(pathChanged) 
    CS_SIGNAL_1(Public, void preferredHighlightBeginChanged())
    CS_SIGNAL_2(preferredHighlightBeginChanged) 
    CS_SIGNAL_1(Public, void preferredHighlightEndChanged())
    CS_SIGNAL_2(preferredHighlightEndChanged) 
    CS_SIGNAL_1(Public, void highlightRangeModeChanged())
    CS_SIGNAL_2(highlightRangeModeChanged) 
    CS_SIGNAL_1(Public, void dragMarginChanged())
    CS_SIGNAL_2(dragMarginChanged) 
    CS_SIGNAL_1(Public, void snapPositionChanged())
    CS_SIGNAL_2(snapPositionChanged) 
    CS_SIGNAL_1(Public, void delegateChanged())
    CS_SIGNAL_2(delegateChanged) 
    CS_SIGNAL_1(Public, void pathItemCountChanged())
    CS_SIGNAL_2(pathItemCountChanged) 
    CS_SIGNAL_1(Public, void flickDecelerationChanged())
    CS_SIGNAL_2(flickDecelerationChanged) 
    CS_SIGNAL_1(Public, void interactiveChanged())
    CS_SIGNAL_2(interactiveChanged) 
    CS_SIGNAL_1(Public, void movingChanged())
    CS_SIGNAL_2(movingChanged) 
    CS_SIGNAL_1(Public, void flickingChanged())
    CS_SIGNAL_2(flickingChanged) 
    CS_SIGNAL_1(Public, void highlightChanged())
    CS_SIGNAL_2(highlightChanged) 
    CS_SIGNAL_1(Public, void highlightItemChanged())
    CS_SIGNAL_2(highlightItemChanged) 
    CS_SIGNAL_1(Public, void highlightMoveDurationChanged())
    CS_SIGNAL_2(highlightMoveDurationChanged) 
    CS_SIGNAL_1(Public, void movementStarted())
    CS_SIGNAL_2(movementStarted) 
    CS_SIGNAL_1(Public, void movementEnded())
    CS_SIGNAL_2(movementEnded) 
    CS_SIGNAL_1(Public, void flickStarted())
    CS_SIGNAL_2(flickStarted) 
    CS_SIGNAL_1(Public, void flickEnded())
    CS_SIGNAL_2(flickEnded) 

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *);
    bool sendMouseEvent(QGraphicsSceneMouseEvent *event);
    bool sceneEventFilter(QGraphicsItem *, QEvent *);
    bool event(QEvent *event);
    void componentComplete();

private :
    CS_SLOT_1(Private, void refill())
    CS_SLOT_2(refill) 
    CS_SLOT_1(Private, void ticked())
    CS_SLOT_2(ticked) 
    CS_SLOT_1(Private, void movementEnding())
    CS_SLOT_2(movementEnding) 
    CS_SLOT_1(Private, void itemsInserted(int index,int count))
    CS_SLOT_2(itemsInserted) 
    CS_SLOT_1(Private, void itemsRemoved(int index,int count))
    CS_SLOT_2(itemsRemoved) 
    CS_SLOT_1(Private, void itemsMoved(int un_named_arg1,int un_named_arg2,int un_named_arg3))
    CS_SLOT_2(itemsMoved) 
    CS_SLOT_1(Private, void modelReset())
    CS_SLOT_2(modelReset) 
    CS_SLOT_1(Private, void createdItem(int index,QDeclarativeItem * item))
    CS_SLOT_2(createdItem) 
    CS_SLOT_1(Private, void destroyingItem(QDeclarativeItem * item))
    CS_SLOT_2(destroyingItem) 
    CS_SLOT_1(Private, void pathUpdated())
    CS_SLOT_2(pathUpdated) 

private:
    friend class QDeclarativePathViewAttached;
    Q_DISABLE_COPY(QDeclarativePathView)
    Q_DECLARE_PRIVATE_D(QGraphicsItem::d_ptr.data(), QDeclarativePathView)
};

class QDeclarativeOpenMetaObject;
class QDeclarativePathViewAttached : public QObject
{
    CS_OBJECT(QDeclarativePathViewAttached)

    CS_PROPERTY_READ(*view, view)
    CS_PROPERTY_CONSTANT(*view)
    CS_PROPERTY_READ(isCurrentItem, isCurrentItem)
    CS_PROPERTY_NOTIFY(isCurrentItem, currentItemChanged)
    CS_PROPERTY_READ(onPath, isOnPath)
    CS_PROPERTY_NOTIFY(onPath, pathChanged)

public:
    QDeclarativePathViewAttached(QObject *parent);
    ~QDeclarativePathViewAttached();

    QDeclarativePathView *view() { return m_view; }

    bool isCurrentItem() const { return m_isCurrent; }
    void setIsCurrentItem(bool c) {
        if (m_isCurrent != c) {
            m_isCurrent = c;
            emit currentItemChanged();
        }
    }

    QVariant value(const QByteArray &name) const;
    void setValue(const QByteArray &name, const QVariant &val);

    bool isOnPath() const { return m_onPath; }
    void setOnPath(bool on) {
        if (on != m_onPath) {
            m_onPath = on;
            emit pathChanged();
        }
    }
    qreal m_percent;

public:
    CS_SIGNAL_1(Public, void currentItemChanged())
    CS_SIGNAL_2(currentItemChanged) 
    CS_SIGNAL_1(Public, void pathChanged())
    CS_SIGNAL_2(pathChanged) 

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
QT_END_HEADER

#endif // QDECLARATIVEPATHVIEW_H
