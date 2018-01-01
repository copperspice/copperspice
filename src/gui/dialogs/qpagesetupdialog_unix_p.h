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

#ifndef QPAGESETUPDialog_Unix_P_H
#define QPAGESETUPDialog_Unix_P_H

#include <qglobal.h>

#ifndef QT_NO_PRINTDIALOG

#include <ui_qpagesetupwidget.h>

QT_BEGIN_NAMESPACE

class QPrinter;
class QPagePreview;
class QCUPSSupport;

class QPageSetupWidget : public QWidget
{
   GUI_CS_OBJECT(QPageSetupWidget)
 public:
   QPageSetupWidget(QWidget *parent = nullptr);
   QPageSetupWidget(QPrinter *printer, QWidget *parent = nullptr);
   void setPrinter(QPrinter *printer);

   /// copy information from the widget and apply that to the printer
   void setupPrinter() const;
   void selectPrinter(QCUPSSupport *m_cups);
   void selectPdfPsPrinter(const QPrinter *p);

 private :
   GUI_CS_SLOT_1(Private, void _q_pageOrientationChanged())
   GUI_CS_SLOT_2(_q_pageOrientationChanged)
   GUI_CS_SLOT_1(Private, void _q_paperSizeChanged())
   GUI_CS_SLOT_2(_q_paperSizeChanged)
   GUI_CS_SLOT_1(Private, void unitChanged(int item))
   GUI_CS_SLOT_2(unitChanged)
   GUI_CS_SLOT_1(Private, void setTopMargin(double newValue))
   GUI_CS_SLOT_2(setTopMargin)
   GUI_CS_SLOT_1(Private, void setBottomMargin(double newValue))
   GUI_CS_SLOT_2(setBottomMargin)
   GUI_CS_SLOT_1(Private, void setLeftMargin(double newValue))
   GUI_CS_SLOT_2(setLeftMargin)
   GUI_CS_SLOT_1(Private, void setRightMargin(double newValue))
   GUI_CS_SLOT_2(setRightMargin)
 
   Ui::QPageSetupWidget widget;
   QPagePreview *m_pagePreview;
   QPrinter *m_printer;
   qreal m_leftMargin;
   qreal m_topMargin;
   qreal m_rightMargin;
   qreal m_bottomMargin;
   QSizeF m_paperSize;
   qreal m_currentMultiplier;
   bool m_blockSignals;
   QCUPSSupport *m_cups;
};

QT_END_NAMESPACE

#endif // QT_NO_PRINTDIALOG
#endif
