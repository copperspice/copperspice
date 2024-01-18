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

#ifndef QSCRIPTSCRIPTDATA_P_H
#define QSCRIPTSCRIPTDATA_P_H

#include <QtCore/qobjectdefs.h>
#include <qscopedpointer_p.h>
#include <QtCore/qdatetime.h>
#include <QtCore/qmap.h>

QT_BEGIN_NAMESPACE

class QDataStream;
class QString;
class QStringList;
class QScriptScriptDataPrivate;

class QScriptScriptData
{
 public:
   friend QDataStream &operator<<(QDataStream &, const QScriptScriptData &);
   friend QDataStream &operator>>(QDataStream &, QScriptScriptData &);

   QScriptScriptData();
   QScriptScriptData(const QString &contents, const QString &fileName,
                     int baseLineNumber, const QDateTime &timeStamp = QDateTime());
   QScriptScriptData(const QScriptScriptData &other);
   ~QScriptScriptData();

   QString contents() const;
   QStringList lines(int startLineNumber, int count) const;
   QString fileName() const;
   int baseLineNumber() const;
   QDateTime timeStamp() const;

   bool isValid() const;

   QScriptScriptData &operator=(const QScriptScriptData &other);

   bool operator==(const QScriptScriptData &other) const;
   bool operator!=(const QScriptScriptData &other) const;

 private:
   QScopedSharedPointer<QScriptScriptDataPrivate> d_ptr;

   Q_DECLARE_PRIVATE(QScriptScriptData)
};

typedef QMap<qint64, QScriptScriptData> QScriptScriptMap;

QDataStream &operator<<(QDataStream &, const QScriptScriptData &);
QDataStream &operator>>(QDataStream &, QScriptScriptData &);

QT_END_NAMESPACE

#endif
