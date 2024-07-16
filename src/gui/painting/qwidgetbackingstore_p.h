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

#ifndef QWIDGET_BACKINGSTORE_P_H
#define QWIDGET_BACKINGSTORE_P_H

#include <qdebug.h>
#include <qwidget.h>

#include <qwidget_p.h>
#include <qbackingstore.h>

class QPlatformTextureList;
class QPlatformTextureListWatcher;
class QWidgetBackingStore;

struct BeginPaintInfo {
   BeginPaintInfo()
      : wasFlushed(0), nothingToPaint(0), backingStoreRecreated(0)
   { }

   uint wasFlushed : 1;
   uint nothingToPaint : 1;
   uint backingStoreRecreated : 1;
};

#ifndef QT_NO_OPENGL
class QPlatformTextureListWatcher : public QObject
{
   GUI_CS_OBJECT(QPlatformTextureListWatcher)

 public:
   QPlatformTextureListWatcher(QWidgetBackingStore *backingStore);
   void watch(QPlatformTextureList *textureList);
   bool isLocked() const;

 private:
   GUI_CS_SLOT_1(Private, void onLockStatusChanged(bool locked))
   GUI_CS_SLOT_2(onLockStatusChanged)

   QHash<QPlatformTextureList *, bool> m_locked;
   QWidgetBackingStore *m_backingStore;
};
#endif

class QWidgetBackingStore
{
 public:
   enum UpdateTime {
      UpdateNow,
      UpdateLater
   };

   enum BufferState {
      BufferValid,
      BufferInvalid
   };

   QWidgetBackingStore(QWidget *t);

   QWidgetBackingStore(const QWidgetBackingStore &) = delete;
   QWidgetBackingStore &operator=(const QWidgetBackingStore &) = delete;

   ~QWidgetBackingStore();

   void sync(QWidget *exposedWidget, const QRegion &exposedRegion);
   void sync();
   void flush(QWidget *widget = nullptr);

   QPoint topLevelOffset() const {
      return tlwOffset;
   }

   QBackingStore *backingStore() const {
      return store;
   }

   bool isDirty() const {
      return ! (dirtyWidgets.isEmpty() && dirty.isEmpty() && !fullUpdatePending && dirtyRenderToTextureWidgets.isEmpty());
   }

   // ### merge into a template function
   void markDirty(const QRegion &rgn, QWidget *widget, UpdateTime updateTime = UpdateLater,
      BufferState bufferState = BufferValid);

   void markDirty(const QRect &rect, QWidget *widget, UpdateTime updateTime = UpdateLater,
      BufferState bufferState = BufferValid);

 private:
   QWidget *tlw;
   QRegion dirtyOnScreen;       // needsFlush
   QRegion dirty;               // needsRepaint
   QRegion dirtyFromPreviousSync;

   QVector<QWidget *> dirtyWidgets;
   QVector<QWidget *> dirtyRenderToTextureWidgets;
   QVector<QWidget *> *dirtyOnScreenWidgets;
   QList<QWidget *> staticWidgets;

   QBackingStore *store;

   uint fullUpdatePending : 1;
   uint updateRequestSent : 1;

   QPoint tlwOffset;

   QPlatformTextureListWatcher *textureListWatcher;
   QElapsedTimer perfTime;
   int perfFrames;

   void sendUpdateRequest(QWidget *widget, UpdateTime updateTime);

   static void qt_flush(QWidget *widget, const QRegion &region, QBackingStore *backingStore,
      QWidget *tlw, const QPoint &tlwOffset, QPlatformTextureList *widgetTextures,
      QWidgetBackingStore *widgetBackingStore);

   void doSync();
   bool bltRect(const QRect &rect, int dx, int dy, QWidget *widget);
   void releaseBuffer();

   void beginPaint(QRegion &toClean, QWidget *widget, QBackingStore *backingStore,
      BeginPaintInfo *returnInfo, bool toCleanIsInTopLevelCoordinates = true);

   void endPaint(const QRegion &cleaned, QBackingStore *backingStore, BeginPaintInfo *beginPaintInfo);

   QRegion dirtyRegion(QWidget *widget = nullptr) const;
   QRegion staticContents(QWidget *widget = nullptr, const QRect &withinClipRect = QRect()) const;

   void markDirtyOnScreen(const QRegion &dirtyOnScreen, QWidget *widget, const QPoint &topLevelOffset);

   void removeDirtyWidget(QWidget *w);

