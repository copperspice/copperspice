/***********************************************************************
*
* Copyright (c) 2012-2019 Barbara Geller
* Copyright (c) 2012-2019 Ansel Sermersheim
*
* Copyright (C) 2015 The Qt Company Ltd.
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

#ifndef QWIDGET_P_H
#define QWIDGET_P_H

#include <QtGui/qwidget.h>
#include <QtCore/qrect.h>
#include <QtCore/qlocale.h>
#include <QtCore/qset.h>
#include <QtGui/qregion.h>
#include <QtGui/qsizepolicy.h>
#include <QtGui/qstyle.h>
#include <QtGui/qapplication.h>
#include <qgraphicseffect_p.h>
#include <QtGui/qgraphicsproxywidget.h>
#include <QtGui/qgraphicsscene.h>
#include <QtGui/qgraphicsview.h>
#include <qgesture_p.h>

#ifdef Q_OS_WIN
#include <QtCore/qt_windows.h>
#include <qdnd_p.h>
#endif

#ifdef Q_WS_X11
#include <QtGui/qx11info_x11.h>
#endif

#ifdef Q_OS_MAC
#include <qt_mac_p.h>
#endif

#if defined(Q_WS_QWS)
#include <QtGui/qinputcontext.h>
#include <QtGui/qscreen_qws.h>
#endif

QT_BEGIN_NAMESPACE

// Extra QWidget data
//  - to minimize memory usage for members that are seldom used
//  - top-level widgets have extra extra data to reduce cost further
#if defined(Q_WS_QWS)
class QWSManager;
#endif

#if defined(Q_OS_MAC)
class QCoreGraphicsPaintEnginePrivate;
#endif

#if defined(Q_WS_QPA)
class QPlatformWindow;
#endif

class QPaintEngine;
class QPixmap;
class QWidgetBackingStore;
class QGraphicsProxyWidget;
class QWidgetItemV2;
class QStyle;
class QUnifiedToolbarSurface;

class QWidgetBackingStoreTracker
{

 public:
   QWidgetBackingStoreTracker();
   ~QWidgetBackingStoreTracker();

   void create(QWidget *tlw);
   void destroy();

   void registerWidget(QWidget *w);
   void unregisterWidget(QWidget *w);
   void unregisterWidgetSubtree(QWidget *w);

   inline QWidgetBackingStore *data() {
      return m_ptr;
   }

   inline QWidgetBackingStore *operator->() {
      return m_ptr;
   }

   inline QWidgetBackingStore &operator*() {
      return *m_ptr;
   }

   inline operator bool() const {
      return (0 != m_ptr);
   }

 private:
   Q_DISABLE_COPY(QWidgetBackingStoreTracker)

   QWidgetBackingStore *m_ptr;
   QSet<QWidget *> m_widgets;
};

struct QTLWExtra {
   // *************************** Cross-platform variables *****************************

   // Regular pointers (keep them together to avoid gaps on 64 bits architectures).
   QIcon *icon; // widget icon
   QPixmap *iconPixmap;
   QWidgetBackingStoreTracker backingStore;
   QWindowSurface *windowSurface;
   QPainter *sharedPainter;

   // Implicit pointers (shared_null).
   QString caption; // widget caption
   QString iconText; // widget icon text
   QString role; // widget role
   QString filePath; // widget file path

   // Other variables.
   short incw, inch; // size increments
   short basew, baseh; // base sizes
   // frame strut, don't use these directly, use QWidgetPrivate::frameStrut() instead.
   QRect frameStrut;
   QRect normalGeometry; // used by showMin/maximized/FullScreen
   Qt::WindowFlags savedFlags; // Save widget flags while showing fullscreen

   // *************************** Cross-platform bit fields ****************************
   uint opacity : 8;
   uint posFromMove : 1;
   uint sizeAdjusted : 1;
   uint inTopLevelResize : 1;
   uint inRepaint : 1;
   uint embedded : 1;

   // *************************** Platform specific values (bit fields first) **********
#if defined(Q_WS_X11)
   uint spont_unmapped: 1; // window was spontaneously unmapped
   uint dnd : 1; // DND properties installed
   uint validWMState : 1; // is WM_STATE valid?
   uint waitingForMapNotify : 1; // show() has been called, haven't got the MapNotify yet
   WId parentWinId; // parent window Id (valid after reparenting)
   WId userTimeWindow; // window id that contains user-time timestamp when WM supports a _NET_WM_USER_TIME_WINDOW atom
   QPoint fullScreenOffset;

#ifndef QT_NO_XSYNC
   WId syncUpdateCounter;
   ulong syncRequestTimestamp;
   qint32 newCounterValueHi;
   quint32 newCounterValueLo;
#endif

#elif defined(Q_OS_WIN)
   uint hotkeyRegistered: 1;   // Hot key from the STARTUPINFO has been registered.
   HICON winIconBig;           // internal big Windows icon
   HICON winIconSmall;         // internal small Windows icon

#elif defined(Q_OS_MAC)
   uint resizer : 4;
   uint isSetGeometry : 1;
   uint isMove : 1;
   quint32 wattr;
   quint32 wclass;

   IconRef windowIcon;                            // the current window icon, if set with setWindowIcon_sys.
   quint32 savedWindowAttributesFromMaximized;    // Saved attributes from when the calling updateMaximizeButton_sys()

   // This value is just to make sure we maximize and restore to the right location,
   // yet we allow apps to be maximized and manually resized.
   // The name is misleading, since this is set when maximizing the window. It is a hint to saveGeometry(..) to record
   // the starting position as 0,0 instead of the normal starting position.
   bool wasMaximized;

#elif defined(Q_WS_QWS)

#ifndef QT_NO_QWS_MANAGER
   QWSManager *qwsManager;
#endif

#elif defined(Q_WS_QPA)
   QPlatformWindow *platformWindow;
   QPlatformWindowFormat platformWindowFormat;
   quint32 screenIndex; // index in qplatformscreenlist
#endif
};

struct QWExtra {
   // *************************** Cross-platform variables *****************************

   // Regular pointers (keep them together to avoid gaps on 64 bits architectures).
   void *glContext;       // if the widget is hijacked by QGLWindowSurface
   QTLWExtra *topextra;   // only useful for TLWs

#ifndef QT_NO_GRAPHICSVIEW
   QGraphicsProxyWidget *proxyWidget; // if the widget is embedded
#endif

#ifndef QT_NO_CURSOR
   QCursor *curs;
#endif

   QPointer<QStyle> style;
   QPointer<QWidget> focus_proxy;

   // Implicit pointers (shared_empty/shared_null).
   QRegion mask; // widget mask
   QString styleSheet;

   // Other variables.
   qint32 minw;
   qint32 minh; // minimum size
   qint32 maxw;
   qint32 maxh; // maximum size
   quint16 customDpiX;
   quint16 customDpiY;
   QSize staticContentsSize;

   // *************************** Cross-platform bit fields ****************************
   uint explicitMinSize : 2;
   uint explicitMaxSize : 2;
   uint autoFillBackground : 1;
   uint nativeChildrenForced : 1;
   uint inRenderWithPainter : 1;
   uint hasMask : 1;

   // *************************** Platform specific values (bit fields first) **********

#if defined(Q_OS_WIN)
#ifndef QT_NO_DRAGANDDROP
   QOleDropTarget *dropTarget; // drop target
   QList<QPointer<QWidget> > oleDropWidgets;
#endif

#elif defined(Q_WS_X11)
   uint compress_events : 1;
   WId xDndProxy; // XDND forwarding to embedded windows

#elif defined(Q_OS_MAC)
   // Cocoa Mask stuff
   QImage maskBits;
   CGImageRef imageMask;

#endif
};

static inline bool bypassGraphicsProxyWidget(const QWidget *p)
{
   while (p) {
      if (p->windowFlags() & Qt::BypassGraphicsProxyWidget) {
         return true;
      }
      p = p->parentWidget();
   }
   return false;
}

class Q_GUI_EXPORT QWidgetPrivate
{
   Q_DECLARE_PUBLIC(QWidget)

 public:
   // *************************** Cross-platform ***************************************
   enum DrawWidgetFlags {
      DrawAsRoot = 0x01,
      DrawPaintOnScreen = 0x02,
      DrawRecursive = 0x04,
      DrawInvisible = 0x08,
      DontSubtractOpaqueChildren = 0x10,
      DontSetCompositionMode = 0x20,
      DontDrawOpaqueChildren = 0x40,
      DontDrawNativeChildren = 0x80
   };

   enum CloseMode {
      CloseNoEvent,
      CloseWithEvent,
      CloseWithSpontaneousEvent
   };

   enum Direction {
      DirectionNorth = 0x01,
      DirectionEast = 0x10,
      DirectionSouth = 0x02,
      DirectionWest = 0x20
   };

   explicit QWidgetPrivate();
   virtual ~QWidgetPrivate();

   QWExtra *extraData() const;
   QTLWExtra *topData() const;
   QTLWExtra *maybeTopData() const;
   QPainter *sharedPainter() const;
   void setSharedPainter(QPainter *painter);
   QWidgetBackingStore *maybeBackingStore() const;

   void init(QWidget *desktopWidget, Qt::WindowFlags f);
   void create_sys(WId window, bool initializeWindow, bool destroyOldWindow);
   void createRecursively();
   void createWinId(WId id = 0);

   void createTLExtra();
   void createExtra();
   void deleteExtra();
   void createSysExtra();
   void deleteSysExtra();
   void createTLSysExtra();
   void deleteTLSysExtra();
   void updateSystemBackground();
   void propagatePaletteChange();

   void setPalette_helper(const QPalette &);
   void resolvePalette();
   QPalette naturalWidgetPalette(uint inheritedMask) const;

   void setMask_sys(const QRegion &);
   void raise_sys();
   void lower_sys();
   void stackUnder_sys(QWidget *);

   void setFocus_sys();

   void updateFont(const QFont &);
   inline void setFont_helper(const QFont &font) {
      if (data.fnt == font && data.fnt.resolve() == font.resolve()) {
         return;
      }
      updateFont(font);
   }
   void resolveFont();
   QFont naturalWidgetFont(uint inheritedMask) const;

   void setLayoutDirection_helper(Qt::LayoutDirection);
   void resolveLayoutDirection();

   void setLocale_helper(const QLocale &l, bool forceUpdate = false);
   void resolveLocale();

   void setStyle_helper(QStyle *newStyle, bool propagate, bool metalHack = false);
   void inheritStyle();

   void setUpdatesEnabled_helper(bool );

   void paintBackground(QPainter *, const QRegion &, int flags = DrawAsRoot) const;
   bool isAboutToShow() const;
   QRegion prepareToRender(const QRegion &region, QWidget::RenderFlags renderFlags);
   void render_helper(QPainter *painter, const QPoint &targetOffset, const QRegion &sourceRegion,
                      QWidget::RenderFlags renderFlags);
   void render(QPaintDevice *target, const QPoint &targetOffset, const QRegion &sourceRegion,
               QWidget::RenderFlags renderFlags, bool readyToRender);
   void drawWidget(QPaintDevice *pdev, const QRegion &rgn, const QPoint &offset, int flags,
                   QPainter *sharedPainter = 0, QWidgetBackingStore *backingStore = 0);


   void paintSiblingsRecursive(QPaintDevice *pdev, const QObjectList &children, int index,
                               const QRegion &rgn, const QPoint &offset, int flags
#ifdef Q_BACKINGSTORE_SUBSURFACES
                               , const QWindowSurface *currentSurface
#endif
                               , QPainter *sharedPainter, QWidgetBackingStore *backingStore);


   QPainter *beginSharedPainter();
   bool endSharedPainter();

#ifndef QT_NO_GRAPHICSVIEW
   static QGraphicsProxyWidget *nearestGraphicsProxyWidget(const QWidget *origin);
#endif

   QWindowSurface *createDefaultWindowSurface();
   QWindowSurface *createDefaultWindowSurface_sys();
   void repaint_sys(const QRegion &rgn);

   QRect clipRect() const;
   QRegion clipRegion() const;
   void subtractOpaqueChildren(QRegion &rgn, const QRect &clipRect) const;
   void subtractOpaqueSiblings(QRegion &source, bool *hasDirtySiblingsAbove = 0,
                               bool alsoNonOpaque = false) const;
   void clipToEffectiveMask(QRegion &region) const;
   void updateIsOpaque();
   void setOpaque(bool opaque);
   void updateIsTranslucent();
   bool paintOnScreen() const;

#ifndef QT_NO_GRAPHICSEFFECT
   void invalidateGraphicsEffectsRecursively();
#endif

   const QRegion &getOpaqueChildren() const;
   void setDirtyOpaqueRegion();

   bool close_helper(CloseMode mode);

   void setWindowIcon_helper();
   void setWindowIcon_sys(bool forceReset = false);
   void setWindowOpacity_sys(qreal opacity);
   void adjustQuitOnCloseAttribute();

   void scrollChildren(int dx, int dy);
   void moveRect(const QRect &, int dx, int dy);
   void scrollRect(const QRect &, int dx, int dy);
   void invalidateBuffer_resizeHelper(const QPoint &oldPos, const QSize &oldSize);
   // ### Qt 4.6: Merge into a template function (after MSVC isn't supported anymore).
   void invalidateBuffer(const QRegion &);
   void invalidateBuffer(const QRect &);
   bool isOverlapped(const QRect &) const;
   void syncBackingStore();
   void syncBackingStore(const QRegion &region);

   void reparentFocusWidgets(QWidget *oldtlw);

   static int pointToRect(const QPoint &p, const QRect &r);

   void setWinId(WId);
   void showChildren(bool spontaneous);
   void hideChildren(bool spontaneous);
   void setParent_sys(QWidget *parent, Qt::WindowFlags);
   void scroll_sys(int dx, int dy);
   void scroll_sys(int dx, int dy, const QRect &r);
   void deactivateWidgetCleanup();
   void setGeometry_sys(int, int, int, int, bool);
   void sendPendingMoveAndResizeEvents(bool recursive = false, bool disableUpdates = false);
   void activateChildLayoutsRecursively();
   void show_recursive();
   void show_helper();
   void show_sys();
   void hide_sys();
   void hide_helper();
   void _q_showIfNotHidden();

   void setEnabled_helper(bool);
   void registerDropSite(bool);
   static void adjustFlags(Qt::WindowFlags &flags, QWidget *w = 0);

   void updateFrameStrut();
   QRect frameStrut() const;

#ifdef QT_KEYPAD_NAVIGATION
   static bool navigateToDirection(Direction direction);
   static QWidget *widgetInNavigationDirection(Direction direction);
   static bool canKeypadNavigate(Qt::Orientation orientation);
   static bool inTabWidget(QWidget *widget);
#endif

   void setWindowIconText_sys(const QString &cap);
   void setWindowIconText_helper(const QString &cap);
   void setWindowTitle_sys(const QString &cap);

#ifndef QT_NO_CURSOR
   void setCursor_sys(const QCursor &cursor);
   void unsetCursor_sys();
#endif

   void setWindowTitle_helper(const QString &cap);
   void setWindowFilePath_helper(const QString &filePath);

   bool setMinimumSize_helper(int &minw, int &minh);
   bool setMaximumSize_helper(int &maxw, int &maxh);
   virtual bool hasHeightForWidth() const;
   void setConstraints_sys();
   bool pointInsideRectAndMask(const QPoint &) const;
   QWidget *childAt_helper(const QPoint &, bool) const;
   QWidget *childAtRecursiveHelper(const QPoint &p, bool, bool includeFrame = false) const;
   void updateGeometry_helper(bool forceUpdate);

   void getLayoutItemMargins(int *left, int *top, int *right, int *bottom) const;
   void setLayoutItemMargins(int left, int top, int right, int bottom);
   void setLayoutItemMargins(QStyle::SubElement element, const QStyleOption *opt = 0);

   // aboutToDestroy() is called just before the contents of
   // QWidget::destroy() is executed. It's used to signal QWidget
   // sub-classes that their internals are about to be released.
   virtual void aboutToDestroy() {}

#ifndef QT_NO_IM
   QInputContext *assignedInputContext() const;
   QInputContext *inputContext() const;
#endif

   inline QWidget *effectiveFocusWidget() {
      QWidget *w = q_func();
      while (w->focusProxy()) {
         w = w->focusProxy();
      }
      return w;
   }

   void setModal_sys();

   // This is an helper function that return the available geometry for
   // a widget and takes care is this one is in QGraphicsView.
   // If the widget is not embed in a scene then the geometry available is
   // null, we let QDesktopWidget decide for us.
   static QRect screenGeometry(const QWidget *widget) {
      QRect screen;
#ifndef QT_NO_GRAPHICSVIEW
      QGraphicsProxyWidget *ancestorProxy = widget->d_func()->nearestGraphicsProxyWidget(widget);
      //It's embedded if it has an ancestor
      if (ancestorProxy) {
         if (!bypassGraphicsProxyWidget(widget) && ancestorProxy->scene() != 0) {
            // One view, let be smart and return the viewport rect then the popup is aligned
            if (ancestorProxy->scene()->views().size() == 1) {
               QGraphicsView *view = ancestorProxy->scene()->views().at(0);
               screen = view->mapToScene(view->viewport()->rect()).boundingRect().toRect();
            } else {
               screen = ancestorProxy->scene()->sceneRect().toRect();
            }
         }
      }
#endif
      return screen;
   }

   inline void setRedirected(QPaintDevice *replacement, const QPoint &offset) {
      Q_ASSERT(q_func()->testAttribute(Qt::WA_WState_InPaintEvent));
      redirectDev = replacement;
      redirectOffset = offset;
   }

   inline QPaintDevice *redirected(QPoint *offset) const {
      if (offset) {
         *offset = redirectDev ? redirectOffset : QPoint();
      }
      return redirectDev;
   }

   inline void restoreRedirected() {
      redirectDev = 0;
   }

   inline void enforceNativeChildren() {
      Q_Q(QWidget);

      if (!extra) {
         createExtra();
      }

      if (extra->nativeChildrenForced) {
         return;
      }

      extra->nativeChildrenForced = 1;

      for (int i = 0; i < q->children().size(); ++i) {
         if (QWidget *child = qobject_cast<QWidget *>(q->children().at(i))) {
            child->setAttribute(Qt::WA_NativeWindow);
         }
      }
   }

   inline bool nativeChildrenForced() const {
      return extra ? extra->nativeChildrenForced : false;
   }

   inline QRect effectiveRectFor(const QRect &rect) const {
#ifndef QT_NO_GRAPHICSEFFECT
      if (graphicsEffect && graphicsEffect->isEnabled()) {
         return graphicsEffect->boundingRectFor(rect).toAlignedRect();
      }
#endif
      return rect;
   }

   QSize adjustedSize() const;

   inline void handleSoftwareInputPanel(Qt::MouseButton button, bool clickCausedFocus) {
      Q_Q(QWidget);
      if (button == Qt::LeftButton && qApp->autoSipEnabled()) {
         QStyle::RequestSoftwareInputPanel behavior = QStyle::RequestSoftwareInputPanel(
                  q->style()->styleHint(QStyle::SH_RequestSoftwareInputPanel));
         if (!clickCausedFocus || behavior == QStyle::RSIP_OnMouseClick) {
            QEvent event(QEvent::RequestSoftwareInputPanel);
            QApplication::sendEvent(q, &event);
         }
      }
   }

#ifndef Q_WS_QWS
   // Almost cross-platform
   void setWSGeometry(bool dontShow = false, const QRect &oldRect = QRect());

   inline QPoint mapToWS(const QPoint &p) const {
      return p - data.wrect.topLeft();
   }

   inline QPoint mapFromWS(const QPoint &p) const {
      return p + data.wrect.topLeft();
   }

   inline QRect mapToWS(const QRect &r) const {
      QRect rr(r);
      rr.translate(-data.wrect.topLeft());
      return rr;
   }

   inline QRect mapFromWS(const QRect &r) const {
      QRect rr(r);
      rr.translate(data.wrect.topLeft());
      return rr;
   }
#endif

   // Variables.
   // Regular pointers (keep them together to avoid gaps on 64 bit architectures).
   QWExtra *extra;
   QWidget *focus_next;
   QWidget *focus_prev;
   QWidget *focus_child;
   QLayout *layout;
   QRegion *needsFlush;
   QPaintDevice *redirectDev;
   QWidgetItemV2 *widgetItem;
   QPaintEngine *extraPaintEngine;
   mutable const QMetaObject *polished;
   QGraphicsEffect *graphicsEffect;
   // All widgets are added into the allWidgets set. Once
   // they receive a window id they are also added to the mapper.
   // This should just ensure that all widgets are deleted by QApplication
   static QWidgetMapper *mapper;
   static QWidgetSet *allWidgets;

#if !defined(QT_NO_IM)
   QPointer<QInputContext> ic;
   Qt::InputMethodHints imHints;
#endif

#ifdef QT_KEYPAD_NAVIGATION
   static QPointer<QWidget> editingWidget;
#endif

   // Implicit pointers (shared_null/shared_empty).
   QRegion opaqueChildren;
   QRegion dirty;

#ifndef QT_NO_TOOLTIP
   QString toolTip;
#endif

#ifndef QT_NO_STATUSTIP
   QString statusTip;
#endif

#ifndef QT_NO_WHATSTHIS
   QString whatsThis;
#endif

#ifndef QT_NO_ACCESSIBILITY
   QString accessibleName;
   QString accessibleDescription;
#endif

   // Other variables.
   uint inheritedFontResolveMask;
   uint inheritedPaletteResolveMask;
   short leftmargin;
   short topmargin;
   short rightmargin;
   short bottommargin;
   signed char leftLayoutItemMargin;
   signed char topLayoutItemMargin;
   signed char rightLayoutItemMargin;
   signed char bottomLayoutItemMargin;
   static int instanceCounter; // Current number of widget instances
   static int maxInstances; // Maximum number of widget instances
   Qt::HANDLE hd;
   QWidgetData data;
   QSizePolicy size_policy;
   QLocale locale;
   QPoint redirectOffset;

#ifndef QT_NO_ACTION
   QList<QAction *> actions;
#endif

#ifndef QT_NO_GESTURES
   QMap<Qt::GestureType, Qt::GestureFlags> gestureContext;
#endif

   // Bit fields.
   uint high_attributes[4]; // the low ones are in QWidget::widget_attributes
   QPalette::ColorRole fg_role : 8;
   QPalette::ColorRole bg_role : 8;
   uint dirtyOpaqueChildren : 1;
   uint isOpaque : 1;
   uint inDirtyList : 1;
   uint isScrolled : 1;
   uint isMoved : 1;
   uint isGLWidget : 1;
   uint usesDoubleBufferedGLContext : 1;

#ifndef QT_NO_IM
   uint inheritsInputMethodHints : 1;
#endif
   uint inSetParent : 1;

   // *************************** Platform specific ************************************
#if defined(Q_WS_X11)
   QX11Info xinfo;
   Qt::HANDLE picture;
   static QWidget *mouseGrabber;
   static QWidget *keyboardGrabber;

   void setWindowRole();
   void sendStartupMessage(const char *message) const;
   void setNetWmWindowTypes();
   void x11UpdateIsOpaque();
   bool isBackgroundInherited() const;
   void updateX11AcceptFocus();
   QPoint mapToGlobal(const QPoint &pos) const;
   QPoint mapFromGlobal(const QPoint &pos) const;

#elif defined(Q_OS_WIN)
   uint noPaintOnScreen : 1; // see qwidget_win.cpp ::paintEngine()

#ifndef QT_NO_GESTURES
   uint nativeGesturePanEnabled : 1;
#endif

   bool shouldShowMaximizeButton();
   void winUpdateIsOpaque();
   void reparentChildren();

#ifndef QT_NO_DRAGANDDROP
   QOleDropTarget *registerOleDnd(QWidget *widget);
   void unregisterOleDnd(QWidget *widget, QOleDropTarget *target);
#endif

   void grabMouseWhileInWindow();
   void registerTouchWindow();
   void winSetupGestures();

#elif defined(Q_OS_MAC)
   uint needWindowChange : 1;

   // Each wiget keeps a list of all its child and grandchild OpenGL widgets.
   // This list is used to update the gl context whenever a parent and a granparent
   // moves, and also to check for intersections with gl widgets within the window when a widget moves
   struct GlWidgetInfo {
      GlWidgetInfo(QWidget *widget) : widget(widget), lastUpdateWidget(0) { }
      bool operator==(const GlWidgetInfo &other) const {
         return (widget == other.widget);
      }
      QWidget *widget;
      QWidget *lastUpdateWidget;
   };

   // dirtyOnWidget contains the areas in the widget that needs to be repained,
   // in the same way as dirtyOnScreen does for the window. Areas are added in
   // dirtyWidget_sys and cleared in the paint event. In scroll_sys we then use
   // this information repaint invalid areas when widgets are scrolled.
   QRegion dirtyOnWidget;

   QList<GlWidgetInfo> glWidgets;

   //these are here just for code compat (HIViews)
   Qt::HANDLE qd_hd;

   void macUpdateSizeAttribute();
   void macUpdateHideOnSuspend();
   void macUpdateOpaqueSizeGrip();
   void macUpdateIgnoreMouseEvents();
   void macUpdateMetalAttribute();
   void macUpdateIsOpaque();
   void macSetNeedsDisplay(QRegion region);
   void setEnabled_helper_sys(bool enable);
   bool isRealWindow() const;
   void adjustWithinMaxAndMinSize(int &w, int &h);
   void applyMaxAndMinSizeOnWindow();
   void update_sys(const QRect &rect);
   void update_sys(const QRegion &rgn);
   void setGeometry_sys_helper(int, int, int, int, bool);
   void setWindowModified_sys(bool b);
   void updateMaximizeButton_sys();
   void setWindowFilePath_sys(const QString &filePath);
   void createWindow_sys();
   void recreateMacWindow();

   void setSubWindowStacking(bool set);
   void setWindowLevel();
   void finishCreateWindow_sys_Cocoa(void * /*NSWindow * */ windowRef);
   void syncCocoaMask();
   void finishCocoaMaskSetup();
   void syncUnifiedMode();

   // Did we add the drawRectOriginal method?
   bool drawRectOriginalAdded;

   // Is the original drawRect method available?
   bool originalDrawMethod;

   // Do we need to change the methods?
   bool changeMethods;

   // Unified toolbar variables
   bool isInUnifiedToolbar;
   QUnifiedToolbarSurface *unifiedSurface;
   QPoint toolbar_offset;
   QWidget *toolbar_ancestor;
   bool flushRequested;
   bool touchEventsEnabled;

   void determineWindowClass();
   void transferChildren();
   bool qt_mac_dnd_event(uint, DragRef);
   void toggleDrawers(bool);

   //mac event functions
   static bool qt_create_root_win();
   static void qt_clean_root_win();
   static bool qt_mac_update_sizer(QWidget *, int up = 0);
   static bool qt_widget_rgn(QWidget *, short, RgnHandle, bool);
   void registerTouchWindow(bool enable = true);

