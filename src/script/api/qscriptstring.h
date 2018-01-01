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

#ifndef QSCRIPTSTRING_H
#define QSCRIPTSTRING_H

#include <QtCore/qstring.h>
#include <QtCore/qsharedpointer.h>

QT_BEGIN_NAMESPACE

class QScriptStringPrivate;

class Q_SCRIPT_EXPORT QScriptString
{
 public:
   QScriptString();
   QScriptString(const QScriptString &other);
   ~QScriptString();

   QScriptString &operator=(const QScriptString &other);

   bool isValid() const;

   bool operator==(const QScriptString &other) const;
   bool operator!=(const QScriptString &other) const;

   quint32 toArrayIndex(bool *ok = 0) const;

   QString toString() const;
   operator QString() const;

 private:
   QExplicitlySharedDataPointer<QScriptStringPrivate> d_ptr;
   friend class QScriptValue;
   Q_DECLARE_PRIVATE(QScriptString)
};

Q_SCRIPT_EXPORT uint qHash(const QScriptString &key);

QT_END_NAMESPACE

#endif // QSCRIPTSTRING_H
