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

#include <qgraphicssceneevent.h>

#ifndef QT_NO_GRAPHICSVIEW

#include <qmap.h>
#include <qpoint.h>
#include <qsize.h>
#include <qstring.h>
#include <qgraphicsview.h>
#include <qgraphicsitem.h>
#include <qgesture.h>
#include <qevent_p.h>

class QGraphicsSceneEventPrivate
{
 public:
   inline QGraphicsSceneEventPrivate()
      : widget(nullptr), q_ptr(nullptr)
   {
   }

   inline virtual ~QGraphicsSceneEventPrivate() {
   }

   QWidget *widget;
   QGraphicsSceneEvent *q_ptr;
};

QGraphicsSceneEvent::QGraphicsSceneEvent(Type type)
   : QEvent(type), d_ptr(new QGraphicsSceneEventPrivate)
{
   d_ptr->q_ptr = this;
}

QGraphicsSceneEvent::QGraphicsSceneEvent(QGraphicsSceneEventPrivate &dd, Type type)
   : QEvent(type), d_ptr(&dd)
{
   d_ptr->q_ptr = this;
}

QGraphicsSceneEvent::~QGraphicsSceneEvent()
{
}

QWidget *QGraphicsSceneEvent::widget() const
{
   return d_ptr->widget;
}

// internal
void QGraphicsSceneEvent::setWidget(QWidget *widget)
{
   d_ptr->widget = widget;
}

class QGraphicsSceneMouseEventPrivate : public QGraphicsSceneEventPrivate
{
   Q_DECLARE_PUBLIC(QGraphicsSceneMouseEvent)

 public:
   QGraphicsSceneMouseEventPrivate()
      : button(Qt::NoButton), buttons(Qt::EmptyFlag), modifiers(Qt::EmptyFlag),
        source(Qt::MouseEventNotSynthesized), flags(Qt::EmptyFlag)
   { }

   QPointF pos;
   QPointF scenePos;
   QPoint screenPos;
   QPointF lastPos;
   QPointF lastScenePos;
   QPoint lastScreenPos;
   QMap<Qt::MouseButton, QPointF> buttonDownPos;
   QMap<Qt::MouseButton, QPointF> buttonDownScenePos;
   QMap<Qt::MouseButton, QPoint> buttonDownScreenPos;

   Qt::MouseButton button;
   Qt::MouseButtons buttons;
   Qt::KeyboardModifiers modifiers;
   Qt::MouseEventSource source;
   Qt::MouseEventFlags flags;
};

// internal
QGraphicsSceneMouseEvent::QGraphicsSceneMouseEvent(Type type)
   : QGraphicsSceneEvent(*new QGraphicsSceneMouseEventPrivate, type)
{
}

QGraphicsSceneMouseEvent::~QGraphicsSceneMouseEvent()
{
}

QPointF QGraphicsSceneMouseEvent::pos() const
{
   Q_D(const QGraphicsSceneMouseEvent);
   return d->pos;
}

// internal
void QGraphicsSceneMouseEvent::setPos(const QPointF &pos)
{
   Q_D(QGraphicsSceneMouseEvent);
   d->pos = pos;
}

QPointF QGraphicsSceneMouseEvent::scenePos() const
{
   Q_D(const QGraphicsSceneMouseEvent);
   return d->scenePos;
}

// internal (cs)
void QGraphicsSceneMouseEvent::setScenePos(const QPointF &pos)
{
   Q_D(QGraphicsSceneMouseEvent);
   d->scenePos = pos;
}

QPoint QGraphicsSceneMouseEvent::screenPos() const
{
   Q_D(const QGraphicsSceneMouseEvent);
   return d->screenPos;
}

// internal (cs)
void QGraphicsSceneMouseEvent::setScreenPos(const QPoint &pos)
{
   Q_D(QGraphicsSceneMouseEvent);
   d->screenPos = pos;
}

QPointF QGraphicsSceneMouseEvent::buttonDownPos(Qt::MouseButton button) const
{
   Q_D(const QGraphicsSceneMouseEvent);
   return d->buttonDownPos.value(button);
}

// internal (cs)
void QGraphicsSceneMouseEvent::setButtonDownPos(Qt::MouseButton button, const QPointF &pos)
{
   Q_D(QGraphicsSceneMouseEvent);
   d->buttonDownPos.insert(button, pos);
}

QPointF QGraphicsSceneMouseEvent::buttonDownScenePos(Qt::MouseButton button) const
{
   Q_D(const QGraphicsSceneMouseEvent);
   return d->buttonDownScenePos.value(button);
}

