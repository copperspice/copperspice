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

#include <qdrawhelper_p.h>

QDrawHelperGammaTables::QDrawHelperGammaTables(qreal smoothing)
{
    const qreal gray_gamma = 2.31;
    for (int i=0; i<256; ++i)
        qt_pow_gamma[i] = uint(qRound(qPow(i / qreal(255.), gray_gamma) * 2047));
    for (int i=0; i<2048; ++i)
        qt_pow_invgamma[i] = uchar(qRound(qPow(i / qreal(2047.0), 1 / gray_gamma) * 255));

    refresh(smoothing);
}

void QDrawHelperGammaTables::refresh(qreal smoothing)
{
    for (int i=0; i<256; ++i) {
        qt_pow_rgb_gamma[i] = uchar(qRound(qPow(i / qreal(255.0), smoothing) * 255));
        qt_pow_rgb_invgamma[i] = uchar(qRound(qPow(i / qreal(255.), 1 / smoothing) * 255));
    }
}

