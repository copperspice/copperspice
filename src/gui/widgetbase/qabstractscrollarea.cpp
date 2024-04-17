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

#include <qabstractscrollarea.h>

#ifndef QT_NO_SCROLLAREA

#include <qapplication.h>
#include <qboxlayout.h>
#include <qdebug.h>
#include <qevent.h>
#include <qheaderview.h>
#include <qmargins.h>
#include <qpainter.h>
#include <qscrollbar.h>
#include <qstyle.h>
#include <qstyleoption.h>
#include <qwidget.h>

#include <qabstractscrollarea_p.h>
#include <qapplication_p.h>
#include <qscrollbar_p.h>

#ifdef Q_OS_WIN
#  include <qlibrary.h>
#  include <qt_windows.h>
#endif

QAbstractScrollAreaPrivate::QAbstractScrollAreaPrivate()
   : hbar(nullptr), vbar(nullptr), vbarpolicy(Qt::ScrollBarAsNeeded), hbarpolicy(Qt::ScrollBarAsNeeded),
     shownOnce(false), inResize(false), sizeAdjustPolicy(QAbstractScrollArea::AdjustIgnored),
     viewport(nullptr), cornerWidget(nullptr), left(0), top(0), right(0), bottom(0),
     xoffset(0), yoffset(0), viewportFilter(nullptr)
{
}

QAbstractScrollAreaPrivate::~QAbstractScrollAreaPrivate()
{
}

QAbstractScrollAreaScrollBarContainer::QAbstractScrollAreaScrollBarContainer(Qt::Orientation orientation,
      QWidget *parent)
   : QWidget(parent), scrollBar(new QScrollBar(orientation, this)),
     layout(new QBoxLayout(orientation == Qt::Horizontal ? QBoxLayout::LeftToRight : QBoxLayout::TopToBottom)),
     orientation(orientation)
{
   setLayout(layout);
   layout->setMargin(0);
   layout->setSpacing(0);
   layout->addWidget(scrollBar);
   layout->setSizeConstraint(QLayout::SetMaximumSize);
}

void QAbstractScrollAreaScrollBarContainer::addWidget(QWidget *widget, LogicalPosition position)
{
   QSizePolicy policy = widget->sizePolicy();

   if (orientation == Qt::Vertical) {
      policy.setHorizontalPolicy(QSizePolicy::Ignored);
   } else {
      policy.setVerticalPolicy(QSizePolicy::Ignored);
   }

   widget->setSizePolicy(policy);
   widget->setParent(this);

   const int insertIndex = (position & LogicalLeft) ? 0 : scrollBarLayoutIndex() + 1;
   layout->insertWidget(insertIndex, widget);
}

QWidgetList QAbstractScrollAreaScrollBarContainer::widgets(LogicalPosition position)
{
   QWidgetList list;
   const int scrollBarIndex = scrollBarLayoutIndex();

   if (position == LogicalLeft) {
      for (int i = 0; i < scrollBarIndex; ++i) {
         list.append(layout->itemAt(i)->widget());
      }

   } else if (position == LogicalRight) {
      const int layoutItemCount = layout->count();

      for (int i = scrollBarIndex + 1; i < layoutItemCount; ++i) {
         list.append(layout->itemAt(i)->widget());
      }
   }

   return list;
}

int QAbstractScrollAreaScrollBarContainer::scrollBarLayoutIndex() const
{
   const int layoutItemCount = layout->count();

   for (int i = 0; i < layoutItemCount; ++i) {
      if (dynamic_cast<QScrollBar *>(layout->itemAt(i)->widget())) {
         return i;
      }
   }

   return -1;
}

void QAbstractScrollAreaPrivate::replaceScrollBar(QScrollBar *scrollBar, Qt::Orientation orientation)
{
   Q_Q(QAbstractScrollArea);

   QAbstractScrollAreaScrollBarContainer *container = scrollBarContainers[orientation];
   bool horizontal = (orientation == Qt::Horizontal);
   QScrollBar *oldBar = horizontal ? hbar : vbar;

   if (horizontal) {
      hbar = scrollBar;
   } else {
      vbar = scrollBar;
   }

   scrollBar->setParent(container);
   container->scrollBar = scrollBar;
   container->layout->removeWidget(oldBar);
   container->layout->insertWidget(0, scrollBar);
   scrollBar->setVisible(oldBar->isVisibleTo(container));
   scrollBar->setInvertedAppearance(oldBar->invertedAppearance());
   scrollBar->setInvertedControls(oldBar->invertedControls());
   scrollBar->setRange(oldBar->minimum(), oldBar->maximum());
   scrollBar->setOrientation(oldBar->orientation());
   scrollBar->setPageStep(oldBar->pageStep());
   scrollBar->setSingleStep(oldBar->singleStep());
   scrollBar->setSliderDown(oldBar->isSliderDown());
   scrollBar->setSliderPosition(oldBar->sliderPosition());
   scrollBar->setTracking(oldBar->hasTracking());
   scrollBar->setValue(oldBar->value());
   scrollBar->installEventFilter(q);
   oldBar->removeEventFilter(q);

   delete oldBar;

   if (horizontal) {
      QObject::connect(scrollBar, &QScrollBar::valueChanged, q, &QAbstractScrollArea::_q_hslide);

   } else {
      QObject::connect(scrollBar, &QScrollBar::valueChanged, q, &QAbstractScrollArea::_q_vslide);

   }

   QObject::connect(scrollBar, &QScrollBar::rangeChanged, q,
         &QAbstractScrollArea::_q_showOrHideScrollBars, Qt::QueuedConnection);
}