#elif defined(Q_WS_QWS)
   void setMaxWindowState_helper();
   void setFullScreenSize_helper();
   void moveSurface(QWindowSurface *surface, const QPoint &offset);
   QRegion localRequestedRegion() const;
   QRegion localAllocatedRegion() const;

   friend class QWSManager;
   friend class QWSManagerPrivate;
   friend class QDecoration;

#ifndef QT_NO_CURSOR
   void updateCursor() const;
#endif

   QScreen *getScreen() const;

#elif defined(Q_WS_QPA)
   void setMaxWindowState_helper();
   void setFullScreenSize_helper();

#ifndef QT_NO_CURSOR
   void updateCursor() const;
#endif

#endif
   static QWidgetPrivate *cs_getPrivate(QWidget *object);

 protected:
   QWidget *q_ptr;
};

struct QWidgetPaintContext {
   inline QWidgetPaintContext(QPaintDevice *d, const QRegion &r, const QPoint &o, int f,
                              QPainter *p, QWidgetBackingStore *b)
      : pdev(d), rgn(r), offset(o), flags(f), sharedPainter(p), backingStore(b), painter(0) {}

   QPaintDevice *pdev;
   QRegion rgn;
   QPoint offset;
   int flags;
   QPainter *sharedPainter;
   QWidgetBackingStore *backingStore;
   QPainter *painter;
};

