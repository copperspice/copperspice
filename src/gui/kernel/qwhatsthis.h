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

#ifndef QWHATSTHIS_H
#define QWHATSTHIS_H

#include <QtCore/qobject.h>
#include <QtGui/qcursor.h>

QT_BEGIN_NAMESPACE

#ifndef QT_NO_WHATSTHIS

class QAction;

class Q_GUI_EXPORT QWhatsThis
{
   QWhatsThis();

 public:
   static void enterWhatsThisMode();
   static bool inWhatsThisMode();
   static void leaveWhatsThisMode();

   static void showText(const QPoint &pos, const QString &text, QWidget *w = 0);
   static void hideText();

   static QAction *createAction(QObject *parent = nullptr);
};

#endif // QT_NO_WHATSTHIS

QT_END_NAMESPACE

#endif // QWHATSTHIS_H
