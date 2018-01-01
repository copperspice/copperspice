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

#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <QtGui/QVBoxLayout>
#include <QtGui/QLabel>
#include <QtGui/QTextEdit>

#include "helpwindow.h"

QT_BEGIN_NAMESPACE

HelpWindow::HelpWindow(QWidget *parent)
    : QWidget(parent, 0)
{
    setAutoFillBackground(true);
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setMargin(0);
    QFrame *frame = new QFrame(this);
    QPalette p = palette();
    p.setColor(QPalette::Background, Qt::white);
    setPalette(p);
    frame->setFrameStyle(QFrame::Box | QFrame::Plain);
    layout->addWidget(frame);

    layout = new QVBoxLayout(frame);
    layout->setMargin(2);
    QLabel *l = new QLabel(tr("<center><b>Wizard Assistant</b></center>"));
    layout->addWidget(l);
    m_textEdit = new QTextEdit();
    m_textEdit->setFrameStyle(QFrame::NoFrame);
    m_textEdit->setReadOnly(true);
    layout->addWidget(m_textEdit);
}

void HelpWindow::setHelp(const QString &topic)
{
    QLatin1String fileStr(":/trolltech/qhelpconverter/doc/%1.html");
    QFile f(QString(fileStr).arg(topic.toLower()));
    f.open(QIODevice::ReadOnly);
    QTextStream s(&f);
    m_textEdit->setText(s.readAll());
}

QT_END_NAMESPACE
