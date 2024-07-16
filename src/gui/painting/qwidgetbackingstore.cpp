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

#include <qplatform_backingstore.h>
#include <qwidgetbackingstore_p.h>

#include <qapplication.h>
#include <qdebug.h>
#include <qevent.h>
#include <qglobal.h>
#include <qgraphicsproxywidget.h>
#include <qpaintengine.h>
#include <qplatformdefs.h>
#include <qvarlengtharray.h>

#include <qapplication_p.h>
#include <qpaintengine_raster_p.h>
#include <qgraphicseffect_p.h>
#include <qwidget_p.h>
#include <qwindow_p.h>

#if defined(Q_OS_WIN)
#  include <qt_windows.h>
#  include <qplatform_nativeinterface.h>
#endif

extern QRegion qt_dirtyRegion(QWidget *);

#ifndef QT_NO_OPENGL

static QPlatformTextureList *qt_dummy_platformTextureList()
{
   static QPlatformTextureList retval;
   return &retval;
}

#endif

void QWidgetBackingStore::qt_flush(QWidget *widget, const QRegion &region, QBackingStore *backingStore,
      QWidget *tlw, const QPoint &tlwOffset, QPlatformTextureList *widgetTextures,
      QWidgetBackingStore *widgetBackingStore)
{
   (void) widgetBackingStore;

#ifdef QT_NO_OPENGL
   (void) widgetTextures;
   Q_ASSERT(! region.isEmpty());
#else
   Q_ASSERT(! region.isEmpty() || widgetTextures);
#endif

   Q_ASSERT(widget);
   Q_ASSERT(backingStore);
   Q_ASSERT(tlw);

   if (tlw->testAttribute(Qt::WA_DontShowOnScreen) || widget->testAttribute(Qt::WA_DontShowOnScreen)) {
      return;
   }

   QPoint offset = tlwOffset;
   if (widget != tlw) {
      offset += widget->mapTo(tlw, QPoint());
   }

   QRegion effectiveRegion = region;

#ifndef QT_NO_OPENGL
   const bool compositionWasActive = widget->d_func()->renderToTextureComposeActive;

   if (widgetTextures == nullptr) {
      widget->d_func()->renderToTextureComposeActive = false;

      // Detect the case of falling back to the normal flush path when no
      // render-to-texture widgets are visible anymore. We will force one
      // last flush to go through the OpenGL-based composition to prevent
      // artifacts. The next flush after this one will use the normal path.

      if (compositionWasActive) {
         widgetTextures = qt_dummy_platformTextureList();
      }

   } else {
      widget->d_func()->renderToTextureComposeActive = true;
   }

   // When changing the composition status, make sure the dirty region covers
   // the entire widget.  Just having e.g. the shown/hidden render-to-texture
   // widget's area marked as dirty is incorrect when changing flush paths.
   if (compositionWasActive != widget->d_func()->renderToTextureComposeActive) {
      effectiveRegion = widget->rect();
   }

   // re-test since we may have been forced to this path via the dummy texture list above
   if (widgetTextures == nullptr) {
      backingStore->flush(effectiveRegion, widget->windowHandle(), offset);

   } else {
      qt_window_private(tlw->windowHandle())->compositing = true;
      widget->window()->d_func()->sendComposeStatus(widget->window(), false);

      // A window may have alpha even when the app did not request
      // WA_TranslucentBackground. Therefore the compositor needs to know whether the app intends
      // to rely on translucency, in order to decide if it should clear to transparent or opaque.
      const bool translucentBackground = widget->testAttribute(Qt::WA_TranslucentBackground);

      // Use the tlw's context, not widget's. The difference is important with native child
      // widgets where tlw != widget.
      backingStore->handle()->composeAndFlush(widget->windowHandle(), effectiveRegion, offset,
            widgetTextures, tlw->d_func()->shareContext(), translucentBackground);

      widget->window()->d_func()->sendComposeStatus(widget->window(), true);
   }

#else
   backingStore->flush(effectiveRegion, widget->windowHandle(), offset);

#endif


}

bool QWidgetBackingStore::bltRect(const QRect &rect, int dx, int dy, QWidget *widget)
{
   const QPoint pos(tlwOffset + widget->mapTo(tlw, rect.topLeft()));
   const QRect tlwRect(QRect(pos, rect.size()));

   if (fullUpdatePending || dirty.intersects(tlwRect)) {
      return false;   // do not want to scroll
   }

   return store->scroll(tlwRect, dx, dy);
}

void QWidgetBackingStore::releaseBuffer()
{
   if (store) {
      store->resize(QSize());
   }
}

void QWidgetBackingStore::beginPaint(QRegion &toClean, QWidget *widget, QBackingStore *backingStore,
   BeginPaintInfo *returnInfo, bool toCleanIsInTopLevelCoordinates)
{
   (void) widget;
   (void) returnInfo;
   (void) toCleanIsInTopLevelCoordinates;

   // always flush repainted areas
   dirtyOnScreen += toClean;

   backingStore->beginPaint(toClean);
}

void QWidgetBackingStore::endPaint(const QRegion &cleaned, QBackingStore *backingStore, BeginPaintInfo *beginPaintInfo)
{
   (void) cleaned;
   (void) beginPaintInfo;

   backingStore->endPaint();
   flush();
}

QRegion QWidgetBackingStore::dirtyRegion(QWidget *widget) const
{
   const bool widgetDirty = widget && widget != tlw;
   const QRect tlwRect(topLevelRect());
   const QRect surfaceGeometry(tlwRect.topLeft(), store->size());

   if (fullUpdatePending || (surfaceGeometry != tlwRect && surfaceGeometry.size() != tlwRect.size())) {
      if (widgetDirty) {
         const QRect dirtyTlwRect = QRect(QPoint(), tlwRect.size());
         const QPoint offset(widget->mapTo(tlw, QPoint()));
         const QRect dirtyWidgetRect(dirtyTlwRect & widget->rect().translated(offset));
         return dirtyWidgetRect.translated(-offset);
      }
      return QRect(QPoint(), tlwRect.size());
   }

   // Calculate the region that needs repaint.
   QRegion r(dirty);
   for (int i = 0; i < dirtyWidgets.size(); ++i) {
      QWidget *w = dirtyWidgets.at(i);
      if (widgetDirty && w != widget && !widget->isAncestorOf(w)) {
         continue;
      }
      r += w->d_func()->dirty.translated(w->mapTo(tlw, QPoint()));
   }

   // Append the region that needs flush.
   r += dirtyOnScreen;

   if (dirtyOnScreenWidgets) { // Only in use with native child widgets.
      for (int i = 0; i < dirtyOnScreenWidgets->size(); ++i) {
         QWidget *w = dirtyOnScreenWidgets->at(i);
         if (widgetDirty && w != widget && !widget->isAncestorOf(w)) {
            continue;
         }
         QWidgetPrivate *wd = w->d_func();
         Q_ASSERT(wd->needsFlush);
         r += wd->needsFlush->translated(w->mapTo(tlw, QPoint()));
      }
   }

   if (widgetDirty) {
      // Intersect with the widget geometry and translate to its coordinates.
      const QPoint offset(widget->mapTo(tlw, QPoint()));
      r &= widget->rect().translated(offset);
      r.translate(-offset);
   }
   return r;
}

