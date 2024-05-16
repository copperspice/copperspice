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

#include <qsplitter.h>

#ifndef QT_NO_SPLITTER

#include <qapplication.h>
#include <qcursor.h>
#include <qdrawutil.h>
#include <qevent.h>
#include <qlayout.h>
#include <qlist.h>
#include <qpainter.h>
#include <qrubberband.h>
#include <qstyle.h>
#include <qstyleoption.h>
#include <qtextstream.h>
#include <qvarlengtharray.h>
#include <qvector.h>
#include <qtimer.h>
#include <qdebug.h>

#include <qlayoutengine_p.h>
#include <qsplitter_p.h>

#include <ctype.h>

QSplitterPrivate::~QSplitterPrivate()
{
}

QSplitterHandle::QSplitterHandle(Qt::Orientation orientation, QSplitter *parent)
   : QWidget(*new QSplitterHandlePrivate, parent, Qt::EmptyFlag)
{
   Q_D(QSplitterHandle);
   d->s = parent;
   setOrientation(orientation);
}

QSplitterHandle::~QSplitterHandle()
{
}

void QSplitterHandle::setOrientation(Qt::Orientation orientation)
{
   Q_D(QSplitterHandle);
   d->orient = orientation;

#ifndef QT_NO_CURSOR
   setCursor(orientation == Qt::Horizontal ? Qt::SplitHCursor : Qt::SplitVCursor);
#endif

}

Qt::Orientation QSplitterHandle::orientation() const
{
   Q_D(const QSplitterHandle);
   return d->orient;
}

bool QSplitterHandle::opaqueResize() const
{
   Q_D(const QSplitterHandle);
   return d->s->opaqueResize();
}


QSplitter *QSplitterHandle::splitter() const
{
   return d_func()->s;
}

void QSplitterHandle::moveSplitter(int pos)
{
   Q_D(QSplitterHandle);
   if (d->s->isRightToLeft() && d->orient == Qt::Horizontal) {
      pos = d->s->contentsRect().width() - pos;
   }
   d->s->moveSplitter(pos, d->s->indexOf(this));
}

int QSplitterHandle::closestLegalPosition(int pos)
{
   Q_D(QSplitterHandle);
   QSplitter *s = d->s;

   if (s->isRightToLeft() && d->orient == Qt::Horizontal) {
      int w = s->contentsRect().width();
      return w - s->closestLegalPosition(w - pos, s->indexOf(this));
   }

   return s->closestLegalPosition(pos, s->indexOf(this));
}

QSize QSplitterHandle::sizeHint() const
{
   Q_D(const QSplitterHandle);
   int hw = d->s->handleWidth();
   QStyleOption opt(0);
   opt.initFrom(d->s);

   opt.state = QStyle::State_None;

   return parentWidget()->style()->sizeFromContents(QStyle::CT_Splitter, &opt, QSize(hw, hw), d->s)
      .expandedTo(QApplication::globalStrut());
}

void QSplitterHandle::resizeEvent(QResizeEvent *event)
{
   Q_D(const QSplitterHandle);

   // When splitters are only 1 pixel large we increase the
   // actual grab area to five pixels

   // Note that QSplitter uses contentsRect for layouting
   // and ensures that handles are drawn on top of widgets
   // We simply use the contents margins for draggin and only
   // paint the mask area

   bool useTinyMode = (d->s->handleWidth() <= 1);
   setAttribute(Qt::WA_MouseNoMask, useTinyMode);

   if (useTinyMode) {
      if (orientation() == Qt::Horizontal) {
         setContentsMargins(2, 0, 2, 0);
      } else {
         setContentsMargins(0, 2, 0, 2);
      }
      setMask(QRegion(contentsRect()));
   }

   QWidget::resizeEvent(event);
}

bool QSplitterHandle::event(QEvent *event)
{
   Q_D(QSplitterHandle);

   switch (event->type()) {
      case QEvent::HoverEnter:
         d->hover = true;
         update();
         break;

      case QEvent::HoverLeave:
         d->hover = false;
         update();
         break;

      default:
         break;
   }
   return QWidget::event(event);
}

void QSplitterHandle::mouseMoveEvent(QMouseEvent *e)
{
   Q_D(QSplitterHandle);

   if (!(e->buttons() & Qt::LeftButton)) {
      return;
   }

   int pos = d->pick(parentWidget()->mapFromGlobal(e->globalPos()))
      - d->mouseOffset;

   if (opaqueResize()) {
      moveSplitter(pos);
   } else {
      d->s->setRubberBand(closestLegalPosition(pos));
   }
}

