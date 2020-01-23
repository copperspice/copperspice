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

#include <qglobal.h>

#ifndef QT_NO_GRAPHICSVIEW

#include <QtCore/qdebug.h>
#include <QtCore/qnumeric.h>
#include <qgraphicswidget_p.h>
#include <qgraphicslayoutitem_p.h>
#include <qgraphicslayout.h>
#include <qgraphicsscene_p.h>
#include <QtGui/qapplication.h>
#include <QtGui/qgraphicsscene.h>
#include <QtGui/qstyleoption.h>
#include <QtGui/QStyleOptionTitleBar>
#include <QtGui/QGraphicsSceneMouseEvent>



void QGraphicsWidgetPrivate::init(QGraphicsItem *parentItem, Qt::WindowFlags wFlags)
{
   Q_Q(QGraphicsWidget);

   attributes = 0;
   isWidget = 1; // QGraphicsItem::isWidget() returns true.
   focusNext = focusPrev = q;
   focusPolicy = Qt::NoFocus;

   adjustWindowFlags(&wFlags);
   windowFlags = wFlags;


   q->setParentItem(parentItem);

   q->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred, QSizePolicy::DefaultType));
   q->setGraphicsItem(q);

   resolveLayoutDirection();
   q->unsetWindowFrameMargins();
   flags |= QGraphicsItem::ItemUsesExtendedStyleOption;
   flags |= QGraphicsItem::ItemSendsGeometryChanges;
   if (windowFlags & Qt::Window) {
      flags |= QGraphicsItem::ItemIsPanel;
   }
}

qreal QGraphicsWidgetPrivate::titleBarHeight(const QStyleOptionTitleBar &options) const
{
   Q_Q(const QGraphicsWidget);
   int height = q->style()->pixelMetric(QStyle::PM_TitleBarHeight, &options);

   return (qreal)height;
}

/*!
    \internal
*/
QGraphicsWidgetPrivate::~QGraphicsWidgetPrivate()
{
   // Remove any lazily allocated data
   delete[] margins;
   delete[] windowFrameMargins;
   delete windowData;
}

/*!
    \internal

     Ensures that margins is allocated.
     This function must be called before any dereferencing.
*/
void QGraphicsWidgetPrivate::ensureMargins() const
{
   if (!margins) {
      margins = new qreal[4];
      for (int i = 0; i < 4; ++i) {
         margins[i] = 0;
      }
   }
}

/*!
    \internal

     Ensures that windowFrameMargins is allocated.
     This function must be called before any dereferencing.
*/
void QGraphicsWidgetPrivate::ensureWindowFrameMargins() const
{
   if (!windowFrameMargins) {
      windowFrameMargins = new qreal[4];
      for (int i = 0; i < 4; ++i) {
         windowFrameMargins[i] = 0;
      }
   }
}

/*!
    \internal

     Ensures that windowData is allocated.
     This function must be called before any dereferencing.
*/
void QGraphicsWidgetPrivate::ensureWindowData()
{
   if (!windowData) {
      windowData = new WindowData;
   }
}

void QGraphicsWidgetPrivate::setPalette_helper(const QPalette &palette)
{
   if (this->palette == palette && this->palette.resolve() == palette.resolve()) {
      return;
   }
   updatePalette(palette);
}

void QGraphicsWidgetPrivate::resolvePalette(uint inheritedMask)
{
   inheritedPaletteResolveMask = inheritedMask;
   QPalette naturalPalette = naturalWidgetPalette();
   QPalette resolvedPalette = palette.resolve(naturalPalette);
   updatePalette(resolvedPalette);
}