/*!
    Returns the static content inside the \a parent if non-zero; otherwise the static content
    for the entire backing store is returned. The content will be clipped to \a withinClipRect
    if non-empty.
*/
QRegion QWidgetBackingStore::staticContents(QWidget *parent, const QRect &withinClipRect) const
{
   if (!parent && tlw->testAttribute(Qt::WA_StaticContents)) {
      const QSize surfaceGeometry(store->size());

      QRect surfaceRect(0, 0, surfaceGeometry.width(), surfaceGeometry.height());
      if (!withinClipRect.isEmpty()) {
         surfaceRect &= withinClipRect;
      }
      return QRegion(surfaceRect);
   }

   QRegion region;
   if (parent && parent->children().isEmpty()) {
      return region;
   }

   const bool clipToRect = !withinClipRect.isEmpty();
   const int count = staticWidgets.count();
   for (int i = 0; i < count; ++i) {
      QWidget *w = staticWidgets.at(i);
      QWidgetPrivate *wd = w->d_func();
      if (!wd->isOpaque || !wd->extra || wd->extra->staticContentsSize.isEmpty()
         || !w->isVisible() || (parent && !parent->isAncestorOf(w))) {
         continue;
      }

      QRect rect(0, 0, wd->extra->staticContentsSize.width(), wd->extra->staticContentsSize.height());
      const QPoint offset = w->mapTo(parent ? parent : tlw, QPoint());
      if (clipToRect) {
         rect &= withinClipRect.translated(-offset);
      }
      if (rect.isEmpty()) {
         continue;
      }

      rect &= wd->clipRect();
      if (rect.isEmpty()) {
         continue;
      }

      QRegion visible(rect);
      wd->clipToEffectiveMask(visible);
      if (visible.isEmpty()) {
         continue;
      }
      wd->subtractOpaqueSiblings(visible, nullptr, true);

      visible.translate(offset);
      region += visible;
   }

   return region;
}

void QWidgetBackingStore::sendUpdateRequest(QWidget *widget, UpdateTime updateTime)
{
   if (! widget) {
      return;
   }

#ifndef QT_NO_OPENGL
   // Having every repaint() leading to a sync/flush is bad as it causes compositing and waiting for
   // vsync each and every time. Change to UpdateLater, except for approx. once per frame to prevent starvation in
   // case the control does not get back to the event loop.

   QWidget *tmpWidget = widget->window();

   if (updateTime == UpdateNow && tmpWidget && tmpWidget->windowHandle()) {

      bool isEnabled = QWindowPrivate::get(tmpWidget->windowHandle())->compositing;

      if (isEnabled) {
         int refresh = 60;
         QScreen *tmpScreen = tmpWidget->windowHandle()->screen();

         if (tmpScreen) {
            refresh = tmpScreen->refreshRate();
         }

         QWindowPrivate *tmpWinP = QWindowPrivate::get(tmpWidget->windowHandle());

         if (tmpWinP->lastComposeTime.isValid()) {
            const qint64 elapsed = tmpWinP->lastComposeTime.elapsed();

            if (elapsed <= qint64(1000.0f / refresh)) {
               updateTime = UpdateLater;
            }
         }
      }
   }
#endif

   switch (updateTime) {
      case UpdateLater:
         updateRequestSent = true;
         QApplication::postEvent(widget, new QEvent(QEvent::UpdateRequest), Qt::LowEventPriority);
         break;

      case UpdateNow: {
         QEvent event(QEvent::UpdateRequest);
         QApplication::sendEvent(widget, &event);
         break;
      }
   }
}

void QWidgetBackingStore::markDirty(const QRegion &rgn, QWidget *widget, UpdateTime updateTime, BufferState bufferState)
{
   Q_ASSERT(tlw->d_func()->extra);
   Q_ASSERT(tlw->d_func()->extra->topextra);
   Q_ASSERT(!tlw->d_func()->extra->topextra->inTopLevelResize);
   Q_ASSERT(widget->isVisible() && widget->updatesEnabled());
   Q_ASSERT(widget->window() == tlw);
   Q_ASSERT(!rgn.isEmpty());

#ifndef QT_NO_GRAPHICSEFFECT
   widget->d_func()->invalidateGraphicsEffectsRecursively();
#endif

   if (widget->d_func()->paintOnScreen()) {
      if (widget->d_func()->dirty.isEmpty()) {
         widget->d_func()->dirty = rgn;
         sendUpdateRequest(widget, updateTime);
         return;

      } else if (qt_region_strictContains(widget->d_func()->dirty, widget->rect())) {
         if (updateTime == UpdateNow) {
            sendUpdateRequest(widget, updateTime);
         }

         return; // Already dirty.
      }

      const bool eventAlreadyPosted = !widget->d_func()->dirty.isEmpty();
      widget->d_func()->dirty += rgn;

      if (!eventAlreadyPosted || updateTime == UpdateNow) {
         sendUpdateRequest(widget, updateTime);
      }

      return;
   }

   if (fullUpdatePending) {
      if (updateTime == UpdateNow) {
         sendUpdateRequest(tlw, updateTime);
      }
      return;
   }

   const QPoint offset = widget->mapTo(tlw, QPoint());

   if (QWidgetPrivate::get(widget)->renderToTexture) {

      if (!widget->d_func()->inDirtyList) {
         addDirtyRenderToTextureWidget(widget);
      }

      if (!updateRequestSent || updateTime == UpdateNow) {
         sendUpdateRequest(tlw, updateTime);
      }

      return;
   }

   const QRect widgetRect = widget->d_func()->effectiveRectFor(widget->rect());
   if (qt_region_strictContains(dirty, widgetRect.translated(offset))) {
      if (updateTime == UpdateNow) {
         sendUpdateRequest(tlw, updateTime);
      }
      return; // Already dirty.
   }

   if (bufferState == BufferInvalid) {
      const bool eventAlreadyPosted = !dirty.isEmpty() || updateRequestSent;

#ifndef QT_NO_GRAPHICSEFFECT
      if (widget->d_func()->graphicsEffect) {
         dirty += widget->d_func()->effectiveRectFor(rgn.boundingRect()).translated(offset);
      } else

#endif
         dirty += rgn.translated(offset);

      if (!eventAlreadyPosted || updateTime == UpdateNow) {
         sendUpdateRequest(tlw, updateTime);
      }
      return;
   }

   if (dirtyWidgets.isEmpty()) {
      addDirtyWidget(widget, rgn);
      sendUpdateRequest(tlw, updateTime);
      return;
   }

   if (widget->d_func()->inDirtyList) {
      if (!qt_region_strictContains(widget->d_func()->dirty, widgetRect)) {
#ifndef QT_NO_GRAPHICSEFFECT
         if (widget->d_func()->graphicsEffect) {
            widget->d_func()->dirty += widget->d_func()->effectiveRectFor(rgn.boundingRect());
         } else
#endif //QT_NO_GRAPHICSEFFECT
            widget->d_func()->dirty += rgn;
      }
   } else {
      addDirtyWidget(widget, rgn);
   }

   if (updateTime == UpdateNow) {
      sendUpdateRequest(tlw, updateTime);
   }
}

