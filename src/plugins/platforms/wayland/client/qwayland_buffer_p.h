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

#ifndef QWAYLAND_BUFFER_H
#define QWAYLAND_BUFFER_H

#include <qrect.h>
#include <qsize.h>

#include <wayland-client-protocol.h>
#include <wayland-client.h>

namespace QtWaylandClient {

class Q_WAYLAND_CLIENT_EXPORT QWaylandBuffer
{
 public:
   QWaylandBuffer();
   virtual ~QWaylandBuffer();

   void init(wl_buffer *buf);

   wl_buffer *buffer() {
      return mBuffer;
   }

   bool busy() const {
      return mBusy;
   }

   bool committed() const {
      return mCommitted;
   }

   virtual QSize size() const = 0;
   virtual int scale() const {
      return 1;
   }

   void setBusy() {
      mBusy = true;
   }

   void setCommitted() {
      mCommitted = true;
   }

 protected:
   struct wl_buffer *mBuffer;

 private:
   static void release(void *data, wl_buffer *);
   static const wl_buffer_listener listener;

   bool mBusy;
   bool mCommitted;
};

}

#endif
