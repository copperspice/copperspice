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

#ifndef QTESTLITECURSOR_H
#define QTESTLITECURSOR_H

#include <QtGui/QPlatformCursor>

#include "qxlibintegration.h"

QT_BEGIN_NAMESPACE

class QXlibCursor : QPlatformCursor
{
public:
    QXlibCursor(QXlibScreen *screen);

    void changeCursor(QCursor * cursor, QWidget * widget);
private:

    Cursor createCursorBitmap(QCursor * cursor);
    Cursor createCursorShape(int cshape);

    QXlibScreen *testLiteScreen() const;
    QMap<int, Cursor> cursorMap;
};

QT_END_NAMESPACE

#endif // QTESTLITECURSOR_H
