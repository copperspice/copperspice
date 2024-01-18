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

#ifndef RECENTFILES_H
#define RECENTFILES_H

#include <qstring.h>
#include <qstringlist.h>
#include <qtimer.h>

class RecentFiles : public QObject
{
   CS_OBJECT(RecentFiles)

 public:
   explicit RecentFiles(const int maxEntries);

   bool isEmpty() {
      return m_strLists.isEmpty();
   }

   void addFiles(const QStringList &names);

   QString lastOpenedFile() const {
      if (m_strLists.isEmpty() || m_strLists.first().isEmpty()) {
         return QString();
      }

      return m_strLists.at(0).at(0);
   }

   const QList<QStringList> &filesLists() const {
      return m_strLists;
   }

   void readConfig();
   void writeConfig() const;

   // slot
   void closeGroup();

 private:
   bool m_groupOpen;
   bool m_clone1st;
   int m_maxEntries;
   QList<QStringList> m_strLists;
   QTimer m_timer;
};

#endif
