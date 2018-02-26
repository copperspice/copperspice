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

#ifndef QAUTHENTICATOR_H
#define QAUTHENTICATOR_H

#include <QtCore/qstring.h>
#include <QtCore/qvariant.h>

QT_BEGIN_NAMESPACE

class QAuthenticatorPrivate;
class QUrl;

class Q_NETWORK_EXPORT QAuthenticator
{
 public:
   QAuthenticator();
   ~QAuthenticator();

   QAuthenticator(const QAuthenticator &other);
   QAuthenticator &operator=(const QAuthenticator &other);

   bool operator==(const QAuthenticator &other) const;
   inline bool operator!=(const QAuthenticator &other) const {
      return !operator==(other);
   }

   QString user() const;
   void setUser(const QString &user);

   QString password() const;
   void setPassword(const QString &password);

   QString realm() const;
   void setRealm(const QString &realm);

   QVariant option(const QString &opt) const;
   QVariantHash options() const;
   void setOption(const QString &opt, const QVariant &value);

   bool isNull() const;
   void detach();

 private:
   friend class QAuthenticatorPrivate;
   QAuthenticatorPrivate *d;
};

QT_END_NAMESPACE

#endif
