/***********************************************************************
*
* Copyright (c) 2012-2020 Barbara Geller
* Copyright (c) 2012-2020 Ansel Sermersheim
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

#ifndef QTOOLTIP_H
#define QTOOLTIP_H

#include <qwidget.h>

QT_BEGIN_NAMESPACE

#ifndef QT_NO_TOOLTIP

class Q_GUI_EXPORT QToolTip
{
   QToolTip() = delete;

 public:
   static void showText(const QPoint &pos, const QString &text, QWidget *w = nullptr);
   static void showText(const QPoint &pos, const QString &text, QWidget *w, const QRect &rect);
   static void showText(const QPoint &pos, const QString &text, QWidget *w, const QRect &rect, int msecShowTime);

   static inline void hideText() {
      showText(QPoint(), QString());
   }

   static bool isVisible();
   static QString text();

   static QPalette palette();
   static void setPalette(const QPalette &);
   static QFont font();
   static void setFont(const QFont &);
};

#endif // QT_NO_TOOLTIP


#endif // QTOOLTIP_H
