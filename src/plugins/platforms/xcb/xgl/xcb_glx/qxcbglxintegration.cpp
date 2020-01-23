/***********************************************************************
*
* Copyright (c) 2012-2020 Barbara Geller
* Copyright (c) 2012-2020 Ansel Sermersheim
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

#include "qxcbglxintegration.h"

#if defined(XCB_HAS_XCB_GLX)
#include <xcb/glx.h>
#endif

#include "qxcbnativeinterface.h"
#include "qxcbglxwindow.h"
#include "qxcbscreen.h"
#include "qglxintegration.h"
#include <QOpenGLContext>
#include "qxcbglxnativeinterfacehandler.h"

#include <X11/Xlibint.h>

#if defined(XCB_HAS_XCB_GLX) && XCB_GLX_MAJOR_VERSION == 1 && XCB_GLX_MINOR_VERSION < 4

#define XCB_GLX_BUFFER_SWAP_COMPLETE 1

typedef struct xcb_glx_buffer_swap_complete_event_t {
   uint8_t            response_type;
   uint8_t            pad0;
   uint16_t           sequence;
   uint16_t           event_type;
   uint8_t            pad1[2];
   xcb_glx_drawable_t drawable;
   uint32_t           ust_hi;
   uint32_t           ust_lo;
   uint32_t           msc_hi;
   uint32_t           msc_lo;
   uint32_t           sbc;
} xcb_glx_buffer_swap_complete_event_t;
#endif

#if defined(XCB_USE_XLIB) && defined(XCB_USE_GLX)
typedef struct {
   int type;
   unsigned long serial;       /* # of last request processed by server */
   Bool send_event;            /* true if this came from a SendEvent request */
   Display *display;           /* Display the event was read from */
   Drawable drawable;  /* drawable on which event was requested in event mask */
   int event_type;
   int64_t ust;
   int64_t msc;
   int64_t sbc;
} QGLXBufferSwapComplete;
#endif

QXcbGlxIntegration::QXcbGlxIntegration()
   : m_connection(nullptr)
   , m_glx_first_event(0)
{
   qCDebug(QT_XCB_GLINTEGRATION) << "Xcb GLX gl-integration created";
}

QXcbGlxIntegration::~QXcbGlxIntegration()
{
}

bool QXcbGlxIntegration::initialize(QXcbConnection *connection)
{
   m_connection = connection;
#ifdef XCB_HAS_XCB_GLX

   const xcb_query_extension_reply_t *reply = xcb_get_extension_data(m_connection->xcb_connection(), &xcb_glx_id);
   if (!reply || !reply->present) {
      return false;
   }

   m_glx_first_event = reply->first_event;

   xcb_generic_error_t *error = 0;
   xcb_glx_query_version_cookie_t xglx_query_cookie = xcb_glx_query_version(m_connection->xcb_connection(),
         XCB_GLX_MAJOR_VERSION,
         XCB_GLX_MINOR_VERSION);
   xcb_glx_query_version_reply_t *xglx_query = xcb_glx_query_version_reply(m_connection->xcb_connection(),
         xglx_query_cookie, &error);
   if (!xglx_query || error) {
      qCWarning(QT_XCB_GLINTEGRATION) << "QXcbConnection: Failed to initialize GLX";
      free(error);
      return false;
   }
   free(xglx_query);
#endif

   m_native_interface_handler.reset(new QXcbGlxNativeInterfaceHandler(connection->nativeInterface()));

   qCDebug(QT_XCB_GLINTEGRATION) << "Xcb GLX gl-integration successfully initialized";
   return true;
}

