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

#include <qprintpreviewwidget.h>
#include <qwidget_p.h>
#include <qprinter_p.h>

#include <qmath.h>
#include <qboxlayout.h>
#include <qgraphicsitem.h>
#include <qgraphicsview.h>
#include <qscrollbar.h>
#include <qstyleoption.h>

#ifndef QT_NO_PRINTPREVIEWWIDGET

class PageItem : public QGraphicsItem
{
 public:
   PageItem(int _pageNum, const QPicture *_pagePicture, QSize _paperSize, QRect _pageRect)
      : pageNum(_pageNum), pagePicture(_pagePicture), paperSize(_paperSize), pageRect(_pageRect) {

      qreal border = qMax(paperSize.height(), paperSize.width()) / 25;

      brect = QRectF(QPointF(-border, -border), QSizeF(paperSize) + QSizeF(2 * border, 2 * border));
      setCacheMode(DeviceCoordinateCache);
   }

   QRectF boundingRect() const override {
      return brect;
   }

   int pageNumber() const {
      return pageNum;
   }

   void paint(QPainter *painter, const QStyleOptionGraphicsItem *item, QWidget *widget) override;

 private:
   int pageNum;
   const QPicture *pagePicture;
   QSize paperSize;
   QRect pageRect;
   QRectF brect;
};

void PageItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
   Q_UNUSED(widget);

   QRectF paperRect(0, 0, paperSize.width(), paperSize.height());

   // Draw shadow
   painter->setClipRect(option->exposedRect);
   qreal shWidth = paperRect.width() / 100;
   QRectF rshadow(paperRect.topRight() + QPointF(0, shWidth),
                  paperRect.bottomRight() + QPointF(shWidth, 0));

   QLinearGradient rgrad(rshadow.topLeft(), rshadow.topRight());
   rgrad.setColorAt(0.0, QColor(0, 0, 0, 255));
   rgrad.setColorAt(1.0, QColor(0, 0, 0, 0));
   painter->fillRect(rshadow, QBrush(rgrad));
   QRectF bshadow(paperRect.bottomLeft() + QPointF(shWidth, 0),
                  paperRect.bottomRight() + QPointF(0, shWidth));
   QLinearGradient bgrad(bshadow.topLeft(), bshadow.bottomLeft());
   bgrad.setColorAt(0.0, QColor(0, 0, 0, 255));
   bgrad.setColorAt(1.0, QColor(0, 0, 0, 0));
   painter->fillRect(bshadow, QBrush(bgrad));
   QRectF cshadow(paperRect.bottomRight(),
                  paperRect.bottomRight() + QPointF(shWidth, shWidth));
   QRadialGradient cgrad(cshadow.topLeft(), shWidth, cshadow.topLeft());
   cgrad.setColorAt(0.0, QColor(0, 0, 0, 255));
   cgrad.setColorAt(1.0, QColor(0, 0, 0, 0));
   painter->fillRect(cshadow, QBrush(cgrad));

   painter->setClipRect(paperRect & option->exposedRect);
   painter->fillRect(paperRect, Qt::white);
   if (! pagePicture) {
      return;
   }
   painter->drawPicture(pageRect.topLeft(), *pagePicture);

   // Effect: make anything drawn in the margins look washed out.
   QPainterPath path;
   path.addRect(paperRect);
   path.addRect(pageRect);
   painter->setPen(QPen(Qt::NoPen));
   painter->setBrush(QColor(255, 255, 255, 180));
   painter->drawPath(path);

   // todo: drawtext "Page N" below paper
}

class GraphicsView : public QGraphicsView
{
   GUI_CS_OBJECT(GraphicsView)

 public:
   GraphicsView(QWidget *parent = nullptr)
      : QGraphicsView(parent) {

#ifdef Q_OS_MAC
      setFrameStyle(QFrame::NoFrame);
#endif

   }

   GUI_CS_SIGNAL_1(Public, void resized())
   GUI_CS_SIGNAL_2(resized)

 protected:
   void resizeEvent(QResizeEvent *e) override {
      QGraphicsView::resizeEvent(e);
      emit resized();
   }

   void showEvent(QShowEvent *e) override {
      QGraphicsView::showEvent(e);
      emit resized();
   }
};

class QPrintPreviewWidgetPrivate : public QWidgetPrivate
{
   Q_DECLARE_PUBLIC(QPrintPreviewWidget)

