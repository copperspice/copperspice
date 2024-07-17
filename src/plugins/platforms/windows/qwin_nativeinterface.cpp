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

#include <qwin_nativeinterface.h>

#include <qwin_window.h>
#include <qwin_context.h>
#include <qwin_fontdatabase.h>
#include <qwin_opengl_context.h>
#include <qwin_opengl_tester.h>
#include <qwin_integration.h>
#include <qwin_mime.h>
#include <qwindow.h>
#include <qopenglcontext.h>

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

static QString customMarginPropertyC = "WindowsCustomMargins";

void *QWindowsNativeInterface::nativeResourceForWindow(const QByteArray &resource, QWindow *window)
{
   if (! window || ! window->handle()) {
      qWarning("QWindowsNativeInterface::nativeResourceForWindow() Called with a null window or a window without a handle, %s",
            resource.constData());

      return nullptr;
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
            return nullptr;
         }
         break;

      case QWindow::OpenGLSurface:
      case QWindow::VulkanSurface:
         break;
   }

   qWarning("QWindowsNativeInterface::nativeResourceForWindow() Invalid key, %s", resource.constData());

   return nullptr;
}

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
      platformWindow->setCustomMargins(value.value<QMargins>());
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
   if (! context || ! context->handle()) {
      qWarning("nativeResourceForContext() Called with a null context or a conttext without a handle, %s", resource.constData());
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

   qWarning("nativeResourceForContext() Invalid key, %s", resource.constData());

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
