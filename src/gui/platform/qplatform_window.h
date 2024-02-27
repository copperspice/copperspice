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

#ifndef QPLATFORM_WINDOW_H
#define QPLATFORM_WINDOW_H

#include <qscopedpointer.h>
#include <qrect.h>
#include <qmargins.h>
#include <qstring.h>
#include <qwindowdefs.h>
#include <qwindow.h>
#include <qplatform_openglcontext.h>
#include <qplatform_surface.h>

class QPlatformScreen;
class QPlatformWindowPrivate;
class QWindow;
class QIcon;
class QRegion;

class Q_GUI_EXPORT QPlatformWindow : public QPlatformSurface
{
   Q_DECLARE_PRIVATE(QPlatformWindow)

 public:
   explicit QPlatformWindow(QWindow *window);

   QPlatformWindow(const QPlatformWindow &) = delete;
   QPlatformWindow &operator=(const QPlatformWindow &) = delete;

   virtual ~QPlatformWindow();

   QWindow *window() const;
   QPlatformWindow *parent() const;
   QPlatformScreen *screen() const;
   QSurfaceFormat format() const override;
   virtual void setGeometry(const QRect &rect);
   virtual QRect geometry() const;
   virtual QRect normalGeometry() const;

   virtual QMargins frameMargins() const;
   virtual void setVisible(bool visible);
   virtual void setWindowFlags(Qt::WindowFlags flags);
   virtual void setWindowState(Qt::WindowState state);
   virtual WId winId() const;
   virtual void setParent(const QPlatformWindow *parent);

   virtual void setWindowTitle(const QString &title);
   virtual void setWindowFilePath(const QString &filePath);
   virtual void setWindowIcon(const QIcon &icon);
   virtual void raise();
   virtual void lower();

   virtual bool isExposed() const;
   virtual bool isActive() const;
   virtual bool isEmbedded(const QPlatformWindow *parentWindow = nullptr) const;
   virtual QPoint mapToGlobal(const QPoint &pos) const;
   virtual QPoint mapFromGlobal(const QPoint &pos) const;
   virtual void propagateSizeHints();
   virtual void setOpacity(qreal level);
   virtual void setMask(const QRegion &region);
   virtual void requestActivateWindow();

   virtual void handleContentOrientationChange(Qt::ScreenOrientation orientation);

   virtual qreal devicePixelRatio() const;

   virtual bool setKeyboardGrabEnabled(bool grab);
   virtual bool setMouseGrabEnabled(bool grab);

   virtual bool setWindowModified(bool modified);

   virtual void windowEvent(QEvent *event);

   virtual bool startSystemResize(const QPoint &pos, Qt::Corner corner);

   virtual void setFrameStrutEventsEnabled(bool enabled);
   virtual bool frameStrutEventsEnabled() const;

   virtual void setAlertState(bool enabled);
   virtual bool isAlertState() const;

   virtual void invalidateSurface();

   static QRect initialGeometry(const QWindow *window, const QRect &initialGeometry, int defaultWidth, int defaultHeight);

   virtual void requestUpdate();

   virtual void *nativeHandle();

   virtual void syncIfNeeded();

   // Window property accessors, platform plugins should use these instead of accessing QWindow directly
   QSize windowMinimumSize() const;
   QSize windowMaximumSize() const;
   QSize windowBaseSize() const;
   QSize windowSizeIncrement() const;
   QRect windowGeometry() const;
   QRect windowFrameGeometry() const;
   QRectF windowClosestAcceptableGeometry(const QRectF &nativeRect) const;
   static QRectF closestAcceptableGeometry(const QWindow *window, const QRectF &nativeRect);

 protected:
   static QString formatWindowTitle(const QString &title, const QString &separator);
   QPlatformScreen *screenForGeometry(const QRect &newGeometry) const;
   static QSize constrainWindowSize(const QSize &size);

   QScopedPointer<QPlatformWindowPrivate> d_ptr;
};

#endif