void QAbstractScrollAreaPrivate::init()
{
   Q_Q(QAbstractScrollArea);

   viewport = new QWidget(q);
   viewport->setObjectName("qt_scrollarea_viewport");
   viewport->setBackgroundRole(QPalette::Base);
   viewport->setAutoFillBackground(true);

   scrollBarContainers[Qt::Horizontal] = new QAbstractScrollAreaScrollBarContainer(Qt::Horizontal, q);
   scrollBarContainers[Qt::Horizontal]->setObjectName("qt_scrollarea_hcontainer");
   hbar = scrollBarContainers[Qt::Horizontal]->scrollBar;
   hbar->setRange(0, 0);

   scrollBarContainers[Qt::Horizontal]->setVisible(false);
   hbar->installEventFilter(q);

   QObject::connect(hbar, &QScrollBar::valueChanged, q, &QAbstractScrollArea::_q_hslide);
   QObject::connect(hbar, &QScrollBar::rangeChanged, q, &QAbstractScrollArea::_q_showOrHideScrollBars, Qt::QueuedConnection);

   scrollBarContainers[Qt::Vertical] = new QAbstractScrollAreaScrollBarContainer(Qt::Vertical, q);
   scrollBarContainers[Qt::Vertical]->setObjectName("qt_scrollarea_vcontainer");
   vbar = scrollBarContainers[Qt::Vertical]->scrollBar;
   vbar->setRange(0, 0);

   scrollBarContainers[Qt::Vertical]->setVisible(false);
   vbar->installEventFilter(q);

   QObject::connect(vbar, &QScrollBar::valueChanged, q, &QAbstractScrollArea::_q_vslide);
   QObject::connect(vbar, &QScrollBar::rangeChanged, q, &QAbstractScrollArea::_q_showOrHideScrollBars, Qt::QueuedConnection);

   viewportFilter.reset(new QAbstractScrollAreaFilter(this));
   viewport->installEventFilter(viewportFilter.data());
   viewport->setFocusProxy(q);

   q->setFocusPolicy(Qt::StrongFocus);
   q->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
   q->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

   layoutChildren();

#if ! defined(Q_OS_DARWIN) && ! defined(QT_NO_GESTURES)
   viewport->grabGesture(Qt::PanGesture);
#endif
}

