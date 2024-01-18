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

#ifndef QSQLFIELD_H
#define QSQLFIELD_H

#include <qvariant.h>
#include <qstring.h>

class QSqlFieldPrivate;

class Q_SQL_EXPORT QSqlField
{

 public:
   enum RequiredStatus { Unknown = -1, Optional = 0, Required = 1 };

   QSqlField(const QString &fieldName = QString(),
      QVariant::Type type = QVariant::Invalid);

   QSqlField(const QSqlField &other);
   QSqlField &operator=(const QSqlField &other);
   bool operator==(const QSqlField &other) const;

   inline bool operator!=(const QSqlField &other) const {
      return !operator==(other);
   }

   ~QSqlField();

   void setValue(const QVariant &value);
   inline QVariant value() const {
      return val;
   }

   void setName(const QString &name);
   QString name() const;
   bool isNull() const;
   void setReadOnly(bool readOnly);
   bool isReadOnly() const;
   void clear();
   QVariant::Type type() const;
   bool isAutoValue() const;

   void setType(QVariant::Type type);

   void setRequiredStatus(RequiredStatus status);
   inline void setRequired(bool status) {
      setRequiredStatus(status ? Required : Optional);
   }

   void setLength(int fieldLength);
   void setPrecision(int precision);
   void setDefaultValue(const QVariant &value);
   void setSqlType(int type);
   void setGenerated(bool gen);
   void setAutoValue(bool autoVal);

   RequiredStatus requiredStatus() const;
   int length() const;
   int precision() const;
   QVariant defaultValue() const;
   int typeID() const;
   bool isGenerated() const;
   bool isValid() const;

 private:
   void detach();
   QVariant val;
   QSqlFieldPrivate *d;
};

Q_SQL_EXPORT QDebug operator<<(QDebug, const QSqlField &);

#endif // QSQLFIELD_H