 public:
   QPrintPreviewWidgetPrivate()
      : scene(0), curPage(1),
        viewMode(QPrintPreviewWidget::SinglePageView),
        zoomMode(QPrintPreviewWidget::FitInView),
        zoomFactor(1), initialized(false), fitting(true) {
   }

   // private slots
   void _q_fit(bool doFitting = false);
   void _q_updateCurrentPage();

   void init();
   void populateScene();
   void layoutPages();
   void generatePreview();
   void setCurrentPage(int pageNumber);
   void zoom(qreal zoom);
   void setZoomFactor(qreal zoomFactor);
   int calcCurrentPage();

   GraphicsView *graphicsView;
   QGraphicsScene *scene;

   int curPage;
   QList<const QPicture *> pictures;
   QList<QGraphicsItem *> pages;

   QPrintPreviewWidget::ViewMode viewMode;
   QPrintPreviewWidget::ZoomMode zoomMode;
   qreal zoomFactor;
   bool ownPrinter;
   QPrinter *printer;
   bool initialized;
   bool fitting;
};

void QPrintPreviewWidgetPrivate::_q_fit(bool doFitting)
{
   Q_Q(QPrintPreviewWidget);

   if (curPage < 1 || curPage > pages.count()) {
      return;
   }

   if (!doFitting && !fitting) {
      return;
   }

   if (doFitting && fitting) {
      QRect viewRect = graphicsView->viewport()->rect();
      if (zoomMode == QPrintPreviewWidget::FitInView) {
         QList<QGraphicsItem *> containedItems = graphicsView->items(viewRect, Qt::ContainsItemBoundingRect);

         for (QGraphicsItem * item : containedItems) {
            PageItem *pg = static_cast<PageItem *>(item);
            if (pg->pageNumber() == curPage) {
               return;
            }
         }
      }

      int newPage = calcCurrentPage();
      if (newPage != curPage) {
         curPage = newPage;
      }
   }

   QRectF target = pages.at(curPage - 1)->sceneBoundingRect();
   if (viewMode == QPrintPreviewWidget::FacingPagesView) {
      // fit two pages
      if (curPage % 2) {
         target.setLeft(target.left() - target.width());
      } else {
         target.setRight(target.right() + target.width());
      }
   } else if (viewMode == QPrintPreviewWidget::AllPagesView) {
      target = scene->itemsBoundingRect();
   }

   if (zoomMode == QPrintPreviewWidget::FitToWidth) {
      QTransform t;
      qreal scale = graphicsView->viewport()->width() / target.width();
      t.scale(scale, scale);
      graphicsView->setTransform(t);
      if (doFitting && fitting) {
         QRectF viewSceneRect = graphicsView->viewportTransform().mapRect(graphicsView->viewport()->rect());
         viewSceneRect.moveTop(target.top());
         graphicsView->ensureVisible(viewSceneRect); // Nah...
      }
   } else {
      graphicsView->fitInView(target, Qt::KeepAspectRatio);
      if (zoomMode == QPrintPreviewWidget::FitInView) {
         int step = qRound(graphicsView->matrix().mapRect(target).height());
         graphicsView->verticalScrollBar()->setSingleStep(step);
         graphicsView->verticalScrollBar()->setPageStep(step);
      }
   }

   zoomFactor = graphicsView->transform().m11() * (float(printer->logicalDpiY()) / q->logicalDpiY());
   emit q->previewChanged();
}

void QPrintPreviewWidgetPrivate::_q_updateCurrentPage()
{
   Q_Q(QPrintPreviewWidget);

   if (viewMode == QPrintPreviewWidget::AllPagesView) {
      return;
   }

   int newPage = calcCurrentPage();
   if (newPage != curPage) {
      curPage = newPage;
      emit q->previewChanged();
   }
}

int QPrintPreviewWidgetPrivate::calcCurrentPage()
{
   int maxArea = 0;
   int newPage = curPage;
   QRect viewRect = graphicsView->viewport()->rect();
   QList<QGraphicsItem *> items = graphicsView->items(viewRect);
   for (int i = 0; i < items.size(); ++i) {
      PageItem *pg = static_cast<PageItem *>(items.at(i));
      QRect overlap = graphicsView->mapFromScene(pg->sceneBoundingRect()).boundingRect() & viewRect;
      int area = overlap.width() * overlap.height();
      if (area > maxArea) {
         maxArea = area;
         newPage = pg->pageNumber();
      } else if (area == maxArea && pg->pageNumber() < newPage) {
         newPage = pg->pageNumber();
      }
   }
   return newPage;
}

