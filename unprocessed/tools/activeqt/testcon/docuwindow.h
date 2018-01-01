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

#ifndef DOCUWINDOW_H
#define DOCUWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE

class QTextBrowser;

class DocuWindow : public QMainWindow
{
    Q_OBJECT
public:
    DocuWindow( const QString& docu, QWidget *parent, QWidget *source );

public slots:
    void save();
    void print();

private:
    QTextBrowser *browser;
};

QT_END_NAMESPACE

#endif // DOCUWINDOW_H
