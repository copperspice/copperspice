/***********************************************************************
*
* Copyright (c) 2012-2018 Barbara Geller
* Copyright (c) 2012-2018 Ansel Sermersheim
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

#include <qabstractscrollarea.h>

#ifndef QT_NO_SCROLLAREA

#include <qscrollbar.h>
#include <qapplication.h>
#include <qstyle.h>
#include <qstyleoption.h>
#include <qevent.h>
#include <qdebug.h>
#include <qboxlayout.h>
#include <qpainter.h>
#include <qmargins.h>

#include <qabstractscrollarea_p.h>
#include <qwidget.h>
#include <qapplication_p.h>

#ifdef Q_OS_MAC
#include <qt_mac_p.h>
#include <qt_cocoa_helpers_mac_p.h>
#endif

QT_BEGIN_NAMESPACE

/*!
    \class QAbstractScrollArea
    \brief The QAbstractScrollArea widget provides a scrolling area with
    on-demand scroll bars.

    \ingroup abstractwidgets

    QAbstractScrollArea is a low-level abstraction of a scrolling
    area. The area provides a central widget called the viewport, in
    which the contents of the area is to be scrolled (i.e, the
    visible parts of the contents are rendered in the viewport).

    Next to the viewport is a vertical scroll bar, and below is a
    horizontal scroll bar. When all of the area contents fits in the
    viewport, each scroll bar can be either visible or hidden
    depending on the scroll bar's Qt::ScrollBarPolicy. When a scroll
    bar is hidden, the viewport expands in order to cover all
    available space. When a scroll bar becomes visible again, the
    viewport shrinks in order to make room for the scroll bar.

    It is possible to reserve a margin area around the viewport, see
    setViewportMargins(). The feature is mostly used to place a
    QHeaderView widget above or beside the scrolling area. Subclasses
    of QAbstractScrollArea should implement margins.

    When inheriting QAbstractScrollArea, you need to do the
    following:

    \list
        \o Control the scroll bars by setting their
           range, value, page step, and tracking their
           movements.
        \o Draw the contents of the area in the viewport according
           to the values of the scroll bars.
        \o Handle events received by the viewport in
           viewportEvent() - notably resize events.
        \o Use \c{viewport->update()} to update the contents of the
          viewport instead of \l{QWidget::update()}{update()}
          as all painting operations take place on the viewport.
    \endlist

    With a scroll bar policy of Qt::ScrollBarAsNeeded (the default),
    QAbstractScrollArea shows scroll bars when they provide a non-zero
    scrolling range, and hides them otherwise.

    The scroll bars and viewport should be updated whenever the viewport
    receives a resize event or the size of the contents changes.
    The viewport also needs to be updated when the scroll bars
    values change. The initial values of the scroll bars are often
    set when the area receives new contents.

    We give a simple example, in which we have implemented a scroll area
    that can scroll any QWidget. We make the widget a child of the
    viewport; this way, we do not have to calculate which part of
    the widget to draw but can simply move the widget with
    QWidget::move(). When the area contents or the viewport size
    changes, we do the following:

    \snippet doc/src/snippets/myscrollarea.cpp 1

    When the scroll bars change value, we need to update the widget
    position, i.e., find the part of the widget that is to be drawn in
    the viewport:

    \snippet doc/src/snippets/myscrollarea.cpp 0

    In order to track scroll bar movements, reimplement the virtual
    function scrollContentsBy(). In order to fine-tune scrolling
    behavior, connect to a scroll bar's
    QAbstractSlider::actionTriggered() signal and adjust the \l
    QAbstractSlider::sliderPosition as you wish.

    For convenience, QAbstractScrollArea makes all viewport events
    available in the virtual viewportEvent() handler. QWidget's
    specialized handlers are remapped to viewport events in the cases
    where this makes sense. The remapped specialized handlers are:
    paintEvent(), mousePressEvent(), mouseReleaseEvent(),
    mouseDoubleClickEvent(), mouseMoveEvent(), wheelEvent(),
    dragEnterEvent(), dragMoveEvent(), dragLeaveEvent(), dropEvent(),
    contextMenuEvent(),  and resizeEvent().

    QScrollArea, which inherits QAbstractScrollArea, provides smooth
    scrolling for any QWidget (i.e., the widget is scrolled pixel by
    pixel). You only need to subclass QAbstractScrollArea if you need
    more specialized behavior. This is, for instance, true if the
    entire contents of the area is not suitable for being drawn on a
    QWidget or if you do not want smooth scrolling.

    \sa QScrollArea
*/