void QPrintPreviewWidgetPrivate::init()
{
   Q_Q(QPrintPreviewWidget);

   graphicsView = new GraphicsView;
   graphicsView->setInteractive(false);
   graphicsView->setDragMode(QGraphicsView::ScrollHandDrag);
   graphicsView->setViewportUpdateMode(QGraphicsView::SmartViewportUpdate);
   QObject::connect(graphicsView->verticalScrollBar(), SIGNAL(valueChanged(int)),
                    q, SLOT(_q_updateCurrentPage()));
   QObject::connect(graphicsView, SIGNAL(resized()), q, SLOT(_q_fit()));

   scene = new QGraphicsScene(graphicsView);
   scene->setBackgroundBrush(Qt::gray);
   graphicsView->setScene(scene);

   QVBoxLayout *layout = new QVBoxLayout;
   q->setLayout(layout);
   layout->setContentsMargins(0, 0, 0, 0);
   layout->addWidget(graphicsView);
}

void QPrintPreviewWidgetPrivate::populateScene()
{
   // remove old pages
   for (int i = 0; i < pages.size(); i++) {
      scene->removeItem(pages.at(i));
   }
   qDeleteAll(pages);
   pages.clear();

   int numPages = pictures.count();
   QSize paperSize = printer->paperRect().size();
   QRect pageRect = printer->pageRect();

   for (int i = 0; i < numPages; i++) {
      PageItem *item = new PageItem(i + 1, pictures.at(i), paperSize, pageRect);
      scene->addItem(item);
      pages.append(item);
   }
}

void QPrintPreviewWidgetPrivate::layoutPages()
{
   int numPages = pages.count();
   if (numPages < 1) {
      return;
   }

   int numPagePlaces = numPages;
   int cols = 1; // singleMode and default
   if (viewMode == QPrintPreviewWidget::AllPagesView) {
      if (printer->orientation() == QPrinter::Portrait) {
         cols = qCeil(qSqrt((float) numPages));
      } else {
         cols = qFloor(qSqrt((float) numPages));
      }
      cols += cols % 2;  // Nicer with an even number of cols
   } else if (viewMode == QPrintPreviewWidget::FacingPagesView) {
      cols = 2;
      numPagePlaces += 1;
   }
   int rows = qCeil(qreal(numPagePlaces) / cols);

   qreal itemWidth = pages.at(0)->boundingRect().width();
   qreal itemHeight = pages.at(0)->boundingRect().height();
   int pageNum = 1;
   for (int i = 0; i < rows && pageNum <= numPages; i++) {
      for (int j = 0; j < cols && pageNum <= numPages; j++) {
         if (!i && !j && viewMode == QPrintPreviewWidget::FacingPagesView) {
            // Front page doesn't have a facing page
            continue;
         } else {
            pages.at(pageNum - 1)->setPos(QPointF(j * itemWidth, i * itemHeight));
            pageNum++;
         }
      }
   }
   scene->setSceneRect(scene->itemsBoundingRect());
}

void QPrintPreviewWidgetPrivate::generatePreview()
{
   //### If QPrinter::setPreviewMode() becomes public, handle the
   //### case that we have been constructed with a printer that
   //### _already_ has been preview-painted to, so we should
   //### initially just show the pages it already contains, and not
   //### emit paintRequested() until the user changes some parameter

   Q_Q(QPrintPreviewWidget);
   printer->d_func()->setPreviewMode(true);
   emit q->paintRequested(printer);
   printer->d_func()->setPreviewMode(false);
   pictures = printer->d_func()->previewPages();
   populateScene(); // i.e. setPreviewPrintedPictures() e.l.
   layoutPages();
   curPage = qBound(1, curPage, pages.count());
   if (fitting) {
      _q_fit();
   }
   emit q->previewChanged();
}

void QPrintPreviewWidgetPrivate::setCurrentPage(int pageNumber)
{
   if (pageNumber < 1 || pageNumber > pages.count()) {
      return;
   }

   int lastPage = curPage;
   curPage = pageNumber;

   if (lastPage != curPage && lastPage > 0 && lastPage <= pages.count()) {
      if (zoomMode != QPrintPreviewWidget::FitInView) {
         QScrollBar *hsc = graphicsView->horizontalScrollBar();
         QScrollBar *vsc = graphicsView->verticalScrollBar();
         QPointF pt = graphicsView->transform().map(pages.at(curPage - 1)->pos());
         vsc->setValue(int(pt.y()) - 10);
         hsc->setValue(int(pt.x()) - 10);
      } else {
         graphicsView->centerOn(pages.at(curPage - 1));
      }
   }
}

