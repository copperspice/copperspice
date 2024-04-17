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

#ifndef QWIDGET_P_H
#define QWIDGET_P_H

#include <qwidget.h>

#include <qapplication.h>
#include <qgraphicsproxywidget.h>
#include <qgraphicsscene.h>
#include <qgraphicsview.h>
#include <qinputmethod.h>
#include <qlocale.h>
#include <qopengl.h>
#include <qrect.h>
#include <qregion.h>
#include <qset.h>
#include <qsizepolicy.h>
#include <qstyle.h>
#include <qsurfaceformat.h>

#include <qgesture_p.h>
#include <qgraphicseffect_p.h>

// Extra QWidget data
//  - to minimize memory usage for members that are seldom used
//  - top-level widgets have extra extra data to reduce cost further

class QGraphicsProxyWidget;
class QOpenGLContext;
class QPaintEngine;
class QPixmap;
class QPlatformTextureList;
class QStyle;
class QUnifiedToolbarSurface;
class QWidgetBackingStore;
class QWidgetItemV2;

// implemented in qshortcut.cpp
bool qWidgetShortcutContextMatcher(QObject *object, Qt::ShortcutContext context);

class QUpdateLaterEvent : public QEvent
{
 public:
   explicit QUpdateLaterEvent(const QRegion &paintRegion)
      : QEvent(UpdateLater), m_region(paintRegion)
   {
   }

   ~QUpdateLaterEvent()
   {
   }

   const QRegion &region() const {
      return m_region;
   }

 protected:
   QRegion m_region;
};

class QWidgetBackingStoreTracker
{
 public:
   QWidgetBackingStoreTracker();

   QWidgetBackingStoreTracker(const QWidgetBackingStoreTracker &) = delete;
   QWidgetBackingStoreTracker &operator=(const QWidgetBackingStoreTracker &) = delete;

   ~QWidgetBackingStoreTracker();

   void create(QWidget *tlw);
   void destroy();

   void registerWidget(QWidget *w);
   void unregisterWidget(QWidget *w);
   void unregisterWidgetSubtree(QWidget *w);

   QWidgetBackingStore *data() {
      return m_ptr;
   }

   QWidgetBackingStore *operator->() {
      return m_ptr;
   }

   QWidgetBackingStore &operator*() {
      return *m_ptr;
   }

   operator bool() const {
      return (m_ptr != nullptr);
   }

 private:
   QWidgetBackingStore *m_ptr;
   QSet<QWidget *> m_widgets;
};

struct QTLWExtra {
   // cross platform variables

   QIcon *icon;

   QWidgetBackingStoreTracker backingStoreTracker;
   QBackingStore *backingStore;

   QPainter *sharedPainter;
   QWindow *window;
   QOpenGLContext *shareContext;

   QString caption;
   QString iconText;
   QString role;
   QString filePath;

   short incw;
   short inch;
   short basew;
   short baseh;

   // frame strut, do not use these directly, use QWidgetPrivate::frameStrut() instead
   QRect frameStrut;
   QRect normalGeometry;
   Qt::WindowFlags savedFlags;

   int initialScreenIndex;
   QVector<QPlatformTextureList *> widgetTextures;
   uint opacity : 8;
   uint posIncludesFrame : 1;
   uint sizeAdjusted : 1;
   uint inTopLevelResize : 1;
   uint inRepaint : 1;
   uint embedded : 1;

};

struct QWExtra {
   // cross platform variables

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

   QRegion mask;
   QString styleSheet;

   qint32 minw;
   qint32 minh;
   qint32 maxw;
   qint32 maxh;
   quint16 customDpiX;
   quint16 customDpiY;
   QSize staticContentsSize;

   uint explicitMinSize : 2;
   uint explicitMaxSize : 2;
   uint autoFillBackground : 1;
   uint nativeChildrenForced : 1;
   uint inRenderWithPainter : 1;
   uint hasMask : 1;
   uint hasWindowContainer : 1;
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
   enum DrawWidgetFlags {
      DrawAsRoot = 0x01,
      DrawPaintOnScreen = 0x02,
      DrawRecursive = 0x04,
      DrawInvisible = 0x08,
      DontSubtractOpaqueChildren = 0x10,
      DontDrawOpaqueChildren = 0x20,
      DontDrawNativeChildren = 0x40,
      DontSetCompositionMode = 0x80
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