void QGraphicsWidgetPrivate::updatePalette(const QPalette &palette)
{
   Q_Q(QGraphicsWidget);
   // Update local palette setting.
   this->palette = palette;

   // Calculate new mask.
   if (q->isWindow() && !q->testAttribute(Qt::WA_WindowPropagation)) {
      inheritedPaletteResolveMask = 0;
   }
   int mask = palette.resolve() | inheritedPaletteResolveMask;

   // Propagate to children.
   for (int i = 0; i < children.size(); ++i) {
      QGraphicsItem *item = children.at(i);
      if (item->isWidget()) {
         QGraphicsWidget *w = static_cast<QGraphicsWidget *>(item);
         if (!w->isWindow() || w->testAttribute(Qt::WA_WindowPropagation)) {
            w->d_func()->resolvePalette(mask);
         }
      } else {
         item->d_ptr->resolvePalette(mask);
      }
   }

   // Notify change.
   QEvent event(QEvent::PaletteChange);
   QApplication::sendEvent(q, &event);
}

void QGraphicsWidgetPrivate::setLayoutDirection_helper(Qt::LayoutDirection direction)
{
   Q_Q(QGraphicsWidget);
   if ((direction == Qt::RightToLeft) == (testAttribute(Qt::WA_RightToLeft))) {
      return;
   }
   q->setAttribute(Qt::WA_RightToLeft, (direction == Qt::RightToLeft));

   // Propagate this change to all children.
   for (int i = 0; i < children.size(); ++i) {
      QGraphicsItem *item = children.at(i);
      if (item->isWidget()) {
         QGraphicsWidget *widget = static_cast<QGraphicsWidget *>(item);
         if (widget->parentWidget() && !widget->testAttribute(Qt::WA_SetLayoutDirection)) {
            widget->d_func()->setLayoutDirection_helper(direction);
         }
      }
   }

   // Send the notification event to this widget item.
   QEvent e(QEvent::LayoutDirectionChange);
   QApplication::sendEvent(q, &e);
}

void QGraphicsWidgetPrivate::resolveLayoutDirection()
{
   Q_Q(QGraphicsWidget);
   if (q->testAttribute(Qt::WA_SetLayoutDirection)) {
      return;
   }
   if (QGraphicsWidget *parentWidget = q->parentWidget()) {
      setLayoutDirection_helper(parentWidget->layoutDirection());
   } else if (scene) {
      // ### shouldn't the scene have a layoutdirection really? how does
      // ### QGraphicsWidget get changes from QApplication::layoutDirection?
      setLayoutDirection_helper(QApplication::layoutDirection());
   } else {
      setLayoutDirection_helper(QApplication::layoutDirection());
   }
}


QPalette QGraphicsWidgetPrivate::naturalWidgetPalette() const
{
   Q_Q(const QGraphicsWidget);
   QPalette palette;
   if (QGraphicsWidget *parent = q->parentWidget()) {
      palette = parent->palette();
   } else if (scene) {
      palette = scene->palette();
   }
   palette.resolve(0);
   return palette;
}

void QGraphicsWidgetPrivate::setFont_helper(const QFont &font)
{
   if (this->font == font && this->font.resolve() == font.resolve()) {
      return;
   }
   updateFont(font);
}

void QGraphicsWidgetPrivate::resolveFont(uint inheritedMask)
{
   Q_Q(QGraphicsWidget);
   inheritedFontResolveMask = inheritedMask;
   if (QGraphicsWidget *p = q->parentWidget()) {
      inheritedFontResolveMask |= p->d_func()->inheritedFontResolveMask;
   }
   QFont naturalFont = naturalWidgetFont();
   QFont resolvedFont = font.resolve(naturalFont);
   updateFont(resolvedFont);
}