void QPrintPreviewWidgetPrivate::zoom(qreal zoom)
{
   zoomFactor *= zoom;
   graphicsView->scale(zoom, zoom);
}

void QPrintPreviewWidgetPrivate::setZoomFactor(qreal _zoomFactor)
{
   Q_Q(QPrintPreviewWidget);
   zoomFactor = _zoomFactor;
   graphicsView->resetTransform();
   int dpi_y = q->logicalDpiY();
   int printer_dpi_y = printer->logicalDpiY();
   graphicsView->scale(zoomFactor * (dpi_y / float(printer_dpi_y)),
                       zoomFactor * (dpi_y / float(printer_dpi_y)));
}

QPrintPreviewWidget::QPrintPreviewWidget(QPrinter *printer, QWidget *parent, Qt::WindowFlags flags)
   : QWidget(*new QPrintPreviewWidgetPrivate, parent, flags)
{
   Q_D(QPrintPreviewWidget);
   d->printer = printer;
   d->ownPrinter = false;
   d->init();
}

QPrintPreviewWidget::QPrintPreviewWidget(QWidget *parent, Qt::WindowFlags flags)
   : QWidget(*new QPrintPreviewWidgetPrivate, parent, flags)
{
   Q_D(QPrintPreviewWidget);
   d->printer = new QPrinter;
   d->ownPrinter = true;
   d->init();
}


/*!
    Destroys the QPrintPreviewWidget.
*/
QPrintPreviewWidget::~QPrintPreviewWidget()
{
   Q_D(QPrintPreviewWidget);
   if (d->ownPrinter) {
      delete d->printer;
   }
}

/*!
    Returns the current view mode. The default view mode is SinglePageView.
*/
QPrintPreviewWidget::ViewMode QPrintPreviewWidget::viewMode() const
{
   Q_D(const QPrintPreviewWidget);
   return d->viewMode;
}

/*!
    Sets the view mode to \a mode. The default view mode is
    SinglePageView.
*/
void QPrintPreviewWidget::setViewMode(ViewMode mode)
{
   Q_D(QPrintPreviewWidget);
   d->viewMode = mode;
   d->layoutPages();
   if (d->viewMode == AllPagesView) {
      d->graphicsView->fitInView(d->scene->itemsBoundingRect(), Qt::KeepAspectRatio);
      d->fitting = false;
      d->zoomMode = QPrintPreviewWidget::CustomZoom;
      d->zoomFactor = d->graphicsView->transform().m11() * (float(d->printer->logicalDpiY()) / logicalDpiY());
      emit previewChanged();
   } else {
      d->fitting = true;
      d->_q_fit();
   }
}

/*!
    Returns the current orientation of the preview. This value is
    obtained from the QPrinter object associated with the preview.
*/
QPrinter::Orientation QPrintPreviewWidget::orientation() const
{
   Q_D(const QPrintPreviewWidget);
   return d->printer->orientation();
}

/*!
    Sets the current orientation to \a orientation. This value will be
    set on the QPrinter object associated with the preview.
*/
void QPrintPreviewWidget::setOrientation(QPrinter::Orientation orientation)
{
   Q_D(QPrintPreviewWidget);
   d->printer->setOrientation(orientation);
   d->generatePreview();
}

/*!
    Prints the preview to the printer associated with the preview.
*/
void QPrintPreviewWidget::print()
{
   Q_D(QPrintPreviewWidget);
   // ### make use of the generated pages
   emit paintRequested(d->printer);
}

/*!
    Zooms the current view in by \a factor. The default value for \a
    factor is 1.1, which means the view will be scaled up by 10%.
*/
void QPrintPreviewWidget::zoomIn(qreal factor)
{
   Q_D(QPrintPreviewWidget);
   d->fitting = false;
   d->zoomMode = QPrintPreviewWidget::CustomZoom;
   d->zoom(factor);
}

/*!
    Zooms the current view out by \a factor. The default value for \a
    factor is 1.1, which means the view will be scaled down by 10%.
*/
void QPrintPreviewWidget::zoomOut(qreal factor)
{
   Q_D(QPrintPreviewWidget);
   d->fitting = false;
   d->zoomMode = QPrintPreviewWidget::CustomZoom;
   d->zoom(1 / factor);
}

/*!
    Returns the zoom factor of the view.
*/
qreal QPrintPreviewWidget::zoomFactor() const
{
   Q_D(const QPrintPreviewWidget);
   return d->zoomFactor;
}