QAbstractScrollAreaPrivate::QAbstractScrollAreaPrivate()
   : hbar(0), vbar(0), vbarpolicy(Qt::ScrollBarAsNeeded), hbarpolicy(Qt::ScrollBarAsNeeded),
     viewport(0), cornerWidget(0), left(0), top(0), right(0), bottom(0),
     xoffset(0), yoffset(0), viewportFilter(0)
#ifdef Q_OS_WIN
     , singleFingerPanEnabled(false)
#endif
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
}

/*! \internal
    Adds a widget to the scroll bar container.
*/
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

/*! \internal
    Retuns a list of scroll bar widgets for the given position. The scroll bar
    itself is not returned.
*/
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

/*! \internal
    Returns the layout index for the scroll bar. This needs to be
    recalculated by a linear search for each use, since items in
    the layout can be removed at any time (i.e. when a widget is
    deleted or re-parented).
*/
int QAbstractScrollAreaScrollBarContainer::scrollBarLayoutIndex() const
{
   const int layoutItemCount = layout->count();
   for (int i = 0; i < layoutItemCount; ++i) {
      if (qobject_cast<QScrollBar *>(layout->itemAt(i)->widget())) {
         return i;
      }
   }
   return -1;
}

/*! \internal
*/
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
   delete oldBar;

   QObject::connect(scrollBar, SIGNAL(valueChanged(int)),
                    q, horizontal ? SLOT(_q_hslide(int)) : SLOT(_q_vslide(int)));

   QObject::connect(scrollBar, SIGNAL(rangeChanged(int, int)),
                    q, SLOT(_q_showOrHideScrollBars()), Qt::QueuedConnection);
}

void QAbstractScrollAreaPrivate::init()
{
   Q_Q(QAbstractScrollArea);

   viewport = new QWidget(q);
   viewport->setObjectName(QLatin1String("qt_scrollarea_viewport"));
   viewport->setBackgroundRole(QPalette::Base);
   viewport->setAutoFillBackground(true);
   scrollBarContainers[Qt::Horizontal] = new QAbstractScrollAreaScrollBarContainer(Qt::Horizontal, q);
   scrollBarContainers[Qt::Horizontal]->setObjectName(QLatin1String("qt_scrollarea_hcontainer"));
   hbar = scrollBarContainers[Qt::Horizontal]->scrollBar;
   hbar->setRange(0, 0);

   scrollBarContainers[Qt::Horizontal]->setVisible(false);
   QObject::connect(hbar, SIGNAL(valueChanged(int)), q, SLOT(_q_hslide(int)));
   QObject::connect(hbar, SIGNAL(rangeChanged(int, int)), q, SLOT(_q_showOrHideScrollBars()), Qt::QueuedConnection);
   scrollBarContainers[Qt::Vertical] = new QAbstractScrollAreaScrollBarContainer(Qt::Vertical, q);
   scrollBarContainers[Qt::Vertical]->setObjectName(QLatin1String("qt_scrollarea_vcontainer"));
   vbar = scrollBarContainers[Qt::Vertical]->scrollBar;
   vbar->setRange(0, 0);

   scrollBarContainers[Qt::Vertical]->setVisible(false);
   QObject::connect(vbar, SIGNAL(valueChanged(int)), q, SLOT(_q_vslide(int)));
   QObject::connect(vbar, SIGNAL(rangeChanged(int, int)), q, SLOT(_q_showOrHideScrollBars()), Qt::QueuedConnection);
   viewportFilter.reset(new QAbstractScrollAreaFilter(this));
   viewport->installEventFilter(viewportFilter.data());
   viewport->setFocusProxy(q);
   q->setFocusPolicy(Qt::WheelFocus);
   q->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
   q->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
   layoutChildren();

#ifndef Q_OS_MAC
#ifndef QT_NO_GESTURES
   viewport->grabGesture(Qt::PanGesture);
#endif
#endif
}

