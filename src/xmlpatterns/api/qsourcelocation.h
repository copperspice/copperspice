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

#ifndef QSOURCELOCATION_H
#define QSOURCELOCATION_H

#include <QtCore/QMetaType>
#include <QtCore/QUrl>

QT_BEGIN_NAMESPACE

class QSourceLocationPrivate;

class Q_XMLPATTERNS_EXPORT QSourceLocation
{
 public:
   QSourceLocation();
   QSourceLocation(const QSourceLocation &other);
   QSourceLocation(const QUrl &uri, int line = -1, int column = -1);
   ~QSourceLocation();
   QSourceLocation &operator=(const QSourceLocation &other);
   bool operator==(const QSourceLocation &other) const;
   bool operator!=(const QSourceLocation &other) const;

   qint64 column() const;
   void setColumn(qint64 newColumn);

   qint64 line() const;
   void setLine(qint64 newLine);

   QUrl uri() const;
   void setUri(const QUrl &newUri);
   bool isNull() const;

 private:
   union {
      qint64 m_line;
      QSourceLocationPrivate *m_ptr;
   };
   qint64 m_column;
   QUrl m_uri;
};

Q_XMLPATTERNS_EXPORT uint qHash(const QSourceLocation &location);
Q_XMLPATTERNS_EXPORT QDebug operator<<(QDebug debug, const QSourceLocation &sourceLocation);

Q_DECLARE_TYPEINFO(QSourceLocation, Q_MOVABLE_TYPE);

QT_END_NAMESPACE

Q_DECLARE_METATYPE(QSourceLocation) /* This macro must appear after QT_END_NAMESPACE. */


#endif