void QSplitterHandle::mousePressEvent(QMouseEvent *e)
{
   Q_D(QSplitterHandle);

   if (e->button() == Qt::LeftButton) {
      d->mouseOffset = d->pick(e->pos());
      d->pressed = true;
      update();
   }
}

void QSplitterHandle::mouseReleaseEvent(QMouseEvent *e)
{
   Q_D(QSplitterHandle);

   if (!opaqueResize() && e->button() == Qt::LeftButton) {
      int pos = d->pick(parentWidget()->mapFromGlobal(e->globalPos()))
         - d->mouseOffset;
      d->s->setRubberBand(-1);
      moveSplitter(pos);
   }

   if (e->button() == Qt::LeftButton) {
      d->pressed = false;
      update();
   }
}

void QSplitterHandle::paintEvent(QPaintEvent *)
{
   Q_D(QSplitterHandle);
   QPainter p(this);

   QStyleOption opt(0);
   opt.rect = contentsRect();
   opt.palette = palette();

   if (orientation() == Qt::Horizontal) {
      opt.state = QStyle::State_Horizontal;
   } else {
      opt.state = QStyle::State_None;
   }

   if (d->hover) {
      opt.state |= QStyle::State_MouseOver;
   }

   if (d->pressed) {
      opt.state |= QStyle::State_Sunken;
   }

   if (isEnabled()) {
      opt.state |= QStyle::State_Enabled;
   }

   parentWidget()->style()->drawControl(QStyle::CE_Splitter, &opt, &p, d->s);
}

int QSplitterLayoutStruct::getWidgetSize(Qt::Orientation orient)
{
   if (sizer == -1) {
      QSize s = widget->sizeHint();
      const int presizer = pick(s, orient);
      const int realsize = pick(widget->size(), orient);
      if (!s.isValid() || (widget->testAttribute(Qt::WA_Resized) && (realsize > presizer))) {
         sizer = pick(widget->size(), orient);
      } else {
         sizer = presizer;
      }
      QSizePolicy p = widget->sizePolicy();
      int sf = (orient == Qt::Horizontal) ? p.horizontalStretch() : p.verticalStretch();
      if (sf > 1) {
         sizer *= sf;
      }
   }

   return sizer;
}

int QSplitterLayoutStruct::getHandleSize(Qt::Orientation orient)
{
   return pick(handle->sizeHint(), orient);
}

void QSplitterPrivate::init()
{
   Q_Q(QSplitter);
   QSizePolicy sp(QSizePolicy::Expanding, QSizePolicy::Preferred);
   if (orient == Qt::Vertical) {
      sp.transpose();
   }
   q->setSizePolicy(sp);
   q->setAttribute(Qt::WA_WState_OwnSizePolicy, false);
}

void QSplitterPrivate::recalc(bool update)
{
   Q_Q(QSplitter);

   int n = list.count();

   /*
     Splitter handles before the first visible widget or right
     before a hidden widget must be hidden.
   */

   bool first = true;
   bool allInvisible = n != 0;

   for (int i = 0; i < n ; ++i) {
      QSplitterLayoutStruct *s = list.at(i);
      bool widgetHidden = s->widget->isHidden();
      if (allInvisible && ! widgetHidden && ! s->collapsed) {
         allInvisible = false;
      }

      s->handle->setHidden(first || widgetHidden);
      if (!widgetHidden) {
         first = false;
      }
   }

   if (allInvisible)
      for (int i = 0; i < n ; ++i) {
         QSplitterLayoutStruct *s = list.at(i);
         if (!s->widget->isHidden()) {
            s->collapsed = false;
            break;
         }
      }

   int fi = 2 * q->frameWidth();
   int maxl = fi;
   int minl = fi;
   int maxt = QWIDGETSIZE_MAX;
   int mint = fi;

   // calculate min/max sizes for the whole splitter
   bool empty = true;
   for (int j = 0; j < n; j++) {
      QSplitterLayoutStruct *s = list.at(j);

      if (! s->widget->isHidden()) {
         empty = false;

         if (!s->handle->isHidden()) {
            minl += s->getHandleSize(orient);
            maxl += s->getHandleSize(orient);
         }

         QSize minS = qSmartMinSize(s->widget);
         minl += pick(minS);
         maxl += pick(s->widget->maximumSize());
         mint = qMax(mint, trans(minS));
         int tm = trans(s->widget->maximumSize());

         if (tm > 0) {
            maxt = qMin(maxt, tm);
         }
      }
   }

   if (empty) {
      if (qobject_cast<QSplitter *>(q->parent())) {
         // nested splitters
         maxl = maxt = 0;

      } else {
         // QSplitter with no children yet
         maxl = QWIDGETSIZE_MAX;
      }

   } else {
      maxl = qMin(maxl, QWIDGETSIZE_MAX);
   }

   if (maxt < mint) {
      maxt = mint;
   }

   if (update) {
      if (orient == Qt::Horizontal) {
         q->setMaximumSize(maxl, maxt);
         if (q->isWindow()) {
            q->setMinimumSize(minl, mint);
         }
      } else {
         q->setMaximumSize(maxt, maxl);
         if (q->isWindow()) {
            q->setMinimumSize(mint, minl);
         }
      }
      doResize();
      q->updateGeometry();
   } else {
      firstShow = true;
   }
}

