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

#ifndef CUSTOMWIDGETSINFO_H
#define CUSTOMWIDGETSINFO_H

#include "treewalker.h"
#include <QtCore/QStringList>
#include <QtCore/QMap>

QT_BEGIN_NAMESPACE

class Driver;
class DomScript;

class CustomWidgetsInfo : public TreeWalker
{
 public:
   CustomWidgetsInfo();

   void acceptUI(DomUI *node) override;

   void acceptCustomWidgets(DomCustomWidgets *node) override;
   void acceptCustomWidget(DomCustomWidget *node) override;

   inline QStringList customWidgets() const {
      return m_customWidgets.keys();
   }

   inline bool hasCustomWidget(const QString &name) const {
      return m_customWidgets.contains(name);
   }

   inline DomCustomWidget *customWidget(const QString &name) const {
      return m_customWidgets.value(name);
   }

   DomScript *customWidgetScript(const QString &name) const;

   QString customWidgetAddPageMethod(const QString &name) const;

   QString realClassName(const QString &className) const;

   bool extends(const QString &className, const QLatin1String &baseClassName) const;

   bool isCustomWidgetContainer(const QString &className) const;

 private:
   typedef QMap<QString, DomCustomWidget *> NameCustomWidgetMap;
   NameCustomWidgetMap m_customWidgets;
};

QT_END_NAMESPACE

#endif // CUSTOMWIDGETSINFO_H
