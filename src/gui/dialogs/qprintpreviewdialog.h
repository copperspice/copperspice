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

#ifndef QPRINTPREVIEWDIALOG_H
#define QPRINTPREVIEWDIALOG_H

#include <QtGui/qdialog.h>

#ifndef QT_NO_PRINTPREVIEWDIALOG

QT_BEGIN_NAMESPACE

class QGraphicsView;
class QPrintPreviewDialogPrivate;

class Q_GUI_EXPORT QPrintPreviewDialog : public QDialog
{
   GUI_CS_OBJECT(QPrintPreviewDialog)
   Q_DECLARE_PRIVATE(QPrintPreviewDialog)

 public:
   explicit QPrintPreviewDialog(QWidget *parent = nullptr, Qt::WindowFlags flags = 0);
   explicit QPrintPreviewDialog(QPrinter *printer, QWidget *parent = nullptr, Qt::WindowFlags flags = 0);
   ~QPrintPreviewDialog();

   using QDialog::open;

   void open(QObject *receiver, const char *member);

   QPrinter *printer();

   void setVisible(bool visible) override;
   void done(int result) override;

   GUI_CS_SIGNAL_1(Public, void paintRequested(QPrinter *printer))
   GUI_CS_SIGNAL_2(paintRequested, printer)

 private:
   GUI_CS_SLOT_1(Private, void _q_fit(QAction *action))
   GUI_CS_SLOT_2(_q_fit)

   GUI_CS_SLOT_1(Private, void _q_zoomIn())
   GUI_CS_SLOT_2(_q_zoomIn)

   GUI_CS_SLOT_1(Private, void _q_zoomOut())
   GUI_CS_SLOT_2(_q_zoomOut)

   GUI_CS_SLOT_1(Private, void _q_navigate(QAction *action))
   GUI_CS_SLOT_2(_q_navigate)

   GUI_CS_SLOT_1(Private, void _q_setMode(QAction *action))
   GUI_CS_SLOT_2(_q_setMode)

   GUI_CS_SLOT_1(Private, void _q_pageNumEdited())
   GUI_CS_SLOT_2(_q_pageNumEdited)

   GUI_CS_SLOT_1(Private, void _q_print())
   GUI_CS_SLOT_2(_q_print)

   GUI_CS_SLOT_1(Private, void _q_pageSetup())
   GUI_CS_SLOT_2(_q_pageSetup)

   GUI_CS_SLOT_1(Private, void _q_previewChanged())
   GUI_CS_SLOT_2(_q_previewChanged)

   GUI_CS_SLOT_1(Private, void _q_zoomFactorChanged())
   GUI_CS_SLOT_2(_q_zoomFactorChanged)

};

QT_END_NAMESPACE

#endif // QT_NO_PRINTPREVIEWDIALOG

#endif // QPRINTPREVIEWDIALOG_H