void QSplitterPrivate::doResize()
{
   Q_Q(QSplitter);
   QRect r = q->contentsRect();
   int n = list.count();
   QVector<QLayoutStruct> a(n * 2);
   int i;

   bool noStretchFactorsSet = true;
   for (i = 0; i < n; ++i) {
      QSizePolicy p = list.at(i)->widget->sizePolicy();
      int sf = orient == Qt::Horizontal ? p.horizontalStretch() : p.verticalStretch();
      if (sf != 0) {
         noStretchFactorsSet = false;
         break;
      }
   }

   int j = 0;
   for (i = 0; i < n; ++i) {
      QSplitterLayoutStruct *s = list.at(i);

#if defined(CS_SHOW_DEBUG_GUI_WIDGETS)
      qDebug("widget %d hidden: %d collapsed: %d handle hidden: %d", i, s->widget->isHidden(),
         s->collapsed, s->handle->isHidden());
#endif

      a[j].init();
      if (s->handle->isHidden()) {
         a[j].maximumSize = 0;
      } else {
         a[j].sizeHint = a[j].minimumSize = a[j].maximumSize = s->getHandleSize(orient);
         a[j].empty = false;
      }
      ++j;

      a[j].init();
      if (s->widget->isHidden() || s->collapsed) {
         a[j].maximumSize = 0;
      } else {
         a[j].minimumSize = pick(qSmartMinSize(s->widget));
         a[j].maximumSize = pick(s->widget->maximumSize());
         a[j].empty = false;

         bool stretch = noStretchFactorsSet;
         if (!stretch) {
            QSizePolicy p = s->widget->sizePolicy();
            int sf = orient == Qt::Horizontal ? p.horizontalStretch() : p.verticalStretch();
            stretch = (sf != 0);
         }
         if (stretch) {
            a[j].stretch = s->getWidgetSize(orient);
            a[j].sizeHint = a[j].minimumSize;
            a[j].expansive = true;
         } else {
            a[j].sizeHint = qMax(s->getWidgetSize(orient), a[j].minimumSize);
         }
      }
      ++j;
   }

   qGeomCalc(a, 0, n * 2, pick(r.topLeft()), pick(r.size()), 0);

#if defined(CS_SHOW_DEBUG_GUI_WIDGETS)
   for (i = 0; i < n * 2; ++i) {
      qDebug("%*s%d: stretch %d, sh %d, minS %d, maxS %d, exp %d, emp %d -> %d, %d",
         i, "", i,
         a[i].stretch,
         a[i].sizeHint,
         a[i].minimumSize,
         a[i].maximumSize,
         a[i].expansive,
         a[i].empty,
         a[i].pos,
         a[i].size);
   }
#endif

   for (i = 0; i < n; ++i) {
      QSplitterLayoutStruct *s = list.at(i);
      setGeo(s, a[i * 2 + 1].pos, a[i * 2 + 1].size, false);
   }
}

void QSplitterPrivate::storeSizes()
{
   for (int i = 0; i < list.size(); ++i) {
      QSplitterLayoutStruct *sls = list.at(i);
      sls->sizer = pick(sls->rect.size());
   }
}

void QSplitterPrivate::addContribution(int index, int *min, int *max, bool mayCollapse) const
{
   QSplitterLayoutStruct *s = list.at(index);
   if (!s->widget->isHidden()) {
      if (!s->handle->isHidden()) {
         *min += s->getHandleSize(orient);
         *max += s->getHandleSize(orient);
      }
      if (mayCollapse || ! s->collapsed) {
         *min += pick(qSmartMinSize(s->widget));
      }

      *max += pick(s->widget->maximumSize());
   }
}

int QSplitterPrivate::findWidgetJustBeforeOrJustAfter(int index, int delta, int &collapsibleSize) const
{
   if (delta < 0) {
      index += delta;
   }

   do {
      QWidget *w = list.at(index)->widget;

      if (!w->isHidden()) {
         if (collapsible(list.at(index))) {
            collapsibleSize = pick(qSmartMinSize(w));
         }
         return index;
      }

      index += delta;

   } while (index >= 0 && index < list.count());

   return -1;
}

