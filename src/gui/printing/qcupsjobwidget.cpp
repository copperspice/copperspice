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

#include <qcupsjobwidget_p.h>

#include <qcheckbox.h>
#include <qdatetime.h>
#include <qfontdatabase.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qtime.h>
#include <qtablewidget.h>
#include <qtablewidgetitem.h>
#include <qheaderview.h>
#include <qprinter.h>
#include <qprintengine.h>

#if ! defined(QT_NO_PRINTER) && ! defined(QT_NO_CUPS)

// internal
QCupsJobWidget::QCupsJobWidget(QWidget *parent)
    : QWidget(parent)
{
    m_ui.setupUi(this);
    //set all the default values
    //TODO restore last used values
    initJobHold();
    initJobBilling();
    initJobPriority();
    initBannerPages();
}

QCupsJobWidget::~QCupsJobWidget()
{
}

void QCupsJobWidget::setPrinter(QPrinter *printer)
{
    m_printer = printer;
}

void QCupsJobWidget::setupPrinter()
{
    QCUPSSupport::setJobHold(m_printer, jobHold(), jobHoldTime());
    QCUPSSupport::setJobBilling(m_printer, jobBilling());
    QCUPSSupport::setJobPriority(m_printer, jobPriority());
    QCUPSSupport::setBannerPages(m_printer, startBannerPage(), endBannerPage());
}

void QCupsJobWidget::initJobHold()
{
    m_ui.jobHoldComboBox->addItem(tr("Print Immediately"),             QVariant::fromValue(QCUPSSupport::NoHold));
    m_ui.jobHoldComboBox->addItem(tr("Hold Indefinitely"),             QVariant::fromValue(QCUPSSupport::Indefinite));
    m_ui.jobHoldComboBox->addItem(tr("Day (06:00 to 17:59)"),          QVariant::fromValue(QCUPSSupport::DayTime));
    m_ui.jobHoldComboBox->addItem(tr("Night (18:00 to 05:59)"),        QVariant::fromValue(QCUPSSupport::Night));
    m_ui.jobHoldComboBox->addItem(tr("Second Shift (16:00 to 23:59)"), QVariant::fromValue(QCUPSSupport::SecondShift));
    m_ui.jobHoldComboBox->addItem(tr("Third Shift (00:00 to 07:59)"),  QVariant::fromValue(QCUPSSupport::ThirdShift));
    m_ui.jobHoldComboBox->addItem(tr("Weekend (Saturday to Sunday)"),  QVariant::fromValue(QCUPSSupport::Weekend));
    m_ui.jobHoldComboBox->addItem(tr("Specific Time"),                 QVariant::fromValue(QCUPSSupport::SpecificTime));

    connect(m_ui.jobHoldComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(toggleJobHoldTime()));

    setJobHold(QCUPSSupport::NoHold, QTime());
    toggleJobHoldTime();
}

void QCupsJobWidget::setJobHold(QCUPSSupport::JobHoldUntil jobHold, const QTime &holdUntilTime)
{
    if (jobHold == QCUPSSupport::SpecificTime && holdUntilTime.isNull()) {
        jobHold = QCUPSSupport::NoHold;
        toggleJobHoldTime();
    }
    m_ui.jobHoldComboBox->setCurrentIndex(m_ui.jobHoldComboBox->findData(QVariant::fromValue(jobHold)));
    m_ui.jobHoldTimeEdit->setTime(holdUntilTime);
}

QCUPSSupport::JobHoldUntil QCupsJobWidget::jobHold() const
{
    return m_ui.jobHoldComboBox->itemData(m_ui.jobHoldComboBox->currentIndex()).value<QCUPSSupport::JobHoldUntil>();
}

void QCupsJobWidget::toggleJobHoldTime()
{
    if (jobHold() == QCUPSSupport::SpecificTime)
        m_ui.jobHoldTimeEdit->setEnabled(true);
    else
        m_ui.jobHoldTimeEdit->setEnabled(false);
}

QTime QCupsJobWidget::jobHoldTime() const
{
    return m_ui.jobHoldTimeEdit->time();
}

void QCupsJobWidget::initJobBilling()
{
    setJobBilling(QString());
}

