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

#ifndef QXCB_WINDOW_H
#define QXCB_WINDOW_H

#include <qplatform_window.h>
#include <qsurfaceformat.h>
#include <qimage.h>
#include <qxcb_object.h>
#include <qxcbwindowfunctions.h>

#include <xcb/xcb.h>
#include <xcb/sync.h>

class QXcbScreen;
class QXcbSyncWindowRequest;
class QIcon;

class Q_XCB_EXPORT QXcbWindow : public QObject, public QXcbObject, public QXcbWindowEventListener, public QPlatformWindow
{
   CS_OBJECT(QXcbWindow)

 public:
   enum NetWmState {
      NetWmStateAbove            = 0x1,
      NetWmStateBelow            = 0x2,
      NetWmStateFullScreen       = 0x4,
      NetWmStateMaximizedHorz    = 0x8,
      NetWmStateMaximizedVert    = 0x10,
      NetWmStateModal            = 0x20,
      NetWmStateStaysOnTop       = 0x40,
      NetWmStateDemandsAttention = 0x80
   };
   using NetWmStates = QFlags<NetWmState>;

   QXcbWindow(QWindow *window);
   ~QXcbWindow();

   using QPlatformWindow::parent;

   void setGeometry(const QRect &rect) override;

   QMargins frameMargins() const override;

   void setVisible(bool visible) override;
   void setWindowFlags(Qt::WindowFlags flags) override;
   void setWindowState(Qt::WindowState state) override;
   WId winId() const override;
   void setParent(const QPlatformWindow *window) override;

   bool isExposed() const override;
   bool isEmbedded(const QPlatformWindow *parentWindow = nullptr) const override;
   QPoint mapToGlobal(const QPoint &pos) const override;
   QPoint mapFromGlobal(const QPoint &pos) const override;

   void setWindowTitle(const QString &title) override;
   void setWindowIconText(const QString &title);
   void setWindowIcon(const QIcon &icon) override;
   void raise() override;
   void lower() override;
   void propagateSizeHints() override;

   void requestActivateWindow() override;

   bool setKeyboardGrabEnabled(bool grab) override;
   bool setMouseGrabEnabled(bool grab) override;

   void setCursor(xcb_cursor_t cursor, bool isBitmapCursor);

   QSurfaceFormat format() const override;

   void windowEvent(QEvent *event) override;

   bool startSystemResize(const QPoint &pos, Qt::Corner corner) override;

   void setOpacity(qreal level) override;
   void setMask(const QRegion &region) override;

   void setAlertState(bool enabled) override;
   bool isAlertState() const override {
      return m_alertState;
   }

   void *nativeHandle() override {
      return &m_window;
   }

   void syncIfNeeded() override;

   xcb_window_t xcb_window() const {
      return m_window;
   }
   uint depth() const {
      return m_depth;
   }
   QImage::Format imageFormat() const {
      return m_imageFormat;
   }
   bool imageNeedsRgbSwap() const {
      return m_imageRgbSwap;
   }

   bool handleGenericEvent(xcb_generic_event_t *event, long *result)  override;

   void handleExposeEvent(const xcb_expose_event_t *event) override;
   void handleClientMessageEvent(const xcb_client_message_event_t *event) override;
   void handleConfigureNotifyEvent(const xcb_configure_notify_event_t *event) override;
   void handleMapNotifyEvent(const xcb_map_notify_event_t *event) override;
   void handleUnmapNotifyEvent(const xcb_unmap_notify_event_t *event) override;
   void handleButtonPressEvent(const xcb_button_press_event_t *event) override;
   void handleButtonReleaseEvent(const xcb_button_release_event_t *event) override;
   void handleMotionNotifyEvent(const xcb_motion_notify_event_t *event) override;

   void handleEnterNotifyEvent(const xcb_enter_notify_event_t *event) override;
   void handleLeaveNotifyEvent(const xcb_leave_notify_event_t *event) override;
   void handleFocusInEvent(const xcb_focus_in_event_t *event) override;
   void handleFocusOutEvent(const xcb_focus_out_event_t *event) override;
   void handlePropertyNotifyEvent(const xcb_property_notify_event_t *event) override;

#ifdef XCB_USE_XINPUT22
   void handleXIMouseEvent(xcb_ge_event_t *, Qt::MouseEventSource source = Qt::MouseEventNotSynthesized) override;
   void handleXIEnterLeave(xcb_ge_event_t *) override;
#endif

   QXcbWindow *toWindow() override;

   void handleMouseEvent(xcb_timestamp_t time, const QPoint &local, const QPoint &global,
      Qt::KeyboardModifiers modifiers, Qt::MouseEventSource source);

   void updateNetWmUserTime(xcb_timestamp_t timestamp);

