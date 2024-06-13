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

#include <recentfiles.h>

#include <globals.h>

#include <qdebug.h>
#include <qfileinfo.h>
#include <qsettings.h>
#include <qstring.h>
#include <qstringlist.h>

static QString configKey()
{
   return settingPath("RecentlyOpenedFiles");
}

RecentFiles::RecentFiles(const int maxEntries)
   : m_groupOpen(false), m_clone1st(false), m_maxEntries(maxEntries)
{
   m_timer.setSingleShot(true);
   m_timer.setInterval(3 * 60 * 1000);

   connect(&m_timer, &QTimer::timeout, this, &RecentFiles::closeGroup);
}

/*
 * The logic is as follows:
 * - The most recent (i.e., topmost) item can be open ("in flux")
 * - The item is closed by either a timeout (3 min or so) or a
 *   "terminal action" (e.g., closing all files)
 * - While the item is open, modifications to the set of open files
 *   will modify that item instead of creating new items
 * - If the open item is modified to be equal to an existing item,
 *   the existing item is deleted, but will be re-created when the
 *   open item is modified even further
 * Cases (actions in parentheses are no-ops):
 * - identical to top item => (do nothing)
 * - closed, new item => insert at top, (clear marker)
 * - closed, existing item => move to top, mark for cloning
 * - open, new item, not marked => replace top, (clear marker)
 * - open, new item, marked => insert at top, clear marker
 * - open, existing item, not marked => replace top, delete copy, mark for cloning
 * - open, existing item, marked => insert at top, delete copy, (mark for cloning)
 * - closing clears marker
 */
void RecentFiles::addFiles(const QStringList &names)
{
   if (m_strLists.isEmpty() || names != m_strLists.first()) {
      if (m_groupOpen && !m_clone1st) {
         // Group being open implies at least one item in the list
         m_strLists.removeFirst();
      }

      m_groupOpen = true;

      // We do *not* sort the actual entries, as that would destroy the user's
      // chosen arrangement. However, we do the searching on sorted lists, so
      // we throw out (probably) obsolete arrangements.
      QList<QStringList> sortedLists = m_strLists;
      for (int i = 0; i < sortedLists.size(); ++i) {
         sortedLists[i].sort();
      }

      QStringList sortedNames = names;
      sortedNames.sort();

      int index = sortedLists.indexOf(sortedNames);

      if (index >= 0) {
         m_strLists.removeAt(index);
         m_clone1st = true;

      } else {
         if (m_strLists.count() >= m_maxEntries) {
            m_strLists.removeLast();
         }
         m_clone1st = false;
      }
      m_strLists.prepend(names);
   }

   m_timer.start();
}

void RecentFiles::closeGroup()
{
   m_timer.stop();
   m_groupOpen = false;
}

void RecentFiles::readConfig()
{
   m_strLists.clear();
   QVariant val = QSettings().value(configKey());

   for (const QVariant & v : val.toList()) {
      m_strLists << v.toStringList();
   }

}

void RecentFiles::writeConfig() const
{
   QList<QVariant> vals;

   for (const QStringList & sl : m_strLists) {
      vals << sl;
   }

   QSettings().setValue(configKey(), vals);
}