void QAbstractScrollAreaPrivate::layoutChildren()
{
   Q_Q(QAbstractScrollArea);

   bool htransient = hbar->style()->styleHint(QStyle::SH_ScrollBar_Transient, nullptr, hbar);
   bool needh = (hbarpolicy != Qt::ScrollBarAlwaysOff) && ((hbarpolicy == Qt::ScrollBarAlwaysOn && !htransient)
               || ((hbarpolicy == Qt::ScrollBarAsNeeded || htransient)
                     && hbar->minimum() < hbar->maximum() && !hbar->sizeHint().isEmpty()));

   bool vtransient = vbar->style()->styleHint(QStyle::SH_ScrollBar_Transient, nullptr, vbar);
   bool needv = (vbarpolicy != Qt::ScrollBarAlwaysOff) && ((vbarpolicy == Qt::ScrollBarAlwaysOn && !vtransient)
               || ((vbarpolicy == Qt::ScrollBarAsNeeded || vtransient)
               && vbar->minimum() < vbar->maximum() && !vbar->sizeHint().isEmpty()));

   QStyleOption opt(0);
   opt.initFrom(q);

   const int hscrollOverlap = hbar->style()->pixelMetric(QStyle::PM_ScrollView_ScrollBarOverlap, &opt, hbar);
   const int vscrollOverlap = vbar->style()->pixelMetric(QStyle::PM_ScrollView_ScrollBarOverlap, &opt, vbar);

   const int hsbExt = hbar->sizeHint().height();
   const int vsbExt = vbar->sizeHint().width();

   const QPoint extPoint(vsbExt, hsbExt);
   const QSize extSize(vsbExt, hsbExt);
   const QRect widgetRect = q->rect();

   const bool hasCornerWidget = (cornerWidget != nullptr);

   QPoint cornerOffset((needv && vscrollOverlap == 0) ? vsbExt : 0, (needh && hscrollOverlap == 0) ? hsbExt : 0);
   QRect controlsRect;
   QRect viewportRect;

   // In FrameOnlyAroundContents mode the frame is drawn between the controls and
   // the viewport, else the frame rect is equal to the widget rect.
   if ((frameStyle != QFrame::NoFrame) &&
         q->style()->styleHint(QStyle::SH_ScrollView_FrameOnlyAroundContents, &opt, q)) {
      controlsRect = widgetRect;
      const int spacing = q->style()->pixelMetric(QStyle::PM_ScrollView_ScrollBarSpacing, &opt, q);
      const QPoint cornerExtra(needv ? spacing + vscrollOverlap : 0, needh ? spacing + hscrollOverlap : 0);

      QRect frameRect = widgetRect;
      frameRect.adjust(0, 0, -cornerOffset.x() - cornerExtra.x(), -cornerOffset.y() - cornerExtra.y());
      q->setFrameRect(QStyle::visualRect(opt.direction, opt.rect, frameRect));

      // The frame rect needs to be in logical coords, however we need to flip
      // the contentsRect back before passing it on to the viewportRect
      // since the viewportRect has its logical coords calculated later.
      viewportRect = QStyle::visualRect(opt.direction, opt.rect, q->contentsRect());

   } else {
      q->setFrameRect(QStyle::visualRect(opt.direction, opt.rect, widgetRect));
      controlsRect = q->contentsRect();
      viewportRect = QRect(controlsRect.topLeft(), controlsRect.bottomRight() - cornerOffset);
   }

   cornerOffset = QPoint(needv ? vsbExt : 0, needh ? hsbExt : 0);

   // If we have a corner widget and are only showing one scroll bar, we need to move it
   // to make room for the corner widget.
   if (hasCornerWidget && ((needv && vscrollOverlap == 0) || (needh && hscrollOverlap == 0))) {
      cornerOffset =  extPoint;
   }

   // The corner point is where the scroll bar rects, the corner widget rect and the
   // viewport rect meets.
   const QPoint cornerPoint(controlsRect.bottomRight() + QPoint(1, 1) - cornerOffset);

   // Some styles paints the corner if both scorllbars are showing and there is
   // no corner widget. Also, on the Mac we paint if there is a native
   // (transparent) sizegrip in the area where a corner widget would be.

   if (needv && needh && !hasCornerWidget && hscrollOverlap == 0 && vscrollOverlap == 0) {
      cornerPaintingRect = QStyle::visualRect(opt.direction, opt.rect, QRect(cornerPoint, extSize));
   } else {
      cornerPaintingRect = QRect();
   }

   // move the scrollbars away from top/left headers
   int vHeaderRight  = 0;
   int hHeaderBottom = 0;

   if ((vscrollOverlap > 0 && needv) || (hscrollOverlap > 0 && needh)) {
      const QList<QHeaderView *> headers = q->findChildren<QHeaderView *>();

      if (headers.count() <= 2) {
         for (const QHeaderView *header : headers) {

            const QRect geo = header->geometry();

            if (header->orientation() == Qt::Vertical && header->isVisible() &&
                  QStyle::visualRect(opt.direction, opt.rect, geo).left() <= opt.rect.width() / 2) {
               vHeaderRight = QStyle::visualRect(opt.direction, opt.rect, geo).right();

            } else if (header->orientation() == Qt::Horizontal && header->isVisible() && geo.top() <= q->frameWidth()) {
               hHeaderBottom = geo.bottom();
            }
         }
      }
   }

   if (needh) {
      QRect horizontalScrollBarRect(QPoint(controlsRect.left() + vHeaderRight, cornerPoint.y()), QPoint(cornerPoint.x() - 1,
            controlsRect.bottom()));

      if (! hasCornerWidget && htransient) {

         horizontalScrollBarRect.adjust(0, 0, cornerOffset.x(), 0);
      }

      scrollBarContainers[Qt::Horizontal]->setGeometry(QStyle::visualRect(opt.direction, opt.rect, horizontalScrollBarRect));
      scrollBarContainers[Qt::Horizontal]->raise();
   }

   if (needv) {
      QRect verticalScrollBarRect  (QPoint(cornerPoint.x(), controlsRect.top() + hHeaderBottom),  QPoint(controlsRect.right(),
                  cornerPoint.y() - 1));

      if (! hasCornerWidget && vtransient) {
         verticalScrollBarRect.adjust(0, 0, 0, cornerOffset.y());
      }

      scrollBarContainers[Qt::Vertical]->setGeometry(QStyle::visualRect(opt.direction, opt.rect, verticalScrollBarRect));
      scrollBarContainers[Qt::Vertical]->raise();
   }

   if (cornerWidget) {
      const QRect cornerWidgetRect(cornerPoint, controlsRect.bottomRight());
      cornerWidget->setGeometry(QStyle::visualRect(opt.direction, opt.rect, cornerWidgetRect));
   }

   scrollBarContainers[Qt::Horizontal]->setVisible(needh);
   scrollBarContainers[Qt::Vertical]->setVisible(needv);

   if (q->isRightToLeft()) {
      viewportRect.adjust(right, top, -left, -bottom);
   } else {
      viewportRect.adjust(left, top, -right, -bottom);
   }

   viewport->setGeometry(QStyle::visualRect(opt.direction, opt.rect, viewportRect)); // resize the viewport last
}

