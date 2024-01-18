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

#ifndef QPLATFORMINTEGRATION_COCOA_H
#define QPLATFORMINTEGRATION_COCOA_H

#include <Cocoa/Cocoa.h>

#include <qcocoacursor.h>
#include <qcocoawindow.h>
#include <qcocoanativeinterface.h>
#include <qcocoainputcontext.h>
#include <qcocoaaccessibility.h>
#include <qcocoaclipboard.h>
#include <qcocoadrag.h>
#include <qcocoaservices.h>
#include <qcocoakeymapper.h>

#include <qscopedpointer.h>
#include <qplatform_integration.h>

#include <qcoretextfontdatabase_p.h>

class QCocoaScreen : public QPlatformScreen
{
 public:
   QCocoaScreen(int screenIndex);
   ~QCocoaScreen();

   // ----------------------------------------------------
   // Virtual methods overridden from QPlatformScreen
   QPixmap grabWindow(WId window, int x, int y, int width, int height) const override;
   QRect geometry() const override {
      return m_geometry;
   }
   QRect availableGeometry() const override {
      return m_availableGeometry;
   }
   int depth() const override {
      return m_depth;
   }
   QImage::Format format() const override {
      return m_format;
   }
   qreal devicePixelRatio() const override;
   QSizeF physicalSize() const override {
      return m_physicalSize;
   }
   QDpi logicalDpi() const override {
      return m_logicalDpi;
   }
   qreal refreshRate() const override {
      return m_refreshRate;
   }
   QString name() const override {
      return m_name;
   }
   QPlatformCursor *cursor() const override {
      return m_cursor;
   }

   QWindow *topLevelWindowAt(const QPoint &point) const override;
   QList<QPlatformScreen *> virtualSiblings() const override {
      return m_siblings;
   }
   QPlatformScreen::SubpixelAntialiasingType subpixelAntialiasingTypeHint() const override;

   void setVirtualSiblings(const QList<QPlatformScreen *> &siblings) {
      m_siblings = siblings;
   }
   NSScreen *osScreen() const;
   void updateGeometry();

 public:
   int m_screenIndex;
   QRect m_geometry;
   QRect m_availableGeometry;
   QDpi m_logicalDpi;
   qreal m_refreshRate;
   int m_depth;
   QString m_name;
   QImage::Format m_format;
   QSizeF m_physicalSize;
   QCocoaCursor *m_cursor;
   QList<QPlatformScreen *> m_siblings;
};

class QCocoaIntegration : public QPlatformIntegration
{
 public:
   enum Option {
      UseFreeTypeFontEngine = 0x1
   };

   using Options = QFlags<Option>;

   QCocoaIntegration(const QStringList &paramList);
   ~QCocoaIntegration();

   static QCocoaIntegration *instance();
   Options options() const;

   bool hasCapability(QPlatformIntegration::Capability cap) const override;
   QPlatformWindow *createPlatformWindow(QWindow *window) const override;
#ifndef QT_NO_OPENGL
   QPlatformOpenGLContext *createPlatformOpenGLContext(QOpenGLContext *context) const override;
#endif
   QPlatformBackingStore *createPlatformBackingStore(QWindow *widget) const override;

   QAbstractEventDispatcher *createEventDispatcher() const override;

   QCoreTextFontDatabase *fontDatabase() const override;
   QCocoaNativeInterface *nativeInterface() const override;
   QPlatformInputContext *inputContext() const override;

#ifndef QT_NO_ACCESSIBILITY
   QCocoaAccessibility *accessibility() const override;
#endif

   QCocoaClipboard *clipboard() const override;
   QCocoaDrag *drag() const override;

   QStringList themeNames() const override;
   QPlatformTheme *createPlatformTheme(const QString &name) const override;
   QCocoaServices *services() const override;
   QVariant styleHint(StyleHint hint) const override;

   Qt::KeyboardModifiers queryKeyboardModifiers() const override;
   QList<int> possibleKeys(const QKeyEvent *event) const override;

   void updateScreens();
   QCocoaScreen *screenAtIndex(int index);

   void setToolbar(QWindow *window, NSToolbar *toolbar);
   NSToolbar *toolbar(QWindow *window) const;
   void clearToolbars();

   void pushPopupWindow(QCocoaWindow *window);
   QCocoaWindow *popPopupWindow();
   QCocoaWindow *activePopupWindow() const;
   QList<QCocoaWindow *> *popupWindowStack();

   void setApplicationIcon(const QIcon &icon) const override;

 private:
   static QCocoaIntegration *mInstance;
   Options mOptions;

   QScopedPointer<QCoreTextFontDatabase> mFontDb;

   QScopedPointer<QPlatformInputContext> mInputContext;

#ifndef QT_NO_ACCESSIBILITY
   QScopedPointer<QCocoaAccessibility> mAccessibility;
#endif

   QScopedPointer<QPlatformTheme> mPlatformTheme;
   QList<QCocoaScreen *> mScreens;
   QCocoaClipboard  *mCocoaClipboard;
   QScopedPointer<QCocoaDrag> mCocoaDrag;
   QScopedPointer<QCocoaNativeInterface> mNativeInterface;
   QScopedPointer<QCocoaServices> mServices;
   QScopedPointer<QCocoaKeyMapper> mKeyboardMapper;

   QHash<QWindow *, NSToolbar *> mToolbars;
   QList<QCocoaWindow *> m_popupWindowStack;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QCocoaIntegration::Options)

#endif

