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

#include "ambientproperties.h"

#include <QtGui>

QT_BEGIN_NAMESPACE

AmbientProperties::AmbientProperties(QWidget *parent)
: QDialog(parent), container(0)
{
    setupUi(this);

    connect(buttonClose, SIGNAL(clicked()), this, SLOT(close()));
}

void AmbientProperties::setControl(QWidget *widget)
{
    container = widget;

    QColor c = container->palette().color(container->backgroundRole());
    QPalette p = backSample->palette(); p.setColor(backSample->backgroundRole(), c); backSample->setPalette(p);

    c = container->palette().color(container->foregroundRole());
    p = foreSample->palette(); p.setColor(foreSample->backgroundRole(), c); foreSample->setPalette(p);

    fontSample->setFont( container->font() );
    buttonEnabled->setChecked( container->isEnabled() );
    enabledSample->setEnabled( container->isEnabled() );
}

void AmbientProperties::on_buttonBackground_clicked()
{
    QColor c = QColorDialog::getColor(backSample->palette().color(backSample->backgroundRole()), this);
    QPalette p = backSample->palette(); p.setColor(backSample->backgroundRole(), c); backSample->setPalette(p);
    p = container->palette(); p.setColor(container->backgroundRole(), c); container->setPalette(p);

    if (QWorkspace *ws = qobject_cast<QWorkspace*>(container)) {
	QWidgetList list( ws->windowList() );
	for (int i = 0; i < list.count(); ++i) {
	    QWidget *widget = list.at(i);
	    p = widget->palette(); p.setColor(widget->backgroundRole(), c); widget->setPalette(p);
	}
    }
}

void AmbientProperties::on_buttonForeground_clicked()
{
    QColor c = QColorDialog::getColor(foreSample->palette().color(foreSample->backgroundRole()), this);
    QPalette p = foreSample->palette(); p.setColor(foreSample->backgroundRole(), c); foreSample->setPalette(p);
    p = container->palette(); p.setColor(container->foregroundRole(), c); container->setPalette(p);

    if (QWorkspace *ws = qobject_cast<QWorkspace*>(container)) {
	QWidgetList list( ws->windowList() );
	for (int i = 0; i < list.count(); ++i) {
	    QWidget *widget = list.at(i);
	    p = widget->palette(); p.setColor(widget->foregroundRole(), c); widget->setPalette(p);
	}
    }
}

void AmbientProperties::on_buttonFont_clicked()
{
    bool ok;
    QFont f = QFontDialog::getFont( &ok, fontSample->font(), this );
    if ( !ok )
	return;
    fontSample->setFont( f );
    container->setFont( f );

    if (QWorkspace *ws = qobject_cast<QWorkspace*>(container)) {
	QWidgetList list( ws->windowList() );
	for (int i = 0; i < list.count(); ++i) {
	    QWidget *widget = list.at(i);
	    widget->setFont( f );
	}
    }
}

void AmbientProperties::on_buttonEnabled_toggled(bool on)
{
    enabledSample->setEnabled( on );
    container->setEnabled( on );
}

QT_END_NAMESPACE