// internal (cs)
void QGraphicsSceneMouseEvent::setButtonDownScenePos(Qt::MouseButton button, const QPointF &pos)
{
   Q_D(QGraphicsSceneMouseEvent);
   d->buttonDownScenePos.insert(button, pos);
}


QPoint QGraphicsSceneMouseEvent::buttonDownScreenPos(Qt::MouseButton button) const
{
   Q_D(const QGraphicsSceneMouseEvent);
   return d->buttonDownScreenPos.value(button);
}

// internal (cs)
void QGraphicsSceneMouseEvent::setButtonDownScreenPos(Qt::MouseButton button, const QPoint &pos)
{
   Q_D(QGraphicsSceneMouseEvent);
   d->buttonDownScreenPos.insert(button, pos);
}

QPointF QGraphicsSceneMouseEvent::lastPos() const
{
   Q_D(const QGraphicsSceneMouseEvent);
   return d->lastPos;
}

// internal (cs)
void QGraphicsSceneMouseEvent::setLastPos(const QPointF &pos)
{
   Q_D(QGraphicsSceneMouseEvent);
   d->lastPos = pos;
}

QPointF QGraphicsSceneMouseEvent::lastScenePos() const
{
   Q_D(const QGraphicsSceneMouseEvent);
   return d->lastScenePos;
}

// internal (cs)
void QGraphicsSceneMouseEvent::setLastScenePos(const QPointF &pos)
{
   Q_D(QGraphicsSceneMouseEvent);
   d->lastScenePos = pos;
}

QPoint QGraphicsSceneMouseEvent::lastScreenPos() const
{
   Q_D(const QGraphicsSceneMouseEvent);
   return d->lastScreenPos;
}

// internal (cs)
void QGraphicsSceneMouseEvent::setLastScreenPos(const QPoint &pos)
{
   Q_D(QGraphicsSceneMouseEvent);
   d->lastScreenPos = pos;
}

Qt::MouseButtons QGraphicsSceneMouseEvent::buttons() const
{
   Q_D(const QGraphicsSceneMouseEvent);
   return d->buttons;
}

// internal
void QGraphicsSceneMouseEvent::setButtons(Qt::MouseButtons buttons)
{
   Q_D(QGraphicsSceneMouseEvent);
   d->buttons = buttons;
}

Qt::MouseButton QGraphicsSceneMouseEvent::button() const
{
   Q_D(const QGraphicsSceneMouseEvent);
   return d->button;
}

// internal
void QGraphicsSceneMouseEvent::setButton(Qt::MouseButton button)
{
   Q_D(QGraphicsSceneMouseEvent);
   d->button = button;
}

Qt::KeyboardModifiers QGraphicsSceneMouseEvent::modifiers() const
{
   Q_D(const QGraphicsSceneMouseEvent);
   return d->modifiers;
}

Qt::MouseEventSource QGraphicsSceneMouseEvent::source() const
{
   Q_D(const QGraphicsSceneMouseEvent);
   return d->source;
}

void QGraphicsSceneMouseEvent::setSource(Qt::MouseEventSource source)
{
   Q_D(QGraphicsSceneMouseEvent);
   d->source = source;
}

Qt::MouseEventFlags QGraphicsSceneMouseEvent::flags() const
{
   Q_D(const QGraphicsSceneMouseEvent);
   return d->flags;
}

void QGraphicsSceneMouseEvent::setFlags(Qt::MouseEventFlags flags)
{
   Q_D(QGraphicsSceneMouseEvent);
   d->flags = flags;
}

void QGraphicsSceneMouseEvent::setModifiers(Qt::KeyboardModifiers modifiers)
{
   Q_D(QGraphicsSceneMouseEvent);
   d->modifiers = modifiers;
}

class QGraphicsSceneWheelEventPrivate : public QGraphicsSceneEventPrivate
{
   Q_DECLARE_PUBLIC(QGraphicsSceneWheelEvent)

 public:
   QGraphicsSceneWheelEventPrivate()
      : buttons(Qt::EmptyFlag), modifiers(Qt::EmptyFlag), delta(0), orientation(Qt::Horizontal) {
   }

   QPointF pos;
   QPointF scenePos;
   QPoint screenPos;

   Qt::MouseButtons buttons;
   Qt::KeyboardModifiers modifiers;
   int delta;
   Qt::Orientation orientation;
};

