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

#include <qsizegrip.h>

#ifndef QT_NO_SIZEGRIP

#include <qapplication.h>
#include <qevent.h>
#include <qpainter.h>
#include <qwindow.h>
#include <qplatform_window.h>
#include <qstyle.h>
#include <qstyleoption.h>
#include <qlayout.h>
#include <qdebug.h>
#include <QDesktopWidget>

#include <qwidget_p.h>
#include <qabstractscrollarea.h>

static QWidget *qt_sizegrip_topLevelWidget(QWidget *w)
{
   while (w && !w->isWindow() && w->windowType() != Qt::SubWindow) {
      w = w->parentWidget();
   }
   return w;
}

class QSizeGripPrivate : public QWidgetPrivate
{
   Q_DECLARE_PUBLIC(QSizeGrip)

 public:
   QSizeGripPrivate();

   void init();
   QPoint p;
   QRect r;
   int d;
   int dxMax;
   int dyMax;
   Qt::Corner m_corner;
   bool gotMousePress;
   QPointer<QWidget> tlw;
   Qt::Corner corner() const;
   inline bool atBottom() const {
      return m_corner == Qt::BottomRightCorner || m_corner == Qt::BottomLeftCorner;
   }

   inline bool atLeft() const {
      return m_corner == Qt::BottomLeftCorner || m_corner == Qt::TopLeftCorner;
   }

   void updateTopLevelWidget() {
      Q_Q(QSizeGrip);
      QWidget *w = qt_sizegrip_topLevelWidget(q);
      if (tlw == w) {
         return;
      }
      if (tlw) {
         tlw->removeEventFilter(q);
      }
      tlw = w;
      if (tlw) {
         tlw->installEventFilter(q);
      }
   }

   // This slot is invoked by QLayout when the size grip is added to
   // a layout or reparented after the tlw is shown. This re-implementation is basically
   // the same as QWidgetPrivate::_q_showIfNotHidden except that it checks
   // for Qt::WindowFullScreen and Qt::WindowMaximized as well.

   void _q_showIfNotHidden() {
      Q_Q(QSizeGrip);
      bool showSizeGrip = !(q->isHidden() && q->testAttribute(Qt::WA_WState_ExplicitShowHide));
      updateTopLevelWidget();
      if (tlw && showSizeGrip) {
         Qt::WindowStates sizeGripNotVisibleState = Qt::WindowFullScreen;
         sizeGripNotVisibleState |= Qt::WindowMaximized;
         // Don't show the size grip if the tlw is maximized or in full screen mode.
         showSizeGrip = !(tlw->windowState() & sizeGripNotVisibleState);
      }

      if (showSizeGrip) {
         q->setVisible(true);
      }
   }

   bool m_platformSizeGrip;
};

QSizeGripPrivate::QSizeGripPrivate()
   : dxMax(0), dyMax(0), gotMousePress(false), tlw(nullptr), m_platformSizeGrip(false)
{
}

Qt::Corner QSizeGripPrivate::corner() const
{
   Q_Q(const QSizeGrip);

   QWidget *tmp = qt_sizegrip_topLevelWidget(const_cast<QSizeGrip *>(q));
   const QPoint sizeGripPos = q->mapTo(tmp, QPoint(0, 0));

   bool isAtBottom = sizeGripPos.y() >= tmp->height() / 2;
   bool isAtLeft   = sizeGripPos.x() <= tmp->width() / 2;

   if (isAtLeft) {
      return isAtBottom ? Qt::BottomLeftCorner : Qt::TopLeftCorner;
   } else {
      return isAtBottom ? Qt::BottomRightCorner : Qt::TopRightCorner;
   }
}

QSizeGrip::QSizeGrip(QWidget *parent)
   : QWidget(*new QSizeGripPrivate, parent, Qt::EmptyFlag)
{
   Q_D(QSizeGrip);
   d->init();
}

void QSizeGripPrivate::init()
{
   Q_Q(QSizeGrip);

   m_corner = q->isLeftToRight() ? Qt::BottomRightCorner : Qt::BottomLeftCorner;

   q->setCursor(m_corner == Qt::TopLeftCorner || m_corner == Qt::BottomRightCorner
      ? Qt::SizeFDiagCursor : Qt::SizeBDiagCursor);

   q->setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed));
   updateTopLevelWidget();
}

