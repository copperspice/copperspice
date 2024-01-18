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

#ifndef QCUPSJOBWIDGET_P_H
#define QCUPSJOBWIDGET_P_H

#include <ui_qcupsjobwidget.h>
#include <qcups_p.h>

#if ! defined(QT_NO_PRINTER) && ! defined(QT_NO_CUPS)

class QTime;
class QPrinter;

class QCupsJobWidget : public QWidget
{
    GUI_CS_OBJECT(QCupsJobWidget)

 public:
   explicit QCupsJobWidget(QWidget *parent = nullptr);

   QCupsJobWidget(const QCupsJobWidget &) = delete;
   QCupsJobWidget &operator=(const QCupsJobWidget &) = delete;

   ~QCupsJobWidget();

   void setPrinter(QPrinter *printer);
   void setupPrinter();

 private:
   void setJobHold(QCUPSSupport::JobHoldUntil jobHold = QCUPSSupport::NoHold, const QTime &holdUntilTime = QTime());
   QCUPSSupport::JobHoldUntil jobHold() const;
   QTime jobHoldTime() const;

   void setJobBilling(const QString &jobBilling = QString());
   QString jobBilling() const;

   void setJobPriority(int priority = 50);
   int jobPriority() const;

   void setStartBannerPage(const QCUPSSupport::BannerPage bannerPage = QCUPSSupport::NoBanner);
   QCUPSSupport::BannerPage startBannerPage() const;

   void setEndBannerPage(const QCUPSSupport::BannerPage bannerPage = QCUPSSupport::NoBanner);
   QCUPSSupport::BannerPage endBannerPage() const;

   void initJobHold();
   void initJobBilling();
   void initJobPriority();
   void initBannerPages();

   QPrinter *m_printer;
   Ui::QCupsJobWidget m_ui;

   GUI_CS_SLOT_1(Private, void toggleJobHoldTime())
   GUI_CS_SLOT_2(toggleJobHoldTime)
};

#endif // QT_NO_PRINTER / QT_NO_CUPS

#endif
