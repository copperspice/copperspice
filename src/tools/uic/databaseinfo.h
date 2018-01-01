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

#ifndef DATABASEINFO_H
#define DATABASEINFO_H

#include "treewalker.h"
#include <QtCore/QStringList>
#include <QtCore/QMap>

QT_BEGIN_NAMESPACE

class Driver;

class DatabaseInfo : public TreeWalker
{
 public:
   DatabaseInfo(Driver *driver);

   void acceptUI(DomUI *node) override;
   void acceptWidget(DomWidget *node) override;

   inline QStringList connections() const {
      return m_connections;
   }

   inline QStringList cursors(const QString &connection) const {
      return m_cursors.value(connection);
   }

   inline QStringList fields(const QString &connection) const {
      return m_fields.value(connection);
   }

 private:
   Driver *driver;
   QStringList m_connections;
   QMap<QString, QStringList> m_cursors;
   QMap<QString, QStringList> m_fields;
};

QT_END_NAMESPACE

#endif // DATABASEINFO_H
