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

#ifndef QFACTORYLOADER_P_H
#define QFACTORYLOADER_P_H

#include <QtCore/qobject.h>
#include <QtCore/qstringlist.h>
#include <qlibrary_p.h>
#include <QScopedPointer>

QT_BEGIN_NAMESPACE

class QFactoryLoaderPrivate;

class Q_CORE_EXPORT QFactoryLoader : public QObject
{
   CORE_CS_OBJECT(QFactoryLoader)
   Q_DECLARE_PRIVATE(QFactoryLoader)

 public:
   QFactoryLoader(const QString &iid, const QString &suffix = QString(), Qt::CaseSensitivity = Qt::CaseSensitive);
   ~QFactoryLoader();

   QStringList keys() const;
   QObject *instance(const QString &key) const;

#ifdef Q_WS_X11
   QLibraryPrivate *library(const QString &key) const;
#endif

   void update();
   void updateDir(const QString &pluginDir, QSettings &settings);

   static void refreshAll();

 protected:
   QScopedPointer<QFactoryLoaderPrivate> d_ptr;
};

QT_END_NAMESPACE

#endif // QFACTORYLOADER_P_H
