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

#include <qtoolbarextension_p.h>
#include <qpixmap.h>
#include <qstyle.h>
#include <qstylepainter.h>
#include <qstyleoption.h>

#ifndef QT_NO_TOOLBUTTON

QToolBarExtension::QToolBarExtension(QWidget *parent)
   : QToolButton(parent)
{
   setObjectName(QLatin1String("qt_toolbar_ext_button"));
   setAutoRaise(true);
   setOrientation(Qt::Horizontal);
   setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
   setCheckable(true);
}

void QToolBarExtension::setOrientation(Qt::Orientation o)
{
   QStyleOption opt;
   opt.initFrom(this);

   if (o == Qt::Horizontal) {
      setIcon(style()->standardIcon(QStyle::SP_ToolBarHorizontalExtensionButton, &opt));
   } else {
      setIcon(style()->standardIcon(QStyle::SP_ToolBarVerticalExtensionButton, &opt));
   }
}

void QToolBarExtension::paintEvent(QPaintEvent *)
{
   QStylePainter p(this);
   QStyleOptionToolButton opt;
   initStyleOption(&opt);
   // We do not need to draw both extension arrows
   opt.features &= ~QStyleOptionToolButton::HasMenu;
   p.drawComplexControl(QStyle::CC_ToolButton, opt);
}


QSize QToolBarExtension::sizeHint() const
{
   int ext = style()->pixelMetric(QStyle::PM_ToolBarExtensionExtent);
   return QSize(ext, ext);
}



#endif // QT_NO_TOOLBUTTON
