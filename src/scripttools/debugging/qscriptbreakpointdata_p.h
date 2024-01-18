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

#ifndef QSCRIPTBREAKPOINTDATA_P_H
#define QSCRIPTBREAKPOINTDATA_P_H

#include <QtCore/qobjectdefs.h>
#include <QtCore/qscopedpointer.h>
#include <QtCore/qmap.h>

QT_BEGIN_NAMESPACE

class QDataStream;
class QVariant;

class QScriptBreakpointDataPrivate;
class QScriptBreakpointData
{
 public:
   friend QDataStream &operator<<(QDataStream &, const QScriptBreakpointData &);
   friend QDataStream &operator>>(QDataStream &, QScriptBreakpointData &);

   QScriptBreakpointData();
   QScriptBreakpointData(qint64 scriptId, int lineNumber);
   QScriptBreakpointData(const QString &fileName, int lineNumber);
   QScriptBreakpointData(const QScriptBreakpointData &other);
   ~QScriptBreakpointData();
   QScriptBreakpointData &operator=(const QScriptBreakpointData &other);

   bool isValid() const;

   // location
   qint64 scriptId() const;
   void setScriptId(qint64 id);

   QString fileName() const;
   void setFileName(const QString &fileName);

   int lineNumber() const;
   void setLineNumber(int lineNumber);

   // data
   bool isEnabled() const;
   void setEnabled(bool enabled);

   bool isSingleShot() const;
   void setSingleShot(bool singleShot);

   int ignoreCount() const;
   void setIgnoreCount(int count);

   QString condition() const;
   void setCondition(const QString &condition);

   QVariant data() const;
   void setData(const QVariant &data);

   bool hit();

   // statistics
   int hitCount() const;


   bool operator==(const QScriptBreakpointData &other) const;
   bool operator!=(const QScriptBreakpointData &other) const;

 private:
   QScopedPointer<QScriptBreakpointDataPrivate> d_ptr;

   Q_DECLARE_PRIVATE(QScriptBreakpointData)
};

typedef QMap<int, QScriptBreakpointData> QScriptBreakpointMap;

QDataStream &operator<<(QDataStream &, const QScriptBreakpointData &);
QDataStream &operator>>(QDataStream &, QScriptBreakpointData &);

QT_END_NAMESPACE

#endif
