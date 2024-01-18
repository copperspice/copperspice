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

#ifndef QWINDOWSSCREEN_H
#define QWINDOWSSCREEN_H

#include <qwin_global.h>
#include <qlist.h>
#include <qvector.h>
#include <qpair.h>
#include <qscopedpointer.h>
#include <qplatform_screen.h>

struct QWindowsScreenData {
   enum Flags {
      PrimaryScreen = 0x1,
      VirtualDesktop = 0x2,
      LockScreen = 0x4 // Temporary screen existing during user change, etc.
   };

   QWindowsScreenData();

   QRect geometry;
   QRect availableGeometry;
   QDpi dpi;
   QSizeF physicalSizeMM;
   int depth;
   QImage::Format format;
   unsigned flags;
   QString name;
   Qt::ScreenOrientation orientation;
   qreal refreshRateHz;
};

class QWindowsScreen : public QPlatformScreen
{
 public:
#ifndef QT_NO_CURSOR
   typedef QScopedPointer<QPlatformCursor> CursorPtr;
#endif

   explicit QWindowsScreen(const QWindowsScreenData &data);

   QRect geometry() const override {
      return m_data.geometry;
   }
   QRect availableGeometry() const override {
      return m_data.availableGeometry;
   }
   int depth() const override {
      return m_data.depth;
   }
   QImage::Format format() const override {
      return m_data.format;
   }
   QSizeF physicalSize() const override {
      return m_data.physicalSizeMM;
   }
   QDpi logicalDpi() const override {
      return m_data.dpi;
   }
   qreal pixelDensity() const override;
   qreal devicePixelRatio() const override {
      return 1.0;
   }
   qreal refreshRate() const override {
      return m_data.refreshRateHz;
   }
   QString name() const override {
      return m_data.name;
   }
   Qt::ScreenOrientation orientation() const override {
      return m_data.orientation;
   }
   QList<QPlatformScreen *> virtualSiblings() const override;
   QWindow *topLevelWindowAt(const QPoint &point) const override;
   static QWindow *windowAt(const QPoint &point, unsigned flags);

   QPixmap grabWindow(WId window, int qX, int qY, int qWidth, int qHeight) const override;
   QPlatformScreen::SubpixelAntialiasingType subpixelAntialiasingTypeHint() const override;

   static Qt::ScreenOrientation orientationPreference();
   static bool setOrientationPreference(Qt::ScreenOrientation o);

   inline void handleChanges(const QWindowsScreenData &newData);

#ifndef QT_NO_CURSOR
   QPlatformCursor *cursor() const override {
      return m_cursor.data();
   }
   const CursorPtr &cursorPtr() const {
      return m_cursor;
   }
#else
   QPlatformCursor *cursor() const  {
      return 0;
   }
#endif

   const QWindowsScreenData &data() const  {
      return m_data;
   }

 private:
   QWindowsScreenData m_data;

#ifndef QT_NO_CURSOR
   const CursorPtr m_cursor;
#endif
};

class QWindowsScreenManager
{
 public:
   typedef QList<QWindowsScreen *> WindowsScreenList;

   QWindowsScreenManager();

   void clearScreens();

   bool handleScreenChanges();
   bool handleDisplayChange(WPARAM wParam, LPARAM lParam);
   const WindowsScreenList &screens() const {
      return m_screens;
   }

   const QWindowsScreen *screenAtDp(const QPoint &p) const;

 private:
   void removeScreen(int index);

   WindowsScreenList m_screens;
   int m_lastDepth;
   WORD m_lastHorizontalResolution;
   WORD m_lastVerticalResolution;
};

#endif