#ifndef QT_NO_GRAPHICSEFFECT
class QWidgetEffectSourcePrivate : public QGraphicsEffectSourcePrivate
{
 public:
   QWidgetEffectSourcePrivate(QWidget *widget)
      : QGraphicsEffectSourcePrivate(), m_widget(widget), context(0), updateDueToGraphicsEffect(false) {
   }

   inline void detach() override {
      m_widget->d_func()->graphicsEffect = 0;
   }

   inline const QGraphicsItem *graphicsItem() const override {
      return 0;
   }

   inline const QWidget *widget() const override {
      return m_widget;
   }

   inline void update() override {
      updateDueToGraphicsEffect = true;
      m_widget->update();
      updateDueToGraphicsEffect = false;
   }

   inline bool isPixmap() const override {
      return false;
   }

   inline void effectBoundingRectChanged() override {
      // ### This function should take a rect parameter; then we can avoid
      // updating too much on the parent widget.
      if (QWidget *parent = m_widget->parentWidget()) {
         parent->update();
      } else {
         update();
      }
   }

   inline const QStyleOption *styleOption() const override {
      return 0;
   }

   inline QRect deviceRect() const override {
      return m_widget->window()->rect();
   }

   QRectF boundingRect(Qt::CoordinateSystem system) const override;
   void draw(QPainter *p) override;

