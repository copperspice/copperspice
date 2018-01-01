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

#include "qvfbratedlg.h"
#include <QLayout>
#include <QLabel>
#include <qslider.h>
#include <QPushButton>

QT_BEGIN_NAMESPACE

QVFbRateDialog::QVFbRateDialog(int rate, QWidget *parent)
    : QDialog(parent)
{
    oldRate = rate;

    QVBoxLayout *tl = new QVBoxLayout(this);
    tl->setMargin(5);

    QLabel *label = new QLabel(tr("Target frame rate:"), this);
    tl->addWidget(label);

    QHBoxLayout *hl = new QHBoxLayout();
    tl->addItem(hl);
    rateSlider = new QSlider(Qt::Horizontal);
    rateSlider->setMinimum(1);
    rateSlider->setMaximum(100);
    rateSlider->setPageStep(10);
    rateSlider->setValue(rate);
    hl->addWidget(rateSlider);
    connect(rateSlider, SIGNAL(valueChanged(int)), this, SLOT(rateChanged(int)));
    rateLabel = new QLabel(tr("%1fps").arg(rate), this);
    hl->addWidget(rateLabel);

    hl = new QHBoxLayout();
    tl->addItem(hl);
    QPushButton *pb = new QPushButton(tr("OK"), this);
    connect(pb, SIGNAL(clicked()), this, SLOT(ok()));
    hl->addWidget(pb);
    pb = new QPushButton(tr("Cancel"), this);
    connect(pb, SIGNAL(clicked()), this, SLOT(cancel()));
    hl->addWidget(pb);
}

void QVFbRateDialog::rateChanged(int r)
{
    if (rateSlider->value() != r)
	rateSlider->setValue(r);
    rateLabel->setText(tr("%1fps").arg(r));
    emit updateRate(r);
}

void QVFbRateDialog::cancel()
{
    rateChanged(oldRate);
    reject();
}

void QVFbRateDialog::ok()
{
    oldRate = rateSlider->value();
    accept();
}

QT_END_NAMESPACE