bool QXcbGlxIntegration::handleXcbEvent(xcb_generic_event_t *event, uint responseType)
{
   bool handled = false;
   // Check if a custom XEvent constructor was registered in xlib for this event type, and call it discarding the constructed XEvent if any.
   // XESetWireToEvent might be used by libraries to intercept messages from the X server e.g. the OpenGL lib waiting for DRI2 events.
   Display *xdisplay = static_cast<Display *>(m_connection->xlib_display());
   XLockDisplay(xdisplay);
   bool locked = true;
   Bool (*proc)(Display *, XEvent *, xEvent *) = XESetWireToEvent(xdisplay, responseType, 0);
   if (proc) {
      XESetWireToEvent(xdisplay, responseType, proc);
      XEvent dummy;
      event->sequence = LastKnownRequestProcessed(xdisplay);
      if (proc(xdisplay, &dummy, (xEvent *)event)) {
#ifdef XCB_HAS_XCB_GLX
         // DRI2 clients don't receive GLXBufferSwapComplete events on the wire.
         // Instead the GLX event is synthesized from the DRI2BufferSwapComplete event
         // by DRI2WireToEvent(). For an application to be able to see the event
         // we have to convert it to an xcb_glx_buffer_swap_complete_event_t and
         // pass it to the native event filter.
         const uint swap_complete = m_glx_first_event + XCB_GLX_BUFFER_SWAP_COMPLETE;
         QAbstractEventDispatcher *dispatcher = QAbstractEventDispatcher::instance();
         if (dispatcher && uint(dummy.type) == swap_complete && responseType != swap_complete) {
            QGLXBufferSwapComplete *xev = reinterpret_cast<QGLXBufferSwapComplete *>(&dummy);
            xcb_glx_buffer_swap_complete_event_t ev;
            memset(&ev, 0, sizeof(xcb_glx_buffer_swap_complete_event_t));
            ev.response_type = xev->type;
            ev.sequence = xev->serial;
            ev.event_type = xev->event_type;
            ev.drawable = xev->drawable;
            ev.ust_hi = xev->ust >> 32;
            ev.ust_lo = xev->ust & 0xffffffff;
            ev.msc_hi = xev->msc >> 32;
            ev.msc_lo = xev->msc & 0xffffffff;
            ev.sbc = xev->sbc & 0xffffffff;
            // Unlock the display before calling the native event filter
            XUnlockDisplay(xdisplay);
            locked = false;
            QByteArray genericEventFilterType = m_connection->nativeInterface()->genericEventFilterType();
            long result = 0;
            handled = dispatcher->filterNativeEvent(genericEventFilterType, &ev, &result);
         }
#endif
      }
   }
   if (locked) {
      XUnlockDisplay(xdisplay);
   }
   return handled;
}

QXcbWindow *QXcbGlxIntegration::createWindow(QWindow *window) const
{
   return new QXcbGlxWindow(window);
}

QPlatformOpenGLContext *QXcbGlxIntegration::createPlatformOpenGLContext(QOpenGLContext *context) const
{
   QXcbScreen *screen = static_cast<QXcbScreen *>(context->screen()->handle());
   QGLXContext *platformContext = new QGLXContext(screen, context->format(),
      context->shareHandle(), context->nativeHandle());
   context->setNativeHandle(platformContext->nativeHandle());
   return platformContext;
}

QPlatformOffscreenSurface *QXcbGlxIntegration::createPlatformOffscreenSurface(QOffscreenSurface *surface) const
{
   static bool vendorChecked = false;
   static bool glxPbufferUsable = true;
   if (!vendorChecked) {
      vendorChecked = true;
      Display *display = glXGetCurrentDisplay();
#ifdef XCB_USE_XLIB
      if (!display) {
         display = static_cast<Display *>(m_connection->xlib_display());
      }
#endif
      const char *glxvendor = glXGetClientString(display, GLX_VENDOR);
      if (glxvendor) {
         if (!strcmp(glxvendor, "ATI") || !strcmp(glxvendor, "Chromium")) {
            glxPbufferUsable = false;
         }
      }
   }
   if (glxPbufferUsable) {
      return new QGLXPbuffer(surface);
   } else {
      return 0;   // trigger fallback to hidden QWindow
   }

}

bool QXcbGlxIntegration::supportsThreadedOpenGL() const
{
   return QGLXContext::supportsThreading();
}

bool QXcbGlxIntegration::supportsSwitchableWidgetComposition() const
{
   static bool vendorChecked = false;
   static bool isSwitchableWidgetCompositionAvailable = true;
   if (!vendorChecked) {
      vendorChecked = true;
      Display *display = glXGetCurrentDisplay();
#ifdef XCB_USE_XLIB
      if (!display) {
         display = static_cast<Display *>(m_connection->xlib_display());
      }
#endif
      const char *glxvendor = glXGetClientString(display, GLX_VENDOR);
      if (glxvendor) {
         if (!strcmp(glxvendor, "Parallels Inc")) {
            isSwitchableWidgetCompositionAvailable = false;
         }
      }
   }

   return isSwitchableWidgetCompositionAvailable;
}