   QPixmap pixmap(Qt::CoordinateSystem system, QPoint *offset, QGraphicsEffect::PixmapPadMode mode) const override;

   QWidget *m_widget;
   QWidgetPaintContext *context;
   QTransform lastEffectTransform;
   bool updateDueToGraphicsEffect;
};
#endif //QT_NO_GRAPHICSEFFECT

inline QWExtra *QWidgetPrivate::extraData() const
{
   return extra;
}

inline QTLWExtra *QWidgetPrivate::topData() const
{
   const_cast<QWidgetPrivate *>(this)->createTLExtra();
   return extra->topextra;
}

inline QTLWExtra *QWidgetPrivate::maybeTopData() const
{
   return extra ? extra->topextra : 0;
}

inline QPainter *QWidgetPrivate::sharedPainter() const
{
   Q_Q(const QWidget);
   QTLWExtra *x = q->window()->d_func()->maybeTopData();
   return x ? x->sharedPainter : 0;
}

inline void QWidgetPrivate::setSharedPainter(QPainter *painter)
{
   Q_Q(QWidget);
   QTLWExtra *x = q->window()->d_func()->topData();
   x->sharedPainter = painter;
}

inline bool QWidgetPrivate::pointInsideRectAndMask(const QPoint &p) const
{
   Q_Q(const QWidget);
   return q->rect().contains(p) && (!extra || !extra->hasMask || q->testAttribute(Qt::WA_MouseNoMask)
                                    || extra->mask.contains(p));
}

inline QWidgetBackingStore *QWidgetPrivate::maybeBackingStore() const
{
   Q_Q(const QWidget);
   QTLWExtra *x = q->window()->d_func()->maybeTopData();
   return x ? x->backingStore.data() : 0;
}

QT_END_NAMESPACE

#endif // QWIDGET_P_H