QAbstractScrollArea::QAbstractScrollArea(QAbstractScrollAreaPrivate &dd, QWidget *parent)
   : QFrame(dd, parent)
{
   Q_D(QAbstractScrollArea);

   try {
      d->init();

   } catch (...) {
      d->viewportFilter.reset();
      throw;
   }
}

QAbstractScrollArea::QAbstractScrollArea(QWidget *parent)
   : QFrame(*new QAbstractScrollAreaPrivate, parent)
{
   Q_D(QAbstractScrollArea);

   try {
      d->init();

   } catch (...) {
      d->viewportFilter.reset();
      throw;
   }
}

QAbstractScrollArea::~QAbstractScrollArea()
{
   Q_D(QAbstractScrollArea);

   // reset it here, otherwise we'll have a dangling pointer in ~QWidget
   d->viewportFilter.reset();
}

void QAbstractScrollArea::setViewport(QWidget *widget)
{
   Q_D(QAbstractScrollArea);

   if (widget != d->viewport) {
      QWidget *oldViewport = d->viewport;

      if (!widget) {
         widget = new QWidget;
      }

      d->viewport = widget;
      d->viewport->setParent(this);
      d->viewport->setFocusProxy(this);
      d->viewport->installEventFilter(d->viewportFilter.data());

#ifndef QT_NO_GESTURES
      d->viewport->grabGesture(Qt::PanGesture);
#endif
      d->layoutChildren();

#ifndef QT_NO_OPENGL
      QWidgetPrivate::get(d->viewport)->initializeViewportFramebuffer();
#endif

      if (isVisible()) {
         d->viewport->show();
      }

      setupViewport(widget);
      delete oldViewport;
   }
}

QWidget *QAbstractScrollArea::viewport() const
{
   Q_D(const QAbstractScrollArea);
   return d->viewport;
}

QSize QAbstractScrollArea::maximumViewportSize() const
{
   Q_D(const QAbstractScrollArea);
   int hsbExt = d->hbar->sizeHint().height();
   int vsbExt = d->vbar->sizeHint().width();

   int f = 2 * d->frameWidth;
   QSize max = size() - QSize(f + d->left + d->right, f + d->top + d->bottom);

   if (d->vbarpolicy == Qt::ScrollBarAlwaysOn) {
      max.rwidth() -= vsbExt;
   }

   if (d->hbarpolicy == Qt::ScrollBarAlwaysOn) {
      max.rheight() -= hsbExt;
   }

   return max;
}

Qt::ScrollBarPolicy QAbstractScrollArea::verticalScrollBarPolicy() const
{
   Q_D(const QAbstractScrollArea);
   return d->vbarpolicy;
}

void QAbstractScrollArea::setVerticalScrollBarPolicy(Qt::ScrollBarPolicy policy)
{
   Q_D(QAbstractScrollArea);
   const Qt::ScrollBarPolicy oldPolicy = d->vbarpolicy;
   d->vbarpolicy = policy;

   if (isVisible()) {
      d->layoutChildren();
   }

   if (oldPolicy != d->vbarpolicy) {
      d->scrollBarPolicyChanged(Qt::Vertical, d->vbarpolicy);
   }
}

QScrollBar *QAbstractScrollArea::verticalScrollBar() const
{
   Q_D(const QAbstractScrollArea);

   return d->vbar;
}

void QAbstractScrollArea::setVerticalScrollBar(QScrollBar *scrollBar)
{
   Q_D(QAbstractScrollArea);

   if (! scrollBar) {
      qWarning("QAbstractScrollArea::setVerticalScrollBar() Unable to set the scroll bar to an invalid value (nullptr)");
      return;
   }

   d->replaceScrollBar(scrollBar, Qt::Vertical);
}

Qt::ScrollBarPolicy QAbstractScrollArea::horizontalScrollBarPolicy() const
{
   Q_D(const QAbstractScrollArea);
   return d->hbarpolicy;
}

void QAbstractScrollArea::setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy policy)
{
   Q_D(QAbstractScrollArea);

   const Qt::ScrollBarPolicy oldPolicy = d->hbarpolicy;
   d->hbarpolicy = policy;

   if (isVisible()) {
      d->layoutChildren();
   }

   if (oldPolicy != d->hbarpolicy) {
      d->scrollBarPolicyChanged(Qt::Horizontal, d->hbarpolicy);
   }
}

QScrollBar *QAbstractScrollArea::horizontalScrollBar() const
{
   Q_D(const QAbstractScrollArea);
   return d->hbar;
}

void QAbstractScrollArea::setHorizontalScrollBar(QScrollBar *scrollBar)
{
   Q_D(QAbstractScrollArea);

   if (! scrollBar) {
      qWarning("QAbstractScrollArea::setHorizontalScrollBar() Unable to set the scroll bar to an invalid value (nullptr)");
      return;
   }

   d->replaceScrollBar(scrollBar, Qt::Horizontal);
}

QWidget *QAbstractScrollArea::cornerWidget() const
{
   Q_D(const QAbstractScrollArea);
   return d->cornerWidget;
}