QSizeGrip::~QSizeGrip()
{
}

QSize QSizeGrip::sizeHint() const
{
   QStyleOption opt(0);
   opt.initFrom(this);

   return (style()->sizeFromContents(QStyle::CT_SizeGrip, &opt, QSize(13, 13), this).
         expandedTo(QApplication::globalStrut()));
}

void QSizeGrip::paintEvent(QPaintEvent *event)
{
   (void) event;

   Q_D(QSizeGrip);

   QPainter painter(this);

   QStyleOptionSizeGrip opt;
   opt.initFrom(this);
   opt.corner = d->m_corner;
   style()->drawControl(QStyle::CE_SizeGrip, &opt, &painter, this);
}

void QSizeGrip::mousePressEvent(QMouseEvent *e)
{
   if (e->button() != Qt::LeftButton) {
      QWidget::mousePressEvent(e);
      return;
   }

   Q_D(QSizeGrip);
   QWidget *tlw = qt_sizegrip_topLevelWidget(this);
   d->p = e->globalPos();
   d->gotMousePress = true;
   d->r = tlw->geometry();

   // Does the platform provide size grip support?
   d->m_platformSizeGrip = false;
   if (tlw->isWindow()
      && tlw->windowHandle()
      && !(tlw->windowFlags() & Qt::X11BypassWindowManagerHint)
      && !tlw->testAttribute(Qt::WA_DontShowOnScreen)
      && !tlw->hasHeightForWidth()) {
      QPlatformWindow *platformWindow = tlw->windowHandle()->handle();
      const QPoint topLevelPos = mapTo(tlw, e->pos());
      d->m_platformSizeGrip = platformWindow && platformWindow->startSystemResize(topLevelPos, d->m_corner);
   }

   if (d->m_platformSizeGrip) {
      return;
   }

   // Find available desktop/workspace geometry.
   QRect availableGeometry;
   bool hasVerticalSizeConstraint = true;
   bool hasHorizontalSizeConstraint = true;

   if (tlw->isWindow()) {
      availableGeometry = QApplication::desktop()->availableGeometry(tlw);

   } else {
      const QWidget *tlwParent = tlw->parentWidget();

      // Check if tlw is inside QAbstractScrollArea/QScrollArea.
      // If that's the case tlw->parentWidget() will return the viewport
      // and tlw->parentWidget()->parentWidget() will return the scroll area.

#ifndef QT_NO_SCROLLAREA
      QAbstractScrollArea *scrollArea = qobject_cast<QAbstractScrollArea *>(tlwParent->parentWidget());
      if (scrollArea) {
         hasHorizontalSizeConstraint = scrollArea->horizontalScrollBarPolicy() == Qt::ScrollBarAlwaysOff;
         hasVerticalSizeConstraint = scrollArea->verticalScrollBarPolicy() == Qt::ScrollBarAlwaysOff;
      }
#endif
      availableGeometry = tlwParent->contentsRect();
   }

   // Find frame geometries, title bar height, and decoration sizes.
   const QRect frameGeometry = tlw->frameGeometry();
   const int titleBarHeight = qMax(tlw->geometry().y() - frameGeometry.y(), 0);
   const int bottomDecoration = qMax(frameGeometry.height() - tlw->height() - titleBarHeight, 0);
   const int leftRightDecoration = qMax((frameGeometry.width() - tlw->width()) / 2, 0);

   // Determine dyMax depending on whether the sizegrip is at the bottom
   // of the widget or not.
   if (d->atBottom()) {
      if (hasVerticalSizeConstraint) {
         d->dyMax = availableGeometry.bottom() - d->r.bottom() - bottomDecoration;
      } else {
         d->dyMax = INT_MAX;
      }
   } else {
      if (hasVerticalSizeConstraint) {
         d->dyMax = availableGeometry.y() - d->r.y() + titleBarHeight;
      } else {
         d->dyMax = -INT_MAX;
      }
   }

   // In RTL mode, the size grip is to the left; find dxMax from the desktop/workspace
   // geometry, the size grip geometry and the width of the decoration.
   if (d->atLeft()) {
      if (hasHorizontalSizeConstraint) {
         d->dxMax = availableGeometry.x() - d->r.x() + leftRightDecoration;
      } else {
         d->dxMax = -INT_MAX;
      }
   } else {
      if (hasHorizontalSizeConstraint) {
         d->dxMax = availableGeometry.right() - d->r.right() - leftRightDecoration;
      } else {
         d->dxMax = INT_MAX;
      }
   }
}

