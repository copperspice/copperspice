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

#ifndef RECENTFILES_H
#define RECENTFILES_H

#include <QString>
#include <QStringList>
#include <QTimer>

QT_BEGIN_NAMESPACE

class RecentFiles : public QObject
{
   Q_OBJECT

 public:
   explicit RecentFiles(const int maxEntries);

   bool isEmpty() {
      return m_strLists.isEmpty();
   }
   void addFiles(const QStringList &names);
   QString lastOpenedFile() const {
      if (m_strLists.isEmpty() || m_strLists.first().isEmpty()) {
         return QString::null;
      }
      return m_strLists.at(0).at(0);
   }
   const QList<QStringList> &filesLists() const {
      return m_strLists;
   }

   void readConfig();
   void writeConfig() const;

 public slots:
   void closeGroup();

 private:
   bool m_groupOpen;
   bool m_clone1st;
   int m_maxEntries;
   QList<QStringList> m_strLists;
   QTimer m_timer;
};

QT_END_NAMESPACE

#endif // RECENTFILES_H