void QGraphicsWidgetPrivate::updateFont(const QFont &font)
{
   Q_Q(QGraphicsWidget);
   // Update the local font setting.
   this->font = font;

   // Calculate new mask.
   if (q->isWindow() && !q->testAttribute(Qt::WA_WindowPropagation)) {
      inheritedFontResolveMask = 0;
   }
   int mask = font.resolve() | inheritedFontResolveMask;

   // Propagate to children.
   for (int i = 0; i < children.size(); ++i) {
      QGraphicsItem *item = children.at(i);
      if (item->isWidget()) {
         QGraphicsWidget *w = static_cast<QGraphicsWidget *>(item);
         if (!w->isWindow() || w->testAttribute(Qt::WA_WindowPropagation)) {
            w->d_func()->resolveFont(mask);
         }
      } else {
         item->d_ptr->resolveFont(mask);
      }
   }

   if (!polished) {
      return;
   }
   // Notify change.
   QEvent event(QEvent::FontChange);
   QApplication::sendEvent(q, &event);
}

QFont QGraphicsWidgetPrivate::naturalWidgetFont() const
{
   Q_Q(const QGraphicsWidget);
   QFont naturalFont; // ### no application font support
   if (QGraphicsWidget *parent = q->parentWidget()) {
      naturalFont = parent->font();
   } else if (scene) {
      naturalFont = scene->font();
   }
   naturalFont.resolve(0);
   return naturalFont;
}

void QGraphicsWidgetPrivate::initStyleOptionTitleBar(QStyleOptionTitleBar *option)
{
   Q_Q(QGraphicsWidget);
   ensureWindowData();
   q->initStyleOption(option);
   option->rect.setHeight(titleBarHeight(*option));
   option->titleBarFlags = windowFlags;
   option->subControls = QStyle::SC_TitleBarCloseButton | QStyle::SC_TitleBarLabel | QStyle::SC_TitleBarSysMenu;
   option->activeSubControls = windowData->hoveredSubControl;
   bool isActive = q->isActiveWindow();
   if (isActive) {
      option->state |= QStyle::State_Active;
      option->titleBarState = Qt::WindowActive;
      option->titleBarState |= QStyle::State_Active;
   } else {
      option->state &= ~QStyle::State_Active;
      option->titleBarState = Qt::WindowNoState;
   }
   QFont windowTitleFont = QApplication::font("QMdiSubWindowTitleBar");
   QRect textRect = q->style()->subControlRect(QStyle::CC_TitleBar, option, QStyle::SC_TitleBarLabel, 0);
   option->text = QFontMetrics(windowTitleFont).elidedText(
         windowData->windowTitle, Qt::ElideRight, textRect.width());
}

void QGraphicsWidgetPrivate::adjustWindowFlags(Qt::WindowFlags *flags)
{
   bool customize =  (*flags & (Qt::CustomizeWindowHint
            | Qt::FramelessWindowHint
            | Qt::WindowTitleHint
            | Qt::WindowSystemMenuHint
            | Qt::WindowMinimizeButtonHint
            | Qt::WindowMaximizeButtonHint
            | Qt::WindowContextHelpButtonHint));

   uint type = (*flags & Qt::WindowType_Mask);
   if (customize)
      ;
   else if (type == Qt::Dialog || type == Qt::Sheet) {
      *flags |= Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowContextHelpButtonHint;
   } else if (type == Qt::Tool) {
      *flags |= Qt::WindowTitleHint | Qt::WindowSystemMenuHint;
   } else if (type == Qt::Window || type == Qt::SubWindow)
      *flags |= Qt::WindowTitleHint | Qt::WindowSystemMenuHint
         | Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint;
}

void QGraphicsWidgetPrivate::windowFrameMouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
   Q_Q(QGraphicsWidget);
   ensureWindowData();
   if (windowData->grabbedSection != Qt::NoSection) {
      if (windowData->grabbedSection == Qt::TitleBarArea) {
         windowData->buttonSunken = false;
         QStyleOptionTitleBar bar;
         initStyleOptionTitleBar(&bar);
         // make sure that the coordinates (rect and pos) we send to the style are positive.
         bar.rect = q->windowFrameRect().toRect();
         bar.rect.moveTo(0, 0);
         bar.rect.setHeight(q->style()->pixelMetric(QStyle::PM_TitleBarHeight, &bar));
         QPointF pos = event->pos();
         if (windowFrameMargins) {
            pos.rx() += windowFrameMargins[Left];
            pos.ry() += windowFrameMargins[Top];
         }
         bar.subControls = QStyle::SC_TitleBarCloseButton;
         if (q->style()->subControlRect(QStyle::CC_TitleBar, &bar,
               QStyle::SC_TitleBarCloseButton,
               event->widget()).contains(pos.toPoint())) {
            q->close();
         }
      }
      if (!(static_cast<QGraphicsSceneMouseEvent *>(event)->buttons())) {
         windowData->grabbedSection = Qt::NoSection;
      }
      event->accept();
   }
}