   static void setWmWindowTypeStatic(QWindow *window, QXcbWindowFunctions::WmWindowTypes windowTypes);
   static void setWmWindowRoleStatic(QWindow *window, const QByteArray &role);
   static uint visualIdStatic(QWindow *window);

   QXcbWindowFunctions::WmWindowTypes wmWindowTypes() const;
   void setWmWindowType(QXcbWindowFunctions::WmWindowTypes types, Qt::WindowFlags flags);
   void setWmWindowRole(const QByteArray &role);

   static void setWindowIconTextStatic(QWindow *window, const QString &text);

   static void setParentRelativeBackPixmapStatic(QWindow *window);
   void setParentRelativeBackPixmap();

   static bool requestSystemTrayWindowDockStatic(const QWindow *window);
   bool requestSystemTrayWindowDock() const;

   static QRect systemTrayWindowGlobalGeometryStatic(const QWindow *window);
   QRect systemTrayWindowGlobalGeometry() const;
   uint visualId() const;

   bool needsSync() const;

   void postSyncWindowRequest();
   void clearSyncWindowRequest() {
      m_pendingSyncRequest = nullptr;
   }

   QXcbScreen *xcbScreen() const;

   virtual void create();
   virtual void destroy();

   CS_SLOT_1(Public, void updateSyncRequestCounter())
   CS_SLOT_2(updateSyncRequestCounter)

 protected:
   virtual void resolveFormat() {
      m_format = window()->requestedFormat();
   }

   virtual void *createVisual() {
      return nullptr;
   }

   QXcbScreen *parentScreen();

   void changeNetWmState(bool set, xcb_atom_t one, xcb_atom_t two = 0);
   NetWmStates netWmStates();
   void setNetWmStates(NetWmStates);

   void setMotifWindowFlags(Qt::WindowFlags flags);

   void updateMotifWmHintsBeforeMap();
   void updateNetWmStateBeforeMap();

   void setTransparentForMouseEvents(bool transparent);
   void updateDoesNotAcceptFocus(bool doesNotAcceptFocus);

   QRect windowToWmGeometry(QRect r) const;
   void sendXEmbedMessage(xcb_window_t window, quint32 message, quint32 detail = 0, quint32 data1 = 0, quint32 data2 = 0);
   void handleXEmbedMessage(const xcb_client_message_event_t *event);

   void show();
   void hide();

   bool relayFocusToModalWindow() const;
   void doFocusIn();
   void doFocusOut();

   bool compressExposeEvent(QRegion &exposeRegion);

   void handleButtonPressEvent(int event_x, int event_y, int root_x, int root_y,
      int detail, Qt::KeyboardModifiers modifiers, xcb_timestamp_t timestamp,
      Qt::MouseEventSource source = Qt::MouseEventNotSynthesized);

   void handleButtonReleaseEvent(int event_x, int event_y, int root_x, int root_y,
      int detail, Qt::KeyboardModifiers modifiers, xcb_timestamp_t timestamp,
      Qt::MouseEventSource source = Qt::MouseEventNotSynthesized);

   void handleMotionNotifyEvent(int event_x, int event_y, int root_x, int root_y,
      Qt::KeyboardModifiers modifiers, xcb_timestamp_t timestamp,
      Qt::MouseEventSource source = Qt::MouseEventNotSynthesized);

   void handleEnterNotifyEvent(int event_x, int event_y, int root_x, int root_y,
      quint8 mode, quint8 detail, xcb_timestamp_t timestamp);

   void handleLeaveNotifyEvent(int root_x, int root_y,
      quint8 mode, quint8 detail, xcb_timestamp_t timestamp);

   xcb_window_t m_window;

   uint m_depth;
   QImage::Format m_imageFormat;
   bool m_imageRgbSwap;

   xcb_sync_int64_t m_syncValue;
   xcb_sync_counter_t m_syncCounter;

   Qt::WindowState m_windowState;

   xcb_gravity_t m_gravity;

   bool m_mapped;
   bool m_transparent;
   bool m_usingSyncProtocol;
   bool m_deferredActivation;
   bool m_embedded;
   bool m_alertState;
   xcb_window_t m_netWmUserTimeWindow;

   QSurfaceFormat m_format;

   mutable bool m_dirtyFrameMargins;
   mutable QMargins m_frameMargins;

   QRegion m_exposeRegion;

   xcb_visualid_t m_visualId;
   int m_lastWindowStateEvent;

   enum SyncState {
      NoSyncNeeded,
      SyncReceived,
      SyncAndConfigureReceived
   };
   SyncState m_syncState;

   QXcbSyncWindowRequest *m_pendingSyncRequest;
   xcb_cursor_t m_currentBitmapCursor;
};

#endif