void QCupsJobWidget::setJobBilling(const QString &jobBilling)
{
    m_ui.jobBillingLineEdit->insert(jobBilling);
}

QString QCupsJobWidget::jobBilling() const
{
    return m_ui.jobBillingLineEdit->text();
}

void QCupsJobWidget::initJobPriority()
{
    setJobPriority(50);
}

void QCupsJobWidget::setJobPriority(int jobPriority)
{
    m_ui.jobPrioritySpinBox->setValue(jobPriority);
}

int QCupsJobWidget::jobPriority() const
{
    return m_ui.jobPrioritySpinBox->value();
}

void QCupsJobWidget::initBannerPages()
{
    m_ui.startBannerPageCombo->addItem(tr("None", "CUPS Banner page"),         QVariant::fromValue(QCUPSSupport::NoBanner));
    m_ui.startBannerPageCombo->addItem(tr("Standard", "CUPS Banner page"),     QVariant::fromValue(QCUPSSupport::Standard));
    m_ui.startBannerPageCombo->addItem(tr("Unclassified", "CUPS Banner page"), QVariant::fromValue(QCUPSSupport::Unclassified));
    m_ui.startBannerPageCombo->addItem(tr("Confidential", "CUPS Banner page"), QVariant::fromValue(QCUPSSupport::Confidential));
    m_ui.startBannerPageCombo->addItem(tr("Classified", "CUPS Banner page"),   QVariant::fromValue(QCUPSSupport::Classified));
    m_ui.startBannerPageCombo->addItem(tr("Secret", "CUPS Banner page"),       QVariant::fromValue(QCUPSSupport::Secret));
    m_ui.startBannerPageCombo->addItem(tr("Top Secret", "CUPS Banner page"),   QVariant::fromValue(QCUPSSupport::TopSecret));

    m_ui.endBannerPageCombo->addItem(tr("None", "CUPS Banner page"),         QVariant::fromValue(QCUPSSupport::NoBanner));
    m_ui.endBannerPageCombo->addItem(tr("Standard", "CUPS Banner page"),     QVariant::fromValue(QCUPSSupport::Standard));
    m_ui.endBannerPageCombo->addItem(tr("Unclassified", "CUPS Banner page"), QVariant::fromValue(QCUPSSupport::Unclassified));
    m_ui.endBannerPageCombo->addItem(tr("Confidential", "CUPS Banner page"), QVariant::fromValue(QCUPSSupport::Confidential));
    m_ui.endBannerPageCombo->addItem(tr("Classified", "CUPS Banner page"),   QVariant::fromValue(QCUPSSupport::Classified));
    m_ui.endBannerPageCombo->addItem(tr("Secret", "CUPS Banner page"),       QVariant::fromValue(QCUPSSupport::Secret));
    m_ui.endBannerPageCombo->addItem(tr("Top Secret", "CUPS Banner page"),   QVariant::fromValue(QCUPSSupport::TopSecret));

    setStartBannerPage(QCUPSSupport::NoBanner);
    setEndBannerPage(QCUPSSupport::NoBanner);
}

void QCupsJobWidget::setStartBannerPage(const QCUPSSupport::BannerPage bannerPage)
{
    m_ui.startBannerPageCombo->setCurrentIndex(m_ui.startBannerPageCombo->findData(QVariant::fromValue(bannerPage)));
}

QCUPSSupport::BannerPage QCupsJobWidget::startBannerPage() const
{
    return m_ui.startBannerPageCombo->itemData(m_ui.startBannerPageCombo->currentIndex()).value<QCUPSSupport::BannerPage>();
}

void QCupsJobWidget::setEndBannerPage(const QCUPSSupport::BannerPage bannerPage)
{
    m_ui.endBannerPageCombo->setCurrentIndex(m_ui.endBannerPageCombo->findData(QVariant::fromValue(bannerPage)));
}

QCUPSSupport::BannerPage QCupsJobWidget::endBannerPage() const
{
    return m_ui.endBannerPageCombo->itemData(m_ui.endBannerPageCombo->currentIndex()).value<QCUPSSupport::BannerPage>();
}

#endif // QT_NO_PRINTER / QT_NO_CUPS

