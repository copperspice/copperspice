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

#ifndef QCOCOAWINDOW_H
#define QCOCOAWINDOW_H

#include <Cocoa/Cocoa.h>

#include <qplatform_window.h>
#include <qrect.h>
#include <qpointer.h>

#ifndef QT_NO_OPENGL
#include <qcocoaglcontext.h>
#endif

#include <qnsview.h>
#include <qt_mac_p.h>

QT_FORWARD_DECLARE_CLASS(QCocoaWindow)

class QCocoaWindowPointer
{
 public:
   void assign(QCocoaWindow *w);
   void clear();

   QCocoaWindow *data() const {
      return watcher.isNull() ? nullptr : window;
   }
   bool isNull() const {
      return watcher.isNull();
   }
   operator QCocoaWindow *() const {
      return data();
   }
   QCocoaWindow *operator->() const {
      return data();
   }
   QCocoaWindow &operator*() const {
      return *data();
   }

 private:
   QPointer<QObject> watcher;
   QCocoaWindow *window;
};


@class QNSWindowHelper;

@protocol QNSWindowProtocol

@property (nonatomic, readonly) QNSWindowHelper *helper;

- (void)superSendEvent: (NSEvent *)theEvent;
- (void)closeAndRelease;

@end

typedef NSWindow<QNSWindowProtocol> QCocoaNSWindow;

@interface QNSWindowHelper : NSObject
{
   QCocoaNSWindow *_window;
   QCocoaWindowPointer _platformWindow;
   BOOL _grabbingMouse;
   BOOL _releaseOnMouseUp;
}

@property (nonatomic, readonly) QCocoaNSWindow *window;
@property (nonatomic, readonly) QCocoaWindowPointer platformWindow;
@property (nonatomic) BOOL grabbingMouse;
@property (nonatomic) BOOL releaseOnMouseUp;

- (id)initWithNSWindow: (QCocoaNSWindow *)window platformWindow: (QCocoaWindow *)platformWindow;
- (void)handleWindowEvent: (NSEvent *)theEvent;
- (void) clearWindow;

@end

@interface QNSWindow : NSWindow<QNSWindowProtocol>
{
   QNSWindowHelper *_helper;
}

@property (nonatomic, readonly) QNSWindowHelper *helper;

- (id)initWithContentRect: (NSRect)contentRect
                styleMask: (NSUInteger)windowStyle
          qPlatformWindow: (QCocoaWindow *)qpw;

@end

@interface QNSPanel : NSPanel<QNSWindowProtocol>
{
   QNSWindowHelper *_helper;
}

@property (nonatomic, readonly) QNSWindowHelper *helper;

- (id)initWithContentRect: (NSRect)contentRect
                styleMask: (NSUInteger)windowStyle
          qPlatformWindow: (QCocoaWindow *)qpw;

@end

@class QNSWindowDelegate;


// QCocoaWindow
//
// QCocoaWindow is an NSView (not an NSWindow!) in the sense
// that it relies on a NSView for all event handling and
// graphics output and does not require a NSWindow, except for
// for the window-related functions like setWindowTitle.
//
// As a consequence of this it is possible to embed the QCocoaWindow
// in an NSView hierarchy by getting a pointer to the "backing"
// NSView and not calling QCocoaWindow::show():
//
// QWindow *qtWindow = new MyWindow();
// qtWindow->create();
// QPlatformNativeInterface *platformNativeInterface = QApplication::platformNativeInterface();
// NSView *qtView = (NSView *)platformNativeInterface->nativeResourceForWindow("nsview", qtWindow);
// [parentView addSubview:qtView];
//
// See the qt_on_cocoa manual tests for a working example, located
// in tests/manual/cocoa at the time of writing.

class QCocoaMenuBar;

class QCocoaWindow : public QPlatformWindow
{
 public:
   QCocoaWindow(QWindow *tlw);
   ~QCocoaWindow();

   void setGeometry(const QRect &rect) override;
   QRect geometry() const override;
   void setCocoaGeometry(const QRect &rect);
   void clipChildWindows();
   void clipWindow(const NSRect &clipRect);
   void show(bool becauseOfAncestor = false);
   void hide(bool becauseOfAncestor = false);
   void setVisible(bool visible) override;
   void setWindowFlags(Qt::WindowFlags flags) override;
   void setWindowState(Qt::WindowState state) override;
   void setWindowTitle(const QString &title) override;
   void setWindowFilePath(const QString &filePath) override;
   void setWindowIcon(const QIcon &icon) override;
   void setAlertState(bool enabled) override;
   bool isAlertState() const override;
   void raise() override;
   void lower() override;
   bool isExposed() const override;
   bool isOpaque() const;
   void propagateSizeHints() override;
   void setOpacity(qreal level) override;
   void setMask(const QRegion &region) override;
   bool setKeyboardGrabEnabled(bool grab) override;
   bool setMouseGrabEnabled(bool grab) override;
   QMargins frameMargins() const override;
   QSurfaceFormat format() const override;

   void requestActivateWindow() override;

   WId winId() const override;
   void setParent(const QPlatformWindow *window) override;

   NSView *contentView() const;
   void setContentView(NSView *contentView);
   QNSView *qtView() const;
   NSWindow *nativeWindow() const;

   void setEmbeddedInForeignView(bool subwindow);