/*!
    This function is equivalent to calling markDirty(QRegion(rect), ...), but
    is more efficient as it eliminates QRegion operations/allocations and can
    use the rect more precisely for additional cut-offs.

    ### Merge into a template function (after MSVC isn't supported anymore).
*/
void QWidgetBackingStore::markDirty(const QRect &rect, QWidget *widget,
   UpdateTime updateTime, BufferState bufferState)
{
   Q_ASSERT(tlw->d_func()->extra);
   Q_ASSERT(tlw->d_func()->extra->topextra);
   Q_ASSERT(!tlw->d_func()->extra->topextra->inTopLevelResize);
   Q_ASSERT(widget->isVisible() && widget->updatesEnabled());
   Q_ASSERT(widget->window() == tlw);
   Q_ASSERT(!rect.isEmpty());

#ifndef QT_NO_GRAPHICSEFFECT
   widget->d_func()->invalidateGraphicsEffectsRecursively();
#endif

   if (widget->d_func()->paintOnScreen()) {
      if (widget->d_func()->dirty.isEmpty()) {
         widget->d_func()->dirty = QRegion(rect);
         sendUpdateRequest(widget, updateTime);
         return;
      } else if (qt_region_strictContains(widget->d_func()->dirty, rect)) {
         if (updateTime == UpdateNow) {
            sendUpdateRequest(widget, updateTime);
         }
         return; // Already dirty.
      }

      const bool eventAlreadyPosted = !widget->d_func()->dirty.isEmpty();
      widget->d_func()->dirty += rect;
      if (!eventAlreadyPosted || updateTime == UpdateNow) {
         sendUpdateRequest(widget, updateTime);
      }
      return;
   }

   if (fullUpdatePending) {
      if (updateTime == UpdateNow) {
         sendUpdateRequest(tlw, updateTime);
      }
      return;
   }

   if (QWidgetPrivate::get(widget)->renderToTexture) {
      if (!widget->d_func()->inDirtyList) {
         addDirtyRenderToTextureWidget(widget);
      }

      if (!updateRequestSent || updateTime == UpdateNow) {
         sendUpdateRequest(tlw, updateTime);
      }
      return;
   }

   const QRect widgetRect = widget->d_func()->effectiveRectFor(rect);
   QRect translatedRect = widgetRect;
   if (widget != tlw) {
      translatedRect.translate(widget->mapTo(tlw, QPoint()));
   }
   translatedRect = translatedRect.intersected(QRect(QPoint(), tlw->size()));
   if (qt_region_strictContains(dirty, translatedRect)) {
      if (updateTime == UpdateNow) {
         sendUpdateRequest(tlw, updateTime);
      }
      return; // Already dirty
   }

   if (bufferState == BufferInvalid) {
      const bool eventAlreadyPosted = !dirty.isEmpty();
      dirty += translatedRect;
      if (!eventAlreadyPosted || updateTime == UpdateNow) {
         sendUpdateRequest(tlw, updateTime);
      }
      return;
   }

   if (dirtyWidgets.isEmpty()) {
      addDirtyWidget(widget, rect);
      sendUpdateRequest(tlw, updateTime);
      return;
   }

   if (widget->d_func()->inDirtyList) {
      if (!qt_region_strictContains(widget->d_func()->dirty, widgetRect)) {
         widget->d_func()->dirty += widgetRect;
      }
   } else {
      addDirtyWidget(widget, rect);
   }

   if (updateTime == UpdateNow) {
      sendUpdateRequest(tlw, updateTime);
   }
}

/*!
    Marks the \a region of the \a widget as dirty on screen. The \a region will be copied from
    the backing store to the \a widget's native parent next time flush() is called.

    Paint on screen widgets are ignored.
*/
void QWidgetBackingStore::markDirtyOnScreen(const QRegion &region, QWidget *widget, const QPoint &topLevelOffset)
{
   if (!widget || widget->d_func()->paintOnScreen() || region.isEmpty()) {
      return;
   }



   // Top-level.
   if (widget == tlw) {
      if (! widget->testAttribute(Qt::WA_WState_InPaintEvent)) {
         dirtyOnScreen += region;
      }
      return;
   }

   // Alien widgets.
   if (!widget->internalWinId() && !widget->isWindow()) {
      QWidget *nativeParent = widget->nativeParentWidget();        // Alien widgets with the top-level as the native parent (common case).

      if (nativeParent == tlw) {
         if (!widget->testAttribute(Qt::WA_WState_InPaintEvent)) {
            dirtyOnScreen += region.translated(topLevelOffset);
         }
         return;
      }

      // Alien widgets with native parent != tlw.
      QWidgetPrivate *nativeParentPrivate = nativeParent->d_func();
      if (!nativeParentPrivate->needsFlush) {
         nativeParentPrivate->needsFlush = new QRegion;
      }
      const QPoint nativeParentOffset = widget->mapTo(nativeParent, QPoint());
      *nativeParentPrivate->needsFlush += region.translated(nativeParentOffset);
      appendDirtyOnScreenWidget(nativeParent);
      return;
   }

   // Native child widgets.
   QWidgetPrivate *widgetPrivate = widget->d_func();
   if (!widgetPrivate->needsFlush) {
      widgetPrivate->needsFlush = new QRegion;
   }
   *widgetPrivate->needsFlush += region;
   appendDirtyOnScreenWidget(widget);
}

void QWidgetBackingStore::removeDirtyWidget(QWidget *widget)
{
   if (! widget) {
      return;
   }

   dirtyWidgetsRemoveAll(widget);
   dirtyOnScreenWidgetsRemoveAll(widget);
   dirtyRenderToTextureWidgets.removeAll(widget);
   resetWidget(widget);

   const int n = widget->children().count();

   for (int i = 0; i < n; ++i) {
      if (QWidget *child = qobject_cast<QWidget *>(widget->children().at(i))) {
         removeDirtyWidget(child);
      }
   }
}

void QWidgetBackingStore::updateLists(QWidget *cur)
{
   if (!cur) {
      return;
   }

   QList<QObject *> children = cur->children();
   for (int i = 0; i < children.size(); ++i) {
      QWidget *child = qobject_cast<QWidget *>(children.at(i));
      if (!child) {
         continue;
      }

      updateLists(child);
   }

   if (cur->testAttribute(Qt::WA_StaticContents)) {
      addStaticWidget(cur);
   }
}