void QGraphicsWidgetPrivate::windowFrameMousePressEvent(QGraphicsSceneMouseEvent *event)
{
   Q_Q(QGraphicsWidget);
   if (event->button() != Qt::LeftButton) {
      return;
   }

   ensureWindowData();
   windowData->startGeometry = q->geometry();
   windowData->grabbedSection = q->windowFrameSectionAt(event->pos());
   ensureWindowData();
   if (windowData->grabbedSection == Qt::TitleBarArea
      && windowData->hoveredSubControl == QStyle::SC_TitleBarCloseButton) {
      windowData->buttonSunken = true;
      q->update();
   }
   event->setAccepted(windowData->grabbedSection != Qt::NoSection);
}

/*!
  Used to calculate the
  Precondition:
  \a widget should support either hfw or wfh

  If \a heightForWidth is set to false, this function will query the width for height
  instead. \a width will then be interpreted as height, \a minh and \a maxh will be interpreted
  as minimum width and maximum width.
 */
static qreal minimumHeightForWidth(qreal width, qreal minh, qreal maxh,
   const QGraphicsWidget *widget,
   bool heightForWidth = true)
{
   qreal minimumHeightForWidth = -1;
   const bool hasHFW = QGraphicsLayoutItemPrivate::get(widget)->hasHeightForWidth();
   if (hasHFW == heightForWidth) {
      minimumHeightForWidth = hasHFW
         ? widget->effectiveSizeHint(Qt::MinimumSize, QSizeF(width, -1)).height()
         : widget->effectiveSizeHint(Qt::MinimumSize, QSizeF(-1, width)).width();    //"width" is here height!
   } else {
      // widthForHeight
      const qreal constraint = width;
      while (maxh - minh > 0.1) {
         qreal middle = minh + (maxh - minh) / 2;
         // ### really bad, if we are a widget with a layout it will call
         // layout->effectiveSizeHint(Qt::MiniumumSize), which again will call
         // sizeHint three times because of how the cache works
         qreal hfw = hasHFW
            ? widget->effectiveSizeHint(Qt::MinimumSize, QSizeF(middle, -1)).height()
            : widget->effectiveSizeHint(Qt::MinimumSize, QSizeF(-1, middle)).width();
         if (hfw > constraint) {
            minh = middle;
         } else if (hfw <= constraint) {
            maxh = middle;
         }
      }
      minimumHeightForWidth = maxh;
   }
   return minimumHeightForWidth;
}

static qreal minimumWidthForHeight(qreal height, qreal minw, qreal maxw,
   const QGraphicsWidget *widget)
{
   return minimumHeightForWidth(height, minw, maxw, widget, false);
}

