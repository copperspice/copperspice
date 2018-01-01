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

#ifndef QDECORATIONSTYLED_QWS_H
#define QDECORATIONSTYLED_QWS_H

#include <QtGui/qdecorationdefault_qws.h>

QT_BEGIN_NAMESPACE

#if !defined(QT_NO_QWS_DECORATION_STYLED) || defined(QT_PLUGIN)

class Q_GUI_EXPORT QDecorationStyled : public QDecorationDefault
{

 public:
   QDecorationStyled();
   virtual ~QDecorationStyled();

   QRegion region(const QWidget *widget, const QRect &rect, int decorationRegion = All);
   bool paint(QPainter *painter, const QWidget *widget, int decorationRegion = All, DecorationState state = Normal);
   int titleBarHeight(const QWidget *widget);
};

#endif // QT_NO_QWS_DECORATION_STYLED

QT_END_NAMESPACE

#endif // QDECORATIONSTYLED_QWS_H