QWidgetBackingStore::QWidgetBackingStore(QWidget *topLevel)
   : tlw(topLevel), dirtyOnScreenWidgets(nullptr), fullUpdatePending(0), updateRequestSent(0),
     textureListWatcher(nullptr), perfFrames(0)
{
   store = tlw->backingStore();
   Q_ASSERT(store);

   // Ensure all existing subsurfaces and static widgets are added to their respective lists.
   updateLists(topLevel);
}

QWidgetBackingStore::~QWidgetBackingStore()
{
   for (int c = 0; c < dirtyWidgets.size(); ++c) {
      resetWidget(dirtyWidgets.at(c));
   }

   for (int c = 0; c < dirtyRenderToTextureWidgets.size(); ++c) {
      resetWidget(dirtyRenderToTextureWidgets.at(c));
   }

   delete dirtyOnScreenWidgets;
}

//parent's coordinates; move whole rect; update parent and widget
//assume the screen blt has already been done, so we don't need to refresh that part
void QWidgetPrivate::moveRect(const QRect &rect, int dx, int dy)
{
   Q_Q(QWidget);

   if (!q->isVisible() || (dx == 0 && dy == 0)) {
      return;
   }

   QWidget *tlw = q->window();
   QTLWExtra *x = tlw->d_func()->topData();
   if (x->inTopLevelResize) {
      return;
   }

   static int accelEnv = -1;
   if (accelEnv == -1) {
      accelEnv = qgetenv("QT_NO_FAST_MOVE").toInt() == 0;
   }

   QWidget *pw = q->parentWidget();
   QPoint toplevelOffset = pw->mapTo(tlw, QPoint());
   QWidgetPrivate *pd = pw->d_func();
   QRect clipR(pd->clipRect());

   const QRect newRect(rect.translated(dx, dy));
   QRect destRect = rect.intersected(clipR);
   if (destRect.isValid()) {
      destRect = destRect.translated(dx, dy).intersected(clipR);
   }

   const QRect sourceRect(destRect.translated(-dx, -dy));
   const QRect parentRect(rect & clipR);
   const bool nativeWithTextureChild = textureChildSeen && q->internalWinId();

   bool accelerateMove = accelEnv && isOpaque && !nativeWithTextureChild

#ifndef QT_NO_GRAPHICSVIEW
      // No accelerate move for proxy widgets.
      && !tlw->d_func()->extra->proxyWidget
#endif
      && !isOverlapped(sourceRect) && !isOverlapped(destRect);

   if (!accelerateMove) {
      QRegion parentR(effectiveRectFor(parentRect));
      if (!extra || !extra->hasMask) {
         parentR -= newRect;
      } else {
         // invalidateBuffer() excludes anything outside the mask
         parentR += newRect & clipR;
      }

      pd->invalidateBuffer(parentR);
      invalidateBuffer((newRect & clipR).translated(- (m_privateData.crect.topLeft()) ));

   } else {

      QWidgetBackingStore *wbs = x->backingStoreTracker.data();
      QRegion childExpose(newRect & clipR);

      if (sourceRect.isValid() && wbs->bltRect(sourceRect, dx, dy, pw)) {
         childExpose -= destRect;
      }

      if (!pw->updatesEnabled()) {
         return;
      }

      const bool childUpdatesEnabled = q->updatesEnabled();
      if (childUpdatesEnabled && !childExpose.isEmpty()) {
         childExpose.translate(- (m_privateData.crect.topLeft()) );
         wbs->markDirty(childExpose, q);
         isMoved = true;
      }

      QRegion parentExpose(parentRect);
      parentExpose -= newRect;
      if (extra && extra->hasMask) {
         parentExpose += QRegion(newRect) - extra->mask.translated(m_privateData.crect.topLeft());
      }

      if (!parentExpose.isEmpty()) {
         wbs->markDirty(parentExpose, pw);
         pd->isMoved = true;
      }

      if (childUpdatesEnabled) {
         QRegion needsFlush(sourceRect);
         needsFlush += destRect;
         wbs->markDirtyOnScreen(needsFlush, pw, toplevelOffset);
      }
   }
}

//widget's coordinates; scroll within rect;  only update widget
void QWidgetPrivate::scrollRect(const QRect &rect, int dx, int dy)
{
   Q_Q(QWidget);

   QWidget *tlw = q->window();
   QTLWExtra *x = tlw->d_func()->topData();

   if (x->inTopLevelResize) {
      return;
   }

   QWidgetBackingStore *wbs = x->backingStoreTracker.data();
   if (!wbs) {
      return;
   }

   static int accelEnv = -1;
   if (accelEnv == -1) {
      accelEnv = qgetenv("QT_NO_FAST_SCROLL").toInt() == 0;
   }

   QRect scrollRect = rect & clipRect();
   bool overlapped = false;
   bool accelerateScroll = accelEnv && isOpaque && !q_func()->testAttribute(Qt::WA_WState_InPaintEvent)
      && !(overlapped = isOverlapped(scrollRect.translated(m_privateData.crect.topLeft())));

   if (! accelerateScroll) {
      if (overlapped) {
         QRegion region(scrollRect);
         subtractOpaqueSiblings(region);
         invalidateBuffer(region);
      } else {
         invalidateBuffer(scrollRect);
      }

   } else {
      const QPoint toplevelOffset = q->mapTo(tlw, QPoint());


      const QRect destRect = scrollRect.translated(dx, dy) & scrollRect;
      const QRect sourceRect = destRect.translated(-dx, -dy);

      QRegion childExpose(scrollRect);
      if (sourceRect.isValid()) {
         if (wbs->bltRect(sourceRect, dx, dy, q)) {
            childExpose -= destRect;
         }
      }

      if (inDirtyList) {
         if (rect == q->rect()) {
            dirty.translate(dx, dy);
         } else {
            QRegion dirtyScrollRegion = dirty.intersected(scrollRect);
            if (!dirtyScrollRegion.isEmpty()) {
               dirty -= dirtyScrollRegion;
               dirtyScrollRegion.translate(dx, dy);
               dirty += dirtyScrollRegion;
            }
         }
      }

      if (!q->updatesEnabled()) {
         return;
      }

      if (!childExpose.isEmpty()) {
         wbs->markDirty(childExpose, q);
         isScrolled = true;
      }

      // Instead of using native scroll-on-screen, we copy from
      // backingstore, giving only one screen update for each
      // scroll, and a solid appearance
      wbs->markDirtyOnScreen(destRect, q, toplevelOffset);
   }
}

#ifndef QT_NO_OPENGL