void QAbstractScrollArea::setCornerWidget(QWidget *widget)
{
   Q_D(QAbstractScrollArea);
   QWidget *oldWidget = d->cornerWidget;

   if (oldWidget != widget) {
      if (oldWidget) {
         oldWidget->hide();
      }

      d->cornerWidget = widget;

      if (widget && widget->parentWidget() != this) {
         widget->setParent(this);
      }

      d->layoutChildren();

      if (widget) {
         widget->show();
      }

   } else {
      d->cornerWidget = widget;
      d->layoutChildren();
   }
}

void QAbstractScrollArea::addScrollBarWidget(QWidget *widget, Qt::Alignment alignment)
{
   Q_D(QAbstractScrollArea);

   if (widget == nullptr) {
      return;
   }

   const Qt::Orientation scrollBarOrientation =
         ((alignment & Qt::AlignLeft) || (alignment & Qt::AlignRight)) ? Qt::Horizontal : Qt::Vertical;

   const QAbstractScrollAreaScrollBarContainer::LogicalPosition position =
         ((alignment & Qt::AlignRight) || (alignment & Qt::AlignBottom))
         ? QAbstractScrollAreaScrollBarContainer::LogicalRight : QAbstractScrollAreaScrollBarContainer::LogicalLeft;

   d->scrollBarContainers[scrollBarOrientation]->addWidget(widget, position);
   d->layoutChildren();

   if (isHidden() == false) {
      widget->show();
   }
}

QWidgetList QAbstractScrollArea::scrollBarWidgets(Qt::Alignment alignment)
{
   Q_D(QAbstractScrollArea);

   QWidgetList list;

   if (alignment & Qt::AlignLeft) {
      list += d->scrollBarContainers[Qt::Horizontal]->widgets(QAbstractScrollAreaScrollBarContainer::LogicalLeft);
   }

   if (alignment & Qt::AlignRight) {
      list += d->scrollBarContainers[Qt::Horizontal]->widgets(QAbstractScrollAreaScrollBarContainer::LogicalRight);
   }

   if (alignment & Qt::AlignTop) {
      list += d->scrollBarContainers[Qt::Vertical]->widgets(QAbstractScrollAreaScrollBarContainer::LogicalLeft);
   }

   if (alignment & Qt::AlignBottom) {
      list += d->scrollBarContainers[Qt::Vertical]->widgets(QAbstractScrollAreaScrollBarContainer::LogicalRight);
   }

   return list;
}

void QAbstractScrollArea::setViewportMargins(int left, int top, int right, int bottom)
{
   Q_D(QAbstractScrollArea);
   d->left = left;
   d->top = top;
   d->right = right;
   d->bottom = bottom;
   d->layoutChildren();
}

void QAbstractScrollArea::setViewportMargins(const QMargins &margins)
{
   setViewportMargins(margins.left(), margins.top(), margins.right(), margins.bottom());
}

QMargins QAbstractScrollArea::viewportMargins() const
{
   Q_D(const QAbstractScrollArea);
   return QMargins(d->left, d->top, d->right, d->bottom);
}

