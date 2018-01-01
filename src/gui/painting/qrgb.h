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

#ifndef QRGB_H
#define QRGB_H

#include <QtCore/qglobal.h>

QT_BEGIN_NAMESPACE

typedef unsigned int QRgb;                     // RGB triplet

const QRgb  RGB_MASK    = 0x00ffffff;          // masks RGB values

inline int qRed(QRgb rgb)                      // get red part of RGB
{
   return ((rgb >> 16) & 0xff);
}

inline int qGreen(QRgb rgb)                    // get green part of RGB
{
   return ((rgb >> 8) & 0xff);
}

inline int qBlue(QRgb rgb)                     // get blue part of RGB
{
   return (rgb & 0xff);
}

inline int qAlpha(QRgb rgb)                    // get alpha part of RGBA
{
   return rgb >> 24;
}

inline QRgb qRgb(int r, int g, int b)          // set RGB value
{
   return (0xffu << 24) | ((r & 0xff) << 16) | ((g & 0xff) << 8) | (b & 0xff);
}

inline QRgb qRgba(int r, int g, int b, int a)  // set RGBA value
{
   return ((a & 0xff) << 24) | ((r & 0xff) << 16) | ((g & 0xff) << 8) | (b & 0xff);
}

inline int qGray(int r, int g, int b)         // convert R,G,B to gray 0..255
{
   return (r * 11 + g * 16 + b * 5) / 32;
}

inline int qGray(QRgb rgb)                    // convert RGB to gray 0..255
{
   return qGray(qRed(rgb), qGreen(rgb), qBlue(rgb));
}

inline bool qIsGray(QRgb rgb)
{
   return qRed(rgb) == qGreen(rgb) && qRed(rgb) == qBlue(rgb);
}

QT_END_NAMESPACE


#endif // QRGB_H
