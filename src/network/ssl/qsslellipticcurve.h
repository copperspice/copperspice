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

#ifndef QSSLELLIPTICCURVE_H
#define QSSLELLIPTICCURVE_H

#include <QtGlobal>
#include <QString>
#include <QMetaType>
#include <qhashfunc.h>

class QDebug;
class QSslEllipticCurve;

// qHash is a friend, but we can not use default arguments for friends
uint qHash(QSslEllipticCurve curve, uint seed = 0);

class Q_NETWORK_EXPORT QSslEllipticCurve {

public:
   constexpr QSslEllipticCurve()
      : id(0)
   {
   }

   static QSslEllipticCurve fromShortName(const QString &name);
   static QSslEllipticCurve fromLongName(const QString &name);

   QString shortName() const Q_REQUIRED_RESULT;
   QString longName() const Q_REQUIRED_RESULT;

   constexpr bool isValid() const {
      return id != 0;
   }

   bool isTlsNamedCurve() const;

private:
   int id;

   friend constexpr bool operator==(QSslEllipticCurve lhs, QSslEllipticCurve rhs);
   friend uint qHash(QSslEllipticCurve curve, uint seed);

   friend class QSslSocketPrivate;
   friend class QSslSocketBackendPrivate;
};

inline uint qHash(QSslEllipticCurve curve, uint seed)
{
   return qHash(curve.id, seed);
}

constexpr inline bool operator==(QSslEllipticCurve lhs, QSslEllipticCurve rhs)
{
   return lhs.id == rhs.id;
}

constexpr inline bool operator!=(QSslEllipticCurve lhs, QSslEllipticCurve rhs)
{
   return !operator==(lhs, rhs);
}

Q_NETWORK_EXPORT QDebug operator<<(QDebug debug, QSslEllipticCurve curve);

#endif
