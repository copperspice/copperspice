/***********************************************************************
*
* Copyright (c) 2012-2026 Barbara Geller
* Copyright (c) 2012-2026 Ansel Sermersheim
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

#ifndef CUSTOMWIDGETSINFO_H
#define CUSTOMWIDGETSINFO_H

#include <qmap.h>
#include <qstringlist.h>

#include <treewalker.h>

class Driver;

class CustomWidgetsInfo : public TreeWalker
{
 public:
   CustomWidgetsInfo();

   void acceptUI(DomUI *node) override;

   void acceptCustomWidgets(DomCustomWidgets *node) override;
   void acceptCustomWidget(DomCustomWidget *node) override;

   QStringList customWidgets() const {
      return m_customWidgets.keys();
   }

   bool hasCustomWidget(const QString &name) const {
      return m_customWidgets.contains(name);
   }

   DomCustomWidget *customWidget(const QString &name) const {
      return m_customWidgets.value(name);
   }

   QString customWidgetAddPageMethod(const QString &name) const;
   QString realClassName(const QString &className) const;
   bool extends(const QString &className, const QString &baseClassName) const;
   bool isCustomWidgetContainer(const QString &className) const;

 private:

   using NameCustomWidgetMap = QMap<QString, DomCustomWidget*>;
   NameCustomWidgetMap m_customWidgets;
};

#endif