/*!
    Sets the zoom factor of the view to \a factor. For example, a
    value of 1.0 indicates an unscaled view, which is approximately
    the size the view will have on paper. A value of 0.5 will halve
    the size of the view, while a value of 2.0 will double the size of
    the view.
*/
void QPrintPreviewWidget::setZoomFactor(qreal factor)
{
   Q_D(QPrintPreviewWidget);
   d->fitting = false;
   d->zoomMode = QPrintPreviewWidget::CustomZoom;
   d->setZoomFactor(factor);
}

/*!
    \obsolete
    Returns the number of pages in the preview.
    \sa pageCount()
*/
int QPrintPreviewWidget::numPages() const
{
   Q_D(const QPrintPreviewWidget);
   return d->pages.size();
}

int QPrintPreviewWidget::pageCount() const
{
   Q_D(const QPrintPreviewWidget);
   return d->pages.size();
}

/*!
    Returns the currently viewed page in the preview.
*/
int QPrintPreviewWidget::currentPage() const
{
   Q_D(const QPrintPreviewWidget);
   return d->curPage;
}

/*!
    Sets the current page in the preview. This will cause the view to
    skip to the beginning of \a page.
*/
void QPrintPreviewWidget::setCurrentPage(int page)
{
   Q_D(QPrintPreviewWidget);
   d->setCurrentPage(page);
}

/*!
    This is a convenience function and is the same as calling \c
    {setZoomMode(QPrintPreviewWidget::FitToWidth)}.
*/
void QPrintPreviewWidget::fitToWidth()
{
   setZoomMode(FitToWidth);
}

/*!
    This is a convenience function and is the same as calling \c
    {setZoomMode(QPrintPreviewWidget::FitInView)}.
*/
void QPrintPreviewWidget::fitInView()
{
   setZoomMode(FitInView);
}

void QPrintPreviewWidget::setZoomMode(QPrintPreviewWidget::ZoomMode zoomMode)
{
   Q_D(QPrintPreviewWidget);
   d->zoomMode = zoomMode;
   if (d->zoomMode == FitInView || d->zoomMode == FitToWidth) {
      d->fitting = true;
      d->_q_fit(true);
   } else {
      d->fitting = false;
   }
}

QPrintPreviewWidget::ZoomMode QPrintPreviewWidget::zoomMode() const
{
   Q_D(const QPrintPreviewWidget);
   return d->zoomMode;
}

/*!
    This is a convenience function and is the same as calling \c
    {setOrientation(QPrinter::Landscape)}.
*/
void QPrintPreviewWidget::setLandscapeOrientation()
{
   setOrientation(QPrinter::Landscape);
}

/*!
    This is a convenience function and is the same as calling \c
    {setOrientation(QPrinter::Portrait)}.
*/
void QPrintPreviewWidget::setPortraitOrientation()
{
   setOrientation(QPrinter::Portrait);
}

/*!
    This is a convenience function and is the same as calling \c
    {setViewMode(QPrintPreviewWidget::SinglePageView)}.
*/
void QPrintPreviewWidget::setSinglePageViewMode()
{
   setViewMode(SinglePageView);
}

/*!
    This is a convenience function and is the same as calling \c
    {setViewMode(QPrintPreviewWidget::FacingPagesView)}.
*/
void QPrintPreviewWidget::setFacingPagesViewMode()
{
   setViewMode(FacingPagesView);
}

/*!
    This is a convenience function and is the same as calling \c
    {setViewMode(QPrintPreviewWidget::AllPagesView)}.
*/
void QPrintPreviewWidget::setAllPagesViewMode()
{
   setViewMode(AllPagesView);
}

void QPrintPreviewWidget::updatePreview()
{
   Q_D(QPrintPreviewWidget);
   d->initialized = true;
   d->generatePreview();
   d->graphicsView->updateGeometry();
}

/*! \reimp
*/
void QPrintPreviewWidget::setVisible(bool visible)
{
   Q_D(QPrintPreviewWidget);
   if (visible && !d->initialized) {
      updatePreview();
   }
   QWidget::setVisible(visible);
}

void QPrintPreviewWidget::_q_fit()
{
   Q_D(QPrintPreviewWidget);
   d->_q_fit();
}

void QPrintPreviewWidget::_q_updateCurrentPage()
{
   Q_D(QPrintPreviewWidget);
   d->_q_updateCurrentPage();
}

#endif // QT_NO_PRINTPREVIEWWIDGET
