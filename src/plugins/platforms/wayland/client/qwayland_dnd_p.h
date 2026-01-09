/***********************************************************************
*
* Copyright (c) 2012-2026 Barbara Geller
* Copyright (c) 2012-2026 Ansel Sermersheim
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

#ifndef QWAYLAND_DND_H
#define QWAYLAND_DND_H

#include <qdrag.h>
#include <qmimedata.h>
#include <qplatform_drag.h>

#include <qsimpledrag_p.h>

namespace QtWaylandClient {

class QWaylandDisplay;

#ifndef QT_NO_DRAGANDDROP

class Q_WAYLAND_CLIENT_EXPORT QWaylandDrag : public QBasicDrag
{
 public:
   QWaylandDrag(QWaylandDisplay *display);
   ~QWaylandDrag();

   QMimeData *platformDropData() override;

   void updateTarget(const QString &mimeType);
   void setResponse(const QPlatformDragQtResponse &response);
   void finishDrag(const QPlatformDropQtResponse &response);

 protected:
   void startDrag() override;
   void cancel() override;
   void move(const QPoint &globalPos) override;
   void drop(const QPoint &globalpos) override;
   void endDrag() override;

 private:
   QWaylandDisplay *m_display;
};

#endif
}

#endif