bool QAbstractScrollArea::eventFilter(QObject *o, QEvent *e)
{
   Q_D(QAbstractScrollArea);

   if ((o == d->hbar || o == d->vbar) && (e->type() == QEvent::HoverEnter || e->type() == QEvent::HoverLeave)) {
      if (d->hbarpolicy == Qt::ScrollBarAsNeeded && d->vbarpolicy == Qt::ScrollBarAsNeeded) {
         QScrollBar *sbar = static_cast<QScrollBar *>(o);
         QScrollBar *sibling = sbar == d->hbar ? d->vbar : d->hbar;

         if (sbar->style()->styleHint(QStyle::SH_ScrollBar_Transient, nullptr, sbar) &&
               sibling->style()->styleHint(QStyle::SH_ScrollBar_Transient, nullptr, sibling)) {
            d->setScrollBarTransient(sibling, e->type() == QEvent::HoverLeave);
         }
      }
   }

   return QFrame::eventFilter(o, e);
}
bool QAbstractScrollArea::event(QEvent *e)
{
   Q_D(QAbstractScrollArea);

   switch (e->type()) {
      case QEvent::AcceptDropsChange:

         // There was a chance with accessibility client we get an vent before the viewport was created.
         // Also, in some cases we might get here from QWidget::event() virtual function which is (indirectly) called
         // from the viewport constructor at the time when the d->viewport is not yet initialized even without any
         // accessibility client. See qabstractscrollarea autotest for a test case.

         if (d->viewport) {
            d->viewport->setAcceptDrops(acceptDrops());
         }

         break;

      case QEvent::MouseTrackingChange:
         d->viewport->setMouseTracking(hasMouseTracking());
         break;

      case QEvent::Resize:
         if (!d->inResize) {
            d->inResize = true;
            d->layoutChildren();
            d->inResize = false;
         }

         break;

      case QEvent::Show:
         if (!d->shownOnce && d->sizeAdjustPolicy == QAbstractScrollArea::AdjustToContentsOnFirstShow) {
            d->sizeHint = QSize();
            updateGeometry();
         }

         d->shownOnce = true;
         return QFrame::event(e);

      case QEvent::Paint: {
         QStyleOption option;
         option.initFrom(this);

         if (d->cornerPaintingRect.isValid()) {
            option.rect = d->cornerPaintingRect;

            QPainter p(this);
            style()->drawPrimitive(QStyle::PE_PanelScrollAreaCorner, &option, &p, this);
         }

      }

      QFrame::paintEvent((QPaintEvent *)e);
      break;

#ifndef QT_NO_CONTEXTMENU

      case QEvent::ContextMenu:
         if (static_cast<QContextMenuEvent *>(e)->reason() == QContextMenuEvent::Keyboard) {
            return QFrame::event(e);
         }

         e->ignore();
         break;
#endif

      case QEvent::MouseButtonPress:
      case QEvent::MouseButtonRelease:
      case QEvent::MouseButtonDblClick:
      case QEvent::MouseMove:
      case QEvent::Wheel:

#ifndef QT_NO_DRAGANDDROP
      case QEvent::Drop:
      case QEvent::DragEnter:
      case QEvent::DragMove:
      case QEvent::DragLeave:
#endif

      // ignore touch events in case they have been propagated from the viewport
      case QEvent::TouchBegin:
      case QEvent::TouchUpdate:
      case QEvent::TouchEnd:
         return false;

#ifndef QT_NO_GESTURES

      case QEvent::Gesture: {
         QGestureEvent *ge = static_cast<QGestureEvent *>(e);
         QPanGesture *g = static_cast<QPanGesture *>(ge->gesture(Qt::PanGesture));

         if (g) {
            QScrollBar *hBar = horizontalScrollBar();
            QScrollBar *vBar = verticalScrollBar();
            QPointF delta = g->delta();

            if (!delta.isNull()) {
               if (QApplication::isRightToLeft()) {
                  delta.rx() *= -1;
               }

               int newX = hBar->value() - delta.x();
               int newY = vBar->value() - delta.y();
               hBar->setValue(newX);
               vBar->setValue(newY);
            }

            return true;
         }

         return false;
      }

#endif

      case QEvent::ScrollPrepare: {
         QScrollPrepareEvent *se = static_cast<QScrollPrepareEvent *>(e);

         if (d->canStartScrollingAt(se->startPos().toPoint())) {
            QScrollBar *hBar = horizontalScrollBar();
            QScrollBar *vBar = verticalScrollBar();

            se->setViewportSize(QSizeF(viewport()->size()));
            se->setContentPosRange(QRectF(0, 0, hBar->maximum(), vBar->maximum()));
            se->setContentPos(QPointF(hBar->value(), vBar->value()));
            se->accept();
            return true;
         }

         return false;
      }

      case QEvent::Scroll: {
         QScrollEvent *se = static_cast<QScrollEvent *>(e);

         QScrollBar *hBar = horizontalScrollBar();
         QScrollBar *vBar = verticalScrollBar();
         hBar->setValue(se->contentPos().x());
         vBar->setValue(se->contentPos().y());

         QPoint delta = d->overshoot - se->overshootDistance().toPoint();

         if (!delta.isNull()) {
            viewport()->move(viewport()->pos() + delta);
         }

         d->overshoot = se->overshootDistance().toPoint();

         return true;
      }

      case QEvent::StyleChange:
      case QEvent::LayoutDirectionChange:
      case QEvent::ApplicationLayoutDirectionChange:
      case QEvent::LayoutRequest:
         d->layoutChildren();
         [[fallthrough]];

      default:
         return QFrame::event(e);
   }

   return true;
}

bool QAbstractScrollArea::viewportEvent(QEvent *e)
{
   switch (e->type()) {
      case QEvent::Resize:
      case QEvent::Paint:
      case QEvent::MouseButtonPress:
      case QEvent::MouseButtonRelease:
      case QEvent::MouseButtonDblClick:
      case QEvent::TouchBegin:
      case QEvent::TouchUpdate:
      case QEvent::TouchEnd:
      case QEvent::MouseMove:
      case QEvent::ContextMenu:

#ifndef QT_NO_WHEELEVENT
      case QEvent::Wheel:
#endif

#ifndef QT_NO_DRAGANDDROP
      case QEvent::Drop:
      case QEvent::DragEnter:
      case QEvent::DragMove:
      case QEvent::DragLeave:
#endif

#ifndef QT_NO_OPENGL
         // QOpenGLWidget needs special support because it has to know
         // its size has changed, so that it can resize its fbo

         if (e->type() == QEvent::Resize) {
            QWidgetPrivate::get(viewport())->resizeViewportFramebuffer();
         }

#endif
         return QFrame::event(e);

      case QEvent::LayoutRequest:

#ifndef QT_NO_GESTURES
      case QEvent::Gesture:
      case QEvent::GestureOverride:
         return event(e);
#endif

      case QEvent::ScrollPrepare:
      case QEvent::Scroll:
         return event(e);

      default:
         break;
   }

   return false;  // let the viewport widget handle the event
}