void QSplitterPrivate::getRange(int index, int *farMin, int *min, int *max, int *farMax) const
{
   Q_Q(const QSplitter);

   int n = list.count();
   if (index <= 0 || index >= n) {
      return;
   }

   int collapsibleSizeBefore = 0;
   int idJustBefore = findWidgetJustBeforeOrJustAfter(index, -1, collapsibleSizeBefore);

   int collapsibleSizeAfter = 0;
   int idJustAfter = findWidgetJustBeforeOrJustAfter(index, +1, collapsibleSizeAfter);

   int minBefore = 0;
   int minAfter = 0;
   int maxBefore = 0;
   int maxAfter = 0;
   int i;

   for (i = 0; i < index; ++i) {
      addContribution(i, &minBefore, &maxBefore, i == idJustBefore);
   }
   for (i = index; i < n; ++i) {
      addContribution(i, &minAfter, &maxAfter, i == idJustAfter);
   }

   QRect r = q->contentsRect();
   int farMinVal;
   int minVal;
   int maxVal;
   int farMaxVal;

   int smartMinBefore = qMax(minBefore, pick(r.size()) - maxAfter);
   int smartMaxBefore = qMin(maxBefore, pick(r.size()) - minAfter);

   minVal = pick(r.topLeft()) + smartMinBefore;
   maxVal = pick(r.topLeft()) + smartMaxBefore;

   farMinVal = minVal;
   if (minBefore - collapsibleSizeBefore >= pick(r.size()) - maxAfter) {
      farMinVal -= collapsibleSizeBefore;
   }
   farMaxVal = maxVal;
   if (pick(r.size()) - (minAfter - collapsibleSizeAfter) <= maxBefore) {
      farMaxVal += collapsibleSizeAfter;
   }

   if (farMin) {
      *farMin = farMinVal;
   }
   if (min) {
      *min = minVal;
   }
   if (max) {
      *max = maxVal;
   }
   if (farMax) {
      *farMax = farMaxVal;
   }
}

int QSplitterPrivate::adjustPos(int pos, int index, int *farMin, int *min, int *max, int *farMax) const
{
   const int Threshold = 40;

   getRange(index, farMin, min, max, farMax);

   if (pos >= *min) {
      if (pos <= *max) {
         return pos;
      } else {
         int delta = pos - *max;
         int width = *farMax - *max;

         if (delta > width / 2 && delta >= qMin(Threshold, width)) {
            return *farMax;
         } else {
            return *max;
         }
      }
   } else {
      int delta = *min - pos;
      int width = *min - *farMin;

      if (delta > width / 2 && delta >= qMin(Threshold, width)) {
         return *farMin;
      } else {
         return *min;
      }
   }
}

bool QSplitterPrivate::collapsible(QSplitterLayoutStruct *s) const
{
   if (s->collapsible != Default) {
      return (bool)s->collapsible;
   } else {
      return childrenCollapsible;
   }
}

void QSplitterPrivate::updateHandles()
{
   Q_Q(QSplitter);
   recalc(q->isVisible());
}

void QSplitterPrivate::setSizes_helper(const QList<int> &sizes, bool clampNegativeSize)
{
   int j = 0;

   for (int i = 0; i < list.size(); ++i) {
      QSplitterLayoutStruct *s = list.at(i);

      s->collapsed = false;
      s->sizer = sizes.value(j++);
      if (clampNegativeSize && s->sizer < 0) {
         s->sizer = 0;
      }
      int smartMinSize = pick(qSmartMinSize(s->widget));

      // Make sure that we reset the collapsed state.
      if (s->sizer == 0) {
         if (collapsible(s) && smartMinSize > 0) {
            s->collapsed = true;
         } else {
            s->sizer = smartMinSize;
         }
      } else {
         if (s->sizer < smartMinSize) {
            s->sizer = smartMinSize;
         }
      }
   }

   doResize();
}