static QSizeF closestAcceptableSize(const QSizeF &proposed,
   const QGraphicsWidget *widget)
{
   const QSizeF current = widget->size();

   qreal minw = proposed.width();
   qreal maxw = current.width();
   qreal minh = proposed.height();
   qreal maxh = current.height();

   qreal middlew = maxw;
   qreal middleh = maxh;
   qreal min_hfw;
   min_hfw = minimumHeightForWidth(maxw, minh, maxh, widget);

   do {
      if (maxw - minw < 0.1) {
         // we still havent found anything, cut off binary search
         minw = maxw;
         minh = maxh;
      }
      middlew = minw + (maxw - minw) / qreal(2.0);
      middleh = minh + (maxh - minh) / qreal(2.0);

      min_hfw = minimumHeightForWidth(middlew, minh, maxh, widget);

      if (min_hfw > middleh) {
         minw = middlew;
         minh = middleh;
      } else if (min_hfw <= middleh) {
         maxw = middlew;
         maxh = middleh;
      }
   } while (maxw != minw);

   min_hfw = minimumHeightForWidth(middlew, minh, maxh, widget);

   QSizeF result;
   if (min_hfw < maxh) {
      result = QSizeF(middlew, min_hfw);
   } else {
      // Needed because of the cut-off we do above.
      result = QSizeF(minimumWidthForHeight(maxh, proposed.width(), current.width(), widget), maxh);
   }
   return result;
}

static void _q_boundGeometryToSizeConstraints(const QRectF &startGeometry,
   QRectF *rect, Qt::WindowFrameSection section,
   const QSizeF &min, const QSizeF &max,
   const QGraphicsWidget *widget)
{
   const QRectF proposedRect = *rect;
   qreal width = qBound(min.width(), proposedRect.width(), max.width());
   qreal height = qBound(min.height(), proposedRect.height(), max.height());

   const bool hasHFW = QGraphicsLayoutItemPrivate::get(widget)->hasHeightForWidth();
   const bool hasWFH = QGraphicsLayoutItemPrivate::get(widget)->hasWidthForHeight();

   const bool widthChanged = proposedRect.width() != widget->size().width();
   const bool heightChanged = proposedRect.height() != widget->size().height();

   if (hasHFW || hasWFH) {
      if (widthChanged || heightChanged) {
         qreal minExtent;
         qreal maxExtent;
         qreal constraint;
         qreal proposed;
         if (hasHFW) {
            minExtent = min.height();
            maxExtent = max.height();
            constraint = width;
            proposed = proposedRect.height();
         } else {
            // width for height
            minExtent = min.width();
            maxExtent = max.width();
            constraint = height;
            proposed = proposedRect.width();
         }
         if (minimumHeightForWidth(constraint, minExtent, maxExtent, widget, hasHFW) > proposed) {
            QSizeF effectiveSize = closestAcceptableSize(QSizeF(width, height), widget);
            width = effectiveSize.width();
            height = effectiveSize.height();
         }
      }
   }

   switch (section) {
      case Qt::LeftSection:
         rect->setRect(startGeometry.right() - qRound(width), startGeometry.top(),
            qRound(width), startGeometry.height());
         break;
      case Qt::TopLeftSection:
         rect->setRect(startGeometry.right() - qRound(width), startGeometry.bottom() - qRound(height),
            qRound(width), qRound(height));
         break;
      case Qt::TopSection:
         rect->setRect(startGeometry.left(), startGeometry.bottom() - qRound(height),
            startGeometry.width(), qRound(height));
         break;
      case Qt::TopRightSection:
         rect->setTop(rect->bottom() - qRound(height));
         rect->setWidth(qRound(width));
         break;
      case Qt::RightSection:
         rect->setWidth(qRound(width));
         break;
      case Qt::BottomRightSection:
         rect->setWidth(qRound(width));
         rect->setHeight(qRound(height));
         break;
      case Qt::BottomSection:
         rect->setHeight(qRound(height));
         break;
      case Qt::BottomLeftSection:
         rect->setRect(startGeometry.right() - qRound(width), startGeometry.top(),
            qRound(width), qRound(height));
         break;
      default:
         break;
   }
}