// internal
QGraphicsSceneWheelEvent::QGraphicsSceneWheelEvent(Type type)
   : QGraphicsSceneEvent(*new QGraphicsSceneWheelEventPrivate, type)
{
}

QGraphicsSceneWheelEvent::~QGraphicsSceneWheelEvent()
{
}

QPointF QGraphicsSceneWheelEvent::pos() const
{
   Q_D(const QGraphicsSceneWheelEvent);
   return d->pos;
}

// internal
void QGraphicsSceneWheelEvent::setPos(const QPointF &pos)
{
   Q_D(QGraphicsSceneWheelEvent);
   d->pos = pos;
}

QPointF QGraphicsSceneWheelEvent::scenePos() const
{
   Q_D(const QGraphicsSceneWheelEvent);
   return d->scenePos;
}

// internal (cs)
void QGraphicsSceneWheelEvent::setScenePos(const QPointF &pos)
{
   Q_D(QGraphicsSceneWheelEvent);
   d->scenePos = pos;
}

QPoint QGraphicsSceneWheelEvent::screenPos() const
{
   Q_D(const QGraphicsSceneWheelEvent);
   return d->screenPos;
}

// internal (cs)
void QGraphicsSceneWheelEvent::setScreenPos(const QPoint &pos)
{
   Q_D(QGraphicsSceneWheelEvent);
   d->screenPos = pos;
}

Qt::MouseButtons QGraphicsSceneWheelEvent::buttons() const
{
   Q_D(const QGraphicsSceneWheelEvent);
   return d->buttons;
}

// internal (cs)
void QGraphicsSceneWheelEvent::setButtons(Qt::MouseButtons buttons)
{
   Q_D(QGraphicsSceneWheelEvent);
   d->buttons = buttons;
}

Qt::KeyboardModifiers QGraphicsSceneWheelEvent::modifiers() const
{
   Q_D(const QGraphicsSceneWheelEvent);
   return d->modifiers;
}

// internal (cs)
void QGraphicsSceneWheelEvent::setModifiers(Qt::KeyboardModifiers modifiers)
{
   Q_D(QGraphicsSceneWheelEvent);
   d->modifiers = modifiers;
}

int QGraphicsSceneWheelEvent::delta() const
{
   Q_D(const QGraphicsSceneWheelEvent);
   return d->delta;
}

// internal
void QGraphicsSceneWheelEvent::setDelta(int delta)
{
   Q_D(QGraphicsSceneWheelEvent);
   d->delta = delta;
}

Qt::Orientation QGraphicsSceneWheelEvent::orientation() const
{
   Q_D(const QGraphicsSceneWheelEvent);
   return d->orientation;
}

// internal
void QGraphicsSceneWheelEvent::setOrientation(Qt::Orientation orientation)
{
   Q_D(QGraphicsSceneWheelEvent);
   d->orientation = orientation;
}

class QGraphicsSceneContextMenuEventPrivate : public QGraphicsSceneEventPrivate
{
   Q_DECLARE_PUBLIC(QGraphicsSceneContextMenuEvent)

 public:
   QGraphicsSceneContextMenuEventPrivate()
      : modifiers(Qt::EmptyFlag), reason(QGraphicsSceneContextMenuEvent::Other)
   {
   }

   QPointF pos;
   QPointF scenePos;
   QPoint screenPos;
   Qt::KeyboardModifiers modifiers;
   QGraphicsSceneContextMenuEvent::Reason reason;
};

// internal
QGraphicsSceneContextMenuEvent::QGraphicsSceneContextMenuEvent(Type type)
   : QGraphicsSceneEvent(*new QGraphicsSceneContextMenuEventPrivate, type)
{
}

QGraphicsSceneContextMenuEvent::~QGraphicsSceneContextMenuEvent()
{
}

QPointF QGraphicsSceneContextMenuEvent::pos() const
{
   Q_D(const QGraphicsSceneContextMenuEvent);
   return d->pos;
}

void QGraphicsSceneContextMenuEvent::setPos(const QPointF &pos)
{
   Q_D(QGraphicsSceneContextMenuEvent);
   d->pos = pos;
}

QPointF QGraphicsSceneContextMenuEvent::scenePos() const
{
   Q_D(const QGraphicsSceneContextMenuEvent);
   return d->scenePos;
}

// internal (cs)
void QGraphicsSceneContextMenuEvent::setScenePos(const QPointF &pos)
{
   Q_D(QGraphicsSceneContextMenuEvent);
   d->scenePos = pos;
}

