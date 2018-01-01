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

#include "mainwindow.h"

#include <QApplication>
#include <QAxFactory>

QAXFACTORY_DEFAULT(MainWindow,
		   QLatin1String("{5f5ce700-48a8-47b1-9b06-3b7f79e41d7c}"),
		   QLatin1String("{3fc86f5f-8b15-4428-8f6b-482bae91f1ae}"),
		   QLatin1String("{02a268cd-24b4-4fd9-88ff-b01b683ef39d}"),
		   QLatin1String("{4a43e44d-9d1d-47e5-a1e5-58fe6f7be0a4}"),
		   QLatin1String("{16ee5998-77d2-412f-ad91-8596e29f123f}"))

QT_USE_NAMESPACE

int main( int argc, char **argv ) 
{
    QApplication app( argc, argv );

    MainWindow mw;
    mw.show();

    return app.exec();;
}
