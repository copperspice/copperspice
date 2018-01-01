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

#ifndef QPRINTPREVIEWWIDGET_H
#define QPRINTPREVIEWWIDGET_H

#include <QtGui/qwidget.h>
#include <QtGui/qprinter.h>

#ifndef QT_NO_PRINTPREVIEWWIDGET

QT_BEGIN_NAMESPACE

class QPrintPreviewWidgetPrivate;

class Q_GUI_EXPORT QPrintPreviewWidget : public QWidget
{
   GUI_CS_OBJECT(QPrintPreviewWidget)
   Q_DECLARE_PRIVATE(QPrintPreviewWidget)

 public:

   enum ViewMode {
      SinglePageView,
      FacingPagesView,
      AllPagesView
   };

   enum ZoomMode {
      CustomZoom,
      FitToWidth,
      FitInView
   };

   explicit QPrintPreviewWidget(QPrinter *printer, QWidget *parent = nullptr, Qt::WindowFlags flags = 0);
   explicit QPrintPreviewWidget(QWidget *parent = nullptr, Qt::WindowFlags flags = 0);
   ~QPrintPreviewWidget();

   qreal zoomFactor() const;
   QPrinter::Orientation orientation() const;
   ViewMode viewMode() const;
   ZoomMode zoomMode() const;
   int currentPage() const;

#ifdef QT_DEPRECATED
   QT_DEPRECATED int numPages() const;
#endif

   int pageCount() const;
   void setVisible(bool visible) override;

   GUI_CS_SLOT_1(Public, void print())
   GUI_CS_SLOT_2(print)

   GUI_CS_SLOT_1(Public, void zoomIn(qreal zoom = 1.1))
   GUI_CS_SLOT_2(zoomIn)
   GUI_CS_SLOT_1(Public, void zoomOut(qreal zoom = 1.1))
   GUI_CS_SLOT_2(zoomOut)
   GUI_CS_SLOT_1(Public, void setZoomFactor(qreal zoomFactor))
   GUI_CS_SLOT_2(setZoomFactor)
   GUI_CS_SLOT_1(Public, void setOrientation(QPrinter::Orientation orientation))
   GUI_CS_SLOT_2(setOrientation)
   GUI_CS_SLOT_1(Public, void setViewMode(ViewMode viewMode))
   GUI_CS_SLOT_2(setViewMode)
   GUI_CS_SLOT_1(Public, void setZoomMode(ZoomMode zoomMode))
   GUI_CS_SLOT_2(setZoomMode)
   GUI_CS_SLOT_1(Public, void setCurrentPage(int pageNumber))
   GUI_CS_SLOT_2(setCurrentPage)

   GUI_CS_SLOT_1(Public, void fitToWidth())
   GUI_CS_SLOT_2(fitToWidth)
   GUI_CS_SLOT_1(Public, void fitInView())
   GUI_CS_SLOT_2(fitInView)
   GUI_CS_SLOT_1(Public, void setLandscapeOrientation())
   GUI_CS_SLOT_2(setLandscapeOrientation)
   GUI_CS_SLOT_1(Public, void setPortraitOrientation())
   GUI_CS_SLOT_2(setPortraitOrientation)
   GUI_CS_SLOT_1(Public, void setSinglePageViewMode())
   GUI_CS_SLOT_2(setSinglePageViewMode)
   GUI_CS_SLOT_1(Public, void setFacingPagesViewMode())
   GUI_CS_SLOT_2(setFacingPagesViewMode)
   GUI_CS_SLOT_1(Public, void setAllPagesViewMode())
   GUI_CS_SLOT_2(setAllPagesViewMode)

   GUI_CS_SLOT_1(Public, void updatePreview())
   GUI_CS_SLOT_2(updatePreview)

   GUI_CS_SIGNAL_1(Public, void paintRequested(QPrinter *printer))
   GUI_CS_SIGNAL_2(paintRequested, printer)
   GUI_CS_SIGNAL_1(Public, void previewChanged())
   GUI_CS_SIGNAL_2(previewChanged)

 private:
   GUI_CS_SLOT_1(Private, void _q_fit())
   GUI_CS_SLOT_2(_q_fit)

   GUI_CS_SLOT_1(Private, void _q_updateCurrentPage())
   GUI_CS_SLOT_2(_q_updateCurrentPage)
};

QT_END_NAMESPACE


#endif // QT_NO_PRINTPREVIEWWIDGET
#endif // QPRINTPREVIEWWIDGET_H