QPoint QGraphicsSceneContextMenuEvent::screenPos() const
{
   Q_D(const QGraphicsSceneContextMenuEvent);
   return d->screenPos;
}

// internal (cs)
void QGraphicsSceneContextMenuEvent::setScreenPos(const QPoint &pos)
{
   Q_D(QGraphicsSceneContextMenuEvent);
   d->screenPos = pos;
}

Qt::KeyboardModifiers QGraphicsSceneContextMenuEvent::modifiers() const
{
   Q_D(const QGraphicsSceneContextMenuEvent);
   return d->modifiers;
}

// internal (cs)
void QGraphicsSceneContextMenuEvent::setModifiers(Qt::KeyboardModifiers modifiers)
{
   Q_D(QGraphicsSceneContextMenuEvent);
   d->modifiers = modifiers;
}

QGraphicsSceneContextMenuEvent::Reason QGraphicsSceneContextMenuEvent::reason() const
{
   Q_D(const QGraphicsSceneContextMenuEvent);
   return d->reason;
}

// internal (cs)
void QGraphicsSceneContextMenuEvent::setReason(Reason reason)
{
   Q_D(QGraphicsSceneContextMenuEvent);
   d->reason = reason;
}

class QGraphicsSceneHoverEventPrivate : public QGraphicsSceneEventPrivate
{
 public:
   QPointF pos;
   QPointF scenePos;
   QPoint screenPos;
   QPointF lastPos;
   QPointF lastScenePos;
   QPoint lastScreenPos;
   Qt::KeyboardModifiers modifiers;
};

// internal (cs)
QGraphicsSceneHoverEvent::QGraphicsSceneHoverEvent(Type type)
   : QGraphicsSceneEvent(*new QGraphicsSceneHoverEventPrivate, type)
{
}

QGraphicsSceneHoverEvent::~QGraphicsSceneHoverEvent()
{
}

QPointF QGraphicsSceneHoverEvent::pos() const
{
   Q_D(const QGraphicsSceneHoverEvent);
   return d->pos;
}

void QGraphicsSceneHoverEvent::setPos(const QPointF &pos)
{
   Q_D(QGraphicsSceneHoverEvent);
   d->pos = pos;
}

QPointF QGraphicsSceneHoverEvent::scenePos() const
{
   Q_D(const QGraphicsSceneHoverEvent);
   return d->scenePos;
}

// internal (cs)
void QGraphicsSceneHoverEvent::setScenePos(const QPointF &pos)
{
   Q_D(QGraphicsSceneHoverEvent);
   d->scenePos = pos;
}

QPoint QGraphicsSceneHoverEvent::screenPos() const
{
   Q_D(const QGraphicsSceneHoverEvent);
   return d->screenPos;
}

// internal (cs)
void QGraphicsSceneHoverEvent::setScreenPos(const QPoint &pos)
{
   Q_D(QGraphicsSceneHoverEvent);
   d->screenPos = pos;
}

QPointF QGraphicsSceneHoverEvent::lastPos() const
{
   Q_D(const QGraphicsSceneHoverEvent);
   return d->lastPos;
}

/*!
    \internal
*/
void QGraphicsSceneHoverEvent::setLastPos(const QPointF &pos)
{
   Q_D(QGraphicsSceneHoverEvent);
   d->lastPos = pos;
}

QPointF QGraphicsSceneHoverEvent::lastScenePos() const
{
   Q_D(const QGraphicsSceneHoverEvent);
   return d->lastScenePos;
}

/*!
    \internal
*/
void QGraphicsSceneHoverEvent::setLastScenePos(const QPointF &pos)
{
   Q_D(QGraphicsSceneHoverEvent);
   d->lastScenePos = pos;
}

QPoint QGraphicsSceneHoverEvent::lastScreenPos() const
{
   Q_D(const QGraphicsSceneHoverEvent);
   return d->lastScreenPos;
}

/*!
    \internal
*/
void QGraphicsSceneHoverEvent::setLastScreenPos(const QPoint &pos)
{
   Q_D(QGraphicsSceneHoverEvent);
   d->lastScreenPos = pos;
}

Qt::KeyboardModifiers QGraphicsSceneHoverEvent::modifiers() const
{
   Q_D(const QGraphicsSceneHoverEvent);
   return d->modifiers;
}

void QGraphicsSceneHoverEvent::setModifiers(Qt::KeyboardModifiers modifiers)
{
   Q_D(QGraphicsSceneHoverEvent);
   d->modifiers = modifiers;
}

