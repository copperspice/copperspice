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

#ifndef DRIVER_H
#define DRIVER_H

#include <option.h>
#include <qhash.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qtextstream.h>

class DomUI;
class DomWidget;
class DomSpacer;
class DomLayout;
class DomLayoutItem;
class DomActionGroup;
class DomAction;
class DomButtonGroup;

class Driver
{
 public:
   Driver();
   virtual ~Driver();

   // tools
   bool printDependencies(const QString &fileName);
   bool uic(const QString &fileName, QTextStream *output = nullptr);
   bool uic(const QString &fileName, DomUI *ui, QTextStream *output = nullptr);

   // configuration
   QTextStream &output() const {
      return *m_output;
   }

   Option &option() {
      return m_option;
   }

   // initialization
   void reset();

   // error
   QStringList problems() {
      return m_problems;
   }

   void addProblem(const QString &problem) {
      m_problems.append(problem);
   }

   // utils
   static QString headerFileName(const QString &fileName);
   QString headerFileName() const;

   static QString normalizedName(const QString &name);
   static QString qtify(const QString &name);
   QString unique(const QString &instanceName = QString(), const QString &className = QString());

   // symbol table
   QString findOrInsertWidget(DomWidget *ui_widget);
   QString findOrInsertSpacer(DomSpacer *ui_spacer);
   QString findOrInsertLayout(DomLayout *ui_layout);
   QString findOrInsertLayoutItem(DomLayoutItem *ui_layoutItem);
   QString findOrInsertName(const QString &name);
   QString findOrInsertActionGroup(DomActionGroup *ui_group);
   QString findOrInsertAction(DomAction *ui_action);
   QString findOrInsertButtonGroup(const DomButtonGroup *ui_group);

   // Find a group by its non-uniqified name
   const DomButtonGroup *findButtonGroup(const QString &attributeName) const;

   bool hasName(const QString &name) const {
      return m_nameRepository.contains(name);
   }

   DomWidget *widgetByName(const QString &name) const;
   DomSpacer *spacerByName(const QString &name) const;
   DomLayout *layoutByName(const QString &name) const;
   DomActionGroup *actionGroupByName(const QString &name) const;
   DomAction *actionByName(const QString &name) const;

   // pixmap
   void insertPixmap(const QString &pixmap);
   bool containsPixmap(const QString &pixmap) const;

 private:
   Option m_option;
   QTextStream m_stdout;
   QTextStream *m_output;

   QStringList m_problems;

   // symbol tables
   QHash<DomWidget *, QString> m_widgets;
   QHash<DomSpacer *, QString> m_spacers;
   QHash<DomLayout *, QString> m_layouts;
   QHash<DomActionGroup *, QString> m_actionGroups;

   typedef QHash<const DomButtonGroup *, QString> ButtonGroupNameHash;
   ButtonGroupNameHash m_buttonGroups;

   QHash<DomAction *, QString> m_actions;
   QHash<QString, bool> m_nameRepository;
   QHash<QString, bool> m_pixmaps;
};

#endif