void QSizeGrip::mouseMoveEvent(QMouseEvent *e)
{
   Q_D(QSizeGrip);
   if (e->buttons() != Qt::LeftButton || d->m_platformSizeGrip) {
      QWidget::mouseMoveEvent(e);
      return;
   }

   QWidget *tlw = qt_sizegrip_topLevelWidget(this);
   if (! d->gotMousePress || tlw->testAttribute(Qt::WA_WState_ConfigPending)) {
      return;
   }

   QPoint np(e->globalPos());

   // Don't extend beyond the available geometry; bound to dyMax and dxMax.
   QSize ns;
   if (d->atBottom()) {
      ns.rheight() = d->r.height() + qMin(np.y() - d->p.y(), d->dyMax);
   } else {
      ns.rheight() = d->r.height() - qMax(np.y() - d->p.y(), d->dyMax);
   }

   if (d->atLeft()) {
      ns.rwidth() = d->r.width() - qMax(np.x() - d->p.x(), d->dxMax);
   } else {
      ns.rwidth() = d->r.width() + qMin(np.x() - d->p.x(), d->dxMax);
   }

   ns = QLayout::closestAcceptableSize(tlw, ns);

   QPoint p;
   QRect nr(p, ns);
   if (d->atBottom()) {
      if (d->atLeft()) {
         nr.moveTopRight(d->r.topRight());
      } else {
         nr.moveTopLeft(d->r.topLeft());
      }
   } else {
      if (d->atLeft()) {
         nr.moveBottomRight(d->r.bottomRight());
      } else {
         nr.moveBottomLeft(d->r.bottomLeft());
      }
   }

   tlw->setGeometry(nr);
}

void QSizeGrip::mouseReleaseEvent(QMouseEvent *mouseEvent)
{
   if (mouseEvent->button() == Qt::LeftButton) {
      Q_D(QSizeGrip);
      d->gotMousePress = false;
      d->p = QPoint();
   } else {
      QWidget::mouseReleaseEvent(mouseEvent);
   }
}

void QSizeGrip::moveEvent(QMoveEvent *)
{
   Q_D(QSizeGrip);
   // We're inside a resize operation; no update necessary.
   if (!d->p.isNull()) {
      return;
   }

   d->m_corner = d->corner();


   setCursor(d->m_corner == Qt::TopLeftCorner || d->m_corner == Qt::BottomRightCorner
      ? Qt::SizeFDiagCursor : Qt::SizeBDiagCursor);

}

void QSizeGrip::showEvent(QShowEvent *showEvent)
{
   QWidget::showEvent(showEvent);
}

void QSizeGrip::hideEvent(QHideEvent *hideEvent)
{

   QWidget::hideEvent(hideEvent);
}

void QSizeGrip::setVisible(bool visible)
{
   QWidget::setVisible(visible);
}

bool QSizeGrip::eventFilter(QObject *o, QEvent *e)
{
   Q_D(QSizeGrip);
   if ((isHidden() && testAttribute(Qt::WA_WState_ExplicitShowHide))
      || e->type() != QEvent::WindowStateChange
      || o != d->tlw) {
      return QWidget::eventFilter(o, e);
   }
   Qt::WindowStates sizeGripNotVisibleState = Qt::WindowFullScreen;

   sizeGripNotVisibleState |= Qt::WindowMaximized;

   // Don't show the size grip if the tlw is maximized or in full screen mode.
   setVisible(!(d->tlw->windowState() & sizeGripNotVisibleState));
   setAttribute(Qt::WA_WState_ExplicitShowHide, false);
   return QWidget::eventFilter(o, e);
}

bool QSizeGrip::event(QEvent *event)
{
   return QWidget::event(event);
}

void QSizeGrip::_q_showIfNotHidden()
{
   Q_D(QSizeGrip);
   d->_q_showIfNotHidden();
}

#endif //QT_NO_SIZEGRIP