class QGraphicsSceneHelpEventPrivate : public QGraphicsSceneEventPrivate
{
 public:
   QPointF scenePos;
   QPoint screenPos;
};

// internal
QGraphicsSceneHelpEvent::QGraphicsSceneHelpEvent(Type type)
   : QGraphicsSceneEvent(*new QGraphicsSceneHelpEventPrivate, type)
{
}

QGraphicsSceneHelpEvent::~QGraphicsSceneHelpEvent()
{
}

QPointF QGraphicsSceneHelpEvent::scenePos() const
{
   Q_D(const QGraphicsSceneHelpEvent);
   return d->scenePos;
}

// internal (cs)
void QGraphicsSceneHelpEvent::setScenePos(const QPointF &pos)
{
   Q_D(QGraphicsSceneHelpEvent);
   d->scenePos = pos;
}

QPoint QGraphicsSceneHelpEvent::screenPos() const
{
   Q_D(const QGraphicsSceneHelpEvent);
   return d->screenPos;
}

// internal (cs)
void QGraphicsSceneHelpEvent::setScreenPos(const QPoint &pos)
{
   Q_D(QGraphicsSceneHelpEvent);
   d->screenPos = pos;
}

class QGraphicsSceneDragDropEventPrivate : public QGraphicsSceneEventPrivate
{
   Q_DECLARE_PUBLIC(QGraphicsSceneDragDropEvent)

 public:
   inline QGraphicsSceneDragDropEventPrivate()
      : source(nullptr), mimeData(nullptr)
   {
   }

   QPointF pos;
   QPointF scenePos;
   QPoint screenPos;
   Qt::MouseButtons buttons;
   Qt::KeyboardModifiers modifiers;
   Qt::DropActions possibleActions;
   Qt::DropAction proposedAction;
   Qt::DropAction dropAction;
   QWidget *source;
   const QMimeData *mimeData;
};

// internal
QGraphicsSceneDragDropEvent::QGraphicsSceneDragDropEvent(Type type)
   : QGraphicsSceneEvent(*new QGraphicsSceneDragDropEventPrivate, type)
{
}

QGraphicsSceneDragDropEvent::~QGraphicsSceneDragDropEvent()
{
}

QPointF QGraphicsSceneDragDropEvent::pos() const
{
   Q_D(const QGraphicsSceneDragDropEvent);
   return d->pos;
}

// internal
void QGraphicsSceneDragDropEvent::setPos(const QPointF &pos)
{
   Q_D(QGraphicsSceneDragDropEvent);
   d->pos = pos;
}

QPointF QGraphicsSceneDragDropEvent::scenePos() const
{
   Q_D(const QGraphicsSceneDragDropEvent);
   return d->scenePos;
}

// internal (cs)
void QGraphicsSceneDragDropEvent::setScenePos(const QPointF &pos)
{
   Q_D(QGraphicsSceneDragDropEvent);
   d->scenePos = pos;
}

QPoint QGraphicsSceneDragDropEvent::screenPos() const
{
   Q_D(const QGraphicsSceneDragDropEvent);
   return d->screenPos;
}

// internal (cs)
void QGraphicsSceneDragDropEvent::setScreenPos(const QPoint &pos)
{
   Q_D(QGraphicsSceneDragDropEvent);
   d->screenPos = pos;
}

Qt::MouseButtons QGraphicsSceneDragDropEvent::buttons() const
{
   Q_D(const QGraphicsSceneDragDropEvent);
   return d->buttons;
}

// internal
void QGraphicsSceneDragDropEvent::setButtons(Qt::MouseButtons buttons)
{
   Q_D(QGraphicsSceneDragDropEvent);
   d->buttons = buttons;
}

Qt::KeyboardModifiers QGraphicsSceneDragDropEvent::modifiers() const
{
   Q_D(const QGraphicsSceneDragDropEvent);
   return d->modifiers;
}

// internal
void QGraphicsSceneDragDropEvent::setModifiers(Qt::KeyboardModifiers modifiers)
{
   Q_D(QGraphicsSceneDragDropEvent);
   d->modifiers = modifiers;
}

Qt::DropActions QGraphicsSceneDragDropEvent::possibleActions() const
{
   Q_D(const QGraphicsSceneDragDropEvent);
   return d->possibleActions;
}

// internal
void QGraphicsSceneDragDropEvent::setPossibleActions(Qt::DropActions actions)
{
   Q_D(QGraphicsSceneDragDropEvent);
   d->possibleActions = actions;
}