static void findTextureWidgetsRecursively(QWidget *tlw, QWidget *widget, QPlatformTextureList *widgetTextures,
   QVector<QWidget *> *nativeChildren)
{
   QWidgetPrivate *wd = QWidgetPrivate::get(widget);

   if (wd->renderToTexture) {
      QPlatformTextureList::Flags flags = Qt::EmptyFlag;

      if (widget->testAttribute(Qt::WA_AlwaysStackOnTop)) {
         flags |= QPlatformTextureList::StacksOnTop;
      }

      const QRect rect(widget->mapTo(tlw, QPoint()), widget->size());
      widgetTextures->appendTexture(widget, wd->textureId(), rect, wd->clipRect(), flags);
   }

   for (int i = 0; i < widget->children().size(); ++i) {
      QWidget *w = qobject_cast<QWidget *>(widget->children().at(i));

      // Stop at native widgets but store them. Stop at hidden widgets too.
      if (w && ! w->isWindow() && w->internalWinId()) {
         nativeChildren->append(w);
      }

      if (w && ! w->isWindow() && ! w->internalWinId() && ! w->isHidden() && QWidgetPrivate::get(w)->textureChildSeen) {
         findTextureWidgetsRecursively(tlw, w, widgetTextures, nativeChildren);
      }
   }
}

static void findAllTextureWidgetsRecursively(QWidget *tlw, QWidget *widget)
{
   // textureChildSeen does not take native child widgets into account and that's good.
   if (QWidgetPrivate::get(widget)->textureChildSeen) {
      QVector<QWidget *> nativeChildren;
      QScopedPointer<QPlatformTextureList> tl(new QPlatformTextureList);

      // Look for texture widgets (incl. widget itself) from 'widget' down,
      // but skip subtrees with a parent of a native child widget.
      findTextureWidgetsRecursively(tlw, widget, tl.data(), &nativeChildren);

      // tl may be empty regardless of textureChildSeen if we have native or hidden children.
      if (! tl->isEmpty()) {
         QWidgetPrivate::get(tlw)->topData()->widgetTextures.append(tl.take());
      }

      // Native child widgets, if there was any, get their own separate QPlatformTextureList.
      for (QWidget *ncw : nativeChildren) {
         if (QWidgetPrivate::get(ncw)->textureChildSeen) {
            findAllTextureWidgetsRecursively(tlw, ncw);
         }
      }
   }
}