   void windowWillMove();
   void windowDidMove();
   void windowDidResize();
   void windowDidEndLiveResize();
   bool windowShouldClose();
   bool windowIsPopupType(Qt::WindowType type = Qt::Widget) const;

   void setSynchedWindowStateFromWindow();

   NSInteger windowLevel(Qt::WindowFlags flags);
   NSUInteger windowStyleMask(Qt::WindowFlags flags);
   void setWindowShadow(Qt::WindowFlags flags);
   void setWindowZoomButton(Qt::WindowFlags flags);

#ifndef QT_NO_OPENGL
   void setCurrentContext(QCocoaGLContext *context);
   QCocoaGLContext *currentContext() const;
#endif

   bool setWindowModified(bool modified) override;

   void setFrameStrutEventsEnabled(bool enabled) override;
   bool frameStrutEventsEnabled() const override {
      return m_frameStrutEventsEnabled;
   }

   void setMenubar(QCocoaMenuBar *mb);
   QCocoaMenuBar *menubar() const;

   NSCursor *effectiveWindowCursor() const;
   void applyEffectiveWindowCursor();
   void setWindowCursor(NSCursor *cursor);

   void registerTouch(bool enable);
   void setContentBorderThickness(int topThickness, int bottomThickness);
   void registerContentBorderArea(quintptr identifier, int upper, int lower);
   void setContentBorderAreaEnabled(quintptr identifier, bool enable);
   void setContentBorderEnabled(bool enable);
   bool testContentBorderAreaPosition(int position) const;
   void applyContentBorderThickness(NSWindow *window);
   void updateNSToolbar();

   qreal devicePixelRatio() const override;
   bool isWindowExposable();
   void exposeWindow();
   void obscureWindow();
   void updateExposedGeometry();
   QWindow *childWindowAt(QPoint windowPoint);
   bool shouldRefuseKeyWindowAndFirstResponder();

   static QPoint bottomLeftClippedByNSWindowOffsetStatic(QWindow *window);
   QPoint bottomLeftClippedByNSWindowOffset() const;
 protected:
   void recreateWindow(const QPlatformWindow *parentWindow);
   QCocoaNSWindow *createNSWindow();
   void setNSWindow(QCocoaNSWindow *window);

   bool shouldUseNSPanel();

   QRect nativeWindowGeometry() const;
   QCocoaWindow *parentCocoaWindow() const;
   void syncWindowState(Qt::WindowState newState);
   void reinsertChildWindow(QCocoaWindow *child);
   void removeChildWindow(QCocoaWindow *child);

   // private:
 public: // for QNSView
   friend class QCocoaBackingStore;
   friend class QCocoaNativeInterface;

   bool alwaysShowToolWindow() const;
   void removeMonitor();

   NSView *m_contentView;
   QNSView *m_qtView;
   QCocoaNSWindow *m_nsWindow;
   QCocoaWindowPointer m_forwardWindow;

   // TODO merge to one variable if possible
   bool m_contentViewIsEmbedded; // true if the m_contentView is actually embedded in a "foreign" NSView hiearchy
   bool m_contentViewIsToBeEmbedded; // true if the m_contentView is intended to be embedded in a "foreign" NSView hiearchy

   QCocoaWindow *m_parentCocoaWindow;
   bool m_isNSWindowChild; // this window is a non-top level QWindow with a NSWindow.
   QList<QCocoaWindow *> m_childWindows;

   Qt::WindowFlags m_windowFlags;
   bool m_effectivelyMaximized;
   Qt::WindowState m_synchedWindowState;
   Qt::WindowModality m_windowModality;
   QPointer<QWindow> m_enterLeaveTargetWindow;
   bool m_windowUnderMouse;

   bool m_inConstructor;
   bool m_inSetVisible;
   bool m_inSetGeometry;
   bool m_inSetStyleMask;

#ifndef QT_NO_OPENGL
   QCocoaGLContext *m_glContext;
#endif

   QCocoaMenuBar *m_menubar;
   NSCursor *m_windowCursor;

   bool m_hasModalSession;
   bool m_frameStrutEventsEnabled;
   bool m_geometryUpdateExposeAllowed;
   bool m_isExposed;
   QRect m_exposedGeometry;
   qreal m_exposedDevicePixelRatio;
   int m_registerTouchCount;
   bool m_resizableTransientParent;
   bool m_hiddenByClipping;
   bool m_hiddenByAncestor;

   static const int NoAlertRequest;
   NSInteger m_alertRequest;
   id monitor;

   bool m_drawContentBorderGradient;
   int m_topContentBorderThickness;
   int m_bottomContentBorderThickness;

   // used by showFullScreen in fake mode
   QRect m_normalGeometry;
   Qt::WindowFlags m_oldWindowFlags;
   NSApplicationPresentationOptions m_presentationOptions;

   struct BorderRange {
      BorderRange(quintptr i, int u, int l) : identifier(i), upper(u), lower(l) { }
      quintptr identifier;
      int upper;
      int lower;
      bool operator<(BorderRange const &right) const {
         return upper < right.upper;
      }
   };
   QHash<quintptr, BorderRange> m_contentBorderAreas; // identifer -> uppper/lower
   QHash<quintptr, bool> m_enabledContentBorderAreas; // identifer -> enabled state (true/false)

   // This object is tracked by QCocoaWindowPointer,
   // preventing the use of dangling pointers.
   QObject sentinel;
   bool m_hasWindowFilePath;
};

#endif // QCOCOAWINDOW_H

