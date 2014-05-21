/***********************************************************************
*
* Copyright (c) 2012-2013 Barbara Geller
* Copyright (c) 2012-2013 Ansel Sermersheim
* Copyright (c) 2011 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
*
* This file is part of CopperSpice.
*
* CopperSpice is free software: you can redistribute it and/or 
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with CopperSpice.  If not, see 
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
