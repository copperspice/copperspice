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

#ifndef QXCB_SCREEN_H
#define QXCB_SCREEN_H

#include <qplatform_screen.h>
#include <qstring.h>
#include <qxcb_object.h>
#include <qxcb_screen.h>

#include <qfontengine_p.h>

class QXcbConnection;
class QXcbCursor;
class QXcbXSettings;
class QDebug;

#include <xcb/xcb.h>
#include <xcb/randr.h>
#include <xcb/xfixes.h>
#include <xcb/xinerama.h>

class QXcbVirtualDesktop : public QXcbObject
{
 public:
   QXcbVirtualDesktop(QXcbConnection *connection, xcb_screen_t *screen, int number);
   ~QXcbVirtualDesktop();

   xcb_screen_t *screen() const {
      return m_screen;
   }
   int number() const {
      return m_number;
   }
   QSize size() const {
      return QSize(m_screen->width_in_pixels, m_screen->height_in_pixels);
   }
   QSize physicalSize() const {
      return QSize(m_screen->width_in_millimeters, m_screen->height_in_millimeters);
   }
   xcb_window_t root() const {
      return m_screen->root;
   }
   QXcbScreen *screenAt(const QPoint &pos) const;

   QList<QPlatformScreen *> screens() const {
      return m_screens;
   }
   void setScreens(QList<QPlatformScreen *> sl) {
      m_screens = sl;
   }
   void removeScreen(QPlatformScreen *s) {
      m_screens.removeOne(s);
   }
   void addScreen(QPlatformScreen *s);
   void setPrimaryScreen(QPlatformScreen *s);

   QXcbXSettings *xSettings() const;

   bool compositingActive() const;

   QRect workArea() const {
      return m_workArea;
   }
   void updateWorkArea();

   void handleXFixesSelectionNotify(xcb_xfixes_selection_notify_event_t *notify_event);
   void subscribeToXFixesSelectionNotify();

 private:
   QRect getWorkArea() const;

   xcb_screen_t *m_screen;
   int m_number;
   QList<QPlatformScreen *> m_screens;

   QXcbXSettings *m_xSettings;
   xcb_atom_t m_net_wm_cm_atom;
   bool m_compositingActive;

   QRect m_workArea;
};

class Q_XCB_EXPORT QXcbScreen : public QXcbObject, public QPlatformScreen
{
 public:
   QXcbScreen(QXcbConnection *connection, QXcbVirtualDesktop *virtualDesktop,
      xcb_randr_output_t outputId, xcb_randr_get_output_info_reply_t *outputInfo,
      const xcb_xinerama_screen_info_t *xineramaScreenInfo = nullptr, int xineramaScreenIdx = -1);
   ~QXcbScreen();

   QString getOutputName(xcb_randr_get_output_info_reply_t *outputInfo);

   QPixmap grabWindow(WId window, int x, int y, int width, int height) const override;

   QWindow *topLevelWindowAt(const QPoint &point) const override;

   QRect geometry() const override {
      return m_geometry;
   }
   QRect availableGeometry() const override {
      return m_availableGeometry;
   }
   int depth() const override {
      return screen()->root_depth;
   }
   QImage::Format format() const override;
   QSizeF physicalSize() const override {
      return m_sizeMillimeters;
   }
   QSize virtualSize() const {
      return m_virtualSize;
   }
   QSizeF physicalVirtualSize() const {
      return m_virtualSizeMillimeters;
   }
   QDpi virtualDpi() const;
   QDpi logicalDpi() const override;
   qreal pixelDensity() const override;
   QPlatformCursor *cursor() const override;
   qreal refreshRate() const override {
      return m_refreshRate;
   }
   Qt::ScreenOrientation orientation() const override {
      return m_orientation;
   }
   QList<QPlatformScreen *> virtualSiblings() const override {
      return m_virtualDesktop->screens();
   }
   QXcbVirtualDesktop *virtualDesktop() const {
      return m_virtualDesktop;
   }

   void setPrimary(bool primary) {
      m_primary = primary;
   }
   bool isPrimary() const {
      return m_primary;
   }

   int screenNumber() const {
      return m_virtualDesktop->number();
   }
   static int virtualDesktopNumberStatic(const QScreen *screen);

   xcb_screen_t *screen() const {
      return m_virtualDesktop->screen();
   }
   xcb_window_t root() const {
      return screen()->root;
   }
   xcb_randr_output_t output() const {
      return m_output;
   }
   xcb_randr_crtc_t crtc() const {
      return m_crtc;
   }
   xcb_randr_mode_t mode() const {
      return m_mode;
   }

   void setOutput(xcb_randr_output_t outputId,
      xcb_randr_get_output_info_reply_t *outputInfo);
   void setCrtc(xcb_randr_crtc_t crtc) {
      m_crtc = crtc;
   }

   void windowShown(QXcbWindow *window);
   QString windowManagerName() const {
      return m_windowManagerName;
   }
   bool syncRequestSupported() const {
      return m_syncRequestSupported;
   }

   const xcb_visualtype_t *visualForId(xcb_visualid_t) const;
   quint8 depthOfVisual(xcb_visualid_t) const;

   QString name() const override {
      return m_outputName;
   }

   void handleScreenChange(xcb_randr_screen_change_notify_event_t *change_event);
   void updateGeometry(const QRect &geom, uint8_t rotation);
   void updateGeometry(xcb_timestamp_t timestamp = XCB_TIME_CURRENT_TIME);
   void updateAvailableGeometry();
   void updateRefreshRate(xcb_randr_mode_t mode);

   void readXResources();

   QFontEngine::HintStyle hintStyle() const {
      return m_hintStyle;
   }
   bool noFontHinting() const {
      return m_noFontHinting;
   }
   QFontEngine::SubpixelAntialiasingType subpixelType() const {
      return m_subpixelType;
   }
   int antialiasingEnabled() const {
      return m_antialiasingEnabled;
   }

   QXcbXSettings *xSettings() const;

   void *nativeHandle() override;

 private:
   static bool xResource(const QByteArray &identifier, const QByteArray &expectedIdentifier, QByteArray &stringValue);
   void sendStartupMessage(const QByteArray &message) const;

   QXcbVirtualDesktop *m_virtualDesktop;
   xcb_randr_output_t m_output;
   xcb_randr_crtc_t m_crtc;
   xcb_randr_mode_t m_mode;
   bool m_primary;
   uint8_t m_rotation;

   QString m_outputName;
   QSizeF m_outputSizeMillimeters;
   QSizeF m_sizeMillimeters;
   QRect m_geometry;
   QRect m_availableGeometry;
   QSize m_virtualSize;
   QSizeF m_virtualSizeMillimeters;
   Qt::ScreenOrientation m_orientation;
   QString m_windowManagerName;
   bool m_syncRequestSupported;
   QMap<xcb_visualid_t, xcb_visualtype_t> m_visuals;
   QMap<xcb_visualid_t, quint8> m_visualDepths;
   QXcbCursor *m_cursor;
   int m_refreshRate;
   int m_forcedDpi;
   int m_pixelDensity;
   QFontEngine::HintStyle m_hintStyle;
   bool m_noFontHinting;
   QFontEngine::SubpixelAntialiasingType m_subpixelType;
   int m_antialiasingEnabled;
};

QDebug operator<<(QDebug debug, const QXcbScreen *screen);

#endif