void QGraphicsWidgetPrivate::windowFrameMouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
   Q_Q(QGraphicsWidget);
   ensureWindowData();
   if (!(event->buttons() & Qt::LeftButton) || windowData->hoveredSubControl != QStyle::SC_TitleBarLabel) {
      return;
   }

   QLineF delta(q->mapFromScene(event->buttonDownScenePos(Qt::LeftButton)), event->pos());
   QLineF parentDelta(q->mapToParent(delta.p1()), q->mapToParent(delta.p2()));
   QLineF parentXDelta(q->mapToParent(QPointF(delta.p1().x(), 0)), q->mapToParent(QPointF(delta.p2().x(), 0)));
   QLineF parentYDelta(q->mapToParent(QPointF(0, delta.p1().y())), q->mapToParent(QPointF(0, delta.p2().y())));

   QRectF newGeometry;
   switch (windowData->grabbedSection) {
      case Qt::LeftSection:
         newGeometry = QRectF(windowData->startGeometry.topLeft()
               + QPointF(parentXDelta.dx(), parentXDelta.dy()),
               windowData->startGeometry.size() - QSizeF(delta.dx(), delta.dy()));
         break;
      case Qt::TopLeftSection:
         newGeometry = QRectF(windowData->startGeometry.topLeft()
               + QPointF(parentDelta.dx(), parentDelta.dy()),
               windowData->startGeometry.size() - QSizeF(delta.dx(), delta.dy()));
         break;
      case Qt::TopSection:
         newGeometry = QRectF(windowData->startGeometry.topLeft()
               + QPointF(parentYDelta.dx(), parentYDelta.dy()),
               windowData->startGeometry.size() - QSizeF(0, delta.dy()));
         break;
      case Qt::TopRightSection:
         newGeometry = QRectF(windowData->startGeometry.topLeft()
               + QPointF(parentYDelta.dx(), parentYDelta.dy()),
               windowData->startGeometry.size() - QSizeF(-delta.dx(), delta.dy()));
         break;
      case Qt::RightSection:
         newGeometry = QRectF(windowData->startGeometry.topLeft(),
               windowData->startGeometry.size() + QSizeF(delta.dx(), 0));
         break;
      case Qt::BottomRightSection:
         newGeometry = QRectF(windowData->startGeometry.topLeft(),
               windowData->startGeometry.size() + QSizeF(delta.dx(), delta.dy()));
         break;
      case Qt::BottomSection:
         newGeometry = QRectF(windowData->startGeometry.topLeft(),
               windowData->startGeometry.size() + QSizeF(0, delta.dy()));
         break;
      case Qt::BottomLeftSection:
         newGeometry = QRectF(windowData->startGeometry.topLeft()
               + QPointF(parentXDelta.dx(), parentXDelta.dy()),
               windowData->startGeometry.size() - QSizeF(delta.dx(), -delta.dy()));
         break;
      case Qt::TitleBarArea:
         newGeometry = QRectF(windowData->startGeometry.topLeft()
               + QPointF(parentDelta.dx(), parentDelta.dy()),
               windowData->startGeometry.size());
         break;
      case Qt::NoSection:
         break;
   }

   if (windowData->grabbedSection != Qt::NoSection) {
      _q_boundGeometryToSizeConstraints(windowData->startGeometry, &newGeometry,
         windowData->grabbedSection,
         q->effectiveSizeHint(Qt::MinimumSize),
         q->effectiveSizeHint(Qt::MaximumSize),
         q);
      q->setGeometry(newGeometry);
   }
}

