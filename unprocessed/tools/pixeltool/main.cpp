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

#include "qpixeltool.h"

#include <qapplication.h>
#include <qfileinfo.h>

QT_USE_NAMESPACE

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    QPixelTool pt;

    if (app.arguments().size() > 1 && QFileInfo(app.arguments().at(1)).exists()) {
        pt.setPreviewImage(QImage(app.arguments().at(1)));
    }

    pt.show();

    QObject::connect(&app, SIGNAL(lastWindowClosed()), &app, SLOT(quit()));

    int ret = app.exec();
    return ret;
}