   void updateLists(QWidget *widget);

   bool syncAllowed();

   void addDirtyWidget(QWidget *widget, const QRegion &rgn) {
      if (widget && ! widget->d_func()->inDirtyList && ! widget->m_widgetData->in_destructor) {
         QWidgetPrivate *widgetPrivate = widget->d_func();

#ifndef QT_NO_GRAPHICSEFFECT
         if (widgetPrivate->graphicsEffect) {
            widgetPrivate->dirty = widgetPrivate->effectiveRectFor(rgn.boundingRect());

         } else
#endif
         {
            widgetPrivate->dirty = rgn;
            dirtyWidgets.append(widget);
            widgetPrivate->inDirtyList = true;
         }
      }
   }

   void addDirtyRenderToTextureWidget(QWidget *widget) {
      if (widget && ! widget->d_func()->inDirtyList && ! widget->m_widgetData->in_destructor) {
         QWidgetPrivate *widgetPrivate = widget->d_func();
         Q_ASSERT(widgetPrivate->renderToTexture);
         dirtyRenderToTextureWidgets.append(widget);
         widgetPrivate->inDirtyList = true;
      }
   }

   void dirtyWidgetsRemoveAll(QWidget *widget) {
      int i = 0;

      while (i < dirtyWidgets.size()) {
         if (dirtyWidgets.at(i) == widget) {
            dirtyWidgets.remove(i);
         } else {
            ++i;
         }
      }
   }

   void addStaticWidget(QWidget *widget) {
      if (! widget) {
         return;
      }

      Q_ASSERT(widget->testAttribute(Qt::WA_StaticContents));
      if (!staticWidgets.contains(widget)) {
         staticWidgets.append(widget);
      }
   }

   inline void removeStaticWidget(QWidget *widget) {
      staticWidgets.removeAll(widget);
   }

   // Move the reparented widget and all its static children from this backing store
   // to the new backing store if reparented into another top-level / backing store.
   inline void moveStaticWidgets(QWidget *reparented) {
      Q_ASSERT(reparented);
      QWidgetBackingStore *newBs = reparented->d_func()->maybeBackingStore();
      if (newBs == this) {
         return;
      }

      int i = 0;
      while (i < staticWidgets.size()) {
         QWidget *w = staticWidgets.at(i);
         if (reparented == w || reparented->isAncestorOf(w)) {
            staticWidgets.removeAt(i);
            if (newBs) {
               newBs->addStaticWidget(w);
            }
         } else {
            ++i;
         }
      }
   }

   QRect topLevelRect() const {
      return tlw->m_widgetData->crect;
   }

   void appendDirtyOnScreenWidget(QWidget *widget) {
      if (! widget) {
         return;
      }

      if (!dirtyOnScreenWidgets) {
         dirtyOnScreenWidgets = new QVector<QWidget *>;
         dirtyOnScreenWidgets->append(widget);
      } else if (!dirtyOnScreenWidgets->contains(widget)) {
         dirtyOnScreenWidgets->append(widget);
      }
   }

   void dirtyOnScreenWidgetsRemoveAll(QWidget *widget) {
      if (! widget || !dirtyOnScreenWidgets) {
         return;
      }

      int i = 0;
      while (i < dirtyOnScreenWidgets->size()) {
         if (dirtyOnScreenWidgets->at(i) == widget) {
            dirtyOnScreenWidgets->remove(i);
         } else {
            ++i;
         }
      }
   }

   void resetWidget(QWidget *widget) {
      if (widget) {
         widget->d_func()->inDirtyList = false;
         widget->d_func()->isScrolled = false;
         widget->d_func()->isMoved = false;
         widget->d_func()->dirty = QRegion();
      }
   }

   void updateStaticContentsSize() {
      for (int i = 0; i < staticWidgets.size(); ++i) {
         QWidgetPrivate *wd = staticWidgets.at(i)->d_func();
         if (!wd->extra) {
            wd->createExtra();
         }
         wd->extra->staticContentsSize = wd->m_privateData.crect.size();
      }
   }

   bool hasStaticContents() const {
#if defined(Q_OS_WIN)
      return !staticWidgets.isEmpty();
#else
      return !staticWidgets.isEmpty() && false;
#endif
   }

   friend QRegion qt_dirtyRegion(QWidget *);
   friend class QWidgetPrivate;
   friend class QWidget;
   friend class QBackingStore;
};

#endif