void QGraphicsWidgetPrivate::windowFrameHoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
   Q_Q(QGraphicsWidget);
   if (!hasDecoration()) {
      return;
   }

   ensureWindowData();

   if (q->rect().contains(event->pos())) {
      if (windowData->buttonMouseOver || windowData->hoveredSubControl != QStyle::SC_None) {
         windowFrameHoverLeaveEvent(event);
      }
      return;
   }

   bool wasMouseOver = windowData->buttonMouseOver;
   QRect oldButtonRect = windowData->buttonRect;
   windowData->buttonRect = QRect();
   windowData->buttonMouseOver = false;
   QPointF pos = event->pos();
   QStyleOptionTitleBar bar;
   // make sure that the coordinates (rect and pos) we send to the style are positive.
   if (windowFrameMargins) {
      pos.rx() += windowFrameMargins[Left];
      pos.ry() += windowFrameMargins[Top];
   }
   initStyleOptionTitleBar(&bar);
   bar.rect = q->windowFrameRect().toRect();
   bar.rect.moveTo(0, 0);
   bar.rect.setHeight(int(titleBarHeight(bar)));

   Qt::CursorShape cursorShape = Qt::ArrowCursor;
   bool needsSetCursorCall = true;
   switch (q->windowFrameSectionAt(event->pos())) {
      case Qt::TopLeftSection:
      case Qt::BottomRightSection:
         cursorShape = Qt::SizeFDiagCursor;
         break;
      case Qt::TopRightSection:
      case Qt::BottomLeftSection:
         cursorShape = Qt::SizeBDiagCursor;
         break;
      case Qt::LeftSection:
      case Qt::RightSection:
         cursorShape = Qt::SizeHorCursor;
         break;
      case Qt::TopSection:
      case Qt::BottomSection:
         cursorShape = Qt::SizeVerCursor;
         break;
      case Qt::TitleBarArea:
         windowData->buttonRect = q->style()->subControlRect(
               QStyle::CC_TitleBar, &bar, QStyle::SC_TitleBarCloseButton, 0);
         if (windowData->buttonRect.contains(pos.toPoint())) {
            windowData->buttonMouseOver = true;
         }
         event->ignore();
         break;
      default:
         needsSetCursorCall = false;
         event->ignore();
   }
#ifndef QT_NO_CURSOR
   if (needsSetCursorCall) {
      q->setCursor(cursorShape);
   }
#endif
   // update buttons if we hover over them
   windowData->hoveredSubControl = q->style()->hitTestComplexControl(QStyle::CC_TitleBar, &bar, pos.toPoint(), 0);
   if (windowData->hoveredSubControl != QStyle::SC_TitleBarCloseButton) {
      windowData->hoveredSubControl = QStyle::SC_TitleBarLabel;
   }

   if (windowData->buttonMouseOver != wasMouseOver) {
      if (!oldButtonRect.isNull()) {
         q->update(QRectF(oldButtonRect).translated(q->windowFrameRect().topLeft()));
      }
      if (!windowData->buttonRect.isNull()) {
         q->update(QRectF(windowData->buttonRect).translated(q->windowFrameRect().topLeft()));
      }
   }
}

void QGraphicsWidgetPrivate::windowFrameHoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
   Q_UNUSED(event);
   Q_Q(QGraphicsWidget);
   if (hasDecoration()) {
      // ### restore the cursor, don't override it
#ifndef QT_NO_CURSOR
      q->unsetCursor();
#endif

      ensureWindowData();

      bool needsUpdate = false;
      if (windowData->hoveredSubControl == QStyle::SC_TitleBarCloseButton
         || windowData->buttonMouseOver) {
         needsUpdate = true;
      }

      // update the hover state (of buttons etc...)
      windowData->hoveredSubControl = QStyle::SC_None;
      windowData->buttonMouseOver = false;
      windowData->buttonRect = QRect();
      if (needsUpdate) {
         q->update(windowData->buttonRect);
      }
   }
}

bool QGraphicsWidgetPrivate::hasDecoration() const
{
   return (windowFlags & Qt::Window) && (windowFlags & Qt::WindowTitleHint);
}

/**
 * is called after a reparent has taken place to fix up the focus chain(s)
 */
