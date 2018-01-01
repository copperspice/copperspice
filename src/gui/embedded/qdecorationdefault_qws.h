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

#ifndef QDECORATIONDEFAULT_QWS_H
#define QDECORATIONDEFAULT_QWS_H

#include <QtGui/qdecoration_qws.h>

QT_BEGIN_NAMESPACE

#if !defined(QT_NO_QWS_DECORATION_DEFAULT) || defined(QT_PLUGIN)

#define CORNER_GRAB 16
#define BORDER_WIDTH  4
#define BOTTOM_BORDER_WIDTH BORDER_WIDTH

class Q_GUI_EXPORT QDecorationDefault : public QDecoration
{

 public:
   QDecorationDefault();
   virtual ~QDecorationDefault();

   virtual QRegion region(const QWidget *widget, const QRect &rect, int decorationRegion = All);
   virtual bool paint(QPainter *painter, const QWidget *widget, int decorationRegion = All,
                      DecorationState state = Normal);

 protected:
   virtual int titleBarHeight(const QWidget *widget);

   virtual void paintButton(QPainter *painter, const QWidget *widget, int buttonRegion,
                            DecorationState state, const QPalette &pal);
   virtual QPixmap pixmapFor(const QWidget *widget, int decorationRegion, int &xoff, int &yoff);
   virtual const char **xpmForRegion(int region);

   QString windowTitleFor(const QWidget *widget) const;

   int menu_width;
   int help_width;
   int close_width;
   int minimize_width;
   int maximize_width;
   int normalize_width;

 private:
   static QPixmap *staticHelpPixmap;
   static QPixmap *staticMenuPixmap;
   static QPixmap *staticClosePixmap;
   static QPixmap *staticMinimizePixmap;
   static QPixmap *staticMaximizePixmap;
   static QPixmap *staticNormalizePixmap;

};

QT_END_NAMESPACE

#endif // QT_NO_QWS_DECORATION_DEFAULT

#endif // QDECORATIONDEFAULT_QWS_H
