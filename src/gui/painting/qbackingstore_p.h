/***********************************************************************
*
* Copyright (c) 2012-2017 Barbara Geller
* Copyright (c) 2012-2017 Ansel Sermersheim
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
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
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#ifndef QBACKINGSTORE_P_H
#define QBACKINGSTORE_P_H

#include <QDebug>
#include <QtGui/qwidget.h>
#include <qwidget_p.h>
#include <qwindowsurface_p.h>

#ifdef Q_WS_QWS
#include <qwindowsurface_qws_p.h>
#endif

QT_BEGIN_NAMESPACE

class QWindowSurface;

struct BeginPaintInfo {
   inline BeginPaintInfo() : wasFlushed(0), nothingToPaint(0), windowSurfaceRecreated(0) {}
   uint wasFlushed : 1;
   uint nothingToPaint : 1;
   uint windowSurfaceRecreated : 1;
};

class QWidgetBackingStore
{
 public:
   QWidgetBackingStore(QWidget *t);
   ~QWidgetBackingStore();

   void sync(QWidget *exposedWidget, const QRegion &exposedRegion);
   void sync();
   void flush(QWidget *widget = 0, QWindowSurface *surface = 0);

   inline QPoint topLevelOffset() const {
      return tlwOffset;
   }

   QWindowSurface *surface() const {
      return windowSurface;
   }

   inline bool isDirty() const {

#if defined(Q_WS_QWS) && ! defined(QT_NO_QWS_MANAGER)
      return ! (dirtyWidgets.isEmpty() && dirty.isEmpty() && ! hasDirtyFromPreviousSync && ! fullUpdatePending
                  && ! hasDirtyWindowDecoration());
#else
      return ! (dirtyWidgets.isEmpty() && dirty.isEmpty() && ! hasDirtyFromPreviousSync && ! fullUpdatePending);

#endif

   }

   // ### Qt 4.6: Merge into a template function (after MSVC isn't supported anymore).
   void markDirty(const QRegion &rgn, QWidget *widget, bool updateImmediately = false,
                  bool invalidateBuffer = false);

   void markDirty(const QRect &rect, QWidget *widget, bool updateImmediately = false,
                  bool invalidateBuffer = false);

 private:
   QWidget *tlw;
   QRegion dirtyOnScreen; // needsFlush
   QRegion dirty; // needsRepaint
   QRegion dirtyFromPreviousSync;
   QVector<QWidget *> dirtyWidgets;
   QVector<QWidget *> *dirtyOnScreenWidgets;
   QList<QWidget *> staticWidgets;
   QWindowSurface *windowSurface;

#ifdef Q_BACKINGSTORE_SUBSURFACES
   QList<QWindowSurface *> subSurfaces;
#endif

   uint hasDirtyFromPreviousSync : 1;
   uint fullUpdatePending : 1;

   QPoint tlwOffset;

   bool bltRect(const QRect &rect, int dx, int dy, QWidget *widget);
   void releaseBuffer();

   void beginPaint(QRegion &toClean, QWidget *widget, QWindowSurface *windowSurface,
                   BeginPaintInfo *returnInfo, bool toCleanIsInTopLevelCoordinates = true);

   void endPaint(const QRegion &cleaned, QWindowSurface *windowSurface, BeginPaintInfo *beginPaintInfo);

   QRegion dirtyRegion(QWidget *widget = 0) const;
   QRegion staticContents(QWidget *widget = 0, const QRect &withinClipRect = QRect()) const;

   void markDirtyOnScreen(const QRegion &dirtyOnScreen, QWidget *widget, const QPoint &topLevelOffset);

   void removeDirtyWidget(QWidget *w);

#if defined(Q_WS_QWS) && !defined(QT_NO_QWS_MANAGER)
   bool hasDirtyWindowDecoration() const;
   void paintWindowDecoration();
#endif

   void updateLists(QWidget *widget);

   inline void addDirtyWidget(QWidget *widget, const QRegion &rgn) {
      if (widget && !widget->d_func()->inDirtyList && !widget->data->in_destructor) {
         QWidgetPrivate *widgetPrivate = widget->d_func();

#ifndef QT_NO_GRAPHICSEFFECT
         if (widgetPrivate->graphicsEffect) {
            widgetPrivate->dirty = widgetPrivate->effectiveRectFor(rgn.boundingRect());

         } else
#endif
         {
            widgetPrivate->dirty = rgn;
         }

         dirtyWidgets.append(widget);
         widgetPrivate->inDirtyList = true;
      }
   }

   inline void dirtyWidgetsRemoveAll(QWidget *widget) {
      int i = 0;

      while (i < dirtyWidgets.size()) {
         if (dirtyWidgets.at(i) == widget) {
            dirtyWidgets.remove(i);
         } else {
            ++i;
         }
      }
   }

   inline void addStaticWidget(QWidget *widget) {
      if (!widget) {
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

   inline QRect topLevelRect() const {
#ifdef Q_WS_QWS
      return tlw->frameGeometry();
#else
      return tlw->data->crect;
#endif
   }

   inline void appendDirtyOnScreenWidget(QWidget *widget) {
      if (!widget) {
         return;
      }

      if (!dirtyOnScreenWidgets) {
         dirtyOnScreenWidgets = new QVector<QWidget *>;
         dirtyOnScreenWidgets->append(widget);
      } else if (!dirtyOnScreenWidgets->contains(widget)) {
         dirtyOnScreenWidgets->append(widget);
      }
   }

   inline void dirtyOnScreenWidgetsRemoveAll(QWidget *widget) {
      if (!widget || !dirtyOnScreenWidgets) {
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

   inline void resetWidget(QWidget *widget) {
      if (widget) {
         widget->d_func()->inDirtyList = false;
         widget->d_func()->isScrolled = false;
         widget->d_func()->isMoved = false;
         widget->d_func()->dirty = QRegion();
      }
   }

   inline void updateStaticContentsSize() {
      for (int i = 0; i < staticWidgets.size(); ++i) {
         QWidgetPrivate *wd = staticWidgets.at(i)->d_func();
         if (!wd->extra) {
            wd->createExtra();
         }
         wd->extra->staticContentsSize = wd->data.crect.size();
      }
   }

   inline bool hasStaticContents() const {
      return !staticWidgets.isEmpty() && windowSurface->hasFeature(QWindowSurface::StaticContents);
   }

   friend QRegion qt_dirtyRegion(QWidget *);
   friend class QWidgetPrivate;
   friend class QWidget;
   friend class QWSManagerPrivate;
   friend class QETWidget;
   friend class QWindowSurface;
   friend class QWSWindowSurface;
};

QT_END_NAMESPACE

#endif // QBACKINGSTORE_P_H