Qt::DropAction QGraphicsSceneDragDropEvent::proposedAction() const
{
   Q_D(const QGraphicsSceneDragDropEvent);
   return d->proposedAction;
}

// internal

void QGraphicsSceneDragDropEvent::setProposedAction(Qt::DropAction action)
{
   Q_D(QGraphicsSceneDragDropEvent);
   d->proposedAction = action;
}

void QGraphicsSceneDragDropEvent::acceptProposedAction()
{
   Q_D(QGraphicsSceneDragDropEvent);
   d->dropAction = d->proposedAction;
}

Qt::DropAction QGraphicsSceneDragDropEvent::dropAction() const
{
   Q_D(const QGraphicsSceneDragDropEvent);
   return d->dropAction;
}

void QGraphicsSceneDragDropEvent::setDropAction(Qt::DropAction action)
{
   Q_D(QGraphicsSceneDragDropEvent);
   d->dropAction = action;
}

QWidget *QGraphicsSceneDragDropEvent::source() const
{
   Q_D(const QGraphicsSceneDragDropEvent);
   return d->source;
}

// internal
void QGraphicsSceneDragDropEvent::setSource(QWidget *source)
{
   Q_D(QGraphicsSceneDragDropEvent);
   d->source = source;
}

const QMimeData *QGraphicsSceneDragDropEvent::mimeData() const
{
   Q_D(const QGraphicsSceneDragDropEvent);
   return d->mimeData;
}

// internal
void QGraphicsSceneDragDropEvent::setMimeData(const QMimeData *data)
{
   Q_D(QGraphicsSceneDragDropEvent);
   d->mimeData = data;
}

class QGraphicsSceneResizeEventPrivate : public QGraphicsSceneEventPrivate
{
   Q_DECLARE_PUBLIC(QGraphicsSceneResizeEvent)

 public:
   inline QGraphicsSceneResizeEventPrivate() {
   }

   QSizeF oldSize;
   QSizeF newSize;
};

QGraphicsSceneResizeEvent::QGraphicsSceneResizeEvent()
   : QGraphicsSceneEvent(*new QGraphicsSceneResizeEventPrivate, QEvent::GraphicsSceneResize)
{
}

QGraphicsSceneResizeEvent::~QGraphicsSceneResizeEvent()
{
}

QSizeF QGraphicsSceneResizeEvent::oldSize() const
{
   Q_D(const QGraphicsSceneResizeEvent);
   return d->oldSize;
}

// internal
void QGraphicsSceneResizeEvent::setOldSize(const QSizeF &size)
{
   Q_D(QGraphicsSceneResizeEvent);
   d->oldSize = size;
}

QSizeF QGraphicsSceneResizeEvent::newSize() const
{
   Q_D(const QGraphicsSceneResizeEvent);
   return d->newSize;
}

// internal
void QGraphicsSceneResizeEvent::setNewSize(const QSizeF &size)
{
   Q_D(QGraphicsSceneResizeEvent);
   d->newSize = size;
}

class QGraphicsSceneMoveEventPrivate : public QGraphicsSceneEventPrivate
{
   Q_DECLARE_PUBLIC(QGraphicsSceneMoveEvent)

 public:
   inline QGraphicsSceneMoveEventPrivate() {
   }

   QPointF oldPos;
   QPointF newPos;
};

QGraphicsSceneMoveEvent::QGraphicsSceneMoveEvent()
   : QGraphicsSceneEvent(*new QGraphicsSceneMoveEventPrivate, QEvent::GraphicsSceneMove)
{
}

QGraphicsSceneMoveEvent::~QGraphicsSceneMoveEvent()
{
}

QPointF QGraphicsSceneMoveEvent::oldPos() const
{
   Q_D(const QGraphicsSceneMoveEvent);
   return d->oldPos;
}

// internal
void QGraphicsSceneMoveEvent::setOldPos(const QPointF &pos)
{
   Q_D(QGraphicsSceneMoveEvent);
   d->oldPos = pos;
}

QPointF QGraphicsSceneMoveEvent::newPos() const
{
   Q_D(const QGraphicsSceneMoveEvent);
   return d->newPos;
}

// internal
void QGraphicsSceneMoveEvent::setNewPos(const QPointF &pos)
{
   Q_D(QGraphicsSceneMoveEvent);
   d->newPos = pos;
}

#endif // QT_NO_GRAPHICSVIEW
