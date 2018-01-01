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

#include "databaseinfo.h"
#include "driver.h"
#include "ui4.h"
#include "utils.h"

QT_BEGIN_NAMESPACE

DatabaseInfo::DatabaseInfo(Driver *drv)
   : driver(drv)
{
}

void DatabaseInfo::acceptUI(DomUI *node)
{
   m_connections.clear();
   m_cursors.clear();
   m_fields.clear();

   TreeWalker::acceptUI(node);

   m_connections = unique(m_connections);
}

void DatabaseInfo::acceptWidget(DomWidget *node)
{
   QHash<QString, DomProperty *> properties = propertyMap(node->elementProperty());

   DomProperty *frameworkCode = properties.value(QLatin1String("frameworkCode"), 0);
   if (frameworkCode && toBool(frameworkCode->elementBool()) == false) {
      return;
   }

   DomProperty *db = properties.value(QLatin1String("database"), 0);
   if (db && db->elementStringList()) {
      QStringList info = db->elementStringList()->elementString();

      QString connection = info.size() > 0 ? info.at(0) : QString();
      if (connection.isEmpty()) {
         return;
      }
      m_connections.append(connection);

      QString table = info.size() > 1 ? info.at(1) : QString();
      if (table.isEmpty()) {
         return;
      }
      m_cursors[connection].append(table);

      QString field = info.size() > 2 ? info.at(2) : QString();
      if (field.isEmpty()) {
         return;
      }
      m_fields[connection].append(field);
   }

   TreeWalker::acceptWidget(node);
}

QT_END_NAMESPACE
