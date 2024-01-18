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

#ifndef QWHATSTHIS_H
#define QWHATSTHIS_H

#include <qobject.h>
#include <qcursor.h>



#ifndef QT_NO_WHATSTHIS

class QAction;

class Q_GUI_EXPORT QWhatsThis
{
   QWhatsThis() = delete;

 public:
   static void enterWhatsThisMode();
   static bool inWhatsThisMode();
   static void leaveWhatsThisMode();

   static void showText(const QPoint &pos, const QString &text, QWidget *w = nullptr);
   static void hideText();

   static QAction *createAction(QObject *parent = nullptr);
};

#endif // QT_NO_WHATSTHIS


#endif // QWHATSTHIS_H
