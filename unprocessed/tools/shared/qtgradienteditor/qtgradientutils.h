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

#ifndef GRADIENTUTILS_H
#define GRADIENTUTILS_H

#include <QtGui/QGradient>
#include <QtGui/QPainter>

QT_BEGIN_NAMESPACE

class QtGradientManager;

class QtGradientUtils
{
public:
    static QString styleSheetCode(const QGradient &gradient);
    // utils methods, they could be outside of this class
    static QString saveState(const QtGradientManager *manager);
    static void restoreState(QtGradientManager *manager, const QString &state);

    static QPixmap gradientPixmap(const QGradient &gradient, const QSize &size = QSize(64, 64), bool checkeredBackground = false);

};

QT_END_NAMESPACE

#endif