void QSplitterPrivate::setGeo(QSplitterLayoutStruct *sls, int p, int s, bool allowCollapse)
{
   Q_Q(QSplitter);
   QWidget *w = sls->widget;
   QRect r;
   QRect contents = q->contentsRect();

   if (orient == Qt::Horizontal) {
      r.setRect(p, contents.y(), s, contents.height());
   } else {
      r.setRect(contents.x(), p, contents.width(), s);
   }
   sls->rect = r;

   int minSize = pick(qSmartMinSize(w));

   if (orient == Qt::Horizontal && q->isRightToLeft()) {
      r.moveRight(contents.width() - r.left());
   }

   if (allowCollapse) {
      sls->collapsed = s <= 0 && minSize > 0 && !w->isHidden();
   }

   //   Hide the child widget, but without calling hide() so that
   //   the splitter handle is still shown.
   if (sls->collapsed) {
      r.moveTopLeft(QPoint(-r.width() - 1, -r.height() - 1));
   }

   w->setGeometry(r);

   if (!sls->handle->isHidden()) {
      QSplitterHandle *h = sls->handle;
      QSize hs = h->sizeHint();
      int left, top, right, bottom;
      h->getContentsMargins(&left, &top, &right, &bottom);
      if (orient == Qt::Horizontal) {
         if (q->isRightToLeft()) {
            p = contents.width() - p + hs.width();
         }
         h->setGeometry(p - hs.width() - left, contents.y(), hs.width() + left + right, contents.height());
      } else {
         h->setGeometry(contents.x(), p - hs.height() - top, contents.width(), hs.height() + top + bottom);
      }
   }
}

void QSplitterPrivate::doMove(bool backwards, int hPos, int index, int delta, bool mayCollapse,
   int *positions, int *widths)
{
   if (index < 0 || index >= list.count()) {
      return;
   }

#if defined(CS_SHOW_DEBUG_GUI_WIDGETS)
   qDebug() << "QSplitterPrivate::doMove" << backwards << hPos << index << delta << mayCollapse;
#endif

   QSplitterLayoutStruct *s = list.at(index);
   QWidget *w = s->widget;

   int nextId = backwards ? index - delta : index + delta;

   if (w->isHidden()) {
      doMove(backwards, hPos, nextId, delta, collapsible(nextId), positions, widths);
   } else {
      int hs = s->handle->isHidden() ? 0 : s->getHandleSize(orient);

      int  ws = backwards ? hPos - pick(s->rect.topLeft())
         : pick(s->rect.bottomRight()) - hPos - hs + 1;
      if (ws > 0 || (!s->collapsed && !mayCollapse)) {
         ws = qMin(ws, pick(w->maximumSize()));
         ws = qMax(ws, pick(qSmartMinSize(w)));
      } else {
         ws = 0;
      }
      positions[index] = backwards ? hPos - ws : hPos + hs;
      widths[index] = ws;
      doMove(backwards, backwards ? hPos - ws - hs : hPos + hs + ws, nextId, delta,
         collapsible(nextId), positions, widths);
   }

}

QSplitterLayoutStruct *QSplitterPrivate::findWidget(QWidget *w) const
{
   for (int i = 0; i < list.size(); ++i) {
      if (list.at(i)->widget == w) {
         return list.at(i);
      }
   }
   return nullptr;
}

void QSplitterPrivate::insertWidget_helper(int index, QWidget *widget, bool show)
{
   Q_Q(QSplitter);

   bool temp = blockChildAdd;
   blockChildAdd = true;

   bool needShow = show && q->isVisible() && ! (widget->isHidden() &&
         widget->testAttribute(Qt::WA_WState_ExplicitShowHide));

   if (widget->parentWidget() != q) {
      widget->setParent(q);
   }

   if (needShow) {
      widget->show();
   }

   insertWidget(index, widget);
   recalc(q->isVisible());

   blockChildAdd = temp;
}

QSplitterLayoutStruct *QSplitterPrivate::insertWidget(int index, QWidget *w)
{
   Q_Q(QSplitter);

   QSplitterLayoutStruct *sls = nullptr;
   int i;
   int last = list.count();
   for (i = 0; i < list.size(); ++i) {
      QSplitterLayoutStruct *s = list.at(i);
      if (s->widget == w) {
         sls = s;
         --last;
         break;
      }
   }

   if (index < 0 || index > last) {
      index = last;
   }

   if (sls) {
      list.move(i, index);
   } else {
      QSplitterHandle *newHandle = nullptr;
      sls = new QSplitterLayoutStruct;
      QString tmp = QLatin1String("qt_splithandle_");
      tmp += w->objectName();
      newHandle = q->createHandle();
      newHandle->setObjectName(tmp);
      sls->handle = newHandle;
      sls->widget = w;
      w->lower();
      list.insert(index, sls);

      if (newHandle && q->isVisible()) {
         newHandle->show();   // will trigger sending of post events
      }
   }
   return sls;
}


QSplitter::QSplitter(QWidget *parent)
   : QFrame(*new QSplitterPrivate, parent)
{
   Q_D(QSplitter);
   d->orient = Qt::Horizontal;
   d->init();
}

