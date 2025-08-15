/***********************************************************************
*
* Copyright (c) 2012-2025 Barbara Geller
* Copyright (c) 2012-2025 Ansel Sermersheim
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

#include <qwayland_dnd_p.h>

#include <qshapedpixmapdndwindow_p.h>
#include <qwayland_data_device_p.h>
#include <qwayland_data_devicemanager_p.h>
#include <qwayland_dataoffer_p.h>
#include <qwayland_display_p.h>
#include <qwayland_inputdevice_p.h>
#include <qwayland_window_p.h>

#ifndef QT_NO_DRAGANDDROP

namespace QtWaylandClient {

QWaylandDrag::QWaylandDrag(QWaylandDisplay *display)
   : m_display(display)
{
}

QWaylandDrag::~QWaylandDrag()
{
}

QMimeData *QWaylandDrag::platformDropData()
{
   if (drag()) {
      return drag()->mimeData();
   }

   return nullptr;
}

void QWaylandDrag::startDrag()
{
   // pending implementation
}

void QWaylandDrag::cancel()
{
   QBasicDrag::cancel();

   m_display->currentInputDevice()->dataDevice()->cancelDrag();
}

void QWaylandDrag::move(const QPoint &globalPos)
{
   (void) globalPos;
}

void QWaylandDrag::drop(const QPoint &globalPos)
{
   (void) globalPos;
}

void QWaylandDrag::endDrag()
{
   // pending implementation
}

void QWaylandDrag::updateTarget(const QString &mimeType)
{
   setCanDrop(! mimeType.isEmpty());

   if (canDrop()) {
      updateCursor(defaultAction(drag()->supportedActions(), m_display->currentInputDevice()->modifiers()));
   } else {
      updateCursor(Qt::IgnoreAction);
   }
}

void QWaylandDrag::setResponse(const QPlatformDragQtResponse &response)
{
   setCanDrop(response.isAccepted());

   if (canDrop()) {
      updateCursor(defaultAction(drag()->supportedActions(), m_display->currentInputDevice()->modifiers()));
   } else {
      updateCursor(Qt::IgnoreAction);
   }
}

void QWaylandDrag::finishDrag(const QPlatformDropQtResponse &response)
{
   setExecutedDropAction(response.acceptedAction());
   QKeyEvent event(QEvent::KeyPress, Qt::Key_Escape, Qt::NoModifier);
   eventFilter(shapedPixmapWindow(), &event);
}

}

#endif
