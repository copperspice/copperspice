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

#include "qwindowsnativeinterface.h"
#include "qwindowswindow.h"
#include "qwindowscontext.h"
#include "qwindowsfontdatabase.h"
#include "qwindowsopenglcontext.h"
#include "qwindowsopengltester.h"
#include "qwindowsintegration.h"
#include "qwindowsmime.h"

#include <QWindow>
#include <QOpenGLContext>

enum ResourceType {
   RenderingContextType,
   EglContextType,
   EglDisplayType,
   EglConfigType,
   HandleType,
   GlHandleType,
   GetDCType,
   ReleaseDCType
};

static int resourceType(const QByteArray &key)
{
   static const char *names[] = { // match ResourceType
      "renderingcontext",
      "eglcontext",
      "egldisplay",
      "eglconfig",
      "handle",
      "glhandle",
      "getdc",
      "releasedc"
   };

   const char **const end = names + sizeof(names) / sizeof(names[0]);
   const char **result = std::find(names, end, key);

   if (result == end) {
      result = std::find(names, end, key.toLower());
   }

   return int(result - names);
}

void *QWindowsNativeInterface::nativeResourceForWindow(const QByteArray &resource, QWindow *window)
{
   if (!window || !window->handle()) {
      qWarning("%s: '%s' requested for null window or window without handle.", __FUNCTION__, resource.constData());
      return 0;
   }
   QWindowsWindow *bw = static_cast<QWindowsWindow *>(window->handle());
   int type = resourceType(resource);
   if (type == HandleType) {
      return bw->handle();
   }
   switch (window->surfaceType()) {
      case QWindow::RasterSurface:
      case QWindow::RasterGLSurface:
         if (type == GetDCType) {
            return bw->getDC();
         }

         if (type == ReleaseDCType) {
            bw->releaseDC();
            return 0;
         }
         break;

      case QWindow::OpenGLSurface:
         break;
   }

   qWarning("%s: Invalid key '%s' requested.", __FUNCTION__, resource.constData());

   return nullptr;
}

static QString customMarginPropertyC = "WindowsCustomMargins";

QVariant QWindowsNativeInterface::windowProperty(QPlatformWindow *window, const QString &name) const
{
   QWindowsWindow *platformWindow = static_cast<QWindowsWindow *>(window);

   if (name == customMarginPropertyC) {
      return QVariant::fromValue(platformWindow->customMargins());
   }

   return QVariant();
}

QVariant QWindowsNativeInterface::windowProperty(QPlatformWindow *window, const QString &name, const QVariant &defaultValue) const
{
   const QVariant result = windowProperty(window, name);
   return result.isValid() ? result : defaultValue;
}

void QWindowsNativeInterface::setWindowProperty(QPlatformWindow *window, const QString &name, const QVariant &value)
{
   QWindowsWindow *platformWindow = static_cast<QWindowsWindow *>(window);

   if (name == customMarginPropertyC) {
      platformWindow->setCustomMargins(qvariant_cast<QMargins>(value));
   }
}

QVariantMap QWindowsNativeInterface::windowProperties(QPlatformWindow *window) const
{
   QVariantMap result;
   const QString customMarginProperty = customMarginPropertyC;
   result.insert(customMarginProperty, windowProperty(window, customMarginProperty));

   return result;
}

void *QWindowsNativeInterface::nativeResourceForIntegration(const QByteArray &resource)
{
#ifndef QT_NO_OPENGL
   if (resourceType(resource) == GlHandleType) {
      if (const QWindowsStaticOpenGLContext *sc = QWindowsIntegration::staticOpenGLContext()) {
         return sc->moduleHandle();
      }
   }
#endif

   return nullptr;
}

#ifndef QT_NO_OPENGL
void *QWindowsNativeInterface::nativeResourceForContext(const QByteArray &resource, QOpenGLContext *context)
{
   if (!context || ! context->handle()) {
      qWarning("nativeResourceForContext(): '%s' requested for null context or context without handle.", resource.constData());
      return nullptr;
   }

   QWindowsOpenGLContext *glcontext = static_cast<QWindowsOpenGLContext *>(context->handle());

   switch (resourceType(resource)) {
      case RenderingContextType:
      case EglContextType:
         return glcontext->nativeContext();

      case EglDisplayType:
         return glcontext->nativeDisplay();

      case EglConfigType:
         return glcontext->nativeConfig();

      default:
         break;
   }

   qWarning("nativeResourceForContext(): Invalid key '%s' requested.", resource.constData());

   return nullptr;
}
#endif

void *QWindowsNativeInterface::createMessageWindow(const QString &classNameTemplate,
   const QString &windowName, void *eventProc) const
{
   QWindowsContext *ctx = QWindowsContext::instance();

   std::wstring tmp = windowName.toStdWString();
   const HWND hwnd = ctx->createDummyWindow(classNameTemplate, tmp.data(), (WNDPROC)eventProc);

   return hwnd;
}

QString QWindowsNativeInterface::registerWindowClass(const QString &classNameIn, void *eventProc) const
{
   return QWindowsContext::instance()->registerWindowClass(classNameIn, (WNDPROC)eventProc);
}

void QWindowsNativeInterface::beep()
{
   MessageBeep(MB_OK);  // For QApplication
}

bool QWindowsNativeInterface::asyncExpose() const
{
   return QWindowsContext::instance()->asyncExpose();
}

void QWindowsNativeInterface::setAsyncExpose(bool value)
{
   QWindowsContext::instance()->setAsyncExpose(value);
}

void QWindowsNativeInterface::registerWindowsMime(void *mimeIn)
{
   QWindowsContext::instance()->mimeConverter().registerMime(reinterpret_cast<QWindowsMime *>(mimeIn));
}

void QWindowsNativeInterface::unregisterWindowsMime(void *mimeIn)
{
   QWindowsContext::instance()->mimeConverter().unregisterMime(reinterpret_cast<QWindowsMime *>(mimeIn));
}

int QWindowsNativeInterface::registerMimeType(const QString &mimeType)
{
   return QWindowsMime::registerMimeType(mimeType);
}

QFont QWindowsNativeInterface::logFontToQFont(const void *logFont, int verticalDpi)
{
   return QWindowsFontDatabase::LOGFONT_to_QFont(*reinterpret_cast<const LOGFONT *>(logFont), verticalDpi);
}

QWindowsNativeInterface::FP_Void QWindowsNativeInterface::platformFunction(const QByteArray &function) const
{
   if (function == QWindowsWindowFunctions::setTouchWindowTouchTypeIdentifier()) {
      return FP_Void(QWindowsWindow::setTouchWindowTouchTypeStatic);

   } else if (function == QWindowsWindowFunctions::setHasBorderInFullScreenIdentifier()) {
      return FP_Void(QWindowsWindow::setHasBorderInFullScreenStatic);
   }

   return nullptr;
}

QVariant QWindowsNativeInterface::gpu() const
{
   return GpuDescription::detect().toVariant();
}