QSplitter::QSplitter(Qt::Orientation orientation, QWidget *parent)
   : QFrame(*new QSplitterPrivate, parent)
{
   Q_D(QSplitter);
   d->orient = orientation;
   d->init();
}

QSplitter::~QSplitter()
{
   Q_D(QSplitter);

   delete d->rubberBand;

   while (! d->list.isEmpty()) {
      delete d->list.takeFirst();
   }
}

void QSplitter::refresh()
{
   Q_D(QSplitter);
   d->recalc(true);
}

void QSplitter::setOrientation(Qt::Orientation orientation)
{
   Q_D(QSplitter);
   if (d->orient == orientation) {
      return;
   }

   if (!testAttribute(Qt::WA_WState_OwnSizePolicy)) {
      QSizePolicy sp = sizePolicy();
      sp.transpose();
      setSizePolicy(sp);
      setAttribute(Qt::WA_WState_OwnSizePolicy, false);
   }

   d->orient = orientation;

   for (int i = 0; i < d->list.size(); ++i) {
      QSplitterLayoutStruct *s = d->list.at(i);
      s->handle->setOrientation(orientation);
   }
   d->recalc(isVisible());
}

Qt::Orientation QSplitter::orientation() const
{
   Q_D(const QSplitter);
   return d->orient;
}


void QSplitter::setChildrenCollapsible(bool collapse)
{
   Q_D(QSplitter);
   d->childrenCollapsible = collapse;
}

bool QSplitter::childrenCollapsible() const
{
   Q_D(const QSplitter);
   return d->childrenCollapsible;
}

void QSplitter::setCollapsible(int index, bool collapse)
{
   Q_D(QSplitter);

   if (index < 0 || index >= d->list.size()) {
      qWarning("QSplitter::setCollapsible() Index %d is out of range", index);
      return;
   }

   d->list.at(index)->collapsible = collapse ? 1 : 0;
}

bool QSplitter::isCollapsible(int index) const
{
   Q_D(const QSplitter);

   if (index < 0 || index >= d->list.size()) {
      qWarning("QSplitter::isCollapsible() Index %d is out of range", index);
      return false;
   }

   return d->list.at(index)->collapsible;
}

void QSplitter::resizeEvent(QResizeEvent *)
{
   Q_D(QSplitter);
   d->doResize();
}

void QSplitter::addWidget(QWidget *widget)
{
   Q_D(QSplitter);
   insertWidget(d->list.count(), widget);
}

void QSplitter::insertWidget(int index, QWidget *widget)
{
   Q_D(QSplitter);
   d->insertWidget_helper(index, widget, true);
}

int QSplitter::indexOf(QWidget *w) const
{
   Q_D(const QSplitter);

   for (int i = 0; i < d->list.size(); ++i) {
      QSplitterLayoutStruct *s = d->list.at(i);

      if (s->widget == w || s->handle == w) {
         return i;
      }
   }

   return -1;
}

QSplitterHandle *QSplitter::createHandle()
{
   Q_D(QSplitter);
   return new QSplitterHandle(d->orient, this);
}

QSplitterHandle *QSplitter::handle(int index) const
{
   Q_D(const QSplitter);
   if (index < 0 || index >= d->list.size()) {
      return nullptr;
   }
   return d->list.at(index)->handle;
}

QWidget *QSplitter::widget(int index) const
{
   Q_D(const QSplitter);

   if (index < 0 || index >= d->list.size()) {
      return nullptr;
   }

   return d->list.at(index)->widget;
}

int QSplitter::count() const
{
   Q_D(const QSplitter);
   return d->list.count();
}

void QSplitter::childEvent(QChildEvent *c)
{
   Q_D(QSplitter);

   if (! c->child()->isWidgetType()) {
      if (c->type() == QEvent::ChildAdded && qobject_cast<QLayout *>(c->child())) {
         qWarning("QSplitter::childEvent() Adding a QLayout to a QSplitter is not supported");
      }
      return;
   }

   QWidget *w = static_cast<QWidget *>(c->child());

   if (w->isWindow()) {
      return;
   }

   if (c->added() && !d->blockChildAdd && !d->findWidget(w)) {
      d->insertWidget_helper(d->list.count(), w, false);

   } else if (c->polished() && !d->blockChildAdd) {
      if (isVisible() && !(w->isHidden() && w->testAttribute(Qt::WA_WState_ExplicitShowHide))) {
         w->show();
      }

   } else if (c->type() == QEvent::ChildRemoved) {

      for (int i = 0; i < d->list.size(); ++i) {
         QSplitterLayoutStruct *s = d->list.at(i);

         if (s->widget == w) {
            d->list.removeAt(i);
            delete s;

            d->recalc(isVisible());
            return;
         }
      }
   }
}