   static QWidgetPrivate *get(QWidget *w) {
      return w->d_func();
   }

   static const QWidgetPrivate *get(const QWidget *w) {
      return w->d_func();
   }

   QWExtra *extraData() const;
   QTLWExtra *topData() const;
   QTLWExtra *maybeTopData() const;
   QPainter *sharedPainter() const;
   void setSharedPainter(QPainter *painter);
   QWidgetBackingStore *maybeBackingStore() const;

   void init(QWidget *desktopWidget, Qt::WindowFlags flags);
   void create_sys(WId window, bool initializeWindow, bool destroyOldWindow);
   void createRecursively();
   void createWinId();

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
   void updateFocusChild();

   void updateFont(const QFont &);

   void setFont_helper(const QFont &font) {
      if (m_privateData.fnt.resolve() == font.resolve() && m_privateData.fnt == font) {
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
         QWidget::RenderFlags renderFlags);

   void drawWidget(QPaintDevice *pdev, const QRegion &rgn, const QPoint &offset, int flags,
         QPainter *sharedPainter = nullptr, QWidgetBackingStore *backingStore = nullptr);

   void sendPaintEvent(const QRegion &toBePainted);

   void paintSiblingsRecursive(QPaintDevice *pdev, const QObjectList &children, int index,
         const QRegion &rgn, const QPoint &offset, int flags,
         QPainter *sharedPainter, QWidgetBackingStore *backingStore);

#ifndef QT_NO_GRAPHICSVIEW
   static QGraphicsProxyWidget *nearestGraphicsProxyWidget(const QWidget *origin);
#endif

   void repaint_sys(const QRegion &rgn);

   QRect clipRect() const;
   QRegion clipRegion() const;

   void setSystemClip(QPaintDevice *paintDevice, const QRegion &region);
   void subtractOpaqueChildren(QRegion &rgn, const QRect &clipRect) const;
   void subtractOpaqueSiblings(QRegion &source, bool *hasDirtySiblingsAbove = nullptr, bool alsoNonOpaque = false) const;

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
   void setWindowIcon_sys();
   void setWindowOpacity_sys(qreal opacity);
   void adjustQuitOnCloseAttribute();

   void scrollChildren(int dx, int dy);
   void moveRect(const QRect &, int dx, int dy);
   void scrollRect(const QRect &, int dx, int dy);
   void invalidateBuffer_resizeHelper(const QPoint &oldPos, const QSize &oldSize);

   // merge into a template function ( emerald )
   void invalidateBuffer(const QRegion &);
   void invalidateBuffer(const QRect &);

   bool isOverlapped(const QRect &) const;
   void syncBackingStore();
   void syncBackingStore(const QRegion &region);

   // tells the input method about the widgets transform
   void updateWidgetTransform(QEvent *event);

   void reparentFocusWidgets(QWidget *oldtlw);

   static int pointToRect(const QPoint &p, const QRect &r);

   void setWinId(WId);
   void showChildren(bool spontaneous);
   void hideChildren(bool spontaneous);
   void setParent_sys(QWidget *parent, Qt::WindowFlags flags);
   void scroll_sys(int dx, int dy);
   void scroll_sys(int dx, int dy, const QRect &r);
   void deactivateWidgetCleanup();
   void setGeometry_sys(int, int, int, int, bool);
   void fixPosIncludesFrame();
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
   static void adjustFlags(Qt::WindowFlags &flags, QWidget *widget = nullptr);

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
   void setWindowFilePath_sys(const QString &filePath);

#ifndef QT_NO_CURSOR
   void setCursor_sys(const QCursor &cursor);
   void unsetCursor_sys();
#endif

   void setWindowTitle_helper(const QString &cap);
   void setWindowFilePath_helper(const QString &filePath);
   void setWindowModified_helper();
   virtual void setWindowFlags(Qt::WindowFlags flags);

   bool setMinimumSize_helper(int &minw, int &minh);
   bool setMaximumSize_helper(int &maxw, int &maxh);

   void setConstraints_sys();
   bool pointInsideRectAndMask(const QPoint &) const;
   QWidget *childAt_helper(const QPoint &, bool) const;
   QWidget *childAtRecursiveHelper(const QPoint &p, bool) const;
   void updateGeometry_helper(bool forceUpdate);

   void getLayoutItemMargins(int *left, int *top, int *right, int *bottom) const;
   void setLayoutItemMargins(int left, int top, int right, int bottom);
   void setLayoutItemMargins(QStyle::SubElement element, const QStyleOption *opt = nullptr);

   // aboutToDestroy() is called just before the contents of
   // QWidget::destroy() is executed. It's used to signal QWidget
   // sub-classes that their internals are about to be released.
   virtual void aboutToDestroy() {
   }

   QWidget *effectiveFocusWidget() {
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

         if (! bypassGraphicsProxyWidget(widget) && ancestorProxy->scene() != nullptr) {
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

   void setRedirected(QPaintDevice *replacement, const QPoint &offset) {
      Q_ASSERT(q_func()->testAttribute(Qt::WA_WState_InPaintEvent));
      redirectDev = replacement;
      redirectOffset = offset;
   }

   QPaintDevice *redirected(QPoint *offset) const {
      if (offset) {
         *offset = redirectDev ? redirectOffset : QPoint();
      }

      return redirectDev;
   }

   void restoreRedirected() {
      redirectDev = nullptr;
   }

   void enforceNativeChildren() {
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

   bool nativeChildrenForced() const {
      return extra ? extra->nativeChildrenForced : false;
   }

   QRect effectiveRectFor(const QRect &rect) const {
#ifndef QT_NO_GRAPHICSEFFECT

      if (graphicsEffect && graphicsEffect->isEnabled()) {
         return graphicsEffect->boundingRectFor(rect).toAlignedRect();
      }

#endif
      return rect;
   }

   QSize adjustedSize() const;

   void handleSoftwareInputPanel(Qt::MouseButton button, bool clickCausedFocus) {
      Q_Q(QWidget);

      if (button == Qt::LeftButton && qApp->autoSipEnabled()) {
         QStyle::RequestSoftwareInputPanel behavior = QStyle::RequestSoftwareInputPanel(
               q->style()->styleHint(QStyle::SH_RequestSoftwareInputPanel));

         if (! clickCausedFocus || behavior == QStyle::RSIP_OnMouseClick) {
            QGuiApplication::inputMethod()->show();
         }
      }
   }

   void setWSGeometry();

   QPoint mapToWS(const QPoint &p) const {
      return p - m_privateData.wrect.topLeft();
   }

   QPoint mapFromWS(const QPoint &p) const {
      return p + m_privateData.wrect.topLeft();
   }

   QRect mapToWS(const QRect &r) const  {
      return r.translated(-m_privateData.wrect.topLeft());
   }

   QRect mapFromWS(const QRect &r) const {
      return r.translated(m_privateData.wrect.topLeft());
   }

   QOpenGLContext *shareContext() const;
   virtual QObject *focusObject() {
      return nullptr;
   }

#ifndef QT_NO_OPENGL
   virtual GLuint textureId() const {
      return 0;
   }

   virtual QImage grabFramebuffer() {
      return QImage();
   }

   virtual void beginBackingStorePainting() {
   }

   virtual void endBackingStorePainting() {
   }

   virtual void beginCompose() {
   }

   virtual void endCompose() {
   }

   void setRenderToTexture() {
      renderToTexture = true;
      setTextureChildSeen();
   }

   void setTextureChildSeen() {
      Q_Q(QWidget);

      if (textureChildSeen) {
         return;
      }

      textureChildSeen = 1;

      if (! q->isWindow()) {
         QWidget *parent = q->parentWidget();

         if (parent) {
            get(parent)->setTextureChildSeen();
         }
      }
   }

   static void sendComposeStatus(QWidget *w, bool end);

   virtual void initializeViewportFramebuffer() {
   }

   virtual void resizeViewportFramebuffer() {
   }

   virtual void resolveSamples() {
   }
#endif

   static void setWidgetParentHelper(QObject *widgetAsObject, QObject *newParent);

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

   // All widgets are added into the allWidgets set.
   // Once they receive a window id they are also added to the mapper.
   static QWidgetMapper *mapper;
   static QWidgetSet *allWidgets;

#if ! defined(QT_NO_IM)
   Qt::InputMethodHints imHints;
#endif

#ifdef QT_KEYPAD_NAVIGATION
   static QPointer<QWidget> editingWidget;
#endif

   QRegion opaqueChildren;
   QRegion dirty;

#ifndef QT_NO_TOOLTIP
   QString toolTip;
   int toolTipDuration;
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
   static int instanceCounter;   // Current number of widget instances
   static int maxInstances;      // Maximum number of widget instances

   Qt::HANDLE hd;
   QWidgetData m_privateData;
   QSizePolicy size_policy;
   QLocale locale;
   QPoint redirectOffset;

#ifndef QT_NO_ACTION
   QList<QAction *> actions;
#endif

#ifndef QT_NO_GESTURES
   QMap<Qt::GestureType, Qt::GestureFlags> gestureContext;
#endif

   uint high_attributes[4];            // low ones are in QWidget::widget_attributes
   QPalette::ColorRole fg_role : 8;
   QPalette::ColorRole bg_role : 8;
   uint dirtyOpaqueChildren : 1;
   uint isOpaque : 1;
   uint retainSizeWhenHiddenChanged : 1;
   uint inDirtyList : 1;
   uint isScrolled : 1;
   uint isMoved : 1;
   uint usesDoubleBufferedGLContext : 1;
   uint mustHaveWindowHandle : 1;
   uint renderToTexture : 1;
   uint textureChildSeen : 1;

#ifndef QT_NO_IM
   uint inheritsInputMethodHints : 1;
#endif

#ifndef QT_NO_OPENGL
   uint renderToTextureReallyDirty : 1;
   uint renderToTextureComposeActive : 1;
#endif

   uint childrenHiddenByWState : 1;
   uint childrenShownByExpose : 1;

#if defined(Q_OS_WIN)
   uint noPaintOnScreen : 1;    // refer to qwidget.cpp ::paintEngine()
#endif

#if defined(Q_OS_DARWIN)
   void macUpdateSizeAttribute();
#endif

   void setNetWmWindowTypes(bool skipIfMissing = false);

   bool stealKeyboardGrab(bool grab);
   bool stealMouseGrab(bool grab);

   static QWidgetPrivate *cs_getPrivate(QWidget *object);
   static QWidget *cs_getPublic(QWidgetPrivate *object);

 protected:
   QWidget *q_ptr;
};

struct QWidgetPaintContext {
   inline QWidgetPaintContext(QPaintDevice *d, const QRegion &r, const QPoint &o, int f,
         QPainter *p, QWidgetBackingStore *b)
      : pdev(d), rgn(r), offset(o), flags(f), sharedPainter(p), backingStore(b), painter(nullptr) {}

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
      : QGraphicsEffectSourcePrivate(), m_widget(widget), context(nullptr), updateDueToGraphicsEffect(false)
   { }

   void detach() override {
      m_widget->d_func()->graphicsEffect = nullptr;
   }

   const QGraphicsItem *graphicsItem() const override {
      return nullptr;
   }

   const QWidget *widget() const override {
      return m_widget;
   }

   void update() override {
      updateDueToGraphicsEffect = true;
      m_widget->update();
      updateDueToGraphicsEffect = false;
   }

   bool isPixmap() const override {
      return false;
   }

   void effectBoundingRectChanged() override {
      // ### should take a rect parameter; then we can avoid updating too much on the parent widget
      if (QWidget *parent = m_widget->parentWidget()) {
         parent->update();
      } else {
         update();
      }
   }

   const QStyleOption *styleOption() const override {
      return nullptr;
   }

   QRect deviceRect() const override {
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
#endif

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
   return extra ? extra->topextra : nullptr;
}

inline QPainter *QWidgetPrivate::sharedPainter() const
{
   Q_Q(const QWidget);
   QTLWExtra *x = q->window()->d_func()->maybeTopData();
   return x ? x->sharedPainter : nullptr;
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
   return x ? x->backingStoreTracker.data() : nullptr;
}

#endif