void QAbstractScrollArea::resizeEvent(QResizeEvent *)
{
}

void QAbstractScrollArea::paintEvent(QPaintEvent *)
{
}

void QAbstractScrollArea::mousePressEvent(QMouseEvent *e)
{
   e->ignore();
}

void QAbstractScrollArea::mouseReleaseEvent(QMouseEvent *e)
{
   e->ignore();
}

void QAbstractScrollArea::mouseDoubleClickEvent(QMouseEvent *e)
{
   e->ignore();
}

void QAbstractScrollArea::mouseMoveEvent(QMouseEvent *e)
{
   e->ignore();
}

#ifndef QT_NO_WHEELEVENT
void QAbstractScrollArea::wheelEvent(QWheelEvent *e)
{
   Q_D(QAbstractScrollArea);

   if (static_cast<QWheelEvent *>(e)->orientation() == Qt::Horizontal) {
      QApplication::sendEvent(d->hbar, e);
   } else {
      QApplication::sendEvent(d->vbar, e);
   }
}
#endif

#ifndef QT_NO_CONTEXTMENU
void QAbstractScrollArea::contextMenuEvent(QContextMenuEvent *e)
{
   e->ignore();
}
#endif

void QAbstractScrollArea::keyPressEvent(QKeyEvent *e)
{
   Q_D(QAbstractScrollArea);

   if (false) {

#ifndef QT_NO_SHORTCUT
   } else if (e == QKeySequence::MoveToPreviousPage) {
      d->vbar->triggerAction(QScrollBar::SliderPageStepSub);

   } else if (e == QKeySequence::MoveToNextPage) {
      d->vbar->triggerAction(QScrollBar::SliderPageStepAdd);
#endif

   } else {

#ifdef QT_KEYPAD_NAVIGATION
      if (QApplication::keypadNavigationEnabled() && !hasEditFocus()) {
         e->ignore();
         return;
      }
#endif

      switch (e->key()) {
         case Qt::Key_Up:
            d->vbar->triggerAction(QScrollBar::SliderSingleStepSub);
            break;

         case Qt::Key_Down:
            d->vbar->triggerAction(QScrollBar::SliderSingleStepAdd);
            break;

         case Qt::Key_Left:
#ifdef QT_KEYPAD_NAVIGATION
            if (QApplication::keypadNavigationEnabled() && hasEditFocus()
                  && (! d->hbar->isVisible() || d->hbar->value() == d->hbar->minimum())) {
               // if we are not using the hbar or we are already at the leftmost point ignore
               e->ignore();
               return;
            }

#endif
            d->hbar->triggerAction(layoutDirection() == Qt::LeftToRight
                  ? QScrollBar::SliderSingleStepSub : QScrollBar::SliderSingleStepAdd);
            break;

         case Qt::Key_Right:
#ifdef QT_KEYPAD_NAVIGATION
            if (QApplication::keypadNavigationEnabled() && hasEditFocus()
                  && (! d->hbar->isVisible() || d->hbar->value() == d->hbar->maximum())) {
               // if we are not using the hbar or we are already at the rightmost point ignore
               e->ignore();
               return;
            }
#endif
            d->hbar->triggerAction(layoutDirection() == Qt::LeftToRight
                  ? QScrollBar::SliderSingleStepAdd : QScrollBar::SliderSingleStepSub);
            break;

         default:
            e->ignore();
            return;
      }
   }

   e->accept();
}

#ifndef QT_NO_DRAGANDDROP
void QAbstractScrollArea::dragEnterEvent(QDragEnterEvent *)
{
}

void QAbstractScrollArea::dragMoveEvent(QDragMoveEvent *)
{
}

void QAbstractScrollArea::dragLeaveEvent(QDragLeaveEvent *)
{
}

void QAbstractScrollArea::dropEvent(QDropEvent *)
{
}
#endif

void QAbstractScrollArea::scrollContentsBy(int, int)
{
   viewport()->update();
}

bool QAbstractScrollAreaPrivate::canStartScrollingAt( const QPoint &startPos )
{
   Q_Q(QAbstractScrollArea);

#ifndef QT_NO_GRAPHICSVIEW

   // do not start scrolling when a drag mode has been set
   // do not start scrolling on a movable item.
   if (QGraphicsView *view = qobject_cast<QGraphicsView *>(q)) {
      if (view->dragMode() != QGraphicsView::NoDrag) {
         return false;
      }

      QGraphicsItem *childItem = view->itemAt(startPos);

      if (childItem && (childItem->flags() & QGraphicsItem::ItemIsMovable)) {
         return false;
      }
   }

#endif

   // do not start scrolling on a QAbstractSlider
   if (qobject_cast<QAbstractSlider *>(q->viewport()->childAt(startPos))) {
      return false;
   }

   return true;
}