void QSplitter::setRubberBand(int pos)
{
   Q_D(QSplitter);

   if (pos < 0) {
      if (d->rubberBand) {
         d->rubberBand->deleteLater();
      }
      return;
   }

   QRect r = contentsRect();
   const int rBord = 3; // customizable?
   int hw = handleWidth();

   if (! d->rubberBand) {
      bool temp = d->blockChildAdd;
      d->blockChildAdd = true;

      d->rubberBand = new QRubberBand(QRubberBand::Line, this);

      // For accessibility to identify this special widget.
      d->rubberBand->setObjectName(QLatin1String("qt_rubberband"));

      d->blockChildAdd = temp;
   }

   const QRect newGeom = d->orient == Qt::Horizontal ? QRect(QPoint(pos + hw / 2 - rBord, r.y()), QSize(2 * rBord,
            r.height()))
      : QRect(QPoint(r.x(), pos + hw / 2 - rBord), QSize(r.width(), 2 * rBord));
   d->rubberBand->setGeometry(newGeom);
   d->rubberBand->show();
}

bool QSplitter::event(QEvent *e)
{
   Q_D(QSplitter);

   switch (e->type()) {
      case QEvent::Hide:
         // Reset firstShow to false here since things can be done to the splitter in between
         if (! d->firstShow) {
            d->firstShow = true;
         }
         break;

      case QEvent::Show:
         if (!d->firstShow) {
            break;
         }
         d->firstShow = false;
         [[fallthrough]];

      case QEvent::HideToParent:
      case QEvent::ShowToParent:
      case QEvent::LayoutRequest:
         d->recalc(isVisible());
         break;

      default:
         break;
   }

   return QWidget::event(e);
}

void QSplitter::moveSplitter(int pos, int index)
{
   Q_D(QSplitter);
   QSplitterLayoutStruct *s = d->list.at(index);
   int farMin;
   int min;
   int max;
   int farMax;

#if defined(CS_SHOW_DEBUG_GUI_WIDGETS)
   int debugp = pos;
#endif

   pos = d->adjustPos(pos, index, &farMin, &min, &max, &farMax);
   int oldP = d->pick(s->rect.topLeft());

#if defined(CS_SHOW_DEBUG_GUI_WIDGETS)
   qDebug() << "QSplitter::moveSplitter" << debugp << index << "adjusted" << pos << "oldP" << oldP;
#endif

   QVarLengthArray<int, 32> poss(d->list.count());
   QVarLengthArray<int, 32> ws(d->list.count());
   bool upLeft;

   d->doMove(false, pos, index, +1, (d->collapsible(s) && (pos > max)), poss.data(), ws.data());
   d->doMove(true, pos, index - 1, +1, (d->collapsible(index - 1) && (pos < min)), poss.data(), ws.data());
   upLeft = (pos < oldP);

   int wid, delta, count = d->list.count();
   if (upLeft) {
      wid = 0;
      delta = 1;
   } else {
      wid = count - 1;
      delta = -1;
   }

   for (; wid >= 0 && wid < count; wid += delta) {
      QSplitterLayoutStruct *sls = d->list.at( wid );
      if (!sls->widget->isHidden()) {
         d->setGeo(sls, poss[wid], ws[wid], true);
      }
   }
   d->storeSizes();

   emit splitterMoved(pos, index);
}


void QSplitter::getRange(int index, int *min, int *max) const
{
   Q_D(const QSplitter);
   d->getRange(index, min, nullptr, nullptr, max);
}

int QSplitter::closestLegalPosition(int pos, int index)
{
   Q_D(QSplitter);
   int x, i, n, u;
   return d->adjustPos(pos, index, &u, &n, &i, &x);
}

bool QSplitter::opaqueResize() const
{
   Q_D(const QSplitter);
   return d->opaqueResizeSet ? d->opaque : style()->styleHint(QStyle::SH_Splitter_OpaqueResize, nullptr, this);
}

void QSplitter::setOpaqueResize(bool on)
{
   Q_D(QSplitter);
   d->opaqueResizeSet = true;
   d->opaque = on;
}

QSize QSplitter::sizeHint() const
{
   Q_D(const QSplitter);

   ensurePolished();
   int l = 0;
   int t = 0;

   for (int i = 0; i < d->list.size(); ++i) {
      QWidget *w = d->list.at(i)->widget;
      if (w->isHidden()) {
         continue;
      }
      QSize s = w->sizeHint();
      if (s.isValid()) {
         l += d->pick(s);
         t = qMax(t, d->trans(s));
      }
   }

   return orientation() == Qt::Horizontal ? QSize(l, t) : QSize(t, l);
}

