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

#include <qglobal.h>
#include <qdesktopwidget.h>
#include <qwidget_p.h>

QT_BEGIN_NAMESPACE

const QRect QDesktopWidget::screenGeometry(const QWidget *widget) const
{
   if (!widget) {
      qWarning("QDesktopWidget::screenGeometry(): Attempt "
               "to get the screen geometry of a null widget");
      return QRect();
   }
   QRect rect = QWidgetPrivate::screenGeometry(widget);
   if (rect.isNull()) {
      return screenGeometry(screenNumber(widget));
   } else {
      return rect;
   }
}

const QRect QDesktopWidget::availableGeometry(const QWidget *widget) const
{
   if (!widget) {
      qWarning("QDesktopWidget::availableGeometry(): Attempt "
               "to get the available geometry of a null widget");
      return QRect();
   }
   QRect rect = QWidgetPrivate::screenGeometry(widget);
   if (rect.isNull()) {
      return availableGeometry(screenNumber(widget));
   } else {
      return rect;
   }
}

QT_END_NAMESPACE