#ifdef Q_OS_WIN
void QAbstractScrollAreaPrivate::setSingleFingerPanEnabled(bool on)
{
   singleFingerPanEnabled = on;

   QWidgetPrivate *dd = static_cast<QWidgetPrivate *>(QWidgetPrivate::cs_getPrivate(viewport));

   if (dd)  {
      dd->winSetupGestures();
   }
}
#endif

void QAbstractScrollAreaPrivate::layoutChildren()
{
   Q_Q(QAbstractScrollArea);
   bool needh = (hbarpolicy == Qt::ScrollBarAlwaysOn
                 || (hbarpolicy == Qt::ScrollBarAsNeeded && hbar->minimum() < hbar->maximum()));

   bool needv = (vbarpolicy == Qt::ScrollBarAlwaysOn
                 || (vbarpolicy == Qt::ScrollBarAsNeeded && vbar->minimum() < vbar->maximum()));

#ifdef Q_OS_MAC
   QWidget *const window = q->window();

   // Use small scroll bars for tool windows, to match the native size grip.
   bool hbarIsSmall = hbar->testAttribute(Qt::WA_MacSmallSize);
   bool vbarIsSmall = vbar->testAttribute(Qt::WA_MacSmallSize);
   const Qt::WindowType windowType = window->windowType();

   if (windowType == Qt::Tool) {
      if (!hbarIsSmall) {
         hbar->setAttribute(Qt::WA_MacMiniSize, false);
         hbar->setAttribute(Qt::WA_MacNormalSize, false);
         hbar->setAttribute(Qt::WA_MacSmallSize, true);
      }
      if (!vbarIsSmall) {
         vbar->setAttribute(Qt::WA_MacMiniSize, false);
         vbar->setAttribute(Qt::WA_MacNormalSize, false);
         vbar->setAttribute(Qt::WA_MacSmallSize, true);
      }
   } else {
      if (hbarIsSmall) {
         hbar->setAttribute(Qt::WA_MacMiniSize, false);
         hbar->setAttribute(Qt::WA_MacNormalSize, false);
         hbar->setAttribute(Qt::WA_MacSmallSize, false);
      }
      if (vbarIsSmall) {
         vbar->setAttribute(Qt::WA_MacMiniSize, false);
         vbar->setAttribute(Qt::WA_MacNormalSize, false);
         vbar->setAttribute(Qt::WA_MacSmallSize, false);
      }
   }
#endif

   const int hsbExt = hbar->sizeHint().height();
   const int vsbExt = vbar->sizeHint().width();
   const QPoint extPoint(vsbExt, hsbExt);
   const QSize extSize(vsbExt, hsbExt);

   const QRect widgetRect = q->rect();
   QStyleOption opt(0);
   opt.init(q);

   const bool hasCornerWidget = (cornerWidget != 0);

   // If the scroll bars are at the very right and bottom of the window we
   // move their positions to be aligned with the size grip.
#ifdef Q_OS_MAC
   // Check if a native sizegrip is present.
   bool hasMacReverseSizeGrip = false;
   bool hasMacSizeGrip = false;
   bool nativeGripPresent = false;

   if (q->testAttribute(Qt::WA_WState_Created)) {
      nativeGripPresent = qt_mac_checkForNativeSizeGrip(q);
   }

   if (nativeGripPresent) {
      // Look for a native size grip at the visual window bottom right and at the
      // absolute window bottom right. In reverse mode, the native size grip does not
      // swich side, so we need to check if it is on the "wrong side".
      const QPoint scrollAreaBottomRight = q->mapTo(window, widgetRect.bottomRight() - QPoint(frameWidth, frameWidth));
      const QPoint windowBottomRight = window->rect().bottomRight();
      const QPoint visualWindowBottomRight = QStyle::visualPos(opt.direction, opt.rect, windowBottomRight);
      const QPoint offset = windowBottomRight - scrollAreaBottomRight;
      const QPoint visualOffset = visualWindowBottomRight - scrollAreaBottomRight;
      hasMacSizeGrip = (visualOffset.manhattanLength() < vsbExt);
      hasMacReverseSizeGrip = (hasMacSizeGrip == false && (offset.manhattanLength() < hsbExt));
   }
#endif

   QPoint cornerOffset(needv ? vsbExt : 0, needh ? hsbExt : 0);
   QRect controlsRect;
   QRect viewportRect;

   // In FrameOnlyAroundContents mode the frame is drawn between the controls and
   // the viewport, else the frame rect is equal to the widget rect.
   if ((frameStyle != QFrame::NoFrame) &&
         q->style()->styleHint(QStyle::SH_ScrollView_FrameOnlyAroundContents, &opt, q)) {
      controlsRect = widgetRect;
      const int extra = q->style()->pixelMetric(QStyle::PM_ScrollView_ScrollBarSpacing, &opt, q);
      const QPoint cornerExtra(needv ? extra : 0, needh ? extra : 0);
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

   // If we have a corner widget and are only showing one scroll bar, we need to move it
   // to make room for the corner widget.
   if (hasCornerWidget && (needv || needh)) {
      cornerOffset =  extPoint;
   }

#ifdef Q_OS_MAC
   // Also move the scroll bars if they are covered by the native Mac size grip.
   if (hasMacSizeGrip) {
      cornerOffset =  extPoint;
   }
#endif

   // The corner point is where the scroll bar rects, the corner widget rect and the
   // viewport rect meets.
   const QPoint cornerPoint(controlsRect.bottomRight() + QPoint(1, 1) - cornerOffset);

   // Some styles paints the corner if both scorllbars are showing and there is
   // no corner widget. Also, on the Mac we paint if there is a native
   // (transparent) sizegrip in the area where a corner widget would be.
   if ((needv && needh && hasCornerWidget == false)
         || ((needv || needh)
#ifdef Q_OS_MAC
             && hasMacSizeGrip
#endif
            )
      ) {
      cornerPaintingRect = QStyle::visualRect(opt.direction, opt.rect, QRect(cornerPoint, extSize));
   } else {
      cornerPaintingRect = QRect();
   }

#ifdef Q_OS_MAC
   if (hasMacReverseSizeGrip) {
      reverseCornerPaintingRect = QRect(controlsRect.bottomRight() + QPoint(1, 1) - extPoint, extSize);
   } else {
      reverseCornerPaintingRect = QRect();
   }
#endif

   if (needh) {
      QRect horizontalScrollBarRect(QPoint(controlsRect.left(), cornerPoint.y()), QPoint(cornerPoint.x() - 1,
                                    controlsRect.bottom()));
#ifdef Q_OS_MAC
      if (hasMacReverseSizeGrip) {
         horizontalScrollBarRect.adjust(vsbExt, 0, 0, 0);
      }
#endif
      scrollBarContainers[Qt::Horizontal]->setGeometry(QStyle::visualRect(opt.direction, opt.rect, horizontalScrollBarRect));
      scrollBarContainers[Qt::Horizontal]->raise();
   }

   if (needv) {
      const QRect verticalScrollBarRect  (QPoint(cornerPoint.x(), controlsRect.top()),  QPoint(controlsRect.right(),
                                          cornerPoint.y() - 1));
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

/*!
    \internal

    Creates a new QAbstractScrollAreaPrivate, \a dd with the given \a parent.
*/
QAbstractScrollArea::QAbstractScrollArea(QAbstractScrollAreaPrivate &dd, QWidget *parent)
   : QFrame(dd, parent)
{
   Q_D(QAbstractScrollArea);

   QT_TRY {
      d->init();

   } QT_CATCH(...) {
      d->viewportFilter.reset();
      QT_RETHROW;
   }
}

/*!
    Constructs a viewport.

    The \a parent argument is sent to the QWidget constructor.
*/
QAbstractScrollArea::QAbstractScrollArea(QWidget *parent)
   : QFrame(*new QAbstractScrollAreaPrivate, parent)
{
   Q_D(QAbstractScrollArea);
   QT_TRY {
      d->init();
   } QT_CATCH(...) {
      d->viewportFilter.reset();
      QT_RETHROW;
   }
}


/*!
  Destroys the viewport.
 */
QAbstractScrollArea::~QAbstractScrollArea()
{
   Q_D(QAbstractScrollArea);
   // reset it here, otherwise we'll have a dangling pointer in ~QWidget
   d->viewportFilter.reset();
}


/*!
  \since 4.2
  Sets the viewport to be the given \a widget.
  The QAbstractScrollArea will take ownership of the given \a widget.

  If \a widget is 0, QAbstractScrollArea will assign a new QWidget instance
  for the viewport.

  \sa viewport()
*/
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
#ifndef Q_OS_MAC
#ifndef QT_NO_GESTURES
      d->viewport->grabGesture(Qt::PanGesture);
#endif
#endif
      d->layoutChildren();

      if (isVisible()) {
         d->viewport->show();
      }

      QMetaObject::invokeMethod(this, "setupViewport", Q_ARG(QWidget *, widget));
      delete oldViewport;
   }
}

/*!
    Returns the viewport widget.

    Use the QScrollArea::widget() function to retrieve the contents of
    the viewport widget.

    \sa QScrollArea::widget()
*/
QWidget *QAbstractScrollArea::viewport() const
{
   Q_D(const QAbstractScrollArea);
   return d->viewport;
}


/*!
Returns the size of the viewport as if the scroll bars had no valid
scrolling range.
*/
// ### still thinking about the name
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

/*!
    \property QAbstractScrollArea::verticalScrollBarPolicy
    \brief the policy for the vertical scroll bar

    The default policy is Qt::ScrollBarAsNeeded.

    \sa horizontalScrollBarPolicy
*/

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


/*!
  Returns the vertical scroll bar.

  \sa verticalScrollBarPolicy, horizontalScrollBar()
 */
QScrollBar *QAbstractScrollArea::verticalScrollBar() const
{
   Q_D(const QAbstractScrollArea);

   return d->vbar;
}

/*!
   \since 4.2
   Replaces the existing vertical scroll bar with \a scrollBar, and sets all
   the former scroll bar's slider properties on the new scroll bar. The former
   scroll bar is then deleted.

   QAbstractScrollArea already provides vertical and horizontal scroll bars by
   default. You can call this function to replace the default vertical
   scroll bar with your own custom scroll bar.

   \sa verticalScrollBar(), setHorizontalScrollBar()
*/
void QAbstractScrollArea::setVerticalScrollBar(QScrollBar *scrollBar)
{
   Q_D(QAbstractScrollArea);
   if (!scrollBar) {
      qWarning("QAbstractScrollArea::setVerticalScrollBar: Cannot set a null scroll bar");
      return;
   }

   d->replaceScrollBar(scrollBar, Qt::Vertical);
}

/*!
    \property QAbstractScrollArea::horizontalScrollBarPolicy
    \brief the policy for the horizontal scroll bar

    The default policy is Qt::ScrollBarAsNeeded.

    \sa verticalScrollBarPolicy
*/

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

/*!
  Returns the horizontal scroll bar.

  \sa horizontalScrollBarPolicy, verticalScrollBar()
 */
QScrollBar *QAbstractScrollArea::horizontalScrollBar() const
{
   Q_D(const QAbstractScrollArea);
   return d->hbar;
}

/*!
    \since 4.2

    Replaces the existing horizontal scroll bar with \a scrollBar, and sets all
    the former scroll bar's slider properties on the new scroll bar. The former
    scroll bar is then deleted.

    QAbstractScrollArea already provides horizontal and vertical scroll bars by
    default. You can call this function to replace the default horizontal
    scroll bar with your own custom scroll bar.

    \sa horizontalScrollBar(), setVerticalScrollBar()
*/
void QAbstractScrollArea::setHorizontalScrollBar(QScrollBar *scrollBar)
{
   Q_D(QAbstractScrollArea);
   if (!scrollBar) {
      qWarning("QAbstractScrollArea::setHorizontalScrollBar: Cannot set a null scroll bar");
      return;
   }

   d->replaceScrollBar(scrollBar, Qt::Horizontal);
}

/*!
    \since 4.2

    Returns the widget in the corner between the two scroll bars.

    By default, no corner widget is present.
*/
QWidget *QAbstractScrollArea::cornerWidget() const
{
   Q_D(const QAbstractScrollArea);
   return d->cornerWidget;
}

/*!
    \since 4.2

    Sets the widget in the corner between the two scroll bars to be
    \a widget.

    You will probably also want to set at least one of the scroll bar
    modes to \c AlwaysOn.

    Passing 0 shows no widget in the corner.

    Any previous corner widget is hidden.

    You may call setCornerWidget() with the same widget at different
    times.

    All widgets set here will be deleted by the scroll area when it is
    destroyed unless you separately reparent the widget after setting
    some other corner widget (or 0).

    Any \e newly set widget should have no current parent.

    By default, no corner widget is present.

    \sa horizontalScrollBarPolicy, horizontalScrollBarPolicy
*/
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

   if (widget == 0) {
      return;
   }

   const Qt::Orientation scrollBarOrientation
      = ((alignment & Qt::AlignLeft) || (alignment & Qt::AlignRight)) ? Qt::Horizontal : Qt::Vertical;
   const QAbstractScrollAreaScrollBarContainer::LogicalPosition position
      = ((alignment & Qt::AlignRight) || (alignment & Qt::AlignBottom))
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
         d->layoutChildren();
         break;

      case QEvent::Paint: {
         QStyleOption option;
         option.initFrom(this);

         if (d->cornerPaintingRect.isValid()) {
            option.rect = d->cornerPaintingRect;

            QPainter p(this);
            style()->drawPrimitive(QStyle::PE_PanelScrollAreaCorner, &option, &p, this);
         }

#ifdef Q_OS_MAC
         if (d->reverseCornerPaintingRect.isValid()) {
            option.rect = d->reverseCornerPaintingRect;
            QPainter p(this);
            style()->drawPrimitive(QStyle::PE_PanelScrollAreaCorner, &option, &p, this);
         }
#endif
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
#endif // QT_NO_GESTURES

      case QEvent::StyleChange:
      case QEvent::LayoutDirectionChange:
      case QEvent::ApplicationLayoutDirectionChange:
      case QEvent::LayoutRequest:
         d->layoutChildren();

         // fall through
      
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
         return QFrame::event(e);

      case QEvent::LayoutRequest:

#ifndef QT_NO_GESTURES
      case QEvent::Gesture:
      case QEvent::GestureOverride:
         return event(e);
#endif

      default:
         break;
   }
   return false; // let the viewport widget handle the event
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

/*!
    This event handler can be reimplemented in a subclass to receive
    mouse release events for the viewport() widget. The event is
    passed in \a e.

    \sa QWidget::mouseReleaseEvent()
*/
void QAbstractScrollArea::mouseReleaseEvent(QMouseEvent *e)
{
   e->ignore();
}

/*!
    This event handler can be reimplemented in a subclass to receive
    mouse double click events for the viewport() widget. The event is
    passed in \a e.

    \sa QWidget::mouseDoubleClickEvent()
*/
void QAbstractScrollArea::mouseDoubleClickEvent(QMouseEvent *e)
{
   e->ignore();
}

/*!
    This event handler can be reimplemented in a subclass to receive
    mouse move events for the viewport() widget. The event is passed
    in \a e.

    \sa QWidget::mouseMoveEvent()
*/
void QAbstractScrollArea::mouseMoveEvent(QMouseEvent *e)
{
   e->ignore();
}

/*!
    This event handler can be reimplemented in a subclass to receive
    wheel events for the viewport() widget. The event is passed in \a
    e.

    \sa QWidget::wheelEvent()
*/
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
/*!
    This event handler can be reimplemented in a subclass to receive
    context menu events for the viewport() widget. The event is passed
    in \a e.

    \sa QWidget::contextMenuEvent()
*/
void QAbstractScrollArea::contextMenuEvent(QContextMenuEvent *e)
{
   e->ignore();
}
#endif // QT_NO_CONTEXTMENU

/*!
    This function is called with key event \a e when key presses
    occur. It handles PageUp, PageDown, Up, Down, Left, and Right, and
    ignores all other key presses.
*/
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
                  && (!d->hbar->isVisible() || d->hbar->value() == d->hbar->minimum())) {
               //if we aren't using the hbar or we are already at the leftmost point ignore
               e->ignore();
               return;
            }