void QGraphicsWidgetPrivate::fixFocusChainBeforeReparenting(QGraphicsWidget *newParent, QGraphicsScene *oldScene,
   QGraphicsScene *newScene)
{
   Q_Q(QGraphicsWidget);

   Q_ASSERT(focusNext && focusPrev);

   if (q_ptr->isPanel()) {
      // panels are never a part of their parent's or ancestors' focus
      // chains. so reparenting a panel is easy; there's nothing to do.
      return;
   }
   // we're not a panel, so find the first widget in the focus chain
   // (this), and the last (this, or the last widget that is still
   // a descendent of this). also find the widgets that currently /
   // before reparenting point to this widgets' focus chain.
   QGraphicsWidget *focusFirst = q;
   QGraphicsWidget *focusBefore = focusPrev;
   QGraphicsWidget *focusLast = focusFirst;
   QGraphicsWidget *focusAfter = focusNext;
   do {
      if (!q->isAncestorOf(focusAfter)) {
         break;
      }
      focusLast = focusAfter;
   } while ((focusAfter = focusAfter->d_func()->focusNext));

   if (!parent && oldScene && oldScene != newScene && oldScene->d_func()->tabFocusFirst == q) {
      // detach from old scene's top level focus chain.
      oldScene->d_func()->tabFocusFirst = (focusAfter != q) ? focusAfter : 0;
   }

   // detach from current focus chain; skip this widget subtree.
   focusBefore->d_func()->focusNext = focusAfter;
   focusAfter->d_func()->focusPrev = focusBefore;
   if (newParent) {
      // attach to new parent's focus chain as the last element
      // in its chain.
      QGraphicsWidget *newFocusFirst = newParent;
      QGraphicsWidget *newFocusLast = newFocusFirst;
      QGraphicsWidget *newFocusAfter = newFocusFirst->d_func()->focusNext;
      do {
         if (!newParent->isAncestorOf(newFocusAfter)) {
            break;
         }
         newFocusLast = newFocusAfter;
      } while ((newFocusAfter = newFocusAfter->d_func()->focusNext));

      newFocusLast->d_func()->focusNext = q;
      focusLast->d_func()->focusNext = newFocusAfter;
      newFocusAfter->d_func()->focusPrev = focusLast;
      focusPrev = newFocusLast;
   } else {
      // no new parent, so just link up our own prev->last widgets.
      focusPrev = focusLast;
      focusLast->d_func()->focusNext = q;
   }

}

void QGraphicsWidgetPrivate::setLayout_helper(QGraphicsLayout *l)
{
   delete (this->layout);
   layout = l;
   if (!l) {
      Q_Q(QGraphicsWidget);
      q->updateGeometry();
   }
}

qreal QGraphicsWidgetPrivate::width() const
{
   Q_Q(const QGraphicsWidget);
   return q->geometry().width();
}

void QGraphicsWidgetPrivate::setWidth(qreal w)
{
   if (qIsNaN(w)) {
      return;
   }
   Q_Q(QGraphicsWidget);
   if (q->geometry().width() == w) {
      return;
   }

   q->setGeometry(QRectF(q->x(), q->y(), w, height()));
}

void QGraphicsWidgetPrivate::resetWidth()
{
   Q_Q(QGraphicsWidget);
   q->setGeometry(QRectF(q->x(), q->y(), 0, height()));
}

qreal QGraphicsWidgetPrivate::height() const
{
   Q_Q(const QGraphicsWidget);
   return q->geometry().height();
}

void QGraphicsWidgetPrivate::setHeight(qreal h)
{
   if (qIsNaN(h)) {
      return;
   }
   Q_Q(QGraphicsWidget);
   if (q->geometry().height() == h) {
      return;
   }

   q->setGeometry(QRectF(q->x(), q->y(), width(), h));
}

void QGraphicsWidgetPrivate::resetHeight()
{
   Q_Q(QGraphicsWidget);
   q->setGeometry(QRectF(q->x(), q->y(), width(), 0));
}

void QGraphicsWidgetPrivate::setGeometryFromSetPos()
{
   if (inSetGeometry) {
      return;
   }
   Q_Q(QGraphicsWidget);
   inSetPos = 1;
   // Ensure setGeometry is called (avoid recursion when setPos is
   // called from within setGeometry).
   q->setGeometry(QRectF(pos, q->size()));
   inSetPos = 0 ;
}


#endif //QT_NO_GRAPHICSVIEW