void QAbstractScrollAreaPrivate::flashScrollBars()
{
   bool htransient = hbar->style()->styleHint(QStyle::SH_ScrollBar_Transient, nullptr, hbar);

   if ((hbarpolicy != Qt::ScrollBarAlwaysOff) && (hbarpolicy == Qt::ScrollBarAsNeeded || htransient)) {
      hbar->d_func()->flash();
   }

   bool vtransient = vbar->style()->styleHint(QStyle::SH_ScrollBar_Transient, nullptr, vbar);

   if ((vbarpolicy != Qt::ScrollBarAlwaysOff) && (vbarpolicy == Qt::ScrollBarAsNeeded || vtransient)) {
      vbar->d_func()->flash();
   }
}

void QAbstractScrollAreaPrivate::setScrollBarTransient(QScrollBar *scrollBar, bool transient)
{
   scrollBar->d_func()->setTransient(transient);
}

void QAbstractScrollAreaPrivate::_q_hslide(int x)
{
   Q_Q(QAbstractScrollArea);
   int dx = xoffset - x;
   xoffset = x;
   q->scrollContentsBy(dx, 0);
   flashScrollBars();
}

void QAbstractScrollAreaPrivate::_q_vslide(int y)
{
   Q_Q(QAbstractScrollArea);
   int dy = yoffset - y;
   yoffset = y;
   q->scrollContentsBy(0, dy);
   flashScrollBars();
}

void QAbstractScrollAreaPrivate::_q_showOrHideScrollBars()
{
   layoutChildren();
}

QPoint QAbstractScrollAreaPrivate::contentsOffset() const
{
   Q_Q(const QAbstractScrollArea);

   QPoint offset;

   if (vbar->isVisible()) {
      offset.setY(vbar->value());
   }

   if (hbar->isVisible()) {
      if (q->isRightToLeft()) {
         offset.setX(hbar->maximum() - hbar->value());
      } else {
         offset.setX(hbar->value());
      }
   }

   return offset;
}

QSize QAbstractScrollArea::minimumSizeHint() const
{
   Q_D(const QAbstractScrollArea);
   int hsbExt = d->hbar->sizeHint().height();
   int vsbExt = d->vbar->sizeHint().width();
   int extra = 2 * d->frameWidth;

   QStyleOption opt;
   opt.initFrom(this);

   if ((d->frameStyle != QFrame::NoFrame)
         && style()->styleHint(QStyle::SH_ScrollView_FrameOnlyAroundContents, &opt, this)) {
      extra += style()->pixelMetric(QStyle::PM_ScrollView_ScrollBarSpacing, &opt, this);
   }

   return QSize(d->scrollBarContainers[Qt::Horizontal]->sizeHint().width() + vsbExt + extra,
               d->scrollBarContainers[Qt::Vertical]->sizeHint().height() + hsbExt + extra);
}

QSize QAbstractScrollArea::sizeHint() const
{
   Q_D(const QAbstractScrollArea);

   if (d->sizeAdjustPolicy == QAbstractScrollArea::AdjustIgnored) {
      return QSize(256, 192);
   }

   if (! d->sizeHint.isValid() || d->sizeAdjustPolicy == QAbstractScrollArea::AdjustToContents) {
      const int f = 2 * d->frameWidth;
      const QSize frame( f, f );
      const QSize scrollbars(d->vbarpolicy == Qt::ScrollBarAlwaysOn ? d->vbar->sizeHint().width() : 0,
            d->hbarpolicy == Qt::ScrollBarAlwaysOn ? d->hbar->sizeHint().height() : 0);
      d->sizeHint = frame + scrollbars + viewportSizeHint();
   }

   return d->sizeHint;
}

QSize QAbstractScrollArea::viewportSizeHint() const
{
   Q_D(const QAbstractScrollArea);

   if (d->viewport) {
      const QSize sh = d->viewport->sizeHint();

      if (sh.isValid()) {
         return sh;
      }
   }

   const int h = qMax(10, fontMetrics().height());
   return QSize(6 * h, 4 * h);
}

QAbstractScrollArea::SizeAdjustPolicy QAbstractScrollArea::sizeAdjustPolicy() const
{
   Q_D(const QAbstractScrollArea);
   return d->sizeAdjustPolicy;
}

void QAbstractScrollArea::setSizeAdjustPolicy(SizeAdjustPolicy policy)
{
   Q_D(QAbstractScrollArea);

   if (d->sizeAdjustPolicy == policy) {
      return;
   }

   d->sizeAdjustPolicy = policy;
   d->sizeHint = QSize();
   updateGeometry();
}

void QAbstractScrollArea::setupViewport(QWidget *viewport)
{
   (void) viewport;
}

void QAbstractScrollArea::_q_hslide(int x)
{
   Q_D(QAbstractScrollArea);
   d->_q_hslide(x);
}

void QAbstractScrollArea::_q_vslide(int y)
{
   Q_D(QAbstractScrollArea);
   d->_q_vslide(y);
}

void QAbstractScrollArea::_q_showOrHideScrollBars()
{
   Q_D(QAbstractScrollArea);
   d->_q_showOrHideScrollBars();
}

#endif // QT_NO_SCROLLAREA