#endif
            d->hbar->triggerAction(
               layoutDirection() == Qt::LeftToRight
               ? QScrollBar::SliderSingleStepSub : QScrollBar::SliderSingleStepAdd);
            break;
         case Qt::Key_Right:
#ifdef QT_KEYPAD_NAVIGATION
            if (QApplication::keypadNavigationEnabled() && hasEditFocus()
                  && (!d->hbar->isVisible() || d->hbar->value() == d->hbar->maximum())) {
               //if we aren't using the hbar or we are already at the rightmost point ignore
               e->ignore();
               return;
            }
#endif
            d->hbar->triggerAction(
               layoutDirection() == Qt::LeftToRight
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

void QAbstractScrollAreaPrivate::_q_hslide(int x)
{
   Q_Q(QAbstractScrollArea);
   int dx = xoffset - x;
   xoffset = x;
   q->scrollContentsBy(dx, 0);
}

void QAbstractScrollAreaPrivate::_q_vslide(int y)
{
   Q_Q(QAbstractScrollArea);
   int dy = yoffset - y;
   yoffset = y;
   q->scrollContentsBy(0, dy);
}

void QAbstractScrollAreaPrivate::_q_showOrHideScrollBars()
{
   layoutChildren();

#ifdef Q_OS_WIN
   // Need to re-subscribe to gestures as the content changes to make sure we
   // enable/disable panning when needed.

   QWidgetPrivate *dd = static_cast<QWidgetPrivate *>(QWidgetPrivate::cs_getPrivate(viewport));

   if (dd)  {
      dd->winSetupGestures();
   }
#endif
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

/*!
    \reimp

*/
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

/*!
    \reimp
*/
QSize QAbstractScrollArea::sizeHint() const
{
   return QSize(256, 192);
#if 0
   Q_D(const QAbstractScrollArea);
   int h = qMax(10, fontMetrics().height());
   int f = 2 * d->frameWidth;
   return QSize((6 * h) + f, (4 * h) + f);
#endif
}

void QAbstractScrollArea::setupViewport(QWidget *viewport)
{
   Q_UNUSED(viewport);
}

void QAbstractScrollArea::_q_hslide(int un_named_arg1)
{
   Q_D(QAbstractScrollArea);
   d->_q_hslide(un_named_arg1);
}

void QAbstractScrollArea::_q_vslide(int un_named_arg1)
{
   Q_D(QAbstractScrollArea);
   d->_q_vslide(un_named_arg1);
}

void QAbstractScrollArea::_q_showOrHideScrollBars()
{
   Q_D(QAbstractScrollArea);
   d->_q_showOrHideScrollBars();
}

QT_END_NAMESPACE

#endif // QT_NO_SCROLLAREA
