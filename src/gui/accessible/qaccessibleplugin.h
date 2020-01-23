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

#ifndef QACCESSIBLEPLUGIN_H
#define QACCESSIBLEPLUGIN_H

#include <qaccessible.h>
#include <qfactoryinterface.h>

#ifndef QT_NO_ACCESSIBILITY

class QStringList;
class QAccessibleInterface;

#define QAccessibleInterface_ID "com.copperspice.CS.AccessibleInterface"

class QAccessiblePluginPrivate;

class Q_GUI_EXPORT QAccessiblePlugin : public QObject
{
   GUI_CS_OBJECT(QAccessiblePlugin)

 public:
   explicit QAccessiblePlugin(QObject *parent = nullptr);
   ~QAccessiblePlugin();
   virtual QAccessibleInterface *create(const QString &key, QObject *object) = 0;
};

#endif // QT_NO_ACCESSIBILITY



#endif