static QPlatformTextureList *widgetTexturesFor(QWidget *tlw, QWidget *widget)
{
   for (QPlatformTextureList *tl : QWidgetPrivate::get(tlw)->topData()->widgetTextures) {
      Q_ASSERT(! tl->isEmpty());

      for (int i = 0; i < tl->count(); ++i) {
         QWidget *w = static_cast<QWidget *>(tl->source(i));

         if ((w->internalWinId() && w == widget) || (! w->internalWinId() && w->nativeParentWidget() == widget)) {
            return tl;
         }
      }
   }

   if (QWidgetPrivate::get(tlw)->textureChildSeen) {
      // No render-to-texture widgets in the (sub-)tree due to hidden or native
      // children. Returning null results in using the normal backingstore flush path
      // without OpenGL-based compositing. This is very desirable normally. However,
      // some platforms cannot handle switching between the non-GL and GL paths for
      // their windows so it has to be opt-in.

      static bool switchableWidgetComposition = QGuiApplicationPrivate::instance()->platformIntegration()
         ->hasCapability(QPlatformIntegration::SwitchableWidgetComposition);

#if defined(Q_OS_WIN)
      // Windows compositor handles fullscreen OpenGL window specially. It has
      // issues with popups and changing between OpenGL-based and normal flushing.
      // Stay with GL for fullscreen windows.
      // Similary, translucent windows should not switch to layered native windows.

      if (! switchableWidgetComposition ||
            tlw->windowState().testFlag(Qt::WindowFullScreen) || tlw->testAttribute(Qt::WA_TranslucentBackground)) {

#else
      if (! switchableWidgetComposition) {

#endif

         return qt_dummy_platformTextureList();
      }
   }

   return nullptr;
}

// Watches one or more QPlatformTextureLists for changes in the lock state and
// triggers a backingstore sync when all the registered lists turn into
// unlocked state. This is essential when a custom composeAndFlush()
// implementation in a platform plugin is not synchronous and keeps
// holding on to the textures for some time even after returning from there.
QPlatformTextureListWatcher::QPlatformTextureListWatcher(QWidgetBackingStore *backingStore)
   : m_backingStore(backingStore)
{
}

void QPlatformTextureListWatcher::watch(QPlatformTextureList *textureList)
{
   connect(textureList, &QPlatformTextureList::locked, this, &QPlatformTextureListWatcher::onLockStatusChanged);
   m_locked[textureList] = textureList->isLocked();
}

bool QPlatformTextureListWatcher::isLocked() const
{
   for (bool v : m_locked) {
      if (v) {
         return true;
      }
   }

   return false;
}

void QPlatformTextureListWatcher::onLockStatusChanged(bool locked)
{
   QPlatformTextureList *tl = static_cast<QPlatformTextureList *>(sender());
   m_locked[tl] = locked;

   if (!isLocked()) {
      m_backingStore->sync();
   }
}

#else

static QPlatformTextureList *widgetTexturesFor(QWidget *tlw, QWidget *widget)
{

   return nullptr;
}

#endif // QT_NO_OPENGL

static inline bool discardSyncRequest(QWidget *tlw, QTLWExtra *tlwExtra)
{
   if (!tlw || !tlwExtra || !tlw->testAttribute(Qt::WA_Mapped) || !tlw->isVisible()) {
      return true;
   }

   return false;
}

bool QWidgetBackingStore::syncAllowed()
{
#ifndef QT_NO_OPENGL
   QTLWExtra *tlwExtra = tlw->d_func()->maybeTopData();

   if (textureListWatcher && !textureListWatcher->isLocked()) {
      textureListWatcher->deleteLater();
      textureListWatcher = nullptr;

   } else if (!tlwExtra->widgetTextures.isEmpty()) {
      bool skipSync = false;

      for (QPlatformTextureList *tl : tlwExtra->widgetTextures) {
         if (tl->isLocked()) {
            if (!textureListWatcher) {
               textureListWatcher = new QPlatformTextureListWatcher(this);
            }

            if (!textureListWatcher->isLocked()) {
               textureListWatcher->watch(tl);
            }
            skipSync = true;
         }
      }

      if (skipSync) {
         // cannot compose due to widget textures being in use
         return false;
      }
   }
#endif
   return true;
}

void QWidgetBackingStore::sync(QWidget *exposedWidget, const QRegion &exposedRegion)
{
   QTLWExtra *tlwExtra = tlw->d_func()->maybeTopData();

   if (!tlw->isVisible() || !tlwExtra || tlwExtra->inTopLevelResize) {
      return;
   }

   if (!exposedWidget || !exposedWidget->internalWinId() || !exposedWidget->isVisible() || !exposedWidget->testAttribute(Qt::WA_Mapped)
      || !exposedWidget->updatesEnabled() || exposedRegion.isEmpty()) {
      return;
   }

   // If there's no preserved contents support we always need
   // to do a full repaint before flushing

   if (!isDirty() && store->size().isValid()) {
      QPlatformTextureList *tl = widgetTexturesFor(tlw, exposedWidget);

      qt_flush(exposedWidget, tl ? QRegion() : exposedRegion, store, tlw, tlwOffset, tl, this);
      return;
   }

   if (exposedWidget != tlw) {
      markDirtyOnScreen(exposedRegion, exposedWidget, exposedWidget->mapTo(tlw, QPoint()));
   } else {
      markDirtyOnScreen(exposedRegion, exposedWidget, QPoint());
   }

   if (syncAllowed()) {
      doSync();
   }
}

void QWidgetBackingStore::sync()
{
   updateRequestSent = false;
   QTLWExtra *tlwExtra = tlw->d_func()->maybeTopData();

   if (discardSyncRequest(tlw, tlwExtra)) {
      // If the top-level is minimized, it's not visible on the screen so we can delay the
      // update until it's shown again. In order to do that we must keep the dirty states.
      // These will be cleared when we receive the first expose after showNormal().
      // However, if the widget is not visible (isVisible() returns false), everything will
      // be invalidated once the widget is shown again, so clear all dirty states.

      if (! tlw->isVisible()) {
         dirty = QRegion();
         for (int i = 0; i < dirtyWidgets.size(); ++i) {
            resetWidget(dirtyWidgets.at(i));
         }

         dirtyWidgets.clear();
         fullUpdatePending = false;
      }
      return;
   }

   if (syncAllowed()) {
      doSync();
   }
}
void QWidgetBackingStore::doSync()
{
   const bool updatesDisabled = ! tlw->updatesEnabled();
   bool repaintAllWidgets = false;

   const bool inTopLevelResize = tlw->d_func()->maybeTopData()->inTopLevelResize;
   const QRect tlwRect(topLevelRect());

   const QRect surfaceGeometry(tlwRect.topLeft(), store->size());

   if ((fullUpdatePending || inTopLevelResize || surfaceGeometry.size() != tlwRect.size()) && ! updatesDisabled) {

      if (hasStaticContents() && !store->size().isEmpty() ) {
         // Repaint existing dirty area and newly visible area.
         const QRect clipRect(0, 0, surfaceGeometry.width(), surfaceGeometry.height());
         const QRegion staticRegion(staticContents(nullptr, clipRect));

         QRegion newVisible(0, 0, tlwRect.width(), tlwRect.height());
         newVisible -= staticRegion;
         dirty      += newVisible;

         store->setStaticContents(staticRegion);

      } else {
         // Repaint everything
         dirty = QRegion(0, 0, tlwRect.width(), tlwRect.height());

         for (int i = 0; i < dirtyWidgets.size(); ++i) {
            resetWidget(dirtyWidgets.at(i));
         }

         dirtyWidgets.clear();
         repaintAllWidgets = true;
      }
   }

   if (inTopLevelResize || surfaceGeometry.size() != tlwRect.size()) {
      store->resize(tlwRect.size());
   }

   if (updatesDisabled) {
      return;
   }

   // Contains everything that needs repaint
   QRegion toClean(dirty);

   // Loop through all update() widgets and remove them from the list before they are
   // painted (in case someone calls update() in paintEvent). If the widget is opaque
   // and does not have transparent overlapping siblings, append it to the
   // opaqueNonOverlappedWidgets list and paint it directly without composition.

   QVarLengthArray<QWidget *, 32> opaqueNonOverlappedWidgets;

   for (int i = 0; i < dirtyWidgets.size(); ++i) {
      QWidget *w = dirtyWidgets.at(i);
      QWidgetPrivate *wd = w->d_func();

      if (wd->m_privateData.in_destructor) {
         continue;
      }

      // Clip with mask() and clipRect().
      wd->dirty &= wd->clipRect();
      wd->clipToEffectiveMask(wd->dirty);

      // Subtract opaque siblings and children.
      bool hasDirtySiblingsAbove = false;

      // We know for sure that the widget is not overlapped if 'isMoved' is true.
      if (! wd->isMoved) {
         wd->subtractOpaqueSiblings(wd->dirty, &hasDirtySiblingsAbove);
      }

      // Make a copy of the widget's dirty region, to restore it in case there is an opaque
      // render-to-texture child that completely covers the widget, because otherwise the
      // render-to-texture child won't be visible, due to its parent widget not being redrawn
      // with a proper blending mask.
      const QRegion dirtyBeforeSubtractedOpaqueChildren = wd->dirty;

      // Scrolled and moved widgets must draw all children.
      if (! wd->isScrolled && !wd->isMoved) {
         wd->subtractOpaqueChildren(wd->dirty, w->rect());
      }

      if (wd->dirty.isEmpty() && wd->textureChildSeen) {
         wd->dirty = dirtyBeforeSubtractedOpaqueChildren;
      }

      if (wd->dirty.isEmpty()) {
         resetWidget(w);
         continue;
      }

      const QRegion widgetDirty(w != tlw ? wd->dirty.translated(w->mapTo(tlw, QPoint())) : wd->dirty);
      toClean += widgetDirty;

#ifndef QT_NO_GRAPHICSVIEW
      if (tlw->d_func()->extra->proxyWidget) {
         resetWidget(w);
         continue;
      }
#endif

      if (! hasDirtySiblingsAbove && wd->isOpaque && ! dirty.intersects(widgetDirty.boundingRect())) {
         opaqueNonOverlappedWidgets.append(w);
      } else {
         resetWidget(w);
         dirty += widgetDirty;
      }
   }
   dirtyWidgets.clear();

#ifndef QT_NO_OPENGL
   // Find all render-to-texture child widgets (including self).
   // The search is cut at native widget boundaries, meaning that each native child widget
   // has its own list for the subtree below it.
   QTLWExtra *tlwExtra = tlw->d_func()->topData();
   qDeleteAll(tlwExtra->widgetTextures);

   tlwExtra->widgetTextures.clear();
   findAllTextureWidgetsRecursively(tlw, tlw);

   qt_window_private(tlw->windowHandle())->compositing = false; // will get updated in qt_flush()
   fullUpdatePending = false;
#endif

   if (toClean.isEmpty()) {
      // Nothing to repaint. However renderToTexture widgets are handled
      // specially, they are not in the regular dirty list, in order to
      // prevent triggering unnecessary backingstore painting when only the
      // OpenGL content changes. Check if we have such widgets in the special
      // dirty list.
      QVarLengthArray<QWidget *, 16> paintPending;
      const int numPaintPending = dirtyRenderToTextureWidgets.count();
      paintPending.reserve(numPaintPending);

      for (int i = 0; i < numPaintPending; ++i) {
         QWidget *w = dirtyRenderToTextureWidgets.at(i);
         paintPending << w;
         resetWidget(w);
      }

      dirtyRenderToTextureWidgets.clear();
      for (int i = 0; i < numPaintPending; ++i) {
         QWidget *w = paintPending[i];
         w->d_func()->sendPaintEvent(w->rect());
         if (w != tlw) {
            QWidget *npw = w->nativeParentWidget();
            if (w->internalWinId() || (npw && npw != tlw)) {
               if (!w->internalWinId()) {
                  w = npw;
               }
               QWidgetPrivate *wPrivate = w->d_func();
               if (!wPrivate->needsFlush) {
                  wPrivate->needsFlush = new QRegion;
               }
               appendDirtyOnScreenWidget(w);
            }
         }
      }
      // We might have newly exposed areas on the screen if this function was
      // called from sync(QWidget *, QRegion)), so we have to make sure those
      // are flushed. We also need to composite the renderToTexture widgets.
      flush();

      return;
   }

#ifndef QT_NO_OPENGL
   for (QPlatformTextureList *tl : tlwExtra->widgetTextures) {
      for (int i = 0; i < tl->count(); ++i) {
         QWidget *w = static_cast<QWidget *>(tl->source(i));

         if (dirtyRenderToTextureWidgets.contains(w)) {
            const QRect rect = tl->geometry(i); // mapped to the tlw already
            // Set a flag to indicate that the paint event for this
            // render-to-texture widget must not to be optimized away.
            w->d_func()->renderToTextureReallyDirty = 1;
            dirty += rect;
            toClean += rect;
         }
      }
   }

   for (int i = 0; i < dirtyRenderToTextureWidgets.count(); ++i) {
      resetWidget(dirtyRenderToTextureWidgets.at(i));
   }

   dirtyRenderToTextureWidgets.clear();
#endif

#ifndef QT_NO_GRAPHICSVIEW
   if (tlw->d_func()->extra->proxyWidget) {
      updateStaticContentsSize();
      dirty = QRegion();
      updateRequestSent = false;
      const QVector<QRect> rects(toClean.rects());

      for (int i = 0; i < rects.size(); ++i) {
         tlw->d_func()->extra->proxyWidget->update(rects.at(i));
      }
      return;
   }
#endif

   BeginPaintInfo beginPaintInfo;
   beginPaint(toClean, tlw, store, &beginPaintInfo);

   if (beginPaintInfo.nothingToPaint) {
      for (int i = 0; i < opaqueNonOverlappedWidgets.size(); ++i) {
         resetWidget(opaqueNonOverlappedWidgets[i]);
      }

      dirty = QRegion();
      updateRequestSent = false;
      return;
   }

   // Must do this before sending any paint events because the size may change in the paint event.
   updateStaticContentsSize();

   const QRegion dirtyCopy(dirty);
   dirty = QRegion();
   updateRequestSent = false;

   // Paint opaque non overlapped widgets.
   for (int i = 0; i < opaqueNonOverlappedWidgets.size(); ++i) {
      QWidget *w = opaqueNonOverlappedWidgets[i];
      QWidgetPrivate *wd = w->d_func();

      int flags = QWidgetPrivate::DrawRecursive;

      // Scrolled and moved widgets must draw all children.
      if (!wd->isScrolled && !wd->isMoved) {
         flags |= QWidgetPrivate::DontDrawOpaqueChildren;
      }

      if (w == tlw) {
         flags |= QWidgetPrivate::DrawAsRoot;
      }

      QRegion toBePainted(wd->dirty);
      resetWidget(w);

      QPoint offset(tlwOffset);

      if (w != tlw) {
         offset += w->mapTo(tlw, QPoint());
      }

      wd->drawWidget(store->paintDevice(), toBePainted, offset, flags, nullptr, this);
   }

   // Paint the rest with composition.
   if (repaintAllWidgets || ! dirtyCopy.isEmpty()) {
      const int flags = QWidgetPrivate::DrawAsRoot | QWidgetPrivate::DrawRecursive;
      tlw->d_func()->drawWidget(store->paintDevice(), dirtyCopy, tlwOffset, flags, nullptr, this);
   }

   endPaint(toClean, store, &beginPaintInfo);
}

void QWidgetBackingStore::flush(QWidget *widget)
{
   const bool hasDirtyOnScreenWidgets = dirtyOnScreenWidgets && !dirtyOnScreenWidgets->isEmpty();
   bool flushed = false;

   if (! dirtyOnScreen.isEmpty()) {
      QWidget *target = widget ? widget : tlw;

      qt_flush(target, dirtyOnScreen, store, tlw, tlwOffset, widgetTexturesFor(tlw, tlw), this);
      dirtyOnScreen = QRegion();
      flushed = true;
   }

   // Render-to-texture widgets are not in dirtyOnScreen so flush if we have not done it above.
   if (!flushed && !hasDirtyOnScreenWidgets) {
#ifndef QT_NO_OPENGL
      if (! tlw->d_func()->topData()->widgetTextures.isEmpty()) {
         QPlatformTextureList *tl = widgetTexturesFor(tlw, tlw);
         if (tl) {
            QWidget *target = widget ? widget : tlw;
            qt_flush(target, QRegion(), store, tlw, tlwOffset, tl, this);
         }
      }
#endif
   }

   if (! hasDirtyOnScreenWidgets) {
      return;
   }

   for (int i = 0; i < dirtyOnScreenWidgets->size(); ++i) {
      QWidget *w = dirtyOnScreenWidgets->at(i);
      QWidgetPrivate *wd = w->d_func();
      Q_ASSERT(wd->needsFlush);

      QPlatformTextureList *widgetTexturesForNative = wd->textureChildSeen ? widgetTexturesFor(tlw, w) : nullptr;
      qt_flush(w, *wd->needsFlush, store, tlw, tlwOffset, widgetTexturesForNative, this);
      *wd->needsFlush = QRegion();
   }

   dirtyOnScreenWidgets->clear();
}

static inline bool discardInvalidateBufferRequest(QWidget *widget, QTLWExtra *tlwExtra)
{
   Q_ASSERT(widget);
   if (QApplication::closingDown()) {
      return true;
   }

   if (!tlwExtra || tlwExtra->inTopLevelResize || !tlwExtra->backingStore) {
      return true;
   }

   if (!widget->isVisible() || !widget->updatesEnabled()) {
      return true;
   }

   return false;
}

/*!
    Invalidates the buffer when the widget is resized.
    Static areas are never invalidated unless absolutely needed.
*/
void QWidgetPrivate::invalidateBuffer_resizeHelper(const QPoint &oldPos, const QSize &oldSize)
{
   Q_Q(QWidget);

   Q_ASSERT(!q->isWindow());
   Q_ASSERT(q->parentWidget());

   const bool staticContents = q->testAttribute(Qt::WA_StaticContents);

   const bool sizeDecreased = (m_privateData.crect.width() < oldSize.width()) ||
         (m_privateData.crect.height() < oldSize.height());

   const QPoint offset(m_privateData.crect.x() - oldPos.x(), m_privateData.crect.y() - oldPos.y());

   const bool parentAreaExposed = !offset.isNull() || sizeDecreased;
   const QRect newWidgetRect(q->rect());
   const QRect oldWidgetRect(0, 0, oldSize.width(), oldSize.height());

   if (! staticContents || graphicsEffect) {
      QRegion staticChildren;
      QWidgetBackingStore *bs = nullptr;

      if (offset.isNull() && (bs = maybeBackingStore())) {
         staticChildren = bs->staticContents(q, oldWidgetRect);
      }
      const bool hasStaticChildren = !staticChildren.isEmpty();

      if (hasStaticChildren) {
         QRegion dirty(newWidgetRect);
         dirty -= staticChildren;
         invalidateBuffer(dirty);
      } else {
         // Entire widget needs repaint.
         invalidateBuffer(newWidgetRect);
      }

      if (!parentAreaExposed) {
         return;
      }

      // Invalidate newly exposed area of the parent.
      if (!graphicsEffect && extra && extra->hasMask) {
         QRegion parentExpose(extra->mask.translated(oldPos));
         parentExpose &= QRect(oldPos, oldSize);
         if (hasStaticChildren) {
            parentExpose -= m_privateData.crect;   // Offset is unchanged, safe to do this.
         }
         q->parentWidget()->d_func()->invalidateBuffer(parentExpose);

      } else {
         if (hasStaticChildren && !graphicsEffect) {
            QRegion parentExpose(QRect(oldPos, oldSize));
            parentExpose -= m_privateData.crect; // Offset is unchanged, safe to do this.
            q->parentWidget()->d_func()->invalidateBuffer(parentExpose);

         } else {
            q->parentWidget()->d_func()->invalidateBuffer(effectiveRectFor(QRect(oldPos, oldSize)));
         }
      }
      return;
   }

   // Move static content to its new position.
   if (!offset.isNull()) {
      if (sizeDecreased) {
         const QSize minSize(qMin(oldSize.width(), m_privateData.crect.width()),
            qMin(oldSize.height(), m_privateData.crect.height()));
         moveRect(QRect(oldPos, minSize), offset.x(), offset.y());
      } else {
         moveRect(QRect(oldPos, oldSize), offset.x(), offset.y());
      }
   }

   // Invalidate newly visible area of the widget.
   if (!sizeDecreased || !oldWidgetRect.contains(newWidgetRect)) {
      QRegion newVisible(newWidgetRect);
      newVisible -= oldWidgetRect;
      invalidateBuffer(newVisible);
   }

   if (!parentAreaExposed) {
      return;
   }

   // Invalidate newly exposed area of the parent.
   const QRect oldRect(oldPos, oldSize);
   if (extra && extra->hasMask) {
      QRegion parentExpose(oldRect);
      parentExpose &= extra->mask.translated(oldPos);
      parentExpose -= (extra->mask.translated(m_privateData.crect.topLeft()) & m_privateData.crect);
      q->parentWidget()->d_func()->invalidateBuffer(parentExpose);
   } else {
      QRegion parentExpose(oldRect);
      parentExpose -= m_privateData.crect;
      q->parentWidget()->d_func()->invalidateBuffer(parentExpose);
   }
}

/*!
    Invalidates the \a rgn (in widget's coordinates) of the backing store, i.e.
    all widgets intersecting with the region will be repainted when the backing store
    is synced.

    ### Merge into a template function (after MSVC isn't supported anymore).
*/
void QWidgetPrivate::invalidateBuffer(const QRegion &rgn)
{
   Q_Q(QWidget);

   QTLWExtra *tlwExtra = q->window()->d_func()->maybeTopData();
   if (discardInvalidateBufferRequest(q, tlwExtra) || rgn.isEmpty()) {
      return;
   }

   QRegion wrgn(rgn);
   wrgn &= clipRect();
   if (!graphicsEffect && extra && extra->hasMask) {
      wrgn &= extra->mask;
   }
   if (wrgn.isEmpty()) {
      return;
   }

   tlwExtra->backingStoreTracker->markDirty(wrgn, q,
      QWidgetBackingStore::UpdateLater, QWidgetBackingStore::BufferInvalid);
}

/*!
    This function is equivalent to calling invalidateBuffer(QRegion(rect), ...), but
    is more efficient as it eliminates QRegion operations/allocations and can
    use the rect more precisely for additional cut-offs.

    ### Merge into a template function (after MSVC isn't supported anymore).
*/
void QWidgetPrivate::invalidateBuffer(const QRect &rect)
{
   Q_Q(QWidget);

   QTLWExtra *tlwExtra = q->window()->d_func()->maybeTopData();
   if (discardInvalidateBufferRequest(q, tlwExtra) || rect.isEmpty()) {
      return;
   }

   QRect wRect(rect);
   wRect &= clipRect();
   if (wRect.isEmpty()) {
      return;
   }

   if (graphicsEffect || !extra || !extra->hasMask) {
      tlwExtra->backingStoreTracker->markDirty(wRect, q,
         QWidgetBackingStore::UpdateLater, QWidgetBackingStore::BufferInvalid);
      return;
   }

   QRegion wRgn(extra->mask);
   wRgn &= wRect;

   if (wRgn.isEmpty()) {
      return;
   }

   tlwExtra->backingStoreTracker->markDirty(wRgn, q,
      QWidgetBackingStore::UpdateLater, QWidgetBackingStore::BufferInvalid);
}

void QWidgetPrivate::repaint_sys(const QRegion &rgn)
{
   if (m_privateData.in_destructor) {
      return;
   }

   Q_Q(QWidget);

   if (discardSyncRequest(q, maybeTopData())) {
      return;
   }

   if (q->testAttribute(Qt::WA_StaticContents)) {
      if (!extra) {
         createExtra();
      }
      extra->staticContentsSize = m_privateData.crect.size();
   }

   QPaintEngine *engine = q->paintEngine();

   // QGLWidget does not support partial updates if:
   // 1) The context is double buffered
   // 2) The context is single buffered and auto-fill background is enabled.

   const bool noPartialUpdateSupport = (engine && (engine->type() == QPaintEngine::OpenGL
         || engine->type() == QPaintEngine::OpenGL2))
         && (usesDoubleBufferedGLContext || q->autoFillBackground());

   QRegion toBePainted(noPartialUpdateSupport ? q->rect() : rgn);

   toBePainted &= clipRect();
   clipToEffectiveMask(toBePainted);

   if (toBePainted.isEmpty()) {
      return;   // Nothing to repaint
   }

   drawWidget(q, toBePainted, QPoint(), QWidgetPrivate::DrawAsRoot | QWidgetPrivate::DrawPaintOnScreen, nullptr);

   if (q->paintingActive()) {
      qWarning("QWidget::repaint() Paint process should not be active after leaving the paint event");
   }
}
