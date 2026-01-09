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

#ifndef QWAYLAND_SERVERBUFFER_INTEGRATION_H
#define QWAYLAND_SERVERBUFFER_INTEGRATION_H

#include <qsize.h>
#include <qopengl.h>

#include <qwayland-server-buffer-extension.h>

namespace QtWaylandClient {

class QWaylandDisplay;

class Q_WAYLAND_CLIENT_EXPORT QWaylandServerBuffer
{
 public:
   enum Format {
      RGBA32,
      A8
   };

   QWaylandServerBuffer();
   virtual ~QWaylandServerBuffer();

   virtual void bindTextureToBuffer() = 0;

   Format format() const;
   QSize size() const;

   void setUserData(void *userData);
   void *userData() const;

 protected:
   Format m_format;
   QSize m_size;

 private:
   void *m_user_data;
};

class Q_WAYLAND_CLIENT_EXPORT QWaylandServerBufferIntegration
{
 public:
   QWaylandServerBufferIntegration();
   virtual ~QWaylandServerBufferIntegration();

   virtual void initialize(QWaylandDisplay *display) = 0;

   virtual QWaylandServerBuffer *serverBuffer(struct qt_server_buffer *buffer) = 0;
};

}

#endif
