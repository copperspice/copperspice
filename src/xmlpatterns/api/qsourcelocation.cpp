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

#include <qxmlpatterns_debug_p.h>
#include <qsourcelocation.h>

QSourceLocation::QSourceLocation() : m_line(-1), m_column(-1)
{
}

QSourceLocation::QSourceLocation(const QSourceLocation &other)
   : m_line(other.m_line), m_column(other.m_column), m_uri(other.m_uri)
{
}

QSourceLocation::QSourceLocation(const QUrl &u, int l, int c)
   : m_line(l), m_column(c), m_uri(u)
{
}

QSourceLocation::~QSourceLocation()
{
}

bool QSourceLocation::operator==(const QSourceLocation &other) const
{
   return    m_line == other.m_line
             && m_column == other.m_column
             && m_uri == other.m_uri;
}

bool QSourceLocation::operator!=(const QSourceLocation &other) const
{
   return operator==(other);
}

QSourceLocation &QSourceLocation::operator=(const QSourceLocation &other)
{
   if (this != &other) {
      m_line = other.m_line;
      m_column = other.m_column;
      m_uri = other.m_uri;
   }

   return *this;
}

qint64 QSourceLocation::column() const
{
   return m_column;
}

void QSourceLocation::setColumn(qint64 newColumn)
{
   Q_ASSERT_X(newColumn != 0, Q_FUNC_INFO,
              "0 is an invalid column number. The first column number is 1.");
   m_column = newColumn;
}

qint64 QSourceLocation::line() const
{
   return m_line;
}

void QSourceLocation::setLine(qint64 newLine)
{
   m_line = newLine;
}

QUrl QSourceLocation::uri() const
{
   return m_uri;
}

void QSourceLocation::setUri(const QUrl &newUri)
{
   m_uri = newUri;
}

QDebug operator<<(QDebug debug, const QSourceLocation &sourceLocation)
{
   debug << "QSourceLocation("
         << sourceLocation.uri().toString()
         << ", line:"
         << sourceLocation.line()
         << ", column:"
         << sourceLocation.column()
         << ')';
   return debug;
}

bool QSourceLocation::isNull() const
{
   return ! m_uri.isValid();
}

uint qHash(const QSourceLocation &location)
{
   /* Not the world's best hash function exactly. */
   return qHash(location.uri().toString()) + location.line() + location.column();
}

