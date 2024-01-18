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

#include <qdrag.h>

#include <qpixmap.h>
#include <qpoint.h>

#include <qdnd_p.h>
#include <qguiapplication_p.h>

#ifndef QT_NO_DRAGANDDROP

QDrag::QDrag(QObject *dragSource)
   : QObject(dragSource), d_ptr(new QDragPrivate)
{
   Q_D(QDrag);

   d->source  = dragSource;
   d->target  = nullptr;
   d->data    = nullptr;
   d->hotspot = QPoint(-10, -10);

   d->executed_action   = Qt::IgnoreAction;
   d->supported_actions = Qt::IgnoreAction;
   d->default_action   = Qt::IgnoreAction;
}

QDrag::~QDrag()
{
   Q_D(QDrag);
   delete d->data;
}

void QDrag::setMimeData(QMimeData *data)
{
   Q_D(QDrag);
   if (d->data == data) {
      return;
   }
   if (d->data != nullptr) {
      delete d->data;
   }
   d->data = data;
}

QMimeData *QDrag::mimeData() const
{
   Q_D(const QDrag);
   return d->data;
}

void QDrag::setPixmap(const QPixmap &pixmap)
{
   Q_D(QDrag);
   d->pixmap = pixmap;
}

QPixmap QDrag::pixmap() const
{
   Q_D(const QDrag);
   return d->pixmap;
}

void QDrag::setHotSpot(const QPoint &hotspot)
{
   Q_D(QDrag);
   d->hotspot = hotspot;
}

QPoint QDrag::hotSpot() const
{
   Q_D(const QDrag);
   return d->hotspot;
}

QObject *QDrag::source() const
{
   Q_D(const QDrag);
   return d->source;
}

QObject *QDrag::target() const
{
   Q_D(const QDrag);
   return d->target;
}

Qt::DropAction QDrag::exec(Qt::DropActions supportedActions)
{
   return exec(supportedActions, Qt::IgnoreAction);
}

Qt::DropAction QDrag::exec(Qt::DropActions supportedActions, Qt::DropAction defaultDropAction)
{
   Q_D(QDrag);

   if (! d->data) {
      qWarning("QDrag::exec() Mime data was not set before starting the drag");
      return d->executed_action;
   }

   Qt::DropAction transformedDefaultDropAction = Qt::IgnoreAction;

   if (defaultDropAction == Qt::IgnoreAction) {
      if (supportedActions & Qt::MoveAction) {
         transformedDefaultDropAction = Qt::MoveAction;

      } else if (supportedActions & Qt::CopyAction) {
         transformedDefaultDropAction = Qt::CopyAction;

      } else if (supportedActions & Qt::LinkAction) {
         transformedDefaultDropAction = Qt::LinkAction;
      }

   } else {
      transformedDefaultDropAction = defaultDropAction;
   }

   d->supported_actions = supportedActions;
   d->default_action   = transformedDefaultDropAction;
   d->executed_action   = QDragManager::self()->drag(this);

   return d->executed_action;
}

Qt::DropAction QDrag::start(Qt::DropActions request)
{
   Q_D(QDrag);
   if (!d->data) {
      qWarning("QDrag::start() Mime data was not set before starting the drag");
      return d->executed_action;
   }

   d->supported_actions = request | Qt::CopyAction;
   d->default_action = Qt::IgnoreAction;
   d->executed_action = QDragManager::self()->drag(this);

   return d->executed_action;
}


void QDrag::setDragCursor(const QPixmap &cursor, Qt::DropAction action)
{
   Q_D(QDrag);

   if (action != Qt::CopyAction && action != Qt::MoveAction && action != Qt::LinkAction) {
      return;
   }

   if (cursor.isNull()) {
      d->customCursors.remove(action);
   } else {
      d->customCursors[action] = cursor;
   }
}
QPixmap QDrag::dragCursor(Qt::DropAction action) const
{
   typedef QMap<Qt::DropAction, QPixmap>::const_iterator Iterator;

   Q_D(const QDrag);
   const Iterator it = d->customCursors.constFind(action);
   if (it != d->customCursors.constEnd()) {
      return it.value();
   }

   Qt::CursorShape shape = Qt::ForbiddenCursor;
   switch (action) {
      case Qt::MoveAction:
         shape = Qt::DragMoveCursor;
         break;
      case Qt::CopyAction:
         shape = Qt::DragCopyCursor;
         break;
      case Qt::LinkAction:
         shape = Qt::DragLinkCursor;
         break;
      default:
         shape = Qt::ForbiddenCursor;
   }

   return QGuiApplicationPrivate::instance()->getPixmapCursor(shape);
}

Qt::DropActions QDrag::supportedActions() const
{
   Q_D(const QDrag);
   return d->supported_actions;
}
Qt::DropAction QDrag::defaultAction() const
{
   Q_D(const QDrag);
   return d->default_action;
}


#endif // QT_NO_DRAGANDDROP