QSize QSplitter::minimumSizeHint() const
{
   Q_D(const QSplitter);
   ensurePolished();
   int l = 0;
   int t = 0;

   for (int i = 0; i < d->list.size(); ++i) {
      QSplitterLayoutStruct *s = d->list.at(i);

      if (! s || !s->widget) {
         continue;
      }

      if (s->widget->isHidden()) {
         continue;
      }

      QSize widgetSize = qSmartMinSize(s->widget);
      if (widgetSize.isValid()) {
         l += d->pick(widgetSize);
         t = qMax(t, d->trans(widgetSize));
      }

      if (!s->handle || s->handle->isHidden()) {
         continue;
      }

      QSize splitterSize = s->handle->sizeHint();
      if (splitterSize.isValid()) {
         l += d->pick(splitterSize);
         t = qMax(t, d->trans(splitterSize));
      }
   }

   return orientation() == Qt::Horizontal ? QSize(l, t) : QSize(t, l);
}

QList<int> QSplitter::sizes() const
{
   Q_D(const QSplitter);
   ensurePolished();

   QList<int> list;
   for (int i = 0; i < d->list.size(); ++i) {
      QSplitterLayoutStruct *s = d->list.at(i);
      list.append(d->pick(s->rect.size()));
   }

   return list;
}

void QSplitter::setSizes(const QList<int> &list)
{
   Q_D(QSplitter);
   d->setSizes_helper(list, true);
}

int QSplitter::handleWidth() const
{
   Q_D(const QSplitter);
   if (d->handleWidth >= 0) {
      return d->handleWidth;
   } else {
      return style()->pixelMetric(QStyle::PM_SplitterWidth, nullptr, this);
   }
}

void QSplitter::setHandleWidth(int width)
{
   Q_D(QSplitter);
   d->handleWidth = width;
   d->updateHandles();
}

void QSplitter::changeEvent(QEvent *ev)
{
   Q_D(QSplitter);
   if (ev->type() == QEvent::StyleChange) {
      d->updateHandles();
   }
   QFrame::changeEvent(ev);
}

static constexpr const qint32 SplitterMagic = 0xff;

QByteArray QSplitter::saveState() const
{
   Q_D(const QSplitter);

   int version = 1;
   QByteArray data;
   QDataStream stream(&data, QIODevice::WriteOnly);

   stream << qint32(SplitterMagic);
   stream << qint32(version);

   QList<int> list;
   for (int i = 0; i < d->list.size(); ++i) {
      QSplitterLayoutStruct *s = d->list.at(i);
      list.append(s->sizer);
   }

   stream << list;
   stream << childrenCollapsible();
   stream << qint32(handleWidth());
   stream << opaqueResize();
   stream << qint32(orientation());
   stream << d->opaqueResizeSet;

   return data;
}

bool QSplitter::restoreState(const QByteArray &state)
{
   Q_D(QSplitter);

   int version = 1;
   QByteArray sd = state;
   QDataStream stream(&sd, QIODevice::ReadOnly);

   QList<int> list;
   bool b;
   qint32 i;
   qint32 marker;
   qint32 v;

   stream >> marker;
   stream >> v;

   if (marker != SplitterMagic || v > version) {
      return false;
   }

   stream >> list;
   d->setSizes_helper(list, false);

   stream >> b;
   setChildrenCollapsible(b);

   stream >> i;
   setHandleWidth(i);

   stream >> b;
   setOpaqueResize(b);

   stream >> i;
   setOrientation(Qt::Orientation(i));
   d->doResize();

   if (v >= 1) {
      stream >> d->opaqueResizeSet;
   }

   return true;
}

void QSplitter::setStretchFactor(int index, int stretch)
{
   Q_D(QSplitter);
   if (index <= -1 || index >= d->list.count()) {
      return;
   }

   QWidget *widget = d->list.at(index)->widget;
   QSizePolicy sp = widget->sizePolicy();
   sp.setHorizontalStretch(stretch);
   sp.setVerticalStretch(stretch);
   widget->setSizePolicy(sp);
}

QTextStream &operator<<(QTextStream &ts, const QSplitter &splitter)
{
   ts << splitter.saveState() << endl;
   return ts;
}

QTextStream &operator>>(QTextStream &ts, QSplitter &splitter)
{
   QString line = ts.readLine();
   line = line.simplified();
   line.replace(QLatin1Char(' '), QString());
   line = line.toUpper();

   splitter.restoreState(line.toLatin1());
   return ts;
}

#endif // QT_NO_SPLITTER
